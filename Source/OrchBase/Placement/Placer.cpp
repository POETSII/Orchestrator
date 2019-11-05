/* Defines placer (see the accompanying header for further information). */

#include "Placer.h"

Placer::Placer(){engine = PNULL;}

Placer::Placer(P_engine* engine):engine(engine)
{
    cache = new CostCache(engine);

    /* Psst, do you want to add a constraint to the placer in a hard-coded
     * manner?  You're in the right place. Just define your constraint
     * here... see the constructor in Constraints.h for what the arguments
     * do. */

    /* constraints.push_back(new MaxDevicesPerThread(true, 10, PNULL, 1)); */
}

Placer::~Placer()
{
    delete cache;

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
    if (colloquialDescription == "buck") output = new BucketFilling(this);
    if (colloquialDescription == "link") output = new BucketFilling(this);

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
     * of that set less than two, remove it. (This means we effectively check
     * each pair twice, but this beats the time required to search for the
     * corresponding pair as the number of cores grows.) */
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
 * otherwise. Arguments:
 *
 * - task: Task to match constraints against (don't care about constraints that
 *   only apply to other tasks). If PNULL, checks against all tasks.
 *
 * - broken: Vector populated with constraints that are not
 *   satisfied. Cleared before being populated. */
bool Placer::are_all_hard_constraints_satisfied(P_task* task,
    std::vector<Constraint*>* broken)
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
                /* Expensive! */
                if (not (*constraintIterator)->is_satisfied(this))
                {
                    broken->push_back(*constraintIterator);
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
void Placer::check_integrity(P_task* task, std::string algorithmDescription)
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
            algorithmDescription.c_str(), task->filename.c_str(),
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
            algorithmDescription.c_str(), task->filename.c_str(),
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
            algorithmDescription.c_str(), task->filename.c_str(),
            corePrint.c_str()));
    }
}

/* Computes the fitness for a task.
 *
 * Fitness is simply the sum of all costs on all edges, along with the sum of
 * any broken soft constraints. Low fitness is good. This evaluation does not
 * factor in broken hard constraints. Arguments:
 *
 *  - task: Task to evaluate the fitness of.
 *
 * Returns the fitness as a float. */
