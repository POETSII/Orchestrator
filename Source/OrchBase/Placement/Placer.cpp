/* Defines placer (see the accompanying header for further information). */

#include "DevT_t.h"
#include "Placer.h"

Placer::Placer():engine(PNULL),cache(PNULL),outFilePath(""){}
Placer::Placer(P_engine* engine):engine(engine),cache(PNULL){}

Placer::~Placer()
{
    if (cache != PNULL) delete cache;

    /* Free memory for each constraint and algorithm. */
    std::list<Constraint*>::iterator constraintIt;
    std::map<GraphI_t*, Algorithm*>::iterator algorithmIt;
    for (constraintIt = constraints.begin(); constraintIt != constraints.end();
         constraintIt++) delete (*constraintIt);
    for (algorithmIt = placedGraphs.begin(); algorithmIt != placedGraphs.end();
         algorithmIt++) delete algorithmIt->second;

    /* Free all supervisors. Freeing supervisors is a litle complicated, as
     * they can be deleted when their GraphI_t object is deleted, and can also
     * be deleted when placement is destroyed, so beware. */
    std::map<GraphI_t*, Algorithm*>::iterator graphIt;
    for (graphIt = placedGraphs.begin(); graphIt != placedGraphs.end();
         graphIt++)
    {
        if ((graphIt->first)->pSupI != PNULL)
        {
            delete (graphIt->first)->pSupI;
            (graphIt->first)->pSupI = PNULL; /* Now the GraphI_t won't
                                             * double-delete */
        }
    }
}

/* Given a string and a set of arguments, creates an instance of a "derived
 * class" algorithm, and returns a pointer it. */
Algorithm* Placer::algorithm_from_string(std::string colloquialDescription)
{
    Algorithm* output = PNULL;
    if (colloquialDescription.substr(0, 2) == "sa")
        output = new SimulatedAnnealing(this, true);  /* With disorder. */
    if (colloquialDescription.substr(0, 2) == "gc")
        output = new SimulatedAnnealing(this, false);  /* Without disorder. */
    if (colloquialDescription.substr(0, 3) == "app" or
        colloquialDescription == "buck" or
        colloquialDescription == "tfil" or
        colloquialDescription == "link") output = new ThreadFilling(this);
    if (colloquialDescription == "rand") output = new SmartRandom(this);
    if (colloquialDescription == "spre") output = new SpreadFilling(this);

    if (output == PNULL)
    {
        throw InvalidAlgorithmDescriptorException(dformat(
            "[ERROR] Invalid string '%s' passed to Placer::algorithm_from_"
            "string (or we ran out of memory).",
            colloquialDescription.c_str()));
    }

    return output;
}

/* A simple filter:
 *
 * - if `unknown` is the name of an algorithm, returns true and sets
 *   `isAlgorithm` to true.
 *
 * - if `unknown` is the name of an argumetn, returns true and sets
 *   `isAlgorithm` to false.
 *
 * - if `unknown` is neither an algorithm nor argument, returns false and does
 *   not set `isAlgorithm`. */
bool Placer::algorithm_or_argument(std::string unknown, bool& isAlgorithm)
{
    if (unknown == "app" or   /* Default */
        unknown == "buck" or  /* Thread filling */
        unknown == "gc" or    /* Gradient climber */
        unknown == "link" or  /* Default */
        unknown == "rand" or  /* Smart-Random */
        unknown == "sa" or    /* Simulated Annealing */
        unknown == "tfil")    /* Thread filling */
    {
        isAlgorithm = true;
        return true;
    }

    else if (unknown == "inpl" or  /* In place */
             unknown == "iter")    /* Maximum number of iterations */
    {
        isAlgorithm = false;
        return true;
    }

    return false;
}

/* Returns true if all core pairs (or single cores, if that's how the model is
 * configured) have devices of one (or zero) type mapped to them, and false
 * otherwise. Arguments:
 *
 * - gi: Application graph instance to scan for devices.
 *
 * - badCoresToDeviceTypes: Map of cores to device types, where the vectors are
 *   more than one element in length. Cleared before being populated. */
bool Placer::are_all_core_pairs_device_locked(GraphI_t* gi,
    std::map<std::pair<P_core*, P_core*>,
             std::set<UniqueDevT> >* badCoresToDeviceTypes)
{
    /* Sanity. */
    badCoresToDeviceTypes->clear();

    /* Build a map of cores to devices, by iterating through all placed
     * threads. */
    std::map<P_core*, std::set<DevI_t*> > coreToDevices;
    std::map<P_thread*, std::list<DevI_t*> >::iterator threadDevsIt;
    std::list<DevI_t*>::iterator deviceIt;

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
    std::map<P_core*, std::set<DevI_t*> >::iterator coreIt;
    std::map<AddressComponent, P_thread*>::iterator threadIt;
    P_core* firstCore;
    P_core* secondCore;

    /* Iterate over each core found from the loop above. */
    for (coreIt = coreToDevices.begin(); coreIt != coreToDevices.end();
         coreIt++)
    {
        firstCore = coreIt->first;
        secondCore = firstCore->pair;  /* May be PNULL. */

        std::pair<P_core*, P_core*> corePair =
            std::make_pair(firstCore, secondCore);
        std::set<UniqueDevT>* devTypSet = &((*badCoresToDeviceTypes)[corePair]);

        /* From each thread in the first core... */
        for (threadIt = firstCore->P_threadm.begin();
             threadIt != firstCore->P_threadm.end(); threadIt++)
        {
            std::list<DevI_t*>* devList =
                &(threadToDevices[threadIt->second]);

            /* ...from each device mapped to that thread... */
            for (deviceIt = devList->begin(); deviceIt != devList->end();
                 deviceIt++)
            {
                /* ...get its device type. */
                UniqueDevT tmpTyp;
                tmpTyp.gi = gi;
                tmpTyp.pT = (*deviceIt)->pT;
                devTypSet->insert(tmpTyp);
            }
        }

        /* And again for its pair, if the pair exists. */
        if (secondCore != PNULL)
        {
            for (threadIt = secondCore->P_threadm.begin();
                 threadIt != secondCore->P_threadm.end(); threadIt++)
            {
                std::list<DevI_t*>* devList =
                    &(threadToDevices[threadIt->second]);

                /* ...from each device mapped to that thread... */
                for (deviceIt = devList->begin(); deviceIt != devList->end();
                     deviceIt++)
                {
                    /* ...get its device type. */
                    UniqueDevT tmpTyp;
                    tmpTyp.gi = gi;
                    tmpTyp.pT = (*deviceIt)->pT;
                    devTypSet->insert(tmpTyp);
                }
            }
        }

        /* Remove the entry from the map if it is of size 1 or less. */
        if (devTypSet->size() < 2) badCoresToDeviceTypes->erase(corePair);
    }

    return badCoresToDeviceTypes->empty();
}

