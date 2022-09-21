#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_MONITORBROKER_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_MONITORBROKER_H

/* Describes how monitor requests are handled. The Monitor Broker spawns a
 * thread for each request, which sends messages to the MonServer every so
 * often. */

#include <pthread.h>
#include <string>

class Mothership;

#include "MonitorWorker.h"
#include "Mothership.h"

class MonitorBroker
{
public:
    MonitorBroker(Mothership*);
    ~MonitorBroker();

    bool register_worker(int key, unsigned updatePeriod, unsigned dataType,
                         unsigned source, int hwAddr, PMsg_p templateMsg);
    bool register_worker(unsigned updatePeriod, unsigned dataType,
                         unsigned source, int hwAddr, PMsg_p templateMsg);
    bool unregister_worker(int key);
    void unregister_all();

    /* Thread logic */
    static void* do_work(void* dataArg);

private:
    std::map<int, MonitorWorker> workers;
    Mothership* mothership;
    int nextKey;
};

#endif
