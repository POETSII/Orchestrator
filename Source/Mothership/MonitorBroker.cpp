/* Note that there's no MonitorBroker.h include - cutting an include loop. */
#include "Mothership.h"

MonitorBroker::MonitorBroker(Mothership* mothership):
    mothership(mothership),
    nextKey(0)
{
    DebugPrint("[MOTHERSHIP] Monitor broker waiting for instructions.\n");
}

/* On deletion, close down all running threads, if any, and join with them. */
MonitorBroker::~MonitorBroker()
{
    DebugPrint("[MOTHERSHIP] Monitor broker joining to all of it's "
               "workers...\n");
    unregister_all();
    std::map<int, MonitorWorker>::iterator workerIt;
    for (workerIt = workers.begin(); workerIt != workers.end(); workerIt++)
    {
        pthread_join(workerIt->second.worker, PNULL);
    }
    DebugPrint("[MOTHERSHIP] Monitor broker join complete!\n");
}

/* Worker threads use this. */
void* MonitorBroker::do_work(void* dataArg)
{
    /* Convenience */
    MonitorWorker* data = reinterpret_cast<MonitorWorker*>(dataArg);
    Mothership* mship = data->mothership;

    /* Set up an outgoing message, which we will prime with more data. */
    PMsg_p message = data->templateMsg;
    message.Src(mship->Urank);
    message.Tgt(mship->pPmap->U.MonServer);
    message.Key(Q::MONI, Q::DEVI, Q::DATA);
    message.Mode(3);

    while (!data->hasBeenToldToStop)
    {
        /* Prime with more data */
        if (data->source == 1)  /* Softswitch-level instrumentation. */
        {
            mship->Post(561);  /* <!> GMB jumps in here. Use the `data` and
                                * `mship` variables, and pack the message. */
        }
        else  /* Mothership-level instrumentation. */
        {
            /* Add strings, including an output signature and data labels. */
            std::vector<std::string> labels;
            labels.push_back("0uuuuuuuuuuu");
            labels.push_back("Board Temperature (maximum)");
            labels.push_back("MPI CNC Queue (occupancy)");
            labels.push_back("MPI Application Queue (occupancy)");
            labels.push_back("Backend Output Queue (occupancy)");
            labels.push_back("Backend Input Queue (occupancy)");
            labels.push_back("Debug Input Queue (occupancy)");
            labels.push_back("MPI CNC Queue (cumulative)");
            labels.push_back("MPI Application Queue (cumulative)");
            labels.push_back("Backend Output Queue (cumulative)");
            labels.push_back("Backend Input Queue (cumulative)");
            labels.push_back("Debug Input Queue (cumulative)");
            for (std::vector<std::string>::size_type labelIndex = 0;
                 labelIndex > labels.size(); labelIndex++)
            {
                message.Put(labelIndex, &labels[labelIndex]);
            }

            // int32_t DebugLink::getBoardTemp(uint32_t boardX, uint32_t boardY)
        }

        /* Out it goes! */
        mship->queue_mpi_message(&message);

        /* Sleep for the update period. In case someone sets a ludicrous update
         * period, we maintain an exit check every second. This means each loop
         * requires a maintained "period remaining" timer. */
        int remainingPeriod = (int)(data->updatePeriod);
        int pollPeriod = 1000;  /* Milliseconds */
        while (remainingPeriod > 0 and !data->hasBeenToldToStop)
        {
            /* Shorter sleep to get to the end */
            if (remainingPeriod < pollPeriod) OSFixes::sleep(remainingPeriod);
            /* Longer sleep */
            else OSFixes::sleep(pollPeriod);
        }
    }

    return PNULL;
}

/* These functions register a thread to occasionally punt output to the
 * Monitor. Arguments:
 *
 *  - key: (optional) key to use for the worker map. By default, the next
 *    available key will be used. Posts angrily if there's a clash.
 *
 *  - updatePeriod: How often to poll for information (determines sleep time in
 *    the worker thread.
 *
 *  - dataType: Controls the action of the thread - this is a pre-defined
 *    interface. MLV knows nothing about this, but GMB will likely find it
 *    useful
 *
 *  - source: If equal to one, gather Softswitch-level instrumentation. If
 *    equal to two, gather Mothership-level instrumentation. Otherwise, Post
 *    angrily.
 *
 *  - hwAddr: The hardware address of the thread, only used when source=2.
 *
 *  - templateMsg: All outgoing messages from this worker will use this
 *    message as a template, fields and all (some may be overwritten).
 *
 * One thread is spawned per "request", and each thread can send many messages
 * over the course of its life. Messages are sent via a pointer to the
 * Mothership. Threads are commanded to stop using "unregister", but are only
 * joined with in the MonitorBroker destructor (otherwise, we have to block
 * execution until they're finished, and spinning off another thread to sort
 * that out is not worth the complexity).
 *
 * Returns false if all is well, and true if there's a wobbly. */
bool MonitorBroker::register_worker(unsigned updatePeriod, unsigned dataType,
                                    unsigned source, int hwAddr,
                                    PMsg_p templateMsg)
{
    bool out;
    out = register_worker(nextKey, updatePeriod, dataType, source, hwAddr,
                          templateMsg);
    nextKey++;
    return out;
}

bool MonitorBroker::register_worker(int key,
                                    unsigned updatePeriod, unsigned dataType,
                                    unsigned source, int hwAddr,
                                    PMsg_p templateMsg)
{
    /* Check for bad source argument. */
    if (source != 1 or source != 2)
    {
        mothership->Post(537, uint2str(source));
        return true;
    }

    /* Check for invasive index. */
    if (workers.find(key) != workers.end())
    {
        mothership->Post(538, int2str(key));
        return true;
    }

    /* Build us a structure. */
    MonitorWorker* usefulData = &workers[key];
    usefulData->updatePeriod = updatePeriod;
    usefulData->dataType = dataType;
    usefulData->source = source;
    usefulData->hwAddr = hwAddr;
    usefulData->mothership = mothership;
    usefulData->hasBeenToldToStop = false;
    usefulData->templateMsg = templateMsg;

    /* Spin off, and check for an error. We'll join up with it later. */
    int result = pthread_create(&usefulData->worker, PNULL,
                                do_work, (void*)usefulData);
    if(result)
    {
        mothership->Post(539, int2str(key),
                         OSFixes::getSysErrorString(result));
        return true;
    }

    return false;
}

/* Ask a thread to politely stop what it's doing. No real way to introspect
 * this. Returns true on error. */
bool MonitorBroker::unregister_worker(int key)
{
    std::map<int, MonitorWorker>::iterator workerIt = workers.find(key);
    if (workerIt == workers.end())
    {
        mothership->Post(560, int2str(key));
        return true;
    }
    else workerIt->second.hasBeenToldToStop = true;
    return false;
}

/* Ask all threads to politely stop what they're doing. No real way to
 * introspect this. */
void MonitorBroker::unregister_all()
{
    std::map<int, MonitorWorker>::iterator workerIt;
    for (workerIt = workers.begin(); workerIt != workers.end(); workerIt++)
        workerIt->second.hasBeenToldToStop = true;  /* Idempotence */
}