/* Returns true if all of the devices in an application graph instance are
 * mapped to a thread, and false otherwise. Arguments:
 *
 * - gi: Application graph instance to scan for devices.
 *
 * - unmapped: Vector populated with unmapped devices (for further
 *   diagnosis). Cleared before being populated. */
bool Placer::are_all_devices_mapped(GraphI_t* gi,
                                    std::vector<DevI_t*>* unmapped)
{
    /* Sanity. */
    unmapped->clear();

    /* Iterate through each device in the gi. */
    WALKPDIGRAPHNODES(unsigned, DevI_t*, unsigned, EdgeI_t*, unsigned,
                      PinI_t*, gi->G, deviceIterator)
    {
        DevI_t* device = gi->G.NodeData(deviceIterator);

        /* Ignore if it's not a normal device (we don't map those). */
        if (device->devTyp != 'D') continue;

        /* If it's mapped, we move on. If not, we add it to unmapped. */
        std::map<DevI_t*, P_thread*>::iterator deviceFinder;
        deviceFinder = deviceToThread.find(device);
        if (deviceFinder == deviceToThread.end()) unmapped->push_back(device);
    }

    return unmapped->empty();
}

/* Returns true if no hard constraints are broken, and false otherwise.
 *
 * Optionally uses the delta method if devices are defined. Arguments:
 *
 * - gi: Application graph instance to match constraints against (don't care
 *   about constraints that only apply to other gis). If PNULL, checks against
 *   all application graph instances in the placer.
 *
 * - broken: Vector populated with constraints that are not satisfied. Cleared
 *   before being populated.
 *
 * - devices: Series of device pointers; delta computation is performed using
 *   these devices if this container is not empty. */
bool Placer::are_all_hard_constraints_satisfied(GraphI_t* gi,
    std::vector<Constraint*>* broken, std::vector<DevI_t*> devices)
{
    /* Sanity. */
    broken->clear();

    /* For each constraint... */
    std::list<Constraint*>::iterator constraintIterator;
    for (constraintIterator = constraints.begin();
         constraintIterator != constraints.end(); constraintIterator++)
    {
        /* Ignore constraints whose application graph instance doesn't match
         * this graph instance. Skip this check if the graph instance input
         * argument is PNULL. */
        if (gi == PNULL or
            (*constraintIterator)->gi == PNULL or
            (*constraintIterator)->gi == gi)
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

/* Checks the integrity of the placement of a given application graph
 * instance. In order, this method:
 *
 * 1) Checks that all devices have been mapped to a thread.
 *
 * 2) Checks that no hard constraints have been violated.
 *
 * 3) Checks that all core pairs have devices of one or fewer types.
 *
 * Throws a BadIntegrityException if a check fails. */
void Placer::check_integrity(GraphI_t* gi, Algorithm* algorithm)
{
    /* Step 1: Check all devices have been mapped. */
    std::vector<DevI_t*> unmappedDevices;
    if (!are_all_devices_mapped(gi, &unmappedDevices))
    {
        /* Prepare a nice printout of the devices that weren't mapped. */
        std::string devicePrint;
        std::vector<DevI_t*>::iterator deviceIt;
        for (deviceIt = unmappedDevices.begin();
             deviceIt != unmappedDevices.end(); deviceIt++)
        {
            devicePrint.append(dformat("\n - %s",
                                       (*deviceIt)->Name().c_str()));
        }

        /* Toys out of the pram. */
        throw BadIntegrityException(dformat(
            "[ERROR] Use of algorithm '%s' on application graph instance '%s' "
            "from file '%s' resulted in some normal devices not being placed "
            "correctly. These devices are:%s\n",
            algorithm->result.method.c_str(), gi->Name().c_str(),
            gi->par->filename.c_str(), devicePrint.c_str()));
    }

    /* Step 2: Check for any violated hard constraints. */
    std::vector<Constraint*> brokenConstraints;
    if (!are_all_hard_constraints_satisfied(gi, &brokenConstraints))
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
            "on the application graph instance '%s' from file '%s'. The "
            "violated constraints are:%s\n",
            algorithm->result.method.c_str(), gi->Name().c_str(),
            gi->par->filename.c_str(), constraintPrint.c_str()));
    }

    /* Step 3: Check device types. */
    std::map<std::pair<P_core*, P_core*>,
             std::set<UniqueDevT> > badCoresToDeviceTypes;
    if (!are_all_core_pairs_device_locked(gi, &badCoresToDeviceTypes))
    {
        /* Prepare a nice printout of core pairs with multiple device types. */
        std::string corePrint;
        std::map<std::pair<P_core*, P_core*>,
                 std::set<UniqueDevT> >::iterator badCoreIt;
        std::set<UniqueDevT>::iterator devTypIt;

        /* For each entry in the map... */
        for (badCoreIt = badCoresToDeviceTypes.begin();
             badCoreIt != badCoresToDeviceTypes.end(); badCoreIt++)
        {
            /* ...we write a line. That line differs in format for cores
             * without a pair. */
            if (badCoreIt->first.second == PNULL)  /* Not a pair */
            {
                corePrint.append(
                    dformat("\n - '%s':",
                            badCoreIt->first.first->FullName().c_str()));
            }
            else  /* Is a pair */
            {
                corePrint.append(
                    dformat("\n - '%s' and '%s':",
                            badCoreIt->first.first->FullName().c_str(),
                            badCoreIt->first.second->FullName().c_str()));
            }

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
                                         (*devTypIt).pT->Name().c_str()));
            }
        }

        /* Toys out of the pram. */
        throw BadIntegrityException(dformat(
            "[ERROR] Some core pairs have multiple device types associated "
            "with them when using algorithm '%s' on the application graph "
            "instance '%s' from file '%s'. The core pairs and their device "
            "types are:%s\n",
            algorithm->result.method.c_str(), gi->Name().c_str(),
            gi->par->filename.c_str(), corePrint.c_str()));
    }
}

