#include "SimulatedAnnealing.h"

SimulatedAnnealing::SimulatedAnnealing(Placer* placer):Algorithm(placer)
{
    iteration = 0;
}

/* Computes the probability with which a transformation is accepted, given the
 * fitness of the states before and after the transformation, and the disorder
 * parameter.
 *
 * A higher acceptance probability means that the transformation is more likely
 * to be accepted. */
float SimulatedAnnealing::acceptance_probability(float fitnessBefore,
                                                 float fitnessAfter)
{
    float ratio = fitnessBefore / fitnessAfter;
    if (ratio > 1) return 1;
    else return ratio * compute_disorder();
}

/* Computes the disorder value as a function of the iteration number, between
 * half (at iteration=0) and zero (lim iteration ->inf). Decrease must be
 * monotonic. For now, it's a simple exponential decay. */
float SimulatedAnnealing::compute_disorder()
{
    return 0.5 * exp(-DISORDER_DECAY * iteration);
}

/* Places a task onto the engine held by a placer using a simulated annealing
 * algorithm.
 *
 * Returns the placement score. */
float SimulatedAnnealing::do_it(P_task* task)
{
    FILE* log = fopen("./simulated_annealing.log", "w");

    /* Build the hardware communication matrix (cost cache), if it hasn't
     * already been built by this placer. */
    if (placer->cache == PNULL)
    {
        fprintf(log, "[I] Hardware cost cache has not been built - building "
                "it now.\n");
        placer->cache = new CostCache(placer->engine);
    }

    /* Initial placement using smart-random. Note that we rely on this being a
     * valid placement in order for this algorithm to select across the
     * domain. (MLV never writes broken code! >_>). If it doesn't work, fall
     * back to bucket filling (which is more stiff and harder to "anneal", but
     * is generally safer). */
    fprintf(log, "[I] Performing initial placement (smart-random).\n");
    SmartRandom initialAlgorithm = SmartRandom(placer);
    if (initialAlgorithm.do_it(task) == -1)
    {
        fprintf(log, "[I] It failed. Trying bucket-filling instead...\n");
        placer->unplace(task, false);  /* Leave the constraints alone. */
        BucketFilling otherInitialAlgorithm = BucketFilling(placer);
        otherInitialAlgorithm.do_it(task);
    }

    /* Compute fitness of initial placement. */
    placer->populate_edge_weights(task);

    /* Compute the fitness score. */
    float fitness = placer->compute_fitness(task);

    fprintf(log, "[I] Initial placement complete with fitness score %f.\n",
        fitness);

    /* Define the valid-cores map. */
    placer->define_valid_cores_map(task, &validCoresForDeviceType);

    /* Define the number of devices allowed per thread before constraints are
     * violated - used during selection. */
    devicesPerThreadSoftMax = placer->constrained_max_devices_per_thread(task);

    /* Iteration loop - we exit when the termination condition is satisfied. */
    std::vector<Constraint*> brokenHardConstraints;
    std::list<Constraint*> dissatisfiedConstraints;
    std::list<Constraint*> satisfiedConstraints;
    std::list<Constraint*>::iterator constraintIt;
    P_device* selectedDevice;
    P_thread* selectedThread;
    P_thread* previousThread;
    P_device* swapDevice;
    bool revert;
    float fitnessChange;  /* Negative represents "before the transformation",
                           * and positive represents "after the
                           * transformation" */

    fprintf(log, "[I] Starting iteration.\n");
    while (!is_finished())
    {
        fprintf(log, "[D] Iteration %u...\n", iteration);
        revert = false;
        fitnessChange = 0;

        /* Select a device, a thread, and (optionally) another device on that
         * thread to swap with. */
        select(task, &selectedDevice, &selectedThread, &swapDevice);

        /* Panic if we chose a device, but there was nowhere to put it. */
        if (selectedThread == PNULL)
        {
            fprintf(log, "[W]     Selected device '%s' of type '%s' can't "
                    "be put anywhere. Choosing a different device...\n",
                    selectedDevice->Name().c_str(),
                    selectedDevice->pP_devtyp->Name().c_str());
            iteration++;
            continue;
        }

        fprintf(log, "[D]     Selected device: '%s'\n",
                selectedDevice->Name().c_str());
        fprintf(log, "[D]     Selected thread: '%s'\n",
                selectedThread->FullName().c_str());

        /* Transform for move operation. */
        if (swapDevice == PNULL)
        {
            /* Compute the fitness from device-device connections before the
             * move. Note the sign convention (see the comment where
             * fitnessChange is declared). */
            fitnessChange -= placer->compute_fitness(task, selectedDevice);

            /* Grab the thread for this device to allow reversion, (in case
             * determination rejects the transformed state), and to allow
             * validCoresForDeviceType to be updated if the transformation is
             * accepted. */
            previousThread = placer->deviceToThread.\
                find(selectedDevice)->second;

            /* Apply transformation. */
            placer->link(selectedThread, selectedDevice);
            placer->populate_edge_weights(task, selectedDevice);

            /* If any hard constraints are broken, decline the
             * transformation without computing fitness. */
            revert = !placer->are_all_hard_constraints_satisfied(
                task, &brokenHardConstraints,
                std::vector<P_device*>(1, selectedDevice));

            if (revert) fprintf(log, "[D]     Hard constraint violated!\n");
            else
            {
                /* Compute the fitness from device-device connections after the
                 * move. */
                fitnessChange += placer->compute_fitness(task, selectedDevice);

                /* Compute constraint change, storing changed constraints in a
                 * pair of maps. */
                dissatisfiedConstraints.clear();
                satisfiedConstraints.clear();

                /* For each soft constraint whose task matches our task (or
                 * that applies to all tasks)... */
                for (constraintIt = placer->constraints.begin();
                     constraintIt != placer->constraints.end(); constraintIt++)
                {
                    if (((*constraintIt)->task == PNULL or
                         (*constraintIt)->task == task)
                        and !(*constraintIt)->mandatory)
                    {
                        /* We only care about:
                         *
                         * - constraints that were satisfied but are no longer,
                         * - constraints that were not satisfied, but now
                         *   are. */
                        bool delta = (*constraintIt)->is_satisfied_delta(
                            placer, std::vector<P_device*>(1, selectedDevice));
                        if (delta && !(*constraintIt)->satisfied)
                        {
                            satisfiedConstraints.push_back(*constraintIt);
                        }
                        if (!delta && (*constraintIt)->satisfied)
                        {
                            dissatisfiedConstraints.push_back(*constraintIt);
                        }
                    }
                }

                /* Apply changed constraints costs to the fitness change. */
                for (constraintIt = dissatisfiedConstraints.begin();
                     constraintIt != dissatisfiedConstraints.end();
                     constraintIt++)
                {
                    fitnessChange += (*constraintIt)->penalty;
                }

                for (constraintIt = satisfiedConstraints.begin();
                     constraintIt != satisfiedConstraints.end();
                     constraintIt++)
                {
                    fitnessChange -= (*constraintIt)->penalty;
                }

                /* Determination - do we keep our selected state? (A roll of
                 * zero means we always keep). */
                float roll = static_cast<float>(rand()) /
                    static_cast<float>(RAND_MAX);  /* Uniform float in [0,1] */
                float acceptance = acceptance_probability(
                    fitness, fitness + fitnessChange);
                revert = roll > acceptance;
                fprintf(log, "[D]     Current fitness: %f, Fitness delta: %f, "
                        "Roll: %f, Acceptance: %f.\n",
                        fitness, fitnessChange, roll, acceptance);
            }

            /* Apply reversion, if deemed appropriate. */
            if (revert)
            {
                fprintf(log, "[D]     [FAIL]\n");
                placer->link(previousThread, selectedDevice);
                placer->populate_edge_weights(task, selectedDevice);
            }

            /* Otherwise, we need to update some structures... */
            else
            {
                fprintf(log, "[D]     [PASS]\n");
                std::map<std::pair<P_core*, P_core*>,
                         std::set<P_devtyp*>> mymap;
                if (!placer->are_all_core_pairs_device_locked(task, &mymap))
                {
                    printf("We've violated the core-pair condition.\n");
                }

                /* Update the fitness value. */
                fitness = fitness + fitnessChange;

                /* Update the state of all constraints that have changed
                 * state. */
                for (constraintIt = dissatisfiedConstraints.begin();
                     constraintIt != dissatisfiedConstraints.end();
                     constraintIt++)
                {
                    (*constraintIt)->satisfied = false;
                }

                for (constraintIt = satisfiedConstraints.begin();
                     constraintIt != satisfiedConstraints.end();
                     constraintIt++)
                {
                    (*constraintIt)->satisfied = true;
                }

                /* Update validCoresForDeviceType, for selection. */
                std::map<P_devtyp*, std::set<P_core*>>::iterator devtypIt;
                P_core* firstCore = previousThread->parent;
                P_core* secondCore = firstCore->pair;

                /* If we removed the last device from this core pair, make the
                 * core pair available for other device types. First check on
                 * the thread level, because it's cheaper. */
                if (placer->threadToDevices[previousThread].empty())
                {
                    /* Determine whether or not the core pair we moved the
                     * thread from is now empty. */
                    bool coresEmpty = true;
                    std::map<AddressComponent, P_thread*>::iterator threadIt;
                    for (threadIt = firstCore->P_threadm.begin();
                         threadIt != firstCore->P_threadm.end(); threadIt++)
                    {
                        if (!placer->threadToDevices[threadIt->second].empty())
                        {
                            coresEmpty = false;
                            break;
                        }
                    }

                    if (coresEmpty)
                    {
                        for (threadIt = secondCore->P_threadm.begin();
                             threadIt != secondCore->P_threadm.end();
                             threadIt++)
                        {
                            if (!placer->
                                threadToDevices[threadIt->second].empty())
                            {
                                coresEmpty = false;
                                break;
                            }
                        }
                    }

                    /* If both cores are empty, make them available in the
                     * validCores map. */
                    if (coresEmpty)
                    {
                        for (devtypIt = validCoresForDeviceType.begin();
                             devtypIt != validCoresForDeviceType.end();
                             devtypIt++)
                        {
                            devtypIt->second.insert(firstCore);
                            devtypIt->second.insert(secondCore);
                        }
                    }
                }

                /* And likewise, "unfree" the new core pair for each other
                 * device type. Handily, set.erase doesn't moan if no element
                 * matches. */
                firstCore = selectedThread->parent;
                secondCore = firstCore->pair;

                for (devtypIt = validCoresForDeviceType.begin();
                     devtypIt != validCoresForDeviceType.end();
                     devtypIt++)
                {
                    if (devtypIt->first != selectedDevice->pP_devtyp)
                    {
                        devtypIt->second.erase(firstCore);
                        devtypIt->second.erase(secondCore);
                    }
                }
            }
        }

        /* Transform for swap operation. */
        else
        {
            // <!>
        }

        iteration++;
    }

    /* Write our result structure, and leave. */
    placer->populate_result_structures(&result, task, fitness);
    fprintf(log, "[I] Final fitness: %f, Iteration count: %d.\n",
            fitness, iteration);
    fclose(log);
    return fitness;
}

