#ifndef __SupervisorApiH__H
#define __SupervisorApiH__H

/* This class holds a series of function pointers that perform
 * "privilege-escalated" jobs using Mothership logic. */

#include <string>

class Mothership;

class SupervisorApi
{
public:
    Mothership* mship;
    std::string appName;
    void (*post)(Mothership* mship, std::string appName, std::string message);
    void (*stop_application)(Mothership* mship, std::string appName);
};

#endif
