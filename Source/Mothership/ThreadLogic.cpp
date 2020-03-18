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

/* Stubs follow */

void* ThreadComms::mpi_cnc_resolver(void* mothershipArg)
{
    Mothership* mothership = (Mothership*)mothershipArg;
    while(1);
    return mothership;
}

void* ThreadComms::mpi_application_resolver(void* mothershipArg)
{
    Mothership* mothership = (Mothership*)mothershipArg;
    while(1);
    return mothership;
}

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
