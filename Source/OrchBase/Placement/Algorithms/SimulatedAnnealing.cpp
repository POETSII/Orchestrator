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
float acceptance_probability(float fitnessBefore, float fitnessAfter)
{
    float ratio = fitnessBefore / fitnessAfter;
    if (ratio > 1) return 1;
    else return ratio * compute_disorder();
}

/* Computes the disorder value as a function of the iteration number, between
 * one (at iteration=0) and zero (lim iteration ->inf). Decrease must be
 * monotonic. For now, it's a simple exponential decay. */
float SimulatedAnnealing::compute_disorder()
{
    return exp(-DISORDER_DECAY * iteration);
}

/* Populates a map with information about which devices can be placed where. */
void SimulatedAnnealing::define_valid_cores_map(P_task* task)
{
    /* Firstly, populate the map with an entry for each core and each device
     * type in this task. NB: It would be handy if the task a container of
     * pointers to each device type, but it doesn't, so we walk the devices
     * instead (won't take too long). */
    WALKPDIGRAPHNODES(unsigned, P_device*, unsigned, P_message*, unsigned,
                      P_pin*, task->pD->G, deviceIterator)
    {
        P_device* device = task->pD->G.NodeData(deviceIterator);

        /* Ignore if it's a supervisor device (we don't map those). */
        if (!(device->pP_devtyp->pOnRTS)) continue;

        /* Skip if an entry in the map already exists for a device of this
         * type */
        P_devtyp* deviceType = device->pP_devtyp;
        std::map<P_devtyp*, std::set<P_core*>>::iterator devTypeFinder;
        devTypFinder = validCoresForDeviceType.find(deviceType);
        if (devTypFinder == validCoresForDeviceType.end())
        {
            /* Populate the entry with every core in the engine. */
            std::set<P_core*>* setToPopulate;
            P_core* currentCore;
            setToPopulate = &(validCoresForDeviceType[deviceType]);
            coreIterator = HardwareIterator(placer->engine);
            currentCore = coreIterator.get_core();
            while (!coreIterator.has_wrapped())
            {
                setToPopulate->insert(currentCore);
                currentCore = coreIterator.next_core();
            }
        }
    }

    /* Secondly, remove entries for each placed device (including those from
     * other tasks! We don't want to interact with them at all...) */
    std::map<P_device*, P_thread*>::iterator deviceIterator;
    std::map<P_devtyp*, std::set<P_core*>>::iterator bigScaryMapIterator;
    for (deviceIterator = placer->deviceToThread.begin();
         deviceIterator != placer->deviceToThread.end(); deviceIterator++)
    {
        P_devtyp* deviceType = deviceIterator->first->pP_devtyp;

        /* Remove the placed core from each device type entry... */
        for (bigScaryMapIterator = validCoresForDeviceType.begin();
             bigScaryMapIterator != validCoresForDeviceType.end();
             bigScaryMapIterator++)
        {
            /* ...except it's own (there's a device placed there, after
             * all). */
            if (bigScaryMapIterator->first != deviceType)
            {
                P_core* placedCore = deviceIterator->second->parent;
                bigScaryMapIterator->second.erase(placedCore);
            }
        }
    }
}

/* Places a task onto the engine held by a placer using a simulated annealing
 * algorithm.
 *
 * Returns the placement score. */
