#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_MONITORBROKER_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_MONITORBROKER_H

/* Describes how monitor requests are handled. The Monitor Broker spawns a
 * thread for each request, which sends messages to the MonServer every so
 * often. */

#include <pthread.h>

class Mothership;

#include "Mothership.h"

struct MonitorWorker
{
    /* <!> Hmm */
};

class MonitorBroker
{
public:
    MonitorBroker(Mothership*);
    ~MonitorBroker();

    int register(int key, std::string ackMsg, unsigned updatePeriod,
                 unsigned dataType, unsigned source, int hwAddr);
    int register(std::string ackMsg, unsigned updatePeriod,
                 unsigned dataType, unsigned source, int hwAddr);
    int unregister(int key);
    int unregister();

private:
    std::map<int, MonitorWorker> workers;
    Mothership* mothership;
    int nextKey;

};
#endif
