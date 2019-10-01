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
    check_all_devices_mapped(task);

    return score;
}

/* Stubs (I'm lazy) <!> */
void Placer::Dump(FILE*){return;}
unsigned Placer::unplace(P_task*){return 0;}
bool Placer::check_all_devices_mapped(P_task*){return false;}
