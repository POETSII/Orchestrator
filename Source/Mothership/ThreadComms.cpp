#include "Mothership.h"

ThreadComms::ThreadComms(Mothership* mothership):
    mothership(mothership)
{
    quit = false;

    /* Initialise mutexes. */
    pthread_mutex_init(&mutex_MPI_cnc_queue, PNULL);
    pthread_mutex_init(&mutex_MPI_app_queue, PNULL);
    pthread_mutex_init(&mutex_backend_output_queue, PNULL);
    pthread_mutex_init(&mutex_backend_api, PNULL);
}

ThreadComms::~ThreadComms()
{
    /* Tear down mutexes. */
    pthread_mutex_destroy(&mutex_MPI_cnc_queue);
    pthread_mutex_destroy(&mutex_MPI_app_queue);
    pthread_mutex_destroy(&mutex_backend_output_queue);
    pthread_mutex_destroy(&mutex_backend_api);
}

/* Starts all of the threads. Returns when one of the following is true:
 *  - All threads have exited
 *  - There was an error starting or joining to a thread
 *  - MPIInputBroker exited without setting the quit flag (ala SYST,KILL). */
void ThreadComms::go()
{
    DebugPrint("[MOTHERSHIP] Starting producer consumer threads.\n");
    try
    {
        start_mpi_input_broker();
        start_mpi_cnc_resolver();
        start_mpi_application_resolver();
        start_backend_output_broker();
        start_backend_input_broker();
        start_debug_input_broker();
    }
    catch (ThreadException &e)
    {
        /* The conditional block is needed here because we might not yet know
         * where our logserver is. */
        if (!(mothership->Post(500, e.message)))
        {
            printf("Mothership ERROR: Could not create pthread %s. Exiting.\n",
                   e.message.c_str());
        }
        return;
    }
    DebugPrint("[MOTHERSHIP] Threads started successfully. The main thread is "
               "now waiting to join.\n");

    try
    {
        join_mpi_input_broker();
        if(!is_it_time_to_go())
        {
            DebugPrint("Mothership WARNING: The MPI Input Broker thread "
                       "exited without stopping other threads!\n");
            return;
        }
        join_mpi_cnc_resolver();
        join_mpi_application_resolver();
        join_backend_output_broker();
        join_backend_input_broker();
        join_debug_input_broker();
    }
    catch (ThreadException &e)
    {
        /* The conditional block is needed here because we might not yet know
         * where our logserver is. */
        if (!(mothership->Post(501, e.message)))
        {
            printf("Mothership ERROR: Could not join to pthread %s. Exiting.\n",
                   e.message.c_str());
        }
        return;
    }
}

/* Code repetition ahoy! (methods for starting threads) */
START_THREAD_DEFINITION(&MPIInputBroker, mpi_input_broker)
START_THREAD_DEFINITION(&MPICncResolver, mpi_cnc_resolver)
START_THREAD_DEFINITION(&MPIApplicationResolver, mpi_application_resolver)
START_THREAD_DEFINITION(&BackendOutputBroker, backend_output_broker)
START_THREAD_DEFINITION(&BackendInputBroker, backend_input_broker)
START_THREAD_DEFINITION(&DebugInputBroker, debug_input_broker)

/* Code repetition ahoy! (methods for joining threads) */
JOIN_THREAD_DEFINITION(MPIInputBroker, mpi_input_broker)
JOIN_THREAD_DEFINITION(MPICncResolver, mpi_cnc_resolver)
JOIN_THREAD_DEFINITION(MPIApplicationResolver, mpi_application_resolver)
JOIN_THREAD_DEFINITION(BackendOutputBroker, backend_output_broker)
JOIN_THREAD_DEFINITION(BackendInputBroker, backend_input_broker)
JOIN_THREAD_DEFINITION(DebugInputBroker, debug_input_broker)

/* More code repetition ahoy! I would use the preprocessor to generate this,
 * but then it would be too hard for your mere-mortal eyes to read. */

/* ===== MPICncQueue ======================================================= */

/* If there are messages in the queue, grabs the next message from MPICncQueue,
 * writes message with it, and returns true. Otherwise, returns false. */
bool ThreadComms::pop_MPI_cnc_queue(PMsg_p* message)
{
    bool returnValue = false;
    pthread_mutex_lock(&mutex_MPI_cnc_queue);
    if (!MPICncQueue.empty())
    {
        returnValue = true;
        *message = MPICncQueue.front();
        MPICncQueue.pop();
    }
    pthread_mutex_unlock(&mutex_MPI_cnc_queue);
    return returnValue;
}

/* If there are messages in the queue, grabs all messages from MPICncQueue,
 * writes them to messages, and returns true. Otherwise, returns false. */
