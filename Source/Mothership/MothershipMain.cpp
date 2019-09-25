#include "Mothership.h"
//#include "TMoth.h"
#include "Pglobals.h"
#include <cstdio>

int main(int argc, char * argv[])
{
    Mothership* mothership = new Mothership(argc, argv, string(csMOTHERSHIPproc));
    printf("Deleting mothership");
    fflush(stdout);
    delete mothership;
    printf("%s Main closing down\n", csMOTHERSHIPproc);
    fflush(stdout);
    return 0;
}