float Placer::compute_fitness(P_task* task)
{
    float fitness = 0;

    /* Iterate through each edge in the task, and add its weight to it. Note
     * that algorithms are responsible for updating the edge weights. */
    WALKPDIGRAPHARCS(unsigned, P_device*, unsigned, P_message*, unsigned,
                     P_pin*, task->pD->G, edgeIt)
    {
        fitness += task->pD->G.ArcData(edgeIt)->weight;
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
            fitness += (*constraintIterator)->penalty;
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

/* Dumps placement information for a task. See the documentation. */
void Placer::dump(P_task* task)
{
    /* Don't do anything if the task has not been placed (by address). */
    if (placedTasks.find(task) == placedTasks.end())
        throw NoTaskToDump(dformat(
            "[ERROR] Task from file '%s' has not been placed, so we can't "
            "dump", task->filename.c_str()));

    /* Get the time. */
    time_t timeNtv;  /* "Native" */
    time(&timeNtv);
    char timeBuf[sizeof "YYYY-MM-DDTHH:MM:SS"];
    strftime(timeBuf, sizeof timeBuf, "%FT%T", localtime(&timeNtv));

    /* Figure out paths. */
    std::string costPath = dformat("placement_task_edges_%s_%s.txt",
                                  task->Name().c_str(), timeBuf);
    std::string diagPath = dformat("placement_diagnostics_%s_%s.txt",
                                   task->Name().c_str(), timeBuf);
    std::string mapPath = dformat("placement_task_to_hardware_%s_%s.txt",
                                  task->Name().c_str(), timeBuf);
    std::string cachePath = dformat("placement_edge_cache_%s.txt", timeBuf);

    /* Call subordinate dumping methods. */
    dump_costs(task, costPath.c_str());
    dump_diagnostics(task, diagPath.c_str());
    dump_map(task, mapPath.c_str());

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
    WALKPDIGRAPHARCS(unsigned, P_device*, unsigned, P_message*, unsigned,
                     P_pin*, task->pD->G, edgeIt)
    {
        /* Get the nodes connected by the arc (No pdigraph internal for
         * this?). */
        P_device* from = task->pD->G.NodeData(edgeIt->second.fr_n);
        P_device* to = task->pD->G.NodeData(edgeIt->second.to_n);

        /* Write a line. */
        out << from->FullName() << "\t" << to->FullName() << "\t"
            << task->pD->G.ArcData(edgeIt)->weight << std::endl;
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
    out << "when:" << result->when << std::endl;
    out << "score:" << result->score << std::endl;
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

        out << device->FullName() << "\t" << thread->FullName() << std::endl;
    }

    out.close();  /* We out, yo. */
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

/* Low-level method to create a thread-device binding. Does no checking. */
void Placer::link(P_thread* thread, P_device* device)
{
    /* Update placer maps. */
    threadToDevices[thread].push_back(device);
    deviceToThread[device] = thread;

    /* Define the device component of the address object in the device. */
    device->addr.SetDevice(threadToDevices[thread].size() - 1);

    /* Define the other components of the address object in the device. */
    thread->get_hardware_address()->
        populate_a_software_address(&(device->addr), false);
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
    /* Danger Will Robinson! (seriously though, how did you get here? --MLV) */
    if (engine == PNULL) throw NoEngineException(
        "You've attempted to place a task without defining an engine pointer "
        "in the placer. If you're running this from OrchBase, how did you get "
        "here?");

    /* Grab an algorithm, may throw. */
    Algorithm* algorithm = algorithm_from_string(algorithmDescription);

    /* Complain if the task has already been placed (by memory address). */
    if (placedTasks.find(task) != placedTasks.end())
        throw AlreadyPlacedException(dformat(
            "[ERROR] Task from file '%s' has already been placed.",
            task->filename.c_str()));

    /* Run the algorithm on the task. */
    float score = algorithm->do_it(task);

    /* Check placement integrity, throwing if there's a problem. */
    check_integrity(task, algorithmDescription);

    /* Update task-keyed maps. */
    placedTasks[task] = algorithm;
    update_task_to_cores_map(task);

    /* Update result structure for this algorithm object. */
    populate_result_structures(&algorithm->result, task, score);

    /* Tell the task it's been placed. */
    task->LinkFlag();

    return score;
}

/* Defines the contents of the result struct of an algorithm. */
void Placer::populate_result_structures(Result* result, P_task* task,
                                        float score)
{
    /* Time! */
    time_t timeNtv;  /* "Native" */
    time(&timeNtv);
    char timeBuf[sizeof "YYYY-MM-DDTHH:MM:SS"];
    strftime(timeBuf, sizeof timeBuf, "%FT%T", localtime(&timeNtv));
    result->when = timeBuf;

    /* Easy one. */
    result->score = score;

    /* Maximum number of devices per thread - compute by going through the
     * cores assigned to this task. */
    result->maxDevicesPerThread = 0;  /* Optimistic; we'll change this later. */

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

    WALKPDIGRAPHARCS(unsigned, P_device*, unsigned, P_message*, unsigned,
                     P_pin*, task->pD->G, edgeIt)
    {
        /* If the weight on the edge is greater than the current stored
         * maximum, update the maximum. */
        P_message* edge = task->pD->G.ArcData(edgeIt);
        result->maxEdgeCost = std::max(result->maxEdgeCost, edge->weight);
    }
}

/* Removes all device-thread relations for all devices in a task, and removes
 * constraints imposed by this task.
 *
 * It's slow and, due the the data structures involved, inefficient. We're fine
 * with this, because unplacing is quite rare.
 *
 * This implementation assumes that each thread contains only devices belonging
 * to a given task (to save many extra list.erases). */
void Placer::unplace(P_task* task)
{
    /* Remove task-imposed constraints. NB: This is not a for loop, because
     * elements are removed from the container within the loop. */
    std::list<Constraint*>::iterator constraintIterator = \
        constraints.begin();
    while (constraintIterator != constraints.end())
    {
        /* Say goodbye! */
        if ((*constraintIterator)->task == task)
        {
            delete *constraintIterator;
            constraintIterator = constraints.erase(constraintIterator);
        }

        /* Safe for now. */
        else constraintIterator++;
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
}

/* Updates taskToCores with the entries of a task, which has been placed
 * correctly. Doesn't check much. */
void Placer::update_task_to_cores_map(P_task* task)
{
    /* Grab the set we're inserting into. */
    std::set<P_core*>* coreSet;
    coreSet = &(taskToCores[task]);

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
