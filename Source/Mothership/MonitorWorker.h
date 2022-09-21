#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_MONITORWORKER_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_MONITORWORKER_H

/* A structure passed to monitor broker threads. */

class Mothership;

#include "PMsg_p.hpp"

struct MonitorWorker
{
    /* Constants */
    unsigned updatePeriod;  /* Milliseconds */
    unsigned dataType;
    unsigned source;  /* Softswitch or Mothership? */
    int hwAddr;
    PMsg_p templateMsg;  /* Saves copying timestamps and housekeeping
                          * explicitly. */

    /* Traversal */
    Mothership* mothership;

    /* Not constants */
    pthread_t worker;
    bool hasBeenToldToStop;
};

#endif
