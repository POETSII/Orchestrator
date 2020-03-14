#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_THREADCOMMS_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_THREADCOMMS_H

/* Describes how the different threads of execution in the Mothership interact,
 * and defines queue and pthread constructs for the Mothership. The
 * documentation is pretty good at describing it - you should go read it if you
 * haven't already. */

#include <pthread.h>
#include <queue>

class Mothership;

#include "OSFixes.hpp"
#include "ThreadException.h"
#include "Mothership.h"
#include "poets_pkt.h"
#include "PMsg_p.hpp"

#define START_THREAD_FN_NAME(THREAD) start_##THREAD
#define JOIN_THREAD_FN_NAME(THREAD) join_##THREAD

#define THREAD_METHOD_DECLARATIONS(THREAD) \
    void START_THREAD_FN_NAME(THREAD)(); \
    void JOIN_THREAD_FN_NAME(THREAD)(); \
    static void* THREAD (void* mothership);

#define START_THREAD_DEFINITION(THREAD_PTR, THREAD_NAME) \
void ThreadComms::START_THREAD_FN_NAME(THREAD_NAME)() \
{ \
    int result = pthread_create(THREAD_PTR, PNULL, THREAD_NAME, \
                                (void*)mothership); \
    if(!result) \
    { \
        throw ThreadException(dformat("'THREAD_NAME': %s", \
            POETS::getSysErrorString(result).c_str())); \
    } \
}

#define JOIN_THREAD_DEFINITION(THREAD_OBJ, THREAD_NAME) \
void ThreadComms::JOIN_THREAD_FN_NAME(THREAD_NAME)() \
{ \
    int result = pthread_join(THREAD_OBJ, PNULL);     \
    if(!result) \
    { \
        throw ThreadException(dformat("'THREAD_NAME': %s", \
            POETS::getSysErrorString(result).c_str())); \
    } \
}

class ThreadComms
{
public:
    ThreadComms(Mothership*);
    ~ThreadComms();
    void go();

    /* Quitting logic. */
    inline void set_quit(){quit = true;};
    inline bool is_it_time_to_go(){return quit;};

    /* Queue and mutex manipulation */
    bool pop_MPI_cnc_queue(PMsg_p*);
    bool pop_MPI_cnc_queue(std::vector<PMsg_p>*);
    void push_MPI_cnc_queue(PMsg_p);
    void push_MPI_cnc_queue(std::vector<PMsg_p>*);

    bool pop_MPI_app_queue(PMsg_p*);
    bool pop_MPI_app_queue(std::vector<PMsg_p>*);
    void push_MPI_app_queue(PMsg_p);
    void push_MPI_app_queue(std::vector<PMsg_p>*);

    bool pop_backend_out_queue(P_Pkt_t*);
    bool pop_backend_out_queue(std::vector<P_Pkt_t>*);
    void push_backend_out_queue(P_Pkt_t);
    void push_backend_out_queue(std::vector<P_Pkt_t>*);

    bool pop_backend_in_queue(P_Pkt_t*);
    bool pop_backend_in_queue(std::vector<P_Pkt_t>*);
    void push_backend_in_queue(P_Pkt_t);
    void push_backend_in_queue(std::vector<P_Pkt_t>*);

    bool pop_debug_in_queue(P_Pkt_t*);
    bool pop_debug_in_queue(std::vector<P_Pkt_t>*);
    void push_debug_in_queue(P_Pkt_t);
    void push_debug_in_queue(std::vector<P_Pkt_t>*);

private:
    Mothership* mothership;
    bool quit;

    /* Mutexes */
    pthread_mutex_t mutex_MPI_cnc_queue;
    pthread_mutex_t mutex_MPI_app_queue;
    pthread_mutex_t mutex_backend_output_queue;

    /* Non-main "Threads" */
    pthread_t MPIInputBroker;
    pthread_t MPICncResolver;
    pthread_t MPIApplicationResolver;
    pthread_t BackendOutputBroker;
    pthread_t BackendInputBroker;
    pthread_t DebugInputBroker;

    /* Thread controls */
    THREAD_METHOD_DECLARATIONS(mpi_input_broker)
    THREAD_METHOD_DECLARATIONS(mpi_cnc_resolver)
    THREAD_METHOD_DECLARATIONS(mpi_application_resolver)
    THREAD_METHOD_DECLARATIONS(backend_output_broker)
    THREAD_METHOD_DECLARATIONS(backend_input_broker)
    THREAD_METHOD_DECLARATIONS(debug_input_broker)

    /* Queues */
    std::queue<PMsg_p> MPICncQueue;
    std::queue<PMsg_p> MPIAppQueue;
    std::queue<P_Pkt_t> BackendOutputQueue;
    std::queue<P_Pkt_t> BackendInputQueue;
    std::queue<P_Pkt_t> DebugInputQueue;
};

#endif