/* Returns the fitness contribution by this device.
 *
 * Adds the weights of all edges involving the passed device, where that device
 * has an entry in deviceToGraphKey (this is assumed). These weights are
 * looked up from giEdgeCosts. Also includes the thread-loading delta.
 *
 * Note that this method doesn't consider constraints.
 *
 * Arguments:
 *
 * - gi: The device must be owned by this application graph instance.
 *
 * - device: The device to investigate. */
float Placer::compute_fitness(GraphI_t* gi, DevI_t* device)
{
    /* Grab the device pair from each edge. */
    std::vector<std::pair<DevI_t*, DevI_t*> > devicePairs;
    get_edges_for_device(gi, device, &devicePairs);

    /* Reduce! */
    float fitness = 0;
    std::vector<std::pair<DevI_t*, DevI_t*> >::iterator devicePairIt;
    for (devicePairIt = devicePairs.begin(); devicePairIt != devicePairs.end();
         devicePairIt++) fitness += giEdgeCosts[gi][*devicePairIt];

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

/* Computes the fitness for an application graph instance.
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
 *  - gi: Application graph instance to evaluate the fitness of.
 *
 * Returns the fitness as a float. */
float Placer::compute_fitness(GraphI_t* gi)
{
    float fitness = 0;

    /* Iterate through each edge in the application graph instance, and add its
     * weight to it. Note that algorithms are responsible for updating the edge
     * weights. */
    std::map<std::pair<DevI_t*, DevI_t*>, float>::iterator edgeIt;
    for (edgeIt = giEdgeCosts[gi].begin(); edgeIt != giEdgeCosts[gi].end();
         edgeIt++) fitness += edgeIt->second;

    /* Iterate through each thread used by the application graph instance and,
     * for each of those threads, increase fitness for each device on that
     * thread beyond the first.
     *
     * We do this by going through each thread in threadToDevices and, if that
     * thread holds devices of a type that are part of this application graph
     * instance, then we increase the fitness based off the number of devices
     * mapped to that thread.
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
    std::map<P_thread*, std::list<DevI_t*> >::iterator threadIt;
    for (threadIt = threadToDevices.begin(); threadIt != threadToDevices.end();
         threadIt++)
    {
        /* Ignore empty threads. */
        if (threadIt->second.empty()) continue;

        /* Does the first device's application graph instance match the graph
         * instance passed to this function as argument? */
        if ((*threadIt->second.begin())->par == gi)
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
        /* Ignore constraints whose application graph instance doesn't match
         * this graph instance. */
        if ((*constraintIterator)->gi == PNULL or
            (*constraintIterator)->gi == gi)
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
 * globally, or to a given application graph instance. */
unsigned Placer::constrained_max_devices_per_thread(GraphI_t* gi)
{
    unsigned maximumSoFar = UINT_MAX;

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
             * application graph instance. */
            if (constraint->gi == gi or constraint->gi == PNULL)
            {
                /* Accept the most strict. */
                unsigned thisMaximum = \
                    dynamic_cast<MaxDevicesPerThread*>(constraint)->maximum;
                if (thisMaximum < maximumSoFar) maximumSoFar = thisMaximum;
            }
        }
    }

    /* If unconstrained, apply a default. */
    if (maximumSoFar == UINT_MAX) return MAX_DEVICES_PER_THREAD_DEFAULT;
    else return maximumSoFar;
}

/* Returns the maximum number of threads that can be used on a core, such that
 * no maxThreadsPerCore constraints are violated that apply either globally, or
 * to a given application graph instance. */
unsigned Placer::constrained_max_threads_per_core(GraphI_t* gi)
{
    unsigned maximumSoFar = UINT_MAX;

    /* Iterate through each constraint. */
    std::list<Constraint*>::iterator constraintIterator;
    for (constraintIterator = constraints.begin();
         constraintIterator != constraints.end();
         constraintIterator++)
    {
        Constraint* constraint = *constraintIterator;

        /* Only care about "maxThreadsPerCore" constraints. */
        if (constraint->category == maxThreadsPerCore)
        {
            /* Only care about constraints that are global, or that match our
             * gi. */
            if (constraint->gi == gi or constraint->gi == PNULL)
            {
                /* Accept the strictest. */
                unsigned thisMaximum = \
                    dynamic_cast<MaxThreadsPerCore*>(constraint)->maximum;
                if (thisMaximum < maximumSoFar) maximumSoFar = thisMaximum;
            }
        }
    }
    return maximumSoFar;
}

/* Populates a map with information about which devices can be placed
 * where. Useful for algorithms. */
