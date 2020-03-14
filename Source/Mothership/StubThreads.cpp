#include "Mothership.h"

/* A temporary file to hold stubbed thread logic. */

void* ThreadComms::mpi_input_broker(void* mothershipArg)
{
    Mothership* mothership = (Mothership*)mothershipArg;
    mothership->mpi_spin();
    return mothership;
}

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
