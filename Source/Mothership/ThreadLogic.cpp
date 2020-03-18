/* This source file defines what the threads do when started via
 * pthread_create. Keep calm; this is just a producer-consumer system. It's all
 * in the Mothership documentation. */

#include "Mothership.h"

void* ThreadComms::mpi_input_broker(void* mothershipArg)
{
    Mothership* mothership = (Mothership*)mothershipArg;
    mothership->mpi_spin();
    return mothership;
}

void* ThreadComms::mpi_cnc_resolver(void* mothershipArg)
{
    std::vector<PMsg_p> messages;
    std::vector<PMsg_p>::iterator messageIt;
    unsigned key;
    Mothership* mothership = (Mothership*)mothershipArg;

    /* We spin until we're told to stop. */
    while (!mothership->threading.is_it_time_to_go())
    {
        /* Is there anything in the queue? */
        mothership->threading.pop_MPI_cnc_queue(&messages);

        /* If the queue is empty, chill for a bit before checking again. */
        if (messages.empty())
        {
            sleep(1);
            continue;
        }

        /* Otherwise, handle each message in turn. */
        for (messageIt=messages.begin(); messageIt!=messages.end();
             messageIt++)
        {
            key = messageIt->Key();
            if (key == PMsg_p::KEY(Q::APP, Q::SPEC))
                mothership->handle_msg_app_spec(&*messageIt);
            else if (key == PMsg_p::KEY(Q::APP, Q::DIST))
                mothership->handle_msg_app_dist(&*messageIt);
            else if (key == PMsg_p::KEY(Q::APP, Q::SUPD))
                mothership->handle_msg_app_supd(&*messageIt);
            else if (key == PMsg_p::KEY(Q::CMND, Q::RECL))
                mothership->handle_msg_cmnd_recl(&*messageIt);
            else if (key == PMsg_p::KEY(Q::CMND, Q::INIT))
                mothership->handle_msg_cmnd_init(&*messageIt);
            else if (key == PMsg_p::KEY(Q::CMND, Q::RUN))
                mothership->handle_msg_cmnd_run(&*messageIt);
            else if (key == PMsg_p::KEY(Q::CMND, Q::STOP))
                mothership->handle_msg_cmnd_stop(&*messageIt);
            else if (key == PMsg_p::KEY(Q::BEND, Q::CNC))
                mothership->handle_msg_bend_cnc(&*messageIt);
            else if (key == PMsg_p::KEY(Q::DUMP))
                mothership->handle_msg_dump(&*messageIt);
            else
                mothership->Post(407, "MPICncResolver", uint2str(key));
        }

        /* We're done with the extracted messages - throw them away. */
        messages.clear();
    }

    return mothership;
}

void* ThreadComms::mpi_application_resolver(void* mothershipArg)
{
    std::vector<PMsg_p> messages;
    std::vector<PMsg_p>::iterator messageIt;
    unsigned key;
    Mothership* mothership = (Mothership*)mothershipArg;

    /* We spin until we're told to stop. */
    while (!mothership->threading.is_it_time_to_go())
    {
        /* Is there anything in the queue? */
        mothership->threading.pop_MPI_app_queue(&messages);

        /* If the queue is empty, chill for a bit before checking again. */
        if (messages.empty())
        {
            sleep(1);
            continue;
        }

        /* Otherwise, handle each message in turn. */
        for (messageIt=messages.begin(); messageIt!=messages.end();
             messageIt++)
        {
            key = messageIt->Key();
            if (key == PMsg_p::KEY(Q::BEND, Q::SUPR))
                mothership->handle_msg_bend_supr(&*messageIt);
            else if (key == PMsg_p::KEY(Q::PKTS))
                mothership->handle_msg_pkts(&*messageIt);
            else
                mothership->Post(407, "MPIAppResolver", uint2str(key));
        }

        /* We're done with the extracted messages - throw them away. */
        messages.clear();
    }

    return mothership;
}

/* Stubs follow */

void* ThreadComms::backend_output_broker(void* mothershipArg)
{
    Mothership* mothership = (Mothership*)mothershipArg;
    while(1);
    return mothership;
}

void* ThreadComms::backend_input_broker(void* mothershipArg)
{
    Mothership* mothership = (Mothership*)mothershipArg;
    while(1);
    return mothership;
}

void* ThreadComms::debug_input_broker(void* mothershipArg)
{
    Mothership* mothership = (Mothership*)mothershipArg;
    while(1);
    return mothership;
}
