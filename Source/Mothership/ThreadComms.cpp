#include "ThreadComms.h"

ThreadComms::ThreadComms(Mothership* mothership):
    mothership(mothership)
{
    quit = false;

    /* Initialise mutexes. */
    pthread_mutex_init(&mutex_MPI_cnc_queue);
    pthread_mutex_init(&mutex_MPI_app_queue);
    pthread_mutex_init(&mutex_backend_output_queue);
}

ThreadComms::~ThreadComms()
{
    /* Tear down mutexes. */
    pthread_mutex_destroy(&mutex_MPI_cnc_queue);
    pthread_mutex_destroy(&mutex_MPI_app_queue);
    pthread_mutex_destroy(&mutex_backend_output_queue);
}

/* Starts all of the threads. Returns when one of the following is true:
 *  - All threads have exited
 *  - There was an error starting or joining to a thread
 *  - MPIInputBroker exited without setting the quit flag (ala SYST,KILL). */
void ThreadComms::go()
{
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
        mothership->Post(400, e.message);
        return;
    }

    try
    {
        join_mpi_input_broker();
        if(!is_it_time_to_go()){return;}
        join_mpi_cnc_resolver();
        join_mpi_application_resolver();
        join_backend_output_broker();
        join_backend_input_broker();
        join_debug_input_broker();
    }
    catch (ThreadException &e)
    {
        mothership->Post(401, e.message);
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
JOIN_THREAD_DEFINITION(&MPIInputBroker, mpi_input_broker)
JOIN_THREAD_DEFINITION(&MPICncResolver, mpi_cnc_resolver)
JOIN_THREAD_DEFINITION(&MPIApplicationResolver, mpi_application_resolver)
JOIN_THREAD_DEFINITION(&BackendOutputBroker, backend_output_broker)
JOIN_THREAD_DEFINITION(&BackendInputBroker, backend_input_broker)
JOIN_THREAD_DEFINITION(&DebugInputBroker, debug_input_broker)

/* More code repetition ahoy! I would use the preprocessor to generate this,
 * but then it would be too hard for your mere-mortal eyes to read. */

/* ===== MPICncQueue ======================================================= */

/* If there are messages in the queue, grabs the next message from MPICncQueue,
 * writes message with it, and returns true. Otherwise, returns false. */
bool ThreadComms::pop_MPI_cnc_queue(PMsg_p* message)
{
    if (MPICncQueue.empty()) return false;
    pthread_mutex_lock(&mutex_MPI_cnc_queue);
    *message = MPICncQueue.front();
    MPICncQueue.pop();
    pthread_mutex_unlock(&mutex_MPI_cnc_queue);
    return true;
}

/* If there are messages in the queue, grabs all messages from MPICncQueue,
 * writes them to messages, and returns true. Otherwise, returns false. */
bool ThreadComms::pop_MPI_cnc_queue(std::vector<PMsg_p>* messages)
{
    if (MPICncQueue.empty()) return false;
    pthread_mutex_lock(&mutex_MPI_app_queue);
    messages->clear();
    while (!MPICncQueue.empty())
    {
        messages->push_back(MPICncQueue.front());
        MPICncQueue.pop();
    }
    pthread_mutex_unlock(&mutex_MPI_app_queue);
    return true;
}

/* Takes message and places it into the queue. */
bool ThreadComms::push_MPI_cnc_queue(PMsg_p message)
{
    pthread_mutex_lock(&mutex_MPI_cnc_queue);
    MPICncQueue.push(message);
    pthread_mutex_unlock(&mutex_MPI_cnc_queue);
}

/* Takes all messages and pushes them into the queue in the order in which they
 * are stored in the vector. */
bool ThreadComms::push_MPI_cnc_queue(std::vector<PMsg_p>* messages)
{
    pthread_mutex_lock(&mutex_MPI_cnc_queue);
    for (std::vector<PMsg_p>::iterator message = messages.begin();
         message != messages.end(); message++)
    {
        MPICncQueue.push(message);
    }
    pthread_mutex_unlock(&mutex_MPI_cnc_queue);
}

/* ===== MPIAppQueue ======================================================= */

/* If there are messages in the queue, grabs the next message from MPIAppQueue,
 * writes message with it, and returns true. Otherwise, returns false. */
bool ThreadComms::pop_MPI_app_queue(PMsg_p* message)
{
    if (MPIAppQueue.empty()) return false;
    pthread_mutex_lock(&mutex_MPI_app_queue);
    *message = MPIAppQueue.front();
    MPIAppQueue.pop();
    pthread_mutex_unlock(&mutex_MPI_app_queue);
    return true;
}

/* If there are messages in the queue, grabs all messages from MPIAppQueue,
 * writes them to messages, and returns true. Otherwise, returns false. */
bool ThreadComms::pop_MPI_app_queue(std::vector<PMsg_p>* messages)
{
    if (MPIAppQueue.empty()) return false;
    pthread_mutex_lock(&mutex_MPI_app_queue);
    messages->clear();
    while (!MPIAppQueue.empty())
    {
        messages->push_back(MPIAppQueue.front());
        MPIAppQueue.pop();
    }
    pthread_mutex_unlock(&mutex_MPI_app_queue);
    return true;
}

/* Takes message and places it into the queue. */
bool ThreadComms::push_MPI_app_queue(PMsg_p message)
{
    pthread_mutex_lock(&mutex_MPI_app_queue);
    MPIAppQueue.push(message);
    pthread_mutex_unlock(&mutex_MPI_app_queue);
}

/* Takes all messages and pushes them into the queue in the order in which they
 * are stored in the vector. */
bool ThreadComms::push_MPI_app_queue(std::vector<PMsg_p>* messages)
{
    pthread_mutex_lock(&mutex_MPI_app_queue);
    for (std::vector<PMsg_p>::iterator message = messages.begin();
         message != messages.end(); message++)
    {
        MPIAppQueue.push(message);
    }
    pthread_mutex_unlock(&mutex_MPI_app_queue);
}

/* ===== BackendOutputQueue ================================================ */

/* If there are packets in the queue, grabs the next packet from
 * BackendOutputQueue, writes packet with it, and returns true. Otherwise,
 * returns false. */
bool ThreadComms::pop_backend_out_queue(P_Pkt_t* packet)
{
    if (BackendOutputQueue.empty()) return false;
    pthread_mutex_lock(&mutex_backend_out_queue);
    *packet = BackendOutputQueue.front();
    BackendOutputQueue.pop();
    pthread_mutex_unlock(&mutex_backend_out_queue);
    return true;
}

/* If there are packets in the queue, grabs all packets from
 * BackendOutputQueue, writes them to packets, and returns true. Otherwise,
 * returns false. */
bool ThreadComms::pop_backend_out_queue(std::vector<P_Pkt_t>* packets)
{
    if (BackendOutputQueue.empty()) return false;
    pthread_mutex_lock(&mutex_backend_out_queue);
    packets->clear();
    while (!BackendOutputQueue.empty())
    {
        packets->push_back(BackendOutputQueue.front());
        BackendOutputQueue.pop();
    }
    pthread_mutex_unlock(&mutex_backend_out_queue);
    return true;
}

/* Takes packet and places it into the queue. */
bool ThreadComms::push_backend_out_queue(P_Pkt_t packet)
{
    pthread_mutex_lock(&mutex_backend_out_queue);
    BackendOutputQueue.push(packet);
    pthread_mutex_unlock(&mutex_backend_out_queue);
}

/* Takes all packets and pushes them into the queue in the order in which they
 * are stored in the vector. */
bool ThreadComms::push_backend_out_queue(std::vector<P_Pkt_t>* packets)
{
    pthread_mutex_lock(&mutex_backend_out_queue);
    for (std::vector<P_Pkt_t>::iterator packet = packets.begin();
         packet != packets.end(); packet++)
    {
        BackendOutputQueue.push(packet);
    }
    pthread_mutex_unlock(&mutex_backend_out_queue);
}

/* ===== BackendInputQueue ================================================= */

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
    if (BackendInputQueue.empty()) return false;
    packets->clear();
    while (!BackendInputQueue.empty())
    {
        packets->push_back(BackendInputQueue.front());
        BackendInputQueue.pop();
    }
    return true;
}