void Placer::define_valid_cores_map(GraphI_t* gi,
        std::map<UniqueDevT, std::set<P_core*> >* validCoresForDeviceType)
{
    /* Firstly, populate the map with an entry for each core and each device
     * type in this application graph instance. NB: It would be handy if the
     * application graph instance held a container of pointers to each device
     * type, but it doesn't, so we walk the devices instead (won't take too
     * long). */
    WALKPDIGRAPHNODES(unsigned, DevI_t*, unsigned, EdgeI_t*, unsigned,
                      PinI_t*, gi->G, deviceIterator)
    {
        DevI_t* device = gi->G.NodeData(deviceIterator);

        /* Ignore if it's not a normal device (we don't map those). */
        if (device->devTyp != 'D') continue;

        /* Skip if an entry in the map already exists for a device of this
         * type */
        UniqueDevT deviceType;
        deviceType.gi = gi;
        deviceType.pT = device->pT;
        std::map<UniqueDevT, std::set<P_core*> >::iterator devTypFinder;
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
     * other application graph instances! We don't want to interact with them
     * at all...) */
    std::map<DevI_t*, P_thread*>::iterator deviceIterator;
    std::map<UniqueDevT, std::set<P_core*> >::iterator bigScaryMapIterator;
    for (deviceIterator = deviceToThread.begin();
         deviceIterator != deviceToThread.end(); deviceIterator++)
    {
        UniqueDevT deviceType;
        deviceType.gi = deviceIterator->first->par;
        deviceType.pT = deviceIterator->first->pT;

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

/* Dumps placement information for an application graph instance. Assumes the
 * graph instance has been placed. See the documentation. */
void Placer::dump(GraphI_t* gi)
{
    /* If the score is zero, we're naturally quite suspicious. For example, if
     * the application graph instance was placed using thread-filling (i.e. the
     * algorithm that works fast but doesn't care about the score), we need to
     * compute the score now. */
    std::map<GraphI_t*, Algorithm*>::iterator gisIt = placedGraphs.find(gi);
    if (gisIt->second->result.score == 0)
    {
        /* Build the cost cache. */
        if (cache == PNULL) cache = new CostCache(engine);

        /* Define edge weights for the gi graph. */
        populate_edge_weights(gi);

        /* Compute the score. */
        float score = compute_fitness(gi);

        /* Fill in fields for the result structure. */
        populate_result_structures(&(gisIt->second->result), gi, score);
    }

    /* Get the time. */
    std::string timeBuf = timestamp();

    /* Figure out paths. */
    std::string costPath = dformat("%splacement_gi_edges_%s_%s.csv",
                                   outFilePath.c_str(), gi->Name().c_str(),
                                   timeBuf.c_str());
    std::string diagPath = dformat("%splacement_diagnostics_%s_%s.txt",
                                   outFilePath.c_str(), gi->Name().c_str(),
                                   timeBuf.c_str());
    std::string mapPath = dformat("%splacement_gi_to_hardware_%s_%s.csv",
                                  outFilePath.c_str(), gi->Name().c_str(),
                                  timeBuf.c_str());
    std::string mapRevPath = dformat("%splacement_hardware_to_gi_%s_%s.csv",
                                     outFilePath.c_str(), gi->Name().c_str(),
                                     timeBuf.c_str());
    std::string cachePath = dformat("%splacement_edge_cache_%s.txt",
                                    outFilePath.c_str(), timeBuf.c_str());
    std::string nodeLoadPath = dformat("%splacement_node_loading_%s.csv",
                                       outFilePath.c_str(), timeBuf.c_str());
    std::string edgeLoadPath = dformat("%splacement_edge_loading_%s.csv",
                                       outFilePath.c_str(), timeBuf.c_str());

    /* Call subordinate dumping methods. */
    dump_costs(gi, costPath.c_str());
    dump_diagnostics(gi, diagPath.c_str());
    dump_edge_loading(edgeLoadPath.c_str());
    dump_map(gi, mapPath.c_str());
    dump_map_reverse(gi, mapRevPath.c_str());
    dump_node_loading(nodeLoadPath.c_str());

    FILE* cacheFile = fopen(cachePath.c_str(), "w");
    if (cacheFile == PNULL)
    {
        throw FileOpenException(
            dformat("File: %s. Message: %s",
                    cachePath.c_str(), OSFixes::getSysErrorString(errno)));
    }
    cache->Dump(cacheFile);
    fclose(cacheFile);
}

/* Dumps the cost of each edge for an application graph instance. */
void Placer::dump_costs(GraphI_t* gi, const char* path)
{
    /* File setup. */
    std::ofstream out;
    out.open(path);
    if (!out.is_open())
    {
        throw FileOpenException(
            dformat("File: %s. Message: %s",
                    path, OSFixes::getSysErrorString(errno).c_str()));
    }

    /* Iterate through each edge in the application graph instance. */
    std::map<std::pair<DevI_t*, DevI_t*>, float>::iterator edgeIt;
    for (edgeIt = giEdgeCosts[gi].begin();
         edgeIt != giEdgeCosts[gi].end(); edgeIt++)
    {
        out << edgeIt->first.first->FullName() << ","
            << edgeIt->first.second->FullName() << ","
            << edgeIt->second << std::endl;
    }

    out.close();
}

/* Dumps diagnostic placement information for an application graph instance. */
void Placer::dump_diagnostics(GraphI_t* gi, const char* path)
{
    /* File setup. */
    std::ofstream out;
    out.open(path);
    if (!out.is_open())
    {
        throw FileOpenException(
            dformat("File: %s. Message: %s",
                    path, OSFixes::getSysErrorString(errno).c_str()));
    }

    /* Get the result object address (Placer::dump checks that the graph
     * instance has already been placed). */
    Result* result = &(placedGraphs[gi]->result);

    out << "maxDevicesPerThread:" << result->maxDevicesPerThread << std::endl;
    out << "maxEdgeCost:" << result->maxEdgeCost << std::endl;
    out << "method:" << result->method << std::endl;
    out << "startTime:" << result->startTime << std::endl;
    out << "endTime:" << result->endTime << std::endl;
    out << "score:" << result->score << std::endl;

    /* Args at the end, as key:value pairs. */
    out << "argCount:" << result->args.size() << std::endl;
    std::map<std::string, std::string>::iterator argIt;
    for (argIt = result->args.begin(); argIt != result->args.end(); argIt++)
        out << argIt->first << ":" << argIt->second << std::endl;

    out.close();
}

/* Dump mailbox-level edge loading using the cost cache. */
void Placer::dump_edge_loading(const char* path)
{
    /* File setup. */
    std::ofstream out;
    out.open(path);
    if (!out.is_open())
    {
        throw FileOpenException(
            dformat("File: %s. Message: %s",
                    path, OSFixes::getSysErrorString(errno).c_str()));
    }

    std::map<P_mailbox*, std::map<P_mailbox*, unsigned> > edgeLoading;

    /* Iterate over each application graph instance. */
    DevI_t* fromDevice;
    DevI_t* toDevice;
    P_mailbox* fromMailbox;
    P_mailbox* toMailbox;

    std::map<GraphI_t*, Algorithm*>::iterator giIt;
    for (giIt = placedGraphs.begin(); giIt != placedGraphs.end(); giIt++)
    {
        /* Iterate over each edge for that application graph instance. */
        std::map<std::pair<DevI_t*, DevI_t*>, float>::iterator edgeIt;
        for (edgeIt = giEdgeCosts[giIt->first].begin();
             edgeIt != giEdgeCosts[giIt->first].end(); edgeIt++)
        {
            /* Ignore edges with supervisor devices. <!> Long-term, we
             * obviously don't want to do this - we want to impose a location
             * of supervisors in the compute fabric, and find the edge based
             * off that. Perhaps a "proxy mailbox" in the bridge board or
             * so. */
            fromDevice = edgeIt->first.first;
            toDevice = edgeIt->first.second;
            if (fromDevice->devTyp != 'D') continue;
            if (toDevice->devTyp != 'D') continue;

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
    std::map<P_mailbox*, std::map<P_mailbox*, unsigned> >::iterator \
        edgeOuterIt;
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


/* Dumps mapping information for an application graph instance. */
void Placer::dump_map(GraphI_t* gi, const char* path)
{
    /* File setup. */
    std::ofstream out;
    out.open(path);
    if (!out.is_open())
    {
        throw FileOpenException(
            dformat("File: %s. Message: %s",
                    path, OSFixes::getSysErrorString(errno).c_str()));
    }

    /* Iterate through each device in the application graph instance. */
    WALKPDIGRAPHNODES(unsigned, DevI_t*, unsigned, EdgeI_t*, unsigned,
                      PinI_t*, gi->G, deviceIterator)
    {
        DevI_t* device = gi->G.NodeData(deviceIterator);

        /* If it's mapped, add it. If not, ignore it (good for non-normal
         * devices). */
        std::map<DevI_t*, P_thread*>::iterator deviceFinder;
        deviceFinder = deviceToThread.find(device);
        if (deviceFinder == deviceToThread.end()) continue;
        P_thread* thread = deviceFinder->second;

        out << device->FullName() << "," << thread->FullName() << std::endl;
    }

    out.close();  /* We out, yo. */
}

/* As with dump_map, but puts the hardware nodes first. */
void Placer::dump_map_reverse(GraphI_t* gi, const char* path)
{
    /* File setup. */
    std::ofstream out;
    out.open(path);
    if (!out.is_open())
    {
        throw FileOpenException(
            dformat("File: %s. Message: %s",
                    path, OSFixes::getSysErrorString(errno).c_str()));
    }

    /* Iterate through each thread in the hardware model. */
    HardwareIterator hardwareIt(engine);
    P_thread* thread = hardwareIt.get_thread();
    while (!hardwareIt.has_wrapped())
    {
        /* Only care about this thread if the application owning the first
         * device on this thread matches with the `gi` argument. */
        std::list<DevI_t*>* devsOnThread;
        devsOnThread = &(threadToDevices[thread]);
        if (!devsOnThread->empty())
        {
            if ((*(devsOnThread->begin()))->par == gi)
            {
                /* For each device on this thread, write an entry. */
                std::list<DevI_t*>::iterator deviceIt;
                for (deviceIt = devsOnThread->begin();
                     deviceIt != devsOnThread->end(); deviceIt++)
                {
                    out << thread->FullName() << ","
                        << (*deviceIt)->FullName() << std::endl;
                }
            }
        }
        thread = hardwareIt.next_thread();
    }

    out.close();  /* We out, yo. */
}

/* Dumps node loading information for the entire placer. Core-level and
 * mailbox-level dumps are written to the same file. */
void Placer::dump_node_loading(const char* path)
{
    /* File setup. */
    std::ofstream out;
    out.open(path);
    if (!out.is_open())
    {
        throw FileOpenException(
            dformat("File: %s. Message: %s",
                    path, OSFixes::getSysErrorString(errno).c_str()));
    }

    std::map<P_core*, unsigned> coreLoading;
    std::map<P_core*, unsigned>::iterator coreIt;
    std::map<P_mailbox*, unsigned> mailboxLoading;
    std::map<P_mailbox*, unsigned>::iterator mailboxIt;

    /* Iterate over each placed device. */
    std::map<DevI_t*, P_thread*>::iterator threadIt;
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

/* Grabs all boxes associated with a given application graph instance in this
 * placer's engine. Arguments:
 *
 * - gi: Application graph instance to search for.
 * - boxes: Set to populate with box addresses found. */
void Placer::get_boxes_for_gi(GraphI_t* gi, std::set<P_box*>* boxes)
{
    boxes->clear();

    /* For each core associated with this application graph instance, add its
     * box to the set. */
    std::set<P_core*>::iterator coreIterator;
    std::set<P_core*>* setToScan = &(giToCores[gi]);
    for (coreIterator = setToScan->begin(); coreIterator != setToScan->end();
         coreIterator++)
    {
        boxes->insert((*coreIterator)->parent->parent->parent);  /* Sowi */
    }
}

/* Grabs all of the cores that are contained in the box passed as argument, and
 * that have devices placed upon them from a given graph instance. Arguments:
 *
 * - gi: Application graph instance to filter on.
 * - box: Box in the hardware model to filter on.
 * - cores: Set to populate with cores (cleared before use). */
void Placer::get_cores_for_gi_in_box(GraphI_t* gi, P_box* box,
                                     std::set<P_core*>& cores)
{
    cores.clear();

    /* Start by getting all of the cores used by a graph instance. If none,
     * return quickly. */
    std::set<P_core*>* coresInApp;
    std::map<GraphI_t*, std::set<P_core*> >::iterator coresFinder;
    coresFinder = giToCores.find(gi);
    if (coresFinder == giToCores.end()) return;
    else coresInApp = &(coresFinder->second);

    /* For each core in the resulting search, add it to the output set if it's
     * box is the same as the box passed as argument. */
    for (std::set<P_core*>::iterator coreIt = coresInApp->begin();
         coreIt != coresInApp->end(); coreIt++)
    {
        /* Sorry */
        if ((*coreIt)->parent->parent->parent == box) cores.insert(*coreIt);
    }

    return;
}

/* Grabs all of the device pairs for each edge in the application graph that
 * involves the given device. */
void Placer::get_edges_for_device(GraphI_t* gi, DevI_t* device,
    std::vector<std::pair<DevI_t*, DevI_t*> >* devicePairs)
{
    devicePairs->clear();

    /* Get all of the arcs in the application graph instance that involve this
     * device, in both directions. */
    std::vector<unsigned> arcKeysIn;
    std::vector<unsigned> arcKeysOut;
    gi->G.FindArcs(deviceToGraphKey[device], arcKeysIn, arcKeysOut);

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
        gi->G.FindNodes(*arcKeyIt, firstDeviceKey, secondDeviceKey);
        devicePairs->push_back(
            std::make_pair(
                *(gi->G.FindNode(firstDeviceKey)),
                *(gi->G.FindNode(secondDeviceKey))));
    }
}

/* Low-level method to create a thread-device binding.
 *
 * Does no constraint checking, does not define the address of the device, but
 * does move the device from it's previous location if it was there before.
 *
 * I mean, it just updates the placer maps. */
void Placer::link(P_thread* thread, DevI_t* device)
{
    /* If this device has already been placed on a thread, remove the device
     * from the thread's threadToDevices entry. */
    std::map<DevI_t*, P_thread*>::iterator threadFinder;
    threadFinder = deviceToThread.find(device);
    if (threadFinder != deviceToThread.end())
    {
        std::list<DevI_t*>* listToEraseFrom;
        listToEraseFrom = &(threadToDevices[threadFinder->second]);
        std::list<DevI_t*>::iterator deviceFinder;
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

/* Maps an application graph instance to the engine associated with this
 * placer, using a certain algorithm.  Arguments:
 *
 * - gi: Pointer to the application graph instance to map.
 *
 * - algorithmDescription: Type of algorithm to use (see
 *   algorithm_from_string).
 *
 * Returns the solution fitness, if appropriate (otherwise returns zero). */
float Placer::place(GraphI_t* gi, std::string algorithmDescription)
{
    /* Grab an algorithm, may throw. */
    Algorithm* algorithm = algorithm_from_string(algorithmDescription);

    return place(gi, algorithm);
}

/* Maps an application graph instance to the engine associated with this
 * placer, using a certain algorithm. Arguments:
 *
 * - gi: Pointer to the application graph instance to map.
 *
 * - algorithm: An algorithm instance on the heap.
 *
 * Returns the solution fitness, if appropriate (otherwise returns zero). */
float Placer::place(GraphI_t* gi, Algorithm* algorithm)
{
    /* Danger Will Robinson! (seriously though, how did you get here? --MLV) */
    if (engine == PNULL) throw NoEngineException(
        "You've attempted to place an application graph instance without "
        "defining an engine pointer in the placer. If you're running this "
        "from OrchBase, how did you get here?");

    /* Check that the options set by the operator make sense for the algorithm
     * being used. May throw. */
    args.validate_args(algorithm->result.method);

    /* Complain if the application graph instance has already been placed (by
     * memory address). Don't do this if an in-place approach is requested. */
    bool inPlace = false;
    if (args.is_set("inpl")) inPlace = args.get_bool("inpl");

    if (placedGraphs.find(gi) != placedGraphs.end() and !inPlace)
    {
        delete algorithm;
        throw AlreadyPlacedException(dformat(
            "[ERROR] Application graph instance '%s' from file '%s' has "
            "already been placed.",
            gi->Name().c_str(), gi->par->filename.c_str()));
    }

    /* Define deviceToGraphKey for devices in this application graph
     * instance. */
    populate_device_to_graph_key_map(gi);

    /* Run the algorithm on the application graph instance. */
    float score = algorithm->do_it(gi);

    /* Write input arguments to the algorithm's result object, then clear
     * them. */
    args.copy_to(algorithm->result.args);
    args.clear();

    /* Check placement integrity, throwing if there's a problem. */
    check_integrity(gi, algorithm);

    /* Update gi-keyed maps. */
    placedGraphs[gi] = algorithm;
    update_gi_to_cores_map(gi);

    /* Update result structure for this algorithm object. */
    populate_result_structures(&algorithm->result, gi, score);

    /* Update software addresses for each device placed in this graph
     * instance. */
    update_software_addresses(gi);

    /* Create supervisor binary.
     *
     * NB: This supervisor binary is not mapped to boxes (because for now,
     * nobody actually cares). Later on, graph instances will need a map of box
     * keys to supervisor values, for when supervisors differ across boxes. */
    if (gi->pSupI == PNULL) gi->pSupI = new P_super;
    return score;
}

/* Load a previously-placed configuration. Is pretty unsafe. Arguments:
 *
 * - gi: Pointer to the application graph instance to map.
 *
 * - path: String denoting path to placement information to load
 *   (e.g. placement_gi_to_hardware_file_...)
 *
 * Returns the solution fitness, if appropriate (otherwise returns zero). */
float Placer::place_load(GraphI_t* gi, std::string path)
{
    return place(gi, new PlacementLoader(this, path));
}

/* For each device in the application graph instance, define the placer's
 * reverse-map so that edges can be looked up as a function of device in the
 * application graph instance */
void Placer::populate_device_to_graph_key_map(GraphI_t* gi)
{
    WALKPDIGRAPHNODES(unsigned, DevI_t*, unsigned, EdgeI_t*, unsigned,
                      PinI_t*, gi->G, deviceIterator)
    {
        deviceToGraphKey[gi->G.NodeData(deviceIterator)] = \
            gi->G.NodeKey(deviceIterator);
    }
}

/* Given a pair of devices connected in the application graph instance and an
 * initialised cost cache, populates the weight on the edge connecting them. */
void Placer::populate_edge_weight(GraphI_t* gi, DevI_t* from, DevI_t* to)
{
    /* Skip this edge if one of the devices is a supervisor device. */
    if (from->devTyp != 'D') return;
    if (to->devTyp != 'D') return;

    /* Store the weight. */
    float weight = cache->compute_cost(deviceToThread[from],
                                       deviceToThread[to]);
    giEdgeCosts[gi][std::make_pair(from, to)] = weight;
}

/* Given the placement maps and an initialised cost cache, populates the
 * weights for each edge of a given application graph instance that involves a
 * given device. */
void Placer::populate_edge_weights(GraphI_t* gi, DevI_t* device)
{
    /* Grab the device pair from each edge. */
    std::vector<std::pair<DevI_t*, DevI_t*> > devicePairs;
    get_edges_for_device(gi, device, &devicePairs);

    /* Populate */
    std::vector<std::pair<DevI_t*, DevI_t*> >::iterator devicePairIt;
    for (devicePairIt = devicePairs.begin(); devicePairIt != devicePairs.end();
         devicePairIt++)
    {
        populate_edge_weight(gi, devicePairIt->first, devicePairIt->second);
    }
}

/* Given the placement maps and an initialised cost cache, populates the
 * weights for each edge in an application graph instance. */
void Placer::populate_edge_weights(GraphI_t* gi)
{
    WALKPDIGRAPHARCS(unsigned, DevI_t*, unsigned, EdgeI_t*, unsigned,
                     PinI_t*, gi->G, edgeIt)
    {
        populate_edge_weight(gi,
                             gi->G.NodeData(edgeIt->second.fr_n),
                             gi->G.NodeData(edgeIt->second.to_n));
    }
}

/* Defines the contents of the result struct of an algorithm. */
void Placer::populate_result_structures(Result* result, GraphI_t* gi,
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
     * cores assigned to this application graph instance. */
    result->maxDevicesPerThread = 0;  /* Optimistic. */

    std::set<P_core*>::iterator coreIt;
    std::set<P_core*>* targetSet = &(giToCores[gi]);
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

    std::map<std::pair<DevI_t*, DevI_t*>, float>::iterator edgeIt;
    for (edgeIt = giEdgeCosts[gi].begin();
         edgeIt != giEdgeCosts[gi].end(); edgeIt++)
    {
        /* If the weight on the edge is greater than the current stored
         * maximum, update the maximum. */
        result->maxEdgeCost = std::max(result->maxEdgeCost, edgeIt->second);
    }
}

/* Redistribute all devices placed on threads in a core as evenly as possible
 * across those threads. Remainder devices are placed on threads earlier in the
 * map.
 *
 * Optionally, respect MaxThreadsPerCore constraints imposed on a particular
 * graph instance.
 *
 * Does not check if any threads are overloaded (which is a valid assumption,
 * given that none of the thread are overloaded before this operation). */
void Placer::redistribute_devices_in_core(P_core* core, GraphI_t* gi)
{
    /* Get all devices in the threads in this core. We don't have to unplace
     * them, because `link` does that. */
    std::vector<DevI_t*> devices;
    std::list<DevI_t*>* devicesOnThread;
    std::map<AddressComponent, P_thread*>::iterator threadIt;
    for (threadIt = core->P_threadm.begin(); threadIt != core->P_threadm.end();
         threadIt++)
    {
        devicesOnThread = &(threadToDevices[threadIt->second]);
        std::list<DevI_t*>::iterator deviceIt;
        for (deviceIt = devicesOnThread->begin();
             deviceIt != devicesOnThread->end(); deviceIt++)
        {
            /* Store */
            devices.push_back(*deviceIt);
        }
    }

    /* Deal one device to each thread in turn, until no devices remain (a bit
     * like hold'em). */
    threadIt = core->P_threadm.begin();
    unsigned threadNum = 1;  /* heresy */
    unsigned threadMax = constrained_max_threads_per_core(gi);
    std::vector<DevI_t*>::iterator deviceIt;
    for (deviceIt = devices.begin(); deviceIt != devices.end(); deviceIt++)
    {
        link(threadIt->second, *deviceIt);
        threadIt++;

        /* Thread wraparound. */
        if (threadIt == core->P_threadm.end() or threadNum == threadMax)
        {
            threadIt = core->P_threadm.begin();
            threadNum = 1;
        }
        else threadNum++;
    }
}

/* Redistribute all devices placed on threads in a core for each core in an
 * application graph instance. */
void Placer::redistribute_devices_in_gi(GraphI_t* gi)
{
    std::set<P_core*>* cores = &(giToCores[gi]);
    std::set<P_core*>::iterator coreIt;
    for (coreIt = cores->begin(); coreIt != cores->end(); coreIt++)
    {
        redistribute_devices_in_core(*coreIt, gi);
    }
}

/* Gets the time in ISO-8601=seconds format, and returns a string of it. */
std::string Placer::timestamp()
{
    time_t timeNtv;  /* "Native" */
    time(&timeNtv);
    char timeBuf[sizeof "YYYY-MM-DDTHH:MM:SS"];
    strftime(timeBuf, sizeof timeBuf,
             "%Y-%m-%dT%H-%M-%S", localtime(&timeNtv));
    return timeBuf;
}

/* Removes all device-thread relations for all devices in an application graph
 * instance, and removes constraints imposed on this graph instance.
 *
 * It's slow and, due the the data structures involved, inefficient. We're fine
 * with this, because unplacing is quite rare.
 *
 * This implementation assumes that each thread contains only devices belonging
 * to a given graph instance (to save many extra list.erases). */
void Placer::unplace(GraphI_t* gi, bool andConstraints)
{
    /* Remove graph-imposed constraints (unless explicitly told otherwise). NB:
     * This is not a for loop, because elements are removed from the container
     * within the loop. */
    if (andConstraints)
    {
        std::list<Constraint*>::iterator constraintIterator =   \
            constraints.begin();
        while (constraintIterator != constraints.end())
        {
            /* Say goodbye! */
            if ((*constraintIterator)->gi == gi)
            {
                delete *constraintIterator;
                constraintIterator = constraints.erase(constraintIterator);
            }

            /* You're safe for now. */
            else constraintIterator++;
        }
    }

    /* Clear the maps - iterate through each device in the application graph
     * instance. */
    WALKPDIGRAPHNODES(unsigned, DevI_t*, unsigned, EdgeI_t*, unsigned,
                      PinI_t*, gi->G, deviceIterator)
    {
        DevI_t* device = gi->G.NodeData(deviceIterator);

        /* Is the device placed? If so grab the thread. If not, skip. We don't
         * complain, because we should be cool with unplacing a
         * partially-placed application graph instance. */
        std::map<DevI_t*, P_thread*>::iterator deviceFinder;
        deviceFinder = deviceToThread.find(device);
        if (deviceFinder == deviceToThread.end()) continue;
        P_thread* thread = deviceFinder->second;

        /* Remove the entry from deviceToThread (erasure by position). */
        deviceToThread.erase(deviceFinder);

        /* Remove the entry from threadToDevices (erasure by key). Note that
         * this is safe because each thread is associated with one graph
         * instance (device type). */
        threadToDevices.erase(thread);
    }

    /* Clear the appropriate entry in placedGraphs. */
    std::map<GraphI_t*, Algorithm*>::iterator placedGraphFinder;
    placedGraphFinder = placedGraphs.find(gi);
    if (placedGraphFinder != placedGraphs.end())
    {
        delete placedGraphFinder->second;
        placedGraphs.erase(placedGraphFinder);
    }

    /* Clear the appropriate entry in giToCores. */
    std::map<GraphI_t*, std::set<P_core*> >::iterator giToCoresFinder;
    giToCoresFinder = giToCores.find(gi);
    if (giToCoresFinder != giToCores.end())
    {
        giToCores.erase(giToCoresFinder);
    }

    /* Clear the appropriate entry in giEdgeCosts. */
    std::map<GraphI_t*, std::map<std::pair<DevI_t*, DevI_t*>,
                               float> >::iterator edgeCostsFinder;
    edgeCostsFinder = giEdgeCosts.find(gi);
    if (edgeCostsFinder != giEdgeCosts.end())
    {
        giEdgeCosts.erase(edgeCostsFinder);
    }

    /* No more supervisor. */
    delete gi->pSupI;
    gi->pSupI = PNULL;
}

/* Updates the software addresses of each device in an application graph
 * instance, clearing it if it is not currently placed (for example, if the
 * graph instance has been unplaced. */
void Placer::update_software_addresses(GraphI_t* gi)
{
    std::map<DevI_t*, P_thread*>::iterator deviceFinder;
    std::map<P_thread*, std::list<DevI_t*> >::iterator threadFinder;
    bool found;  /* Is this device currently placed? */

    /* Iterate through each device in the application graph instance. */
    WALKPDIGRAPHNODES(unsigned, DevI_t*, unsigned, EdgeI_t*, unsigned,
                      PinI_t*, gi->G, deviceIterator)
    {
        /* Grab the device, for readability. */
        DevI_t* device = gi->G.NodeData(deviceIterator);

        /* Determine whether or not the device currently placed. */
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

        /* If the device has been placed, update it. */
        if (found)
        {
            /* Get the index of the device within the list associated with the
             * current thread (in order to define the device component of the
             * address object). */
            std::list<DevI_t*>::iterator first, found;
            first = threadFinder->second.begin();
            found = std::find(threadFinder->second.begin(),
                              threadFinder->second.end(), device);

            /* Define the device component of the address object in the
             * device. */
            device->addr.set_ismothership(0);
            device->addr.set_iscnc(0);
            device->addr.set_device(std::distance(first, found));
        }

        /* If the device has not been placed, and is a supervisor device, set
         * the Mothership and CNC component of the address. */
        else if (device->devTyp == 'S')
        {
            device->addr.set_ismothership(1);
            device->addr.set_iscnc(1);
            device->addr.set_device(0);
        }

        /* If the device has not been placed and is not a supervisor device,
         * clear the address. */
        else
        {
            device->addr.set_ismothership(0);
            device->addr.set_iscnc(0);
            device->addr.set_device(0);
        }

        device->addr.set_opcode(0);
        device->addr.set_task(0);
    }
}

/* Updates giToCores with the entries of an application graph instance, which
 * has been placed correctly. Doesn't check much. */
void Placer::update_gi_to_cores_map(GraphI_t* gi)
{
    /* Grab the set we're inserting into. */
    std::set<P_core*>* coreSet;
    coreSet = &(giToCores[gi]);

    /* Iterate through each device. */
    WALKPDIGRAPHNODES(unsigned, DevI_t*, unsigned, EdgeI_t*, unsigned,
                      PinI_t*, gi->G, deviceIterator)
    {
        /* Grab the device, for readability. */
        DevI_t* device = gi->G.NodeData(deviceIterator);

        /* Ignore if it's not a normal device (we don't map those). */
        if (device->devTyp != 'D') continue;

        /* Assume the device is placed - grab the core it's placed on. */
        coreSet->insert(deviceToThread[device]->parent);
    }
}
