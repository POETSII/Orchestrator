#include "SimulatedAnnealing.h"

SimulatedAnnealing::SimulatedAnnealing(Placer* placer, bool disorder):
    Algorithm(placer),
    disorder(disorder)
{
    if (disorder) result.method = "sa";
    else result.method = "gc";
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
 * monotonic. For now, it's a simple exponential decay with half life of a
 * third of the number of iterations (thats what the factor 3 is for).
 *
 * In the case of simulated annealing that only accepts better
 * configurations, disorder is always zero. */
float SimulatedAnnealing::compute_disorder()
{
    if (disorder) return 0.5 * exp(log(0.5) * 3 * iteration / maxIteration);
    else return 0;
}

/* Places a gi onto the engine held by a placer using a simulated annealing
 * algorithm.
 *
 * Returns the placement score. */
float SimulatedAnnealing::do_it(GraphI_t* gi)
{
    std::string time = placer->timestamp();

    /* Grab arguments, if any are set. */
    bool inPlace = false;
    if (placer->args.is_set("inpl")) inPlace = placer->args.get_bool("inpl");

    maxIteration = ITERATION_MAX_DEFAULT;
    if (placer->args.is_set("iter"))
        maxIteration = placer->args.get_uint("iter");

    /* Generic setup. */
    std::string algorithmName;
    if (disorder) algorithmName = "simulated_annealing";
    else algorithmName = "gradientless_climber";

    std::string fPath = dformat("%s%s_%s_%s.txt", placer->outFilePath.c_str(),
                                algorithmName.c_str(), gi->Name().c_str(),
                                time.c_str());
    FILE* log = fopen(fPath.c_str(), "w");
    if (log == PNULL) throw FileOpenException(
        dformat("File: %s. Message: %s",
                fPath.c_str(), OSFixes::getSysErrorString(errno).c_str()));

    result.startTime = time;
    fprintf(log, "[I] Placement starting at %s.\n", time.c_str());

    /* Build the hardware communication matrix (cost cache), if it hasn't
     * already been built by this placer. */
    if (placer->cache == PNULL)
    {
        fprintf(log, "[I] Hardware cost cache has not been built - building "
                "it now.\n");
        try
        {
            placer->cache = new CostCache(placer->engine);
        }

        catch (CostCacheException& e)
        {
            fprintf(log, "[E] Unable to generate costcache. Handing control "
                    "back to the operator.\n");
            fclose(log);
            throw e;
        }
    }

    /* If the caller requests an in-place anneal, check if the graph is
     * placed. If it's not, issue a warning. */
    std::map<GraphI_t*, Algorithm*>::iterator algorithmIt;
    algorithmIt = placer->placedGraphs.find(gi);
    if (algorithmIt == placer->placedGraphs.end())  /* Not placed. */
    {
        if (inPlace) fprintf(log, "[W] Initial placement requested, but this "
                                  "graph instance has not been placed.\n");

        /* Initial placement using smart-random. Note that we rely on this
         * being a valid placement in order for this algorithm to select across
         * the domain. (MLV never writes broken code! >_>). If it doesn't work,
         * fall back to thread-filling, which is generally safer. */
        fprintf(log, "[I] Performing initial placement (smart-random).\n");
        SmartRandom initialAlgorithm = SmartRandom(placer);
        if (initialAlgorithm.do_it(gi) == -1)
        {
            fprintf(log, "[I] It failed. Trying spread-filling instead...\n");
            placer->unplace(gi, false);  /* Leave the constraints alone. */
            ThreadFilling otherInitialAlgorithm = ThreadFilling(placer);
            otherInitialAlgorithm.do_it(gi);
        }
    }
    else if (inPlace)  /* Was already placed, as user intended. */
    {
        fprintf(log, "[I] Annealing on an existing placement.\n");
        delete algorithmIt->second;  /* Clear old algorithm object. */
    }

    /* Compute fitness of initial placement. */
    placer->populate_edge_weights(gi);

    /* Compute the fitness score. */
    float fitness = placer->compute_fitness(gi);

    fprintf(log, "[I] Initial placement complete with fitness score %f.\n",
        fitness);

    /* Define the valid-cores map. */
    placer->define_valid_cores_map(gi, &validCoresForDeviceType);

    /* Define the number of devices allowed per thread before constraints are
     * violated - used during selection. */
    devicesPerThreadSoftMax = placer->constrained_max_devices_per_thread(gi);

    /* Define the number of threads used per core before constraints are
     * violated - used during selection. */
    threadsPerCoreSoftMax = placer->constrained_max_threads_per_core(gi);

    /* Iteration loop - we exit when the termination condition is satisfied.
     *
     * Also note that, if there are no normal devices in the gi, we don't
     * anneal (because it will cause selection to fall over). */
    bool trivialGraph = true;
    WALKPDIGRAPHNODES(unsigned, DevI_t*,
                      unsigned, EdgeI_t*,
                      unsigned, PinI_t*, gi->G, thisDevice)
    {
        /* Are you a normal device? */
        if ((*thisDevice).second.data->devTyp == 'D')
        {
            trivialGraph = false;
            break;
        }
    }

    fprintf(log, "[I] Starting iteration.\n");
    std::vector<Constraint*> brokenHardConstraints;
    std::list<Constraint*> dissatisfiedConstraints;
    std::list<Constraint*> satisfiedConstraints;
    std::list<Constraint*>::iterator constraintIt;
    DevI_t* selectedDevice;
    P_thread* selectedThread;
    P_thread* previousThread;
    DevI_t* swapDevice;
    bool revert;
    float fitnessChange;  /* Negative represents "before the transformation",
                           * and positive represents "after the
                           * transformation" */

    fPath = dformat("%s%s_fitness_graph_%s_%s.csv",
                    placer->outFilePath.c_str(), algorithmName.c_str(),
                    gi->Name().c_str(), time.c_str());
    FILE* data = fopen(fPath.c_str(), "w");
    if (data == PNULL)
    {
        int err = errno;
        fprintf(log, "[E] Failed to open data file. Exiting.");
        fclose(log);
        throw FileOpenException(
            dformat("File: %s. Message: %s",
                    fPath.c_str(), OSFixes::getSysErrorString(err).c_str()));
    }

    if (trivialGraph)
    {
        fprintf(log, "[I] ...hang on, there are no normal devices in this "
                "gi! There's nothing to anneal!\n");
    }
    else while (!is_finished())
    {
        fprintf(data, "%u,%f\n", iteration, fitness);
        fprintf(log, "[D] Iteration %u...\n", iteration);
        revert = false;
        fitnessChange = 0;

        /* Select a device, a thread, and (optionally) another device on that
         * thread to swap with. */
        select(gi, &selectedDevice, &selectedThread, &swapDevice);

        /* Panic if we chose a device, but there was nowhere to put it. */
        if (selectedThread == PNULL)
        {
            fprintf(log, "[W]     Selected device '%s' of type '%s' can't "
                    "be put anywhere. Choosing a different device...\n",
                    selectedDevice->Name().c_str(),
                    selectedDevice->pT->Name().c_str());
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
            fitnessChange -= placer->compute_fitness(gi, selectedDevice);

            /* Grab the thread for this device to allow reversion, (in case
             * determination rejects the transformed state), and to allow
             * validCoresForDeviceType to be updated if the transformation is
             * accepted. */
            previousThread = placer->deviceToThread.\
                find(selectedDevice)->second;

            /* Apply transformation. */
            placer->link(selectedThread, selectedDevice);
            placer->populate_edge_weights(gi, selectedDevice);

            /* If any hard constraints are broken, decline the
             * transformation without computing fitness. */
            revert = !placer->are_all_hard_constraints_satisfied(
                gi, &brokenHardConstraints,
                std::vector<DevI_t*>(1, selectedDevice));

            if (revert) fprintf(log, "[D]     Hard constraint violated!\n");
            else
            {
                /* Compute the fitness from device-device connections after the
                 * move. */
                fitnessChange += placer->compute_fitness(gi, selectedDevice);

                /* Compute constraint change, storing changed constraints in a
                 * pair of maps. */
                dissatisfiedConstraints.clear();
                satisfiedConstraints.clear();

                /* For each soft constraint whose gi matches our gi (or
                 * that applies to all gis)... */
                for (constraintIt = placer->constraints.begin();
                     constraintIt != placer->constraints.end(); constraintIt++)
                {
                    if (((*constraintIt)->gi == PNULL or
                         (*constraintIt)->gi == gi)
                        and !(*constraintIt)->mandatory)
                    {
                        /* We only care about:
                         *
                         * - constraints that were satisfied but are no longer,
                         * - constraints that were not satisfied, but now
                         *   are. */
                        bool delta = (*constraintIt)->is_satisfied_delta(
                            placer, std::vector<DevI_t*>(1, selectedDevice));
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
                placer->populate_edge_weights(gi, selectedDevice);
            }

            /* Otherwise, we need to update some structures... */
            else
            {
                fprintf(log, "[D]     [PASS]\n");

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
                std::map<UniqueDevT, std::set<P_core*> >::iterator devtypIt;
                P_core* firstCore = previousThread->parent;
                P_core* secondCore = firstCore->pair;  /* May be PNULL. */

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

                    if (coresEmpty and secondCore != PNULL)
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
                            if (secondCore != PNULL)
                            {
                                devtypIt->second.insert(secondCore);
                            }
                        }
                    }
                }

                /* And likewise, "unfree" the new core pair for each other
                 * device type. Handily, set.erase doesn't moan if no element
                 * matches. */
                firstCore = selectedThread->parent;
                secondCore = firstCore->pair;  /* May be PNULL. */

                for (devtypIt = validCoresForDeviceType.begin();
                     devtypIt != validCoresForDeviceType.end();
                     devtypIt++)
                {
                    if (devtypIt->first.pT != selectedDevice->pT)
                    {
                        devtypIt->second.erase(firstCore);
                        if (secondCore != PNULL)
                        {
                            devtypIt->second.erase(secondCore);
                        }
                    }
                }
            }
        }

        /* Transform for swap operation. */
        else
        {
            // Not yet implemented <!>
        }

        iteration++;
    }

    /* Redistribute devices in cores to evenly load threads. */
    placer->redistribute_devices_in_gi(gi);

    /* Write our result structure, and leave. We don't mind that there is a
     * small difference between the many millions of fitness deltas and the
     * actual fitness - the difference is relatively small, and is due to
     * rounding. */
    float finalFitness = placer->compute_fitness(gi);
    time = placer->timestamp();
    result.endTime = time;
    placer->populate_result_structures(&result, gi, finalFitness);
    fprintf(log, "[I] Final fitness from deltas: %f, Iteration count: %d, "
            "actual final fitness: %f\n",
            fitness, iteration, finalFitness);
    fprintf(log, "[I] Placement complete at %s.\n", time.c_str());
    fclose(log);
    fclose(data);
    return fitness;
}

