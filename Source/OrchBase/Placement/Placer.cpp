/* Defines placer (see the accompanying header for further information). */

#include "Placer.h"

Placer::Placer(){engine = PNULL;}

Placer::Placer(P_engine* engine):engine(engine),cache(PNULL)
{
    /* Psst, do you want to add a constraint to the placer in a hard-coded
     * manner?  You're in the right place. Just define your constraint
     * here... see the constructor for what the arguments do. */

    /* constraints.push_back(new MaxDevicesPerThread(true, 10, PNULL, 1)); */
}

Placer::~Placer()
{
    if (cache != PNULL) delete cache;

    /* Free memory for each constraint and algorithm. */
    std::list<Constraint*>::iterator constraintIt;
    std::map<P_task*, Algorithm*>::iterator algorithmIt;
    for (constraintIt = constraints.begin(); constraintIt != constraints.end();
         constraintIt++) delete (*constraintIt);
    for (algorithmIt = placedTasks.begin(); algorithmIt != placedTasks.end();
         algorithmIt++) delete algorithmIt->second;
}

/* Given a string and a set of arguments, creates an instance of a "derived
 * class" algorithm, and returns a pointer it. */
Algorithm* Placer::algorithm_from_string(std::string colloquialDescription)
{
    Algorithm* output = PNULL;
    if (colloquialDescription.substr(0, 2) == "sa")
        output = new SimulatedAnnealing(this);
    if (colloquialDescription == "buck") output = new BucketFilling(this);
    if (colloquialDescription == "link") output = new BucketFilling(this);
    if (colloquialDescription == "rand") output = new SmartRandom(this);

    if (output == PNULL)
    {
        throw InvalidAlgorithmDescriptorException(dformat(
            "[ERROR] Invalid string '%s' passed to Placer::algorithm_from_"
            "string (or we ran out of memory).",
            colloquialDescription.c_str()));
    }

    return output;
}

/* Returns true if all core pairs devices of one (or zero) type mapped to them,
 * and false otherwise. Arguments:
 *
 * - task: Task to scan for devices.
 *
 * - badCoresToDeviceTypes: Map of cores to device types, where the vectors are
 *   more than one element in length. Cleared before being populated. */
bool Placer::are_all_core_pairs_device_locked(P_task* task,
    std::map<std::pair<P_core*, P_core*>,
             std::set<P_devtyp*>>* badCoresToDeviceTypes)
{
    /* Sanity. */
    badCoresToDeviceTypes->clear();

    /* Build a map of cores to devices, by iterating through all placed
     * threads. */
    std::map<P_core*, std::set<P_device*>> coreToDevices;
    std::map<P_thread*, std::list<P_device*>>::iterator threadDevsIt;
    std::list<P_device*>::iterator deviceIt;

    for (threadDevsIt = threadToDevices.begin();
         threadDevsIt != threadToDevices.end(); threadDevsIt++)
    {
        for (deviceIt = threadDevsIt->second.begin();
             deviceIt != threadDevsIt->second.end(); deviceIt++)
        {
            coreToDevices[threadDevsIt->first->parent].insert(*deviceIt);
        }
    }

    /* For each core pair, build a set of device types for the devices that
     * inhabit that core pair, and store it in the "bad" map. Then, if the size
     * of that set is less than two, remove it. (This means we effectively
     * check each pair twice, but this beats the time required to search for
     * the corresponding pair as the number of cores grows.) */
    std::map<P_core*, std::set<P_device*>>::iterator coreIt;
    std::map<AddressComponent, P_thread*>::iterator threadIt;
    P_core* firstCore;
    P_core* secondCore;

    /* Iterate over each core found from the loop above. */
    for (coreIt = coreToDevices.begin(); coreIt != coreToDevices.end();
         coreIt++)
    {
        firstCore = coreIt->first;
        secondCore = firstCore->pair;

        std::pair<P_core*, P_core*> corePair =
            std::make_pair(firstCore, secondCore);
        std::set<P_devtyp*>* devTypSet = &((*badCoresToDeviceTypes)[corePair]);

        /* From each thread in that core... */
        for (threadIt = firstCore->P_threadm.begin();
             threadIt != firstCore->P_threadm.end(); threadIt++)
        {
            std::list<P_device*>* devList =
                &(threadToDevices[threadIt->second]);

            /* ...from each device mapped to that thread... */
            for (deviceIt = devList->begin(); deviceIt != devList->end();
                 deviceIt++)
            {
                /* ...get its device type. */
                devTypSet->insert((*deviceIt)->pP_devtyp);
            }
        }

        /* Remove the entry from the map if it is of size 1 or less. */
        if (devTypSet->size() < 2) badCoresToDeviceTypes->erase(corePair);
    }

    return badCoresToDeviceTypes->empty();
}

/* Returns true if all of the devices in a task are mapped to a thread, and
 * false otherwise. Arguments:
 *
 * - task: Task to scan for devices.
 *
 * - unmapped: Vector populated with unmapped devices (for further
 *   diagnosis). Cleared before being populated. */
bool Placer::are_all_devices_mapped(P_task* task,
                                    std::vector<P_device*>* unmapped)
{
    /* Sanity. */
    unmapped->clear();

    /* Iterate through each device in the task. */
    WALKPDIGRAPHNODES(unsigned, P_device*, unsigned, P_message*, unsigned,
                      P_pin*, task->pD->G, deviceIterator)
    {
        P_device* device = task->pD->G.NodeData(deviceIterator);

        /* Ignore if it's a supervisor device (we don't map those). */
        if (!(device->pP_devtyp->pOnRTS)) continue;

        /* If it's mapped, we move on. If not, we add it to unmapped. */
        std::map<P_device*, P_thread*>::iterator deviceFinder;
        deviceFinder = deviceToThread.find(device);
        if (deviceFinder == deviceToThread.end()) unmapped->push_back(device);
    }

    return unmapped->empty();
}

