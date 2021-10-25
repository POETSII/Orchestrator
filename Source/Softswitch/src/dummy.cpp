#include "tinsel.h"

int main(int argc, char** argv)
{
    // Block on tinselIdle to facilitate hardware idle.
    while(true) tinselIdle(1);
    
    return 0;
}
