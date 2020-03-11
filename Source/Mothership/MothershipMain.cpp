#include <stdio.h>

#include "OSFixes.hpp"
#include "Mothership.h"

int main(int argc, char** argv)
{
    try
    {
        Mothership mothership(argc, argv);
        mothership.go();
    }

    catch (bad_alloc&)
    {
        fprintf(stderr, "MOTHERSHIP OUT OF MEMORY!\n");
    }

    catch (Unrec_t& unrecoverable) {
        fprintf(stderr, "MOTHERSHIP UNRECOVERABLE!\n");
        unrecoverable.Post();
    }

    catch (...)
    {
        fprintf(stderr, "MOTHERSHIP UNHANDLED EXCEPTION!\n");
    }

    printf("%s (%s) Main closing down\n", csMOTHERSHIPproc,
           POETS::get_hostname().c_str());

    return 0;
}