float SimulatedAnnealing::do_it(P_task* task)
{
    /* Build the hardware communication matrix (cost cache), if it hasn't
     * already been built by this placer. */
    if (placer->cache == PNULL) placer->cache = new CostCache(placer->engine);

    /* Initial placement using bucket filling. Note that we rely on this being
     * a valid placement in order for this algorithm to select across the
     * domain. (MLV never writes broken code! >_>) */
    BucketFilling initialAlgorithm = BucketFilling(placer);
    initialAlgorithm.do_it(task);

    /* Compute fitness of initial placement. */
    placer->populate_edge_weights(task);

    /* Compute the fitness score. */
    float fitness = placer->compute_fitness(task);

    /* Define the valid-cores map. */
    define_valid_cores_map(task);

    /* Define the number of devices in the task we're placing. */
    numberOfDevices = task->pD->G.SizeNodes();

    /* Define the number of devices allowed per thread before constraints are
     * violated - used during selection. */
    devicesPerThreadSoftMax = placer->constrained_max_devices_per_thread(task);

    /* Iteration loop - we exit when the termination condition is satisfied. */
    std::vector<Constraint*> brokenHardConstraints;
    std::list<Constraint*> dissatisfiedConstraints;
    std::list<Constraint*> satisfiedConstraints;
    P_device* selectedDevice;
    P_thread* selectedThread;
    P_thread* previousThread;
    P_device* swapDevice;
    bool revert;
    float fitnessChange;  /* Negative represents "before the transformation",
                           * and positive represents "after the
                           * transformation" */
    while (!is_finished())
    {
        revert = false;
        fitnessChange = 0;

        /* Select a device, a thread, and (optionally) another device on that
         * thread to swap with. */
        select(task, &selectedDevice, &selectedThread, &swapDevice);

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
            previousThread = deviceToThread[selectedDevice];

            /* Apply transformation. */
            placer->link(selectedThread, selectedDevice);
            placer->populate_edge_weights(task, selectedDevice);

            /* If any hard constraints are broken, decline the
             * transformation without computing fitness. */
            revert = !placer->are_all_hard_constraints_satisfied(
                task, &brokenHardConstraints,
                std::vector<P_device*>(1, selectedDevice));
            if (!revert)
            {
                /* Compute the fitness from device-device connections after the
                 * move. */
                fitnessChange += placer->compute_fitness(task, selectedDevice);

                /* Compute constraint change, storing changed constraints in a
                 * pair of maps. */
                std::list<Constraint*>::iterator constraintIt;
                dissatisfiedConstraints.clear();
                satisfiedConstraints.clear();

                /* For each soft constraint whose task matches our task (or
                 * that applies to all tasks)... */
                for (constraintIt = constraints.begin();
                     constraintIt != constraints.end(); constraintIt++)
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
                revert = roll > acceptance_probability(
                    fitness, fitness + fitnessChange);
            }

            /* Apply reversion, if deemed appropriate. */
            if (revert)
            {
                placer->link(previousThread, selectedDevice);
                placer->populate_edge_weights(task, selectedDevice);
            }

            /* Otherwise, we need to update some structures... */
            else
            {
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

                /* If we removed the last device from this core pair, make the
                 * core pair available for other device types. First check on
                 * the thread level, because it's cheaper. */
                if (placer->threadToDevices[previousThread].empty())
                {
                    /* Determine whether or not the core pair we moved the
                     * thread from is now empty. */
                    bool coresEmpty = true;
                    P_core* firstCore = previousThread->parent;
                    P_core* secondCore = previousThread->parent->pair;
                    std::map<AddressComponent, P_thread*>::iterator threadIt;
                    for (threadIt = firstCore->P_threadm.begin();
                         threadIt != firstCore->P_threadm.end(); threadIt++)
                    {
                        if (!threadToDevices[threadIt->second].empty())
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
                            if (!threadToDevices[threadIt->second].empty())
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

                /* And likewise, "unfree" this core pair for each other device
                 * type. Handily, set.erase doesn't moan if no element
                 * matches. */
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
    device = PNULL;
    thread = PNULL;
    swapDevice = PNULL;

    /* Choose a non-supervisor device. Note that this will loop infinitely if
     * there are only supervisor devices in this task (but I'm assuming nobody
     * is going to do that... */
    do
    {
        std::map<unsigned, P_device*>::iterator deviceIterator;
        deviceIterator = task->pD->G.NodeBegin();
        std::advance(deviceIterator, rand() % numberOfDevices);
        device = &(deviceIterator->second);
    } while (!((*device)->pP_devtyp->pOnRTS))

    /* Choose a (valid) core. */
    P_core* core;
    P_devtyp* deviceType = (*device)->pP_devtyp;
    std::map<P_devtyp*, std::set<P_core*>>::iterator bigScaryMapIterator;
    bigScaryMapIterator = validCoresForDeviceType.find(deviceType);
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
    thread = &(threadIterator->second);

    /* <!> NB: Disabling swap operations for now, to make debugging simpler. */
    return; // <!>

    /* Choose a device (or empty space) on that core. This determines whether
     * the operation is a move or swap - if we roll and land on an "empty
     * space", we move. If not, we swap. Note that the swap does not check for
     * the type of devices on that core (because the core couldn't be
     * selected if they are different). */

    /* Determine the maximum slot size - the number of devices on this thread,
     * or the maximum imposed by the constraint (whichever is bigger). */
    unsigned slotMax = std::max(devicesPerThreadSoftMax,
                                placer->threadToDevices[*thread].size());

    /* Roll! */
    unsigned swapDeviceIndex = rand() % slotMax;

    /* Define swapDevice appropriately. */
    if (swapDeviceIndex <= placer->threadToDevices[*thread].size())  /* Swap */
    {
        std::list<P_device*>::iterator swapDeviceIterator;
        swapDeviceIterator = threadToDevices[*thread].begin();
        std::advance(swapDeviceIterator, swapDeviceIndex);
        swapDevice = &(*swapDeviceIterator);
    }
    /* Otherwise, it's a move operation, and swapDevice is PNULL. */
}
