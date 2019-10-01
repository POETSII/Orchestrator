#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_PLACER_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_PLACER_H

/* Describes the encapsulated placement logic, which maps a POETS application
 * (loaded from an XML, presumably) to the POETS hardware stack (specifically,
 * a POETS Engine).
 *
 * The Placer is a self-contained description of how devices (P_device) are
 * placed onto threads (P_thread).
 *
 * See the placement documentation for further information. */

#include <list>
#include <map>

#include "Algorithm.h"
#include "Constraint.h"
#include "DumpUtils.h"
#include "HardwareModel.h"
#include "P_device.h"
#include "P_task.h"

class Placer: public DumpChan
{
public:
    Placer(P_engine* engine);
    ~Placer();
    P_engine* engine;

    /* Placement information for the entire system is held in these maps. */
    std::map<P_device*, P_thread*> deviceToThread;
    std::map<P_thread*, std::list<P_device*>> threadToDevices;

    /* Constraint management. */
    std::list<Constraint*> constraints;

    /* Information on tasks that have been placed. */
    std::map<P_task*, Algorithm*> placedTasks;

    /* Fitness evaluation. */
    float compute_fitness(P_task* task);

    /* Diagnostics */
    void Dump(FILE* = stdout);

    /* Doing the dirty */
    float place(P_engine*, P_task*, std::string);
    unsigned unplace(P_engine*, P_task*);

    /* Integrity */
    bool check_all_devices_mapped(P_engine*, P_task*);
};

#endif