/* Returns true if no hard constraints are broken, and false
 * otherwise.
 *
 * Optionally uses the delta method if devices are defined. Arguments:
 *
 * - task: Task to match constraints against (don't care about constraints that
 *   only apply to other tasks). If PNULL, checks against all tasks.
 *
 * - broken: Vector populated with constraints that are not
 *   satisfied. Cleared before being populated.
 *
 * - devices: Series of device pointers; delta computation is performed using
 *   these devices if this container is not empty. */
bool Placer::are_all_hard_constraints_satisfied(P_task* task,
    std::vector<Constraint*>* broken, std::vector<P_device*> devices)
{
    /* Sanity. */
    broken->clear();

    /* For each constraint... */
    std::list<Constraint*>::iterator constraintIterator;
    for (constraintIterator = constraints.begin();
         constraintIterator != constraints.end(); constraintIterator++)
    {
        /* Ignore constraints whose task doesn't match this task. Skip this
         * check if the task input argument is PNULL. */
        if (task == PNULL or
            (*constraintIterator)->task == PNULL or
            (*constraintIterator)->task == task)
        {
            /* Ignore soft constraints. */
            if ((*constraintIterator)->mandatory)
            {
                /* Global check (expensive) */
                if (devices.empty())
                {
                    if (not (*constraintIterator)->is_satisfied(this))
                    {
                        broken->push_back(*constraintIterator);
                    }
                }

                /* Local check */
                else
                {
                    if (not (*constraintIterator)->
                        is_satisfied_delta(this, devices))
                    {
                        broken->push_back(*constraintIterator);
                    }
                }
            }
        }
    }

    return broken->empty();
}

/* Checks the integrity of the placement of a given task. In order, this
 * method:
 *
 * 1) Checks that all devices have been mapped to a thread.
 *
 * 2) Checks that no hard constraints have been violated.
 *
 * 3) Checks that all core pairs have devices of only one (or zero) types.
 *
 * Throws a BadIntegrityException if a check fails. */
void Placer::check_integrity(P_task* task, Algorithm* algorithm)
{
    /* Step 1: Check all devices have been mapped. */
    std::vector<P_device*> unmappedDevices;
    if (!are_all_devices_mapped(task, &unmappedDevices))
    {
        /* Prepare a nice printout of the devices that weren't mapped. */
        std::string devicePrint;
        std::vector<P_device*>::iterator deviceIt;
        for (deviceIt = unmappedDevices.begin();
             deviceIt != unmappedDevices.end(); deviceIt++)
        {
            devicePrint.append(dformat("\n - %s",
                                       (*deviceIt)->Name().c_str()));
        }

        /* Toys out of the pram. */
        throw BadIntegrityException(dformat(
            "[ERROR] Use of algorithm '%s' on the task from file '%s' "
            "resulted in some normal devices not being placed "
            "correctly. These devices are:%s\n",
            algorithm->result.method.c_str(), task->filename.c_str(),
            devicePrint.c_str()));
    }

    /* Step 2: Check for any violated hard constraints. */
    std::vector<Constraint*> brokenConstraints;
    if (!are_all_hard_constraints_satisfied(task, &brokenConstraints))
    {
        /* Prepare a nice printout of the constraints that weren't
         * satisfied. */
        std::string constraintPrint;
        std::vector<Constraint*>::iterator constraintIt;
        for (constraintIt = brokenConstraints.begin();
             constraintIt != brokenConstraints.end(); constraintIt++)
        {
            constraintPrint.append(dformat("\n - %s",
                                           (*constraintIt)->Name().c_str()));
        }

        /* Toys out of the pram. */
        throw BadIntegrityException(dformat(
            "[ERROR] Hard constraints were violated when using algorithm '%s' "
            "on the task from file '%s'. The violated constraints are:%s\n",
            algorithm->result.method.c_str(), task->filename.c_str(),
            constraintPrint.c_str()));
    }

    /* Step 3: Check device types. */
    std::map<std::pair<P_core*, P_core*>,
             std::set<P_devtyp*>> badCoresToDeviceTypes;
    if (!are_all_core_pairs_device_locked(task, &badCoresToDeviceTypes))
    {
        /* Prepare a nice printout of core pairs with multiple device types. */
        std::string corePrint;
        std::map<std::pair<P_core*, P_core*>,
                 std::set<P_devtyp*>>::iterator badCoreIt;
        std::set<P_devtyp*>::iterator devTypIt;

        /* For each entry in the map... */
        for (badCoreIt = badCoresToDeviceTypes.begin();
             badCoreIt != badCoresToDeviceTypes.end(); badCoreIt++)
        {
            /* ...we write a line. */
            corePrint.append(
                dformat("\n - '%s' and '%s':",
                        badCoreIt->first.first->FullName().c_str(),
                        badCoreIt->first.second->FullName().c_str()));

            /* For each device type associated with that core pair... */
            for (devTypIt = badCoreIt->second.begin();
                 devTypIt != badCoreIt->second.end(); devTypIt++)
            {
                /* We write an entry after the comma, separating device types
                 * with a comma. */
                if (devTypIt != badCoreIt->second.begin())
                {
                    corePrint.append(",");
                }
                corePrint.append(dformat(" '%s'",
                                         (*devTypIt)->Name().c_str()));
            }
        }

        /* Toys out of the pram. */
        throw BadIntegrityException(dformat(
            "[ERROR] Some core pairs have multiple device types associated "
            "with them when using algorithm '%s' on the task from file '%s'. "
            "The core pairs and their device types are:%s\n",
            algorithm->result.method.c_str(), task->filename.c_str(),
            corePrint.c_str()));
    }
}