/* Takes packet and places it into the queue. */
bool ThreadComms::push_backend_in_queue(P_Pkt_t packet)
{
    BackendInputQueue.push(packet);
}

/* Takes all packets and pushes them into the queue in the order in which they
 * are stored in the vector. */
bool ThreadComms::push_backend_in_queue(std::vector<P_Pkt_t>* packets)
{
    for (std::vector<P_Pkt_t>::iterator packet = packets.begin();
         packet != packets.end(); packet++)
    {
        BackendInputQueue.push(packet);
    }
}

/* ===== DebugInputQueue =================================================== */

/* If there are packets in the queue, grabs the next packet from
 * DebugInputQueue, writes packet with it, and returns true. Otherwise, returns
 * false. */
bool ThreadComms::pop_debug_in_queue(P_Pkt_t* packet)
{
    if (DebugInputQueue.empty()) return false;
    *packet = DebugInputQueue.front();
    DebugInputQueue.pop();
    return true;
}

/* If there are packets in the queue, grabs all packets from DebugInputQueue,
 * writes them to packets, and returns true. Otherwise, returns false. */
bool ThreadComms::pop_debug_in_queue(std::vector<P_Pkt_t>* packets)
{
    if (DebugInputQueue.empty()) return false;
    packets->clear();
    while (!DebugInputQueue.empty())
    {
        packets->push_back(DebugInputQueue.front());
        DebugInputQueue.pop();
    }
    return true;
}

/* Takes packet and places it into the queue. */
bool ThreadComms::push_debug_in_queue(P_Pkt_t packet)
{
    DebugInputQueue.push(packet);
}

/* Takes all packets and pushes them into the queue in the order in which they
 * are stored in the vector. */
bool ThreadComms::push_debug_in_queue(std::vector<P_Pkt_t>* packets)
{
    for (std::vector<P_Pkt_t>::iterator packet = packets.begin();
         packet != packets.end(); packet++)
    {
        DebugInputQueue.push(packet);
    }
}
