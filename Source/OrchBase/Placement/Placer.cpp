/* Defines placer (see the accompanying header for further information). */

#include "Placer.h"

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

/* Stubs (I'm lazy) <!> */
void Placer::Dump(FILE*){return;}
float Placer::place(P_engine*, P_task*, std::string){return 0;}
unsigned Placer::unplace(P_engine*, P_task*){return 0;}
bool Placer::check_all_devices_mapped(P_engine*, P_task*){return false;}