bool ThreadComms::pop_MPI_cnc_queue(std::vector<PMsg_p>* messages)
{
    bool returnValue = false;
    messages->clear();
    pthread_mutex_lock(&mutex_MPI_cnc_queue);
    while (!MPICncQueue.empty())
    {
        returnValue = true;
        messages->push_back(MPICncQueue.front());
        MPICncQueue.pop();
    }
    pthread_mutex_unlock(&mutex_MPI_cnc_queue);
    return returnValue;
}

/* Takes message and places it into the queue. */
void ThreadComms::push_MPI_cnc_queue(PMsg_p message)
{
    pthread_mutex_lock(&mutex_MPI_cnc_queue);
    MPICncQueue.push(message);
    pthread_mutex_unlock(&mutex_MPI_cnc_queue);
}

/* Takes all messages and pushes them into the queue in the order in which they
 * are stored in the vector. */
void ThreadComms::push_MPI_cnc_queue(std::vector<PMsg_p>* messages)
{
    pthread_mutex_lock(&mutex_MPI_cnc_queue);
    for (std::vector<PMsg_p>::iterator message = messages->begin();
         message != messages->end(); message++)
    {
        MPICncQueue.push(*message);
    }
    pthread_mutex_unlock(&mutex_MPI_cnc_queue);
}

/* ===== MPIAppQueue ======================================================= */

/* If there are messages in the queue, grabs the next message from MPIAppQueue,
 * writes message with it, and returns true. Otherwise, returns false. */
bool ThreadComms::pop_MPI_app_queue(PMsg_p* message)
{
    bool returnValue = false;
    pthread_mutex_lock(&mutex_MPI_app_queue);
    if (!MPIAppQueue.empty())
    {
        returnValue = true;
        *message = MPIAppQueue.front();
        MPIAppQueue.pop();
    }
    pthread_mutex_unlock(&mutex_MPI_app_queue);
    return returnValue;
}

/* If there are messages in the queue, grabs all messages from MPIAppQueue,
 * writes them to messages, and returns true. Otherwise, returns false. */
bool ThreadComms::pop_MPI_app_queue(std::vector<PMsg_p>* messages)
{
    bool returnValue = false;
    messages->clear();
    pthread_mutex_lock(&mutex_MPI_app_queue);
    while (!MPIAppQueue.empty())
    {
        returnValue = true;
        messages->push_back(MPIAppQueue.front());
        MPIAppQueue.pop();
    }
    pthread_mutex_unlock(&mutex_MPI_app_queue);
    return returnValue;
}

/* Takes message and places it into the queue. */
void ThreadComms::push_MPI_app_queue(PMsg_p message)
{
    pthread_mutex_lock(&mutex_MPI_app_queue);
    MPIAppQueue.push(message);
    pthread_mutex_unlock(&mutex_MPI_app_queue);
}

/* Takes all messages and pushes them into the queue in the order in which they
 * are stored in the vector. */
void ThreadComms::push_MPI_app_queue(std::vector<PMsg_p>* messages)
{
    pthread_mutex_lock(&mutex_MPI_app_queue);
    for (std::vector<PMsg_p>::iterator message = messages->begin();
         message != messages->end(); message++)
    {
        MPIAppQueue.push(*message);
    }
    pthread_mutex_unlock(&mutex_MPI_app_queue);
}

/* ===== BackendOutputQueue ================================================ */

/* If there are packets in the queue, grabs the next packet from
 * BackendOutputQueue, writes packet with it, and returns true. Otherwise,
 * returns false. */
bool ThreadComms::pop_backend_out_queue(P_Addr_Pkt_t* packet)
{
    bool returnValue = false;
    pthread_mutex_lock(&mutex_backend_output_queue);
    if (!BackendOutputQueue.empty())
    {
        returnValue = true;
        *packet = BackendOutputQueue.front();
        BackendOutputQueue.pop();
    }
    pthread_mutex_unlock(&mutex_backend_output_queue);
    return returnValue;
}

/* If there are packets in the queue, grabs all packets from
 * BackendOutputQueue, writes them to packets, and returns true. Otherwise,
 * returns false. */
bool ThreadComms::pop_backend_out_queue(
    std::vector<P_Addr_Pkt_t>* packets)
{
    bool returnValue = false;
    packets->clear();
    pthread_mutex_lock(&mutex_backend_output_queue);
    while (!BackendOutputQueue.empty())
    {
        returnValue = true;
        packets->push_back(BackendOutputQueue.front());
        BackendOutputQueue.pop();
    }
    pthread_mutex_unlock(&mutex_backend_output_queue);
    return returnValue;
}

/* Takes packet and places it into the queue. */
void ThreadComms::push_backend_out_queue(P_Addr_Pkt_t packet)
{
    pthread_mutex_lock(&mutex_backend_output_queue);
    BackendOutputQueue.push(packet);
    pthread_mutex_unlock(&mutex_backend_output_queue);
}