/* Returns true if the termination condition holds, and false otherwise. It's
 * pretty simple - for now we're just counting iterations - but this probably
 * will change as we become more curious. */
bool SimulatedAnnealing::is_finished(){return iteration >= maxIteration;}

/* Performs the selection operation for simulated annealing. Arguments
 *
 * - gi: Holds the graph from which devices are selected.
 *
 * - device: Pointer to a device pointer to select for the atomic
 *   transformation.
 *
 * - thread: Pointer to a thread pointer to either move the aforementioned
 *   device to, or that holds a device to swap with.
 *
 * - swapDevice: If the operation is a swap operation, points to the device to
 *   swap with. If not, is set to PNULL. */
void SimulatedAnnealing::select(GraphI_t* gi, DevI_t** device,
                                P_thread** thread, DevI_t** swapDevice)
{
    /* Sanity. */
    *device = PNULL;
    *thread = PNULL;
    *swapDevice = PNULL;

    /* Choose a normal device (i.e. not a supervisor or external). Note that
     * this will loop infinitely if there are only supervisor devices in this
     * gi (but I'm assuming nobody is going to call this without checking the
     * gi first...) */
    do
    {
        unsigned nodeKey;  /* Unused */
        gi->G.RandomNode(nodeKey, *device);
    } while ((*device)->devTyp != 'D');

    /* Choose a (valid) core. If there are no valid cores, set thread to NULL
     * and leave. */
    P_core* core;
    UniqueDevT deviceType;
    deviceType.gi = gi;
    deviceType.pT = (*device)->pT;
    std::map<UniqueDevT, std::set<P_core*> >::iterator bigScaryMapIterator;
    bigScaryMapIterator = validCoresForDeviceType.find(deviceType);
    if (bigScaryMapIterator->second.empty()) return;

    /* Note that we don't need to check for "unfound" device types, because
     * validCoresForDeviceType has been populated with all non-supervisor
     * device types earlier. */
    std::set<P_core*>::iterator coreIterator;
    coreIterator = bigScaryMapIterator->second.begin();
    std::advance(coreIterator, rand() % bigScaryMapIterator->second.size());
    core = *coreIterator;

    /* Choose a thread on that core. The choice may be constrained. */
    unsigned threadLimit = std::min(unsigned(core->P_threadm.size()),
                                    threadsPerCoreSoftMax);
    std::map<AddressComponent, P_thread*>::iterator threadIterator;
    threadIterator = core->P_threadm.begin();
    std::advance(threadIterator, rand() % threadLimit);
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
        std::list<DevI_t*>::iterator swapDeviceIterator;
        swapDeviceIterator = placer->threadToDevices[*thread].begin();
        std::advance(swapDeviceIterator, swapDeviceIndex);
        swapDevice = &(*swapDeviceIterator);
    }
    /* Otherwise, it's a move operation, and swapDevice is PNULL. */
}
