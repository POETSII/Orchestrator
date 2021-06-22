#include <stdio.h>

#include "Debug.h"
#include "Mothership.h"
#include "OSFixes.hpp"
#include "Unrec_t.h"

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

    catch (OrchestratorException& e)
    {
        fprintf(stderr, "MOTHERSHIP INTERNAL UNHANDLED EXCEPTION: %s\n",
                e.message.c_str());
    }

    catch (...)
    {
        fprintf(stderr, "MOTHERSHIP UNHANDLED EXCEPTION!\n");
    }

    DebugPrint("%s (%s) Main closing down.\n", csMOTHERSHIPproc,
               OSFixes::get_hostname().c_str());
    return 0;
}