/* Takes all packets and pushes them into the queue in the order in which they
 * are stored in the vector. */
void ThreadComms::push_backend_out_queue(
    std::vector<P_Addr_Pkt_t>* packets)
{
    std::vector<P_Addr_Pkt_t>::iterator packet;
    pthread_mutex_lock(&mutex_backend_output_queue);
    for (packet = packets->begin(); packet != packets->end(); packet++)
    {
        BackendOutputQueue.push(*packet);
    }
    pthread_mutex_unlock(&mutex_backend_output_queue);
}

/* ===== BackendInputQueue ================================================= */

/* Returns true if the number of packets in this queue is too big to fit into
 * an MPI message given the send buffer size, and false otherwise. */
bool ThreadComms::is_backend_in_queue_full()
{
    /* Be wary - the integer division is intentional. The limit is around 8
     * million packets per message as of 2020-04-16 on Tinsel. Alternatively,
     * the size can be overridden by setting BACKEND_QUEUE_PACKET_MAXIMUM.
     *
     * Note that with both of these options, one can still get an error like:
     *
     * > MPIR_Bsend_isend(311): Insufficient space in Bsend buffer; requested
     * > 673116946; total buffer size is 1000000000
     *
     * This occurs because the receiver (Mothership::CommonBase::MPISpinner())
     * hasn't received the previous message. Normally production systems would
     * introduce an MPI error handler for this, but we haven't done that
     * because time. */
#ifndef BACKEND_QUEUE_PACKET_MAXIMUM
    return BackendInputQueue.size() > (SNDBUFSIZ - MPI_BSEND_OVERHEAD) /
        sizeof(P_Pkt_t);
#else
    return BackendInputQueue.size() > BACKEND_QUEUE_PACKET_MAXIMUM ;
#endif
}

/* If there are packets in the queue, grabs the next packet from
 * BackendInputQueue, writes packet with it, and returns true. Otherwise,
 * returns false. */
bool ThreadComms::pop_backend_in_queue(P_Pkt_t* packet)
{
    if (BackendInputQueue.empty()) return false;
    *packet = BackendInputQueue.front();
    BackendInputQueue.pop();
    return true;
}

/* If there are packets in the queue, grabs all packets from
 * BackendInputQueue, writes them to packets, and returns true. Otherwise,
 * returns false. */
bool ThreadComms::pop_backend_in_queue(std::vector<P_Pkt_t>* packets)
{
    bool returnValue = false;
    packets->clear();
    while (!BackendInputQueue.empty())
    {
        returnValue = true;
        packets->push_back(BackendInputQueue.front());
        BackendInputQueue.pop();
    }
    return returnValue;
}

/* Takes packet and places it into the queue. */
void ThreadComms::push_backend_in_queue(P_Pkt_t packet)
{
    BackendInputQueue.push(packet);
}

/* Takes all packets and pushes them into the queue in the order in which they
 * are stored in the vector. */
void ThreadComms::push_backend_in_queue(std::vector<P_Pkt_t>* packets)
{
    for (std::vector<P_Pkt_t>::iterator packet = packets->begin();
         packet != packets->end(); packet++)
    {
        BackendInputQueue.push(*packet);
    }
}

/* ===== DebugInputQueue =================================================== */

/* If there are packets in the queue, grabs the next packet from
 * DebugInputQueue, writes packet with it, and returns true. Otherwise, returns
 * false. */
bool ThreadComms::pop_debug_in_queue(P_Debug_Pkt_t* packet)
{
    if (DebugInputQueue.empty()) return false;
    *packet = DebugInputQueue.front();
    DebugInputQueue.pop();
    return true;
}

/* If there are packets in the queue, grabs all packets from DebugInputQueue,
 * writes them to packets, and returns true. Otherwise, returns false. */
bool ThreadComms::pop_debug_in_queue(std::vector<P_Debug_Pkt_t>* packets)
{
    bool returnValue = false;
    packets->clear();
    while (!DebugInputQueue.empty())
    {
        returnValue = true;
        packets->push_back(DebugInputQueue.front());
        DebugInputQueue.pop();
    }
    return returnValue;
}

/* Takes packet and places it into the queue. */
void ThreadComms::push_debug_in_queue(P_Debug_Pkt_t packet)
{
    DebugInputQueue.push(packet);
}

/* Takes all packets and pushes them into the queue in the order in which they
 * are stored in the vector. */
void ThreadComms::push_debug_in_queue(std::vector<P_Debug_Pkt_t>* packets)
{
    for (std::vector<P_Debug_Pkt_t>::iterator packet = packets->begin();
         packet != packets->end(); packet++)
    {
        DebugInputQueue.push(*packet);
    }
}