/* Returns the fitness contribution by this device.
 *
 * Adds the weights of all edges involving the passed device, where that device
 * has an entry in deviceToGraphKey (this is assumed). These weights are
 * looked up from taskEdgeCosts. Also includes the thread-loading delta.
 *
 * Note that this method doesn't consider constraints.
 *
 * Arguments:
 *
 * - task: The device must be owned by this task.
 *
 * - device: The device to investigate. */
float Placer::compute_fitness(P_task* task, P_device* device)
{
    /* Grab the device pair from each edge. */
    std::vector<std::pair<P_device*, P_device*>> devicePairs;
    get_edges_for_device(task, device, &devicePairs);

    /* Reduce! */
    float fitness = 0;
    std::vector<std::pair<P_device*, P_device*>>::iterator devicePairIt;
    for (devicePairIt = devicePairs.begin(); devicePairIt != devicePairs.end();
         devicePairIt++) fitness += taskEdgeCosts[task][*devicePairIt];

    /* Thread loading cost for this device. This can be interpreted as the
     * amount contributed to the total fitness by this device with regards to
     * thread loading. The fitness contribution by thread loading is
     * proportional (THREAD_LOADING_SCALE_FACTOR is the constant of
     * proportionality) to T^2, where T is the number of devices mapped to a
     * given thread, summed for each thread. From this, we infer the
     * "contribution" by one device as the total contribution for it's thread,
     * minus the contribution if the thread was not there, i.e.:
     *
     *  T^2 - (T-1)^2 == 2T-1 */
    fitness += (threadToDevices[deviceToThread[device]].size() * 2 - 1)
        * THREAD_LOADING_SCALING_FACTOR;
    return fitness;
}

/* Computes the fitness for a task.
 *
 * Fitness is the sum of:
 *
 *  - All costs on all edges,
 *  - Each device placed on each thread, beyond the first, and
 *  - The penalty of all broken soft constraints.
 *
 * Low fitness is good. This evaluation does not factor in broken hard
 * constraints. Arguments:
 *
 *  - task: Task to evaluate the fitness of.
 *
 * Returns the fitness as a float. */
float Placer::compute_fitness(P_task* task)
{
    float fitness = 0;

    /* Iterate through each edge in the task, and add its weight to it. Note
     * that algorithms are responsible for updating the edge weights. */
    std::map<std::pair<P_device*, P_device*>, float>::iterator edgeIt;
    for (edgeIt = taskEdgeCosts[task].begin();
         edgeIt != taskEdgeCosts[task].end(); edgeIt++)
    {
        fitness += edgeIt->second;
    }

    /* Iterate through each thread used by the task and, for each of those
     * threads, increase fitness for each device on that thread beyond the
     * first.
     *
     * We do this by going through each thread in threadToDevices and, if that
     * thread holds devices of a type that are part of this task, then we
     * increase the fitness based off the number of devices mapped to that
     * thread.
     *
     * The increase is equal to the square of the number of devices placed on
     * the thread, subject to a scaling factor (otherwise it completely
     * dominates the edge weights). This corresponds with observations from the
     * heated plate, where a linear decrease in thread loading results in a
     * linear decrease in runtime for a large device/thread ratio. For example:
     *
     * - 90 devices, mapped under 10 devices per thread onto 9 threads will
     *   result in a fitness contribution proportional to 9 * (10 ^ 2) = 900.
     *
     * - 90 devices, mapped under 30 devices per thread onto 3 threads will
     *   result in a fitness contribution proportional to 3 * (30 ^ 2) = 2700.
     *
     * These numbers are subjected to a scaling factor to ensure that the
     * benefit of "sprading the load" is traded off fairly with the edge
     * weights, which want the application to be as tightly packed as
     * possible. */
    std::map<P_thread*, std::list<P_device*>>::iterator threadIt;
    for (threadIt = threadToDevices.begin(); threadIt != threadToDevices.end();
         threadIt++)
    {
        /* Ignore empty threads. */
        if (threadIt->second.empty()) continue;

        /* Does the first device's task match the task passed to this function
         * as argument? */
        if ((*threadIt->second.begin())->par->par == task)
        {
            /* If so, we count it. */
            fitness += threadIt->second.size() * threadIt->second.size() *
                THREAD_LOADING_SCALING_FACTOR;
        }
    }

    /* Iterate through each constraint, and add their soft-cost to the
     * fitness. */
    std::list<Constraint*>::iterator constraintIterator;
    for (constraintIterator = constraints.begin();
         constraintIterator != constraints.end(); constraintIterator++)
    {
        /* Ignore constraints whose task doesn't match this task. */
        if ((*constraintIterator)->task == PNULL or
            (*constraintIterator)->task == task)
        {
            /* Ignore satisfied constraints. */
            if (not (*constraintIterator)->is_satisfied(this))
            {
                fitness += (*constraintIterator)->penalty;
            }
        }
    }

    return fitness;
}

