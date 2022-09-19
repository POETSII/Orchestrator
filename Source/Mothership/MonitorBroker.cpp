#include "PMsg.hpp"
#include "MonitorBroker.h"

MonitorBroker::MonitorBroker(Mothership* mothership):
    mothership(mothership),
    nextKey(0)
{
    DebugPrint("[MOTHERSHIP] Monitor broker waiting for instructions.\n");
}

MonitorBroker::~MonitorBroker()
{
    unregister();
}
