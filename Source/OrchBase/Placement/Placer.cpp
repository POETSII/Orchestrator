/* Defines placer (see the accompanying header for further information). */

#include "Placer.h"

Placer::Placer(P_engine* engine):engine(engine){}

Placer::~Placer()
{
    /* Free memory for each constraint and algorithm. */
    std::list<Constraint*>::iterator constraintIterator;
    std::map<P_task*, Algorithm*>::iterator algorithmIterator;
    for (constraintIterator = constraints.begin();
         constraintIterator != constraints.end();
         delete (*constraintIterator)++);
    for (algorithmIterator = placedTasks.begin();
         algorithmIterator != placedTasks.end();
         delete algorithmIterator++->second);
}

/* Given a string and a set of arguments, creates an instance of a "derived
 * class" algorithm, and returns a pointer it. */
Algorithm* Placer::algorithm_from_string(std::string colloquialDescription)
{
    Algorithm* output = PNULL;
    if (colloquialDescription == "bucket") output = new BucketFilling;

    if (output == PNULL)
    {
        throw InvalidAlgorithmDescriptorException(dformat(
            "[ERROR] Invalid string '%s' passed to Placer::algorithm_from_"
            "string (or we ran out of memory).",
            colloquialDescription.c_str()));
    }

    return output;
}

/* Returns true if all of the devices in a task are mapped to a thread, and
 * false otherwise. Arguments:
 *
 * - task: Task to scan for devices.
 *
 * - unmapped: Vector populated with unmapped devices (for further
 *   diagnosis). Cleared before being populated. */
bool Placer::check_all_devices_mapped(P_task* task,
                                      std::vector<P_device*>* unmapped)
{
    /* Sanity. */
    unmapped->clear();

    /* Iterate through each device in the task. */
    WALKPDIGRAPHNODES(unsigned, P_device*, unsigned, P_message*, unsigned,
                      P_pin*, task->pD->G, deviceIterator)
    {
        P_device* device = task->pD->G.NodeData(deviceIterator);

        /* If it's mapped, we move on. If not, we add it to unmapped. */
        std::map<P_device*, P_thread*>::iterator deviceFinder;
        deviceFinder = deviceToThread.find(device);
        if (deviceFinder != deviceToThread.end()) unmapped->push_back(device);
    }

    return unmapped->empty();
}

/* Computes the fitness for a task.
 *
 * Fitness is simply the sum of all costs on all edges, along with the sum of
 * any broken soft constraints. This evaluation does not factor in broken hard
 * constraints. Arguments:
 *
 *  - task: Task to evaluate the fitness of.
 *
 * Returns the fitness as a float. */
float Placer::compute_fitness(P_task* task)
{
    return 0; // <!>
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

/* Low-level method to create a thread-device binding. Does no checking. */
void Placer::link(P_thread* thread, P_device device)
{
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
    /* Grab an algorithm (exception will propagate). */
    Algorithm* algorithm = algorithm_from_string(algorithmDescription);

    /* Don't do it if the task has already been placed (by address). */
    if (placedTasks.find(task) != placedTasks.end())
        throw AlreadyPlacedException(dformat(
            "[ERROR] Task from file %s has already been placed.",
            task->filename.c_str()));

    /* Run it. */
    float score = algorithm->do_it(task, this);

    /* Check integrity. */
    std::vector<P_device*> unmappedDevices;
    if (check_all_devices_mapped(task, &unmappedDevices))
        /* <!> Panic */
    {}

    return score;
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
    /* Remove task-imposed constraints. */
    std::list<Constraint*>::iterator constraintIterator;
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
}

/* Stubs (I'm lazy) <!> */
void Placer::Dump(FILE*){return;}