/* Returns the maximum number of devices permitted to be placed on a thread
 * such that no maxDevicesPerThread constraints are violated that apply either
 * globally, or to a given task. */
unsigned Placer::constrained_max_devices_per_thread(P_task* task)
{
    unsigned maximumSoFar = MAX_DEVICES_PER_THREAD_DEFAULT;

    /* Iterate through each constraint. */
    std::list<Constraint*>::iterator constraintIterator;
    for (constraintIterator = constraints.begin();
         constraintIterator != constraints.end();
         constraintIterator++)
    {
        Constraint* constraint = *constraintIterator;

        /* Only care about "maxDevicesPerThread" constraints. */
        if (constraint->category == maxDevicesPerThread)
        {
            /* Only care about constraints that are global, or that match our
             * task. */
            if (constraint->task == task or constraint->task == PNULL)
            {
                /* Accept the strictest. */
                unsigned thisMaximum = \
                    dynamic_cast<MaxDevicesPerThread*>(constraint)->maximum;
                if (thisMaximum < maximumSoFar) maximumSoFar = thisMaximum;
            }
        }
    }
    return maximumSoFar;
}

/* Populates a map with information about which devices can be placed
 * where. Useful for algorithms. */
void Placer::define_valid_cores_map(P_task* task,
        std::map<P_devtyp*, std::set<P_core*>>* validCoresForDeviceType)
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
        std::map<P_devtyp*, std::set<P_core*>>::iterator devTypFinder;
        devTypFinder = validCoresForDeviceType->find(deviceType);
        if (devTypFinder == validCoresForDeviceType->end())
        {
            /* Populate the entry with every core in the engine. */
            std::set<P_core*>* setToPopulate;
            P_core* currentCore;
            setToPopulate = &((*validCoresForDeviceType)[deviceType]);
            HardwareIterator coreIterator = HardwareIterator(engine);
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
    for (deviceIterator = deviceToThread.begin();
         deviceIterator != deviceToThread.end(); deviceIterator++)
    {
        P_devtyp* deviceType = deviceIterator->first->pP_devtyp;

        /* Remove the placed core from each device type entry... */
        for (bigScaryMapIterator = validCoresForDeviceType->begin();
             bigScaryMapIterator != validCoresForDeviceType->end();
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

/* Dumps placement information for a task. Assumes the task has been
 * placed. See the documentation. */
void Placer::dump(P_task* task)
{
    /* If the score is zero, we're naturally quite suspicious. For example, if
     * the task was placed using bucket-filling (i.e. the algorithm that works
     * the fastest but doesn't care about the score), we need to compute the
     * score now. */
    std::map<P_task*, Algorithm*>::iterator tasksIt = placedTasks.find(task);
    if (tasksIt->second->result.score == 0)
    {
        /* Build the cost cache. */
        if (cache == PNULL) cache = new CostCache(engine);

        /* Define edge weights for the task graph. */
        populate_edge_weights(task);

        /* Compute the score. */
        float score = compute_fitness(task);

        /* Fill in fields for the result structure. */
        populate_result_structures(&(tasksIt->second->result), task, score);
    }

    /* Get the time. */
    std::string timeBuf = timestamp();

    /* Figure out paths. */
    std::string costPath = dformat("placement_task_edges_%s_%s.csv",
                                   task->Name().c_str(), timeBuf.c_str());
    std::string diagPath = dformat("placement_diagnostics_%s_%s.txt",
                                   task->Name().c_str(), timeBuf.c_str());
    std::string mapPath = dformat("placement_task_to_hardware_%s_%s.csv",
                                  task->Name().c_str(), timeBuf.c_str());
    std::string cachePath = dformat("placement_edge_cache_%s.txt",
                                    timeBuf.c_str());
    std::string nodeLoadPath = dformat("placement_node_loading_%s.csv",
                                       timeBuf.c_str());
    std::string edgeLoadPath = dformat("placement_edge_loading_%s.csv",
                                       timeBuf.c_str());

    /* Call subordinate dumping methods. */
    dump_costs(task, costPath.c_str());
    dump_diagnostics(task, diagPath.c_str());
    dump_edge_loading(edgeLoadPath.c_str());
    dump_map(task, mapPath.c_str());
    dump_node_loading(nodeLoadPath.c_str());

    FILE* cacheFile = fopen(cachePath.c_str(), "w");
    cache->Dump(cacheFile);
    fclose(cacheFile);
}

/* Dumps the cost of each edge for a task. */
void Placer::dump_costs(P_task* task, const char* path)
{
    /* File setup. */
    ofstream out;
    out.open(path);

    /* Iterate through each edge in the task. */
    std::map<std::pair<P_device*, P_device*>, float>::iterator edgeIt;
    for (edgeIt = taskEdgeCosts[task].begin();
         edgeIt != taskEdgeCosts[task].end(); edgeIt++)
    {
        out << edgeIt->first.first->FullName() << ","
            << edgeIt->first.second->FullName() << ","
            << edgeIt->second << std::endl;
    }

    out.close();
}

/* Dumps diagnostic placement information for a task. */
void Placer::dump_diagnostics(P_task* task, const char* path)
{
    /* File setup. */
    ofstream out;
    out.open(path);

    /* Get the result object address (Placer::dump checks that the task has
     * already been placed). */
    Result* result = &(placedTasks[task]->result);

    out << "maxDevicesPerThread:" << result->maxDevicesPerThread << std::endl;
    out << "maxEdgeCost:" << result->maxEdgeCost << std::endl;
    out << "method:" << result->method << std::endl;
    out << "startTime:" << result->startTime << std::endl;
    out << "endTime:" << result->endTime << std::endl;
    out << "score:" << result->score << std::endl;
    out.close();
}

/* Dump mailbox-level edge loading using the cost cache. */
void Placer::dump_edge_loading(const char* path)
{
    /* File setup. */
    ofstream out;
    out.open(path);

    std::map<P_mailbox*, std::map<P_mailbox*, unsigned>> edgeLoading;

    /* Iterate over each task. */
    P_device* fromDevice;
    P_device* toDevice;
    P_mailbox* fromMailbox;
    P_mailbox* toMailbox;

    std::map<P_task*, Algorithm*>::iterator taskIt;
    for (taskIt = placedTasks.begin(); taskIt != placedTasks.end(); taskIt++)
    {
        /* Iterate over each edge for that task. */
        std::map<std::pair<P_device*, P_device*>, float>::iterator edgeIt;
        for (edgeIt = taskEdgeCosts[taskIt->first].begin();
             edgeIt != taskEdgeCosts[taskIt->first].end(); edgeIt++)
        {
            /* Ignore edges with supervisor devices. <!> Long-term, we
             * obviously don't want to do this - we want to impose a location
             * of supervisors in the compute fabric, and find the edge based
             * off that. Perhaps a "proxy mailbox" in the bridge board or
             * so. */
            fromDevice = edgeIt->first.first;
            toDevice = edgeIt->first.second;
            if (!(fromDevice->pP_devtyp->pOnRTS)) continue;
            if (!(toDevice->pP_devtyp->pOnRTS)) continue;

            /* Grab the mailboxes. */
            fromMailbox = deviceToThread[fromDevice]->parent->parent;
            toMailbox = deviceToThread[toDevice]->parent->parent;

            /* Walk through the 'pathNext' cache in the CostCache to get all of
             * the edges, and increment each edge in edgeLoading. */
            std::vector<P_mailbox*> path;
            cache->get_path(fromMailbox, toMailbox, &path);
            std::vector<P_mailbox*>::iterator mailboxIt;
            P_mailbox* previousMailbox;
            for (mailboxIt = path.begin(); mailboxIt != path.end();
                 mailboxIt++)
            {
                /* Add backwards - skip index 0. */
                if (mailboxIt != path.begin())
                {
                    edgeLoading[previousMailbox][*mailboxIt] += 1;
                }
                previousMailbox = *mailboxIt;
                continue;
            }
        }
    }

    /* Writeout. */
    std::map<P_mailbox*, std::map<P_mailbox*, unsigned>>::iterator edgeOuterIt;
    std::map<P_mailbox*, unsigned>::iterator edgeInnerIt;
    for (edgeOuterIt = edgeLoading.begin(); edgeOuterIt != edgeLoading.end();
         edgeOuterIt++)
    {
        for (edgeInnerIt = edgeOuterIt->second.begin();
             edgeInnerIt != edgeOuterIt->second.end(); edgeInnerIt++)
        {
            out << edgeOuterIt->first->FullName() << ","
                << edgeInnerIt->first->FullName() << ","
                << edgeInnerIt->second << std::endl;
        }
    }
    out.close();
}


/* Dumps mapping information for a task. */
void Placer::dump_map(P_task* task, const char* path)
{
    /* File setup. */
    ofstream out;
    out.open(path);

    /* Iterate through each device in the task. */
    WALKPDIGRAPHNODES(unsigned, P_device*, unsigned, P_message*, unsigned,
                      P_pin*, task->pD->G, deviceIterator)
    {
        P_device* device = task->pD->G.NodeData(deviceIterator);

        /* If it's mapped, add it. If not, ignore it (good for non-normal
         * devices). */
        std::map<P_device*, P_thread*>::iterator deviceFinder;
        deviceFinder = deviceToThread.find(device);
        if (deviceFinder == deviceToThread.end()) continue;
        P_thread* thread = deviceFinder->second;

        out << device->FullName() << "," << thread->FullName() << std::endl;
    }

    out.close();  /* We out, yo. */
}

/* Dumps node loading information for the entire placer. Core-level and
 * mailbox-level dumps are written to the same file. */
void Placer::dump_node_loading(const char* path)
{
    /* File setup. */
    ofstream out;
    out.open(path);

    std::map<P_core*, unsigned> coreLoading;
    std::map<P_core*, unsigned>::iterator coreIt;
    std::map<P_mailbox*, unsigned> mailboxLoading;
    std::map<P_mailbox*, unsigned>::iterator mailboxIt;

    /* Iterate over each placed device. */
    std::map<P_device*, P_thread*>::iterator threadIt;
    for (threadIt = deviceToThread.begin(); threadIt != deviceToThread.end();
         threadIt++)
    {
        /* Increment loading. */
        coreLoading[threadIt->second->parent] += 1;
        mailboxLoading[threadIt->second->parent->parent] += 1;
    }

    /* Blargh. */
    out << "[core]" << std::endl;
    for (coreIt = coreLoading.begin(); coreIt != coreLoading.end(); coreIt++)
    {
        out << coreIt->first->FullName() << "," << coreIt->second << std::endl;
    }

    out << "[mailbox]" << std::endl;
    for (mailboxIt = mailboxLoading.begin(); mailboxIt != mailboxLoading.end();
         mailboxIt++)
    {
        out << mailboxIt->first->FullName() << "," << mailboxIt->second
            << std::endl;
    }

    out.close();
}

/* Grabs all boxes associated with a given task in this placer's
 * engine. Arguments:
 *
 * - task: Task to search for.
 * - boxes: Set to populate with box addresses found. */
void Placer::get_boxes_for_task(P_task* task, std::set<P_box*>* boxes)
{
    boxes->clear();

    /* For each core associated with this task, add its box to the set. */
    std::set<P_core*>::iterator coreIterator;
    std::set<P_core*>* setToScan = &(taskToCores[task]);
    for (coreIterator = setToScan->begin(); coreIterator != setToScan->end();
         coreIterator++)
    {
        boxes->insert((*coreIterator)->parent->parent->parent);  /* Sowi */
    }
}

/* Grabs all of the device pairs for each edge in the application graph that
 * involves the given device. */
void Placer::get_edges_for_device(P_task* task, P_device* device,
    std::vector<std::pair<P_device*, P_device*>>* devicePairs)
{
    devicePairs->clear();

    /* Get all of the arcs in the task graph that involve this device, in both
     * directions. */
    std::vector<unsigned> arcKeysIn;
    std::vector<unsigned> arcKeysOut;
    task->pD->G.FindArcs(deviceToGraphKey[device], arcKeysIn, arcKeysOut);

    /* Treat each arc the same, regardless of its direction. */
    std::vector<unsigned> arcKeys;  /* All */
    arcKeys.reserve(arcKeysIn.size() + arcKeysOut.size());
    arcKeys.insert(arcKeys.end(), arcKeysIn.begin(), arcKeysIn.end());
    arcKeys.insert(arcKeys.end(), arcKeysOut.begin(), arcKeysOut.end());

    /* Grab the device pair from each edge. */
    for (std::vector<unsigned>::iterator arcKeyIt = arcKeys.begin();
         arcKeyIt != arcKeys.end(); arcKeyIt++)
    {
        unsigned firstDeviceKey = 0;
        unsigned secondDeviceKey = 0;
        task->pD->G.FindNodes(*arcKeyIt, firstDeviceKey, secondDeviceKey);
        devicePairs->push_back(
            std::make_pair(
                *(task->pD->G.FindNode(firstDeviceKey)),
                *(task->pD->G.FindNode(secondDeviceKey))));
    }
}

/* Low-level method to create a thread-device binding.
 *
 * Does no constraint checking, does not define the address of the device, but
 * does move the device from it's previous location if it was there before.
 *
 * I mean, it just updates the placer maps. */
void Placer::link(P_thread* thread, P_device* device)
{
    /* If this device has already been placed on a thread, remove the device
     * from the thread's threadToDevices entry. */
    std::map<P_device*, P_thread*>::iterator threadFinder;
    threadFinder = deviceToThread.find(device);
    if (threadFinder != deviceToThread.end())
    {
        std::list<P_device*>* listToEraseFrom;
        listToEraseFrom = &(threadToDevices[threadFinder->second]);
        std::list<P_device*>::iterator deviceFinder;
        deviceFinder = std::find(listToEraseFrom->begin(),
                                 listToEraseFrom->end(), device);

        /* Conditional block to prevent us from erasing at an "end"
         * iterator. */
        if (deviceFinder != listToEraseFrom->end())
        {
            listToEraseFrom->erase(deviceFinder);
        }
    }

    /* Link! */
    threadToDevices[thread].push_back(device);
    deviceToThread[device] = thread;
}

/* Maps a task to the engine associated with this placer, using a certain
 * algorithm.  Arguments:
 *
 * - task: Pointer to task to map.
 *
 * - algorithmDescription: Type of algorithm to use (see
 *   algorithm_from_string).
 *
 * Returns the solution fitness, if appropriate (otherwise returns zero). */
float Placer::place(P_task* task, std::string algorithmDescription)
{
    /* Grab an algorithm, may throw. */
    Algorithm* algorithm = algorithm_from_string(algorithmDescription);

    return place(task, algorithm);
}

/* Maps a task to the engine associated with this placer, using a certain
 * algorithm.  Arguments:
 *
 * - task: Pointer to task to map.
 *
 * - algorithm: An algorithm instance on the heap.
 *
 * Returns the solution fitness, if appropriate (otherwise returns zero). */
float Placer::place(P_task* task, Algorithm* algorithm)
{
    /* Danger Will Robinson! (seriously though, how did you get here? --MLV) */
    if (engine == PNULL) throw NoEngineException(
        "You've attempted to place a task without defining an engine pointer "
        "in the placer. If you're running this from OrchBase, how did you get "
        "here?");

    /* Complain if the task has already been placed (by memory address). */
    if (placedTasks.find(task) != placedTasks.end())
    {
        delete algorithm;
        throw AlreadyPlacedException(dformat(
            "[ERROR] Task from file '%s' has already been placed.",
            task->filename.c_str()));
    }

    /* Define deviceToGraphKey for devices in this task. */
    populate_device_to_graph_key_map(task);

    /* Run the algorithm on the task. */
    float score = algorithm->do_it(task);

    /* Check placement integrity, throwing if there's a problem. */
    check_integrity(task, algorithm);

    /* Update task-keyed maps. */
    placedTasks[task] = algorithm;
    update_task_to_cores_map(task);

    /* Update result structure for this algorithm object. */
    populate_result_structures(&algorithm->result, task, score);

    /* Update software addresses for each device placed in this task. */
    update_software_addresses(task);

    /* Tell the task it's been placed. */
    task->LinkFlag();

    return score;
}

/* Load a previously-placed configuration. Is pretty unsafe. Arguments:
 *
 * - task: Pointer to task to map.
 *
 * - path: String denoting path to placement information to load
 *   (e.g. placement_task_to_hardware_file_...)
 *
 * Returns the solution fitness, if appropriate (otherwise returns zero). */
float Placer::place_load(P_task* task, std::string path)
{
    return place(task, new PlacementLoader(this, path));
}

/* For each device in the task, define the placer's reverse-map so that edges
 * can be looked up as a function of device in the task graph. */
void Placer::populate_device_to_graph_key_map(P_task* task)
{
    WALKPDIGRAPHNODES(unsigned, P_device*, unsigned, P_message*, unsigned,
                      P_pin*, task->pD->G, deviceIterator)
    {
        deviceToGraphKey[task->pD->G.NodeData(deviceIterator)] = \
            task->pD->G.NodeKey(deviceIterator);
    }
}

/* Given a pair of devices connected in the task graph and an initialised cost
 * cache, populates the weight on the edge connecting them. */
void Placer::populate_edge_weight(P_task* task, P_device* from, P_device* to)
{
    /* Skip this edge if one of the devices is a supervisor device. */
    if (!(from)->pP_devtyp->pOnRTS) return;
    if (!(to)->pP_devtyp->pOnRTS) return;

    /* Store the weight. */
    float weight = cache->compute_cost(deviceToThread[from],
                                       deviceToThread[to]);
    taskEdgeCosts[task][std::make_pair(from, to)] = weight;
}

/* Given the placement maps and an initialised cost cache, populates the
 * weights on each edge of the application graph for a given task that involves
 * a given device. */
void Placer::populate_edge_weights(P_task* task, P_device* device)
{
    /* Grab the device pair from each edge. */
    std::vector<std::pair<P_device*, P_device*>> devicePairs;
    get_edges_for_device(task, device, &devicePairs);

    /* Populate */
    std::vector<std::pair<P_device*, P_device*>>::iterator devicePairIt;
    for (devicePairIt = devicePairs.begin(); devicePairIt != devicePairs.end();
         devicePairIt++)
    {
        populate_edge_weight(task, devicePairIt->first, devicePairIt->second);
    }
}

/* Given the placement maps and an initialised cost cache, populates the
 * weights on each edge of the application graph for a given task. */
void Placer::populate_edge_weights(P_task* task)
{
    WALKPDIGRAPHARCS(unsigned, P_device*, unsigned, P_message*, unsigned,
                     P_pin*, task->pD->G, edgeIt)
    {
        populate_edge_weight(task,
                             task->pD->G.NodeData(edgeIt->second.fr_n),
                             task->pD->G.NodeData(edgeIt->second.to_n));
    }
}

/* Defines the contents of the result struct of an algorithm. */
void Placer::populate_result_structures(Result* result, P_task* task,
                                        float score)
{
    /* Time! */
    if (result->endTime.empty())
    {
        result->endTime = timestamp();
        result->startTime = "1970-01-01T00:00:00";
    }

    /* Easy one. */
    result->score = score;

    /* Maximum number of devices per thread - compute by going through the
     * cores assigned to this task. */
    result->maxDevicesPerThread = 0;  /* Optimistic. */

    std::set<P_core*>::iterator coreIt;
    std::set<P_core*>* targetSet = &(taskToCores[task]);
    for (coreIt = targetSet->begin(); coreIt != targetSet->end(); coreIt++)
    {
        /* Iterate through each thread on this core. */
        std::map<AddressComponent, P_thread*>::iterator threadIt;
        for (threadIt = (*coreIt)->P_threadm.begin();
             threadIt != (*coreIt)->P_threadm.end(); threadIt++)
        {
            /* If the number of devices mapped to this thread is greater than
             * the current stored maximum, update the maximum. */
            result->maxDevicesPerThread = std::max(result->maxDevicesPerThread,
                (unsigned) threadToDevices[threadIt->second].size());
        }
    }

    /* Maximum edge cost - compute by going through the edges in the
     * application graph. */
    result->maxEdgeCost = 0;  /* Again, mindless optimism. */

    std::map<std::pair<P_device*, P_device*>, float>::iterator edgeIt;
    for (edgeIt = taskEdgeCosts[task].begin();
         edgeIt != taskEdgeCosts[task].end(); edgeIt++)
    {
        /* If the weight on the edge is greater than the current stored
         * maximum, update the maximum. */
        result->maxEdgeCost = std::max(result->maxEdgeCost, edgeIt->second);
    }
}

/* Gets the time in ISO-8601=seconds format, and returns a string of it. */
std::string Placer::timestamp()
{
    time_t timeNtv;  /* "Native" */
    time(&timeNtv);
    char timeBuf[sizeof "YYYY-MM-DDTHH:MM:SS"];
    strftime(timeBuf, sizeof timeBuf, "%FT%T", localtime(&timeNtv));
    return timeBuf;
}

/* Removes all device-thread relations for all devices in a task, and removes
 * constraints imposed by this task.
 *
 * It's slow and, due the the data structures involved, inefficient. We're fine
 * with this, because unplacing is quite rare.
 *
 * This implementation assumes that each thread contains only devices belonging
 * to a given task (to save many extra list.erases). */
void Placer::unplace(P_task* task, bool andConstraints)
{
    /* Remove task-imposed constraints (unless explicitly told otherwise). NB:
     * This is not a for loop, because elements are removed from the container
     * within the loop. */
    if (andConstraints)
    {
        std::list<Constraint*>::iterator constraintIterator =   \
            constraints.begin();
        while (constraintIterator != constraints.end())
        {
            /* Say goodbye! */
            if ((*constraintIterator)->task == task)
            {
                delete *constraintIterator;
                constraintIterator = constraints.erase(constraintIterator);
            }

            /* You're safe for now. */
            else constraintIterator++;
        }
    }

    /* Clear the maps - iterate through each device in the task. */
    WALKPDIGRAPHNODES(unsigned, P_device*, unsigned, P_message*, unsigned,
                      P_pin*, task->pD->G, deviceIterator)
    {
        P_device* device = task->pD->G.NodeData(deviceIterator);

        /* Is the device placed? If so grab the thread. If not, skip. We don't
         * complain, because we should be cool with unplacing a
         * partially-placed task. */
        std::map<P_device*, P_thread*>::iterator deviceFinder;
        deviceFinder = deviceToThread.find(device);
        if (deviceFinder == deviceToThread.end()) continue;
        P_thread* thread = deviceFinder->second;

        /* Remove the entry from deviceToThread (erasure by position). */
        deviceToThread.erase(deviceFinder);

        /* Remove the entry from threadToDevices (erasure by key). Note that
         * this is safe because each thread is associated with one task (device
         * type) at a time. */
        threadToDevices.erase(thread);
    }

    /* Clear the appropriate entry in placedTasks. */
    std::map<P_task*, Algorithm*>::iterator placedTaskFinder;
    placedTaskFinder = placedTasks.find(task);
    if (placedTaskFinder != placedTasks.end())
    {
        delete placedTaskFinder->second;
        placedTasks.erase(placedTaskFinder);
    }

    /* Clear the appropriate entry in taskToCores. */
    std::map<P_task*, std::set<P_core*>>::iterator taskToCoresFinder;
    taskToCoresFinder = taskToCores.find(task);
    if (taskToCoresFinder != taskToCores.end())
    {
        taskToCores.erase(taskToCoresFinder);
    }

    /* Clear the appropriate entry in taskEdgeCosts. */
    std::map<P_task*, std::map<std::pair<P_device*, P_device*>,
                               float>>::iterator edgeCostsFinder;
    edgeCostsFinder = taskEdgeCosts.find(task);
    if (edgeCostsFinder != taskEdgeCosts.end())
    {
        taskEdgeCosts.erase(edgeCostsFinder);
    }
}

/* Updates the software addresses of each device in a task, clearing it if it
 * not currently placed (for example, if the task has been unplaced. */
void Placer::update_software_addresses(P_task* task)
{
    std::map<P_device*, P_thread*>::iterator deviceFinder;
    std::map<P_thread*, std::list<P_device*>>::iterator threadFinder;
    bool found;  /* Is this device currently placed? */

    /* Iterate through each device in the task. */
    WALKPDIGRAPHNODES(unsigned, P_device*, unsigned, P_message*, unsigned,
                      P_pin*, task->pD->G, deviceIterator)
    {
        /* Grab the device, for readability. */
        P_device* device = task->pD->G.NodeData(deviceIterator);

        /* Is the device currently placed? */
        found = false;
        deviceFinder = deviceToThread.find(device);
        if (deviceFinder != deviceToThread.end())
        {
            threadFinder = threadToDevices.find(deviceFinder->second);
            if (threadFinder != threadToDevices.end())
            {
                found = true;
            }
        }

        /* If it has not been placed, clear the address. */
        if (!found) device->addr.Reset();

        /* If it has, update it. */
        else
        {
            /* Get the index of the device within the list associated with the
             * current thread (in order to define the device component of the
             * address object). */
            std::list<P_device*>::iterator first, found;
            first = threadFinder->second.begin();
            found = std::find(threadFinder->second.begin(),
                              threadFinder->second.end(), device);

            /* Define the device component of the address object in the
             * device. */
            device->addr.SetDevice(std::distance(first, found));

            /* Define the other components of the address object in the
             * device. */
            threadFinder->first->get_hardware_address()->
                populate_a_software_address(&(device->addr), false);
        }
    }
}

/* Updates taskToCores with the entries of a task, which has been placed
 * correctly. Doesn't check much. */
void Placer::update_task_to_cores_map(P_task* task)
{
    /* Grab the set we're inserting into. */
    std::set<P_core*>* coreSet;
    coreSet = &(taskToCores[task]);

    /* Iterate through every thread in the hardware model. If a device is
     * placed on that thread, and that device is owned by the task, then add
     * the core to the coreSet.
    HardwareIterator engineIt(engine);
    P_thread* thread;
    P_device* firstDevice;
    while (!engineIt.has_wrapped())
    {
        thread = engineIt.get_thread();
        if (threadToDevices.find(thread) != threadToDevices.end())
        {
            firstDevice = *(threadToDevices[thread].begin());
            if (firstDevice->par->par == task)
            {
                coreSet->insert(thread->parent);
                engineIt.next_core();
                continue;
            }
        }
        engineIt.next_thread();
    } */

    /* Iterate through each device. */
    WALKPDIGRAPHNODES(unsigned, P_device*, unsigned, P_message*, unsigned,
                      P_pin*, task->pD->G, deviceIterator)
    {
        /* Grab the device, for readability. */
        P_device* device = task->pD->G.NodeData(deviceIterator);

        /* Ignore if it's a supervisor device (we don't map those). */
        if (!(device->pP_devtyp->pOnRTS)) continue;

        /* Assume the device is placed - grab the core it's placed on. */
        coreSet->insert(deviceToThread[device]->parent);
    }
}
