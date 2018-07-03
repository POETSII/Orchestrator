#include "TMoth.h"
#include "Pglobals.h"
#include <cstdio>

int main(int argc, char * argv[])
{
    TMoth* mothership = new TMoth(argc, argv, string(csMOTHERSHIPproc));
    printf("%s Main closing down\n", csMOTHERSHIPproc);
    fflush(stdout);
    delete mothership;
    return 0;
}
