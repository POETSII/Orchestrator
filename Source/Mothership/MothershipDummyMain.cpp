#include "MothershipDummy.h"
#include "Pglobals.h"
#include <cstdio>

int main(int argc, char * argv[])
{
    MothershipDummy* mothership = new MothershipDummy(argc, argv, string(csMOTHERSHIPproc));
    delete mothership;
    printf("%s Main closing down\n", csMOTHERSHIPproc);
    fflush(stdout);
    return 0;
}
