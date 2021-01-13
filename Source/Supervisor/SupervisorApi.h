#ifndef __SupervisorApiH__H
#define __SupervisorApiH__H

#include <string>

/* This class holds a series of function pointers that perform
 * "privilege-escalated" jobs using Mothership logic. It also defines a set of
 * macros so that the application-writer can call the functions pointed to by
 * the function pointers. */

#define SUPERVISOR_STOP_APPLICATION() \
    ((Supervisor::__api.stop_application)\
     (Supervisor::__api.mship, Supervisor::__api.appName))

class Mothership;

class SupervisorApi
{
public:
    Mothership* mship;
    std::string appName;
    void (*stop_application)(Mothership* mship, std::string appName);
};

#endif