/* Returns true if the termination condition holds, and false otherwise. It's
 * pretty simple - for now we're just counting iterations - but this probably
 * will change as we become more curious. */
bool SimulatedAnnealing::is_finished(){return iteration >= ITERATION_MAX;}

/* Performs the selection operation for simulated annealing. Arguments
 *
 * - task: Holds the graph from which devices are selected.
 *
 * - device: Pointer to a device pointer to select for the atomic
 *   transformation.
 *
 * - thread: Pointer to a thread pointer to either move the aforementioned
 *   device to, or that holds a device to swap with.
 *
 * - swapDevice: If the operation is a swap operation, points to the device to
 *   swap with. If not, is set to PNULL. */
void SimulatedAnnealing::select(P_task* task, P_device** device,
                                P_thread** thread, P_device** swapDevice)
{
    /* Sanity. */
    *device = PNULL;
    *thread = PNULL;
    *swapDevice = PNULL;

    /* Choose a non-supervisor device. Note that this will loop infinitely if
     * there are only supervisor devices in this task (but I'm assuming nobody
     * is going to do that... */
    do
    {
        unsigned nodeKey;  /* Unused */
        task->pD->G.RandomNode(nodeKey, *device);
    } while (!((*device)->pP_devtyp->pOnRTS));

    /* Choose a (valid) core. If there are no valid cores, set thread to NULL
     * and leave. */
    P_core* core;
    P_devtyp* deviceType = (*device)->pP_devtyp;
    std::map<P_devtyp*, std::set<P_core*>>::iterator bigScaryMapIterator;
    bigScaryMapIterator = validCoresForDeviceType.find(deviceType);
    if (bigScaryMapIterator->second.empty()) return;

    /* Note that we don't need to check for "unfound" device types, because
     * validCoresForDeviceType has been populated with all non-supervisor
     * device types earlier. */
    std::set<P_core*>::iterator coreIterator;
    coreIterator = bigScaryMapIterator->second.begin();
    std::advance(coreIterator, rand() % bigScaryMapIterator->second.size());
    core = *coreIterator;

    /* Choose a thread on that core. */
    std::map<AddressComponent, P_thread*>::iterator threadIterator;
    threadIterator = core->P_threadm.begin();
    std::advance(threadIterator, rand() % core->P_threadm.size());
    *thread = threadIterator->second;

    /* <!> NB: Disabling swap operations for now, to make debugging simpler. */
    return; // <!>

    /* Choose a device (or empty space) on that core. This determines whether
     * the operation is a move or swap - if we roll and land on an "empty
     * space", we move. If not, we swap. Note that the swap does not check for
     * the type of devices on that core (because the core couldn't be
     * selected if they are different). */

    /* Determine the maximum slot size - the number of devices on this thread,
     * or the maximum imposed by the constraint (whichever is bigger). */
    unsigned slotMax = std::max<unsigned>(devicesPerThreadSoftMax,
        placer->threadToDevices[*thread].size());

    /* Roll! */
    unsigned swapDeviceIndex = rand() % slotMax;

    /* Define swapDevice appropriately. */
    if (swapDeviceIndex <= placer->threadToDevices[*thread].size())  /* Swap */
    {
        std::list<P_device*>::iterator swapDeviceIterator;
        swapDeviceIterator = placer->threadToDevices[*thread].begin();
        std::advance(swapDeviceIterator, swapDeviceIndex);
        swapDevice = &(*swapDeviceIterator);
    }
    /* Otherwise, it's a move operation, and swapDevice is PNULL. */
}
