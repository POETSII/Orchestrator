#ifndef __SupervisorApiH__H
#define __SupervisorApiH__H

/* This class holds a series of function pointers that perform
 * "privilege-escalated" jobs using Mothership logic. */

#include <string>
#include <vector>
#include "poets_pkt.h"

class Mothership;

class SupervisorApi
{
public:
    Mothership* mship;
    std::string appName;
    std::string (*get_output_directory)(Mothership* mship, std::string appName,
                                        std::string suffix);
    void (*post)(Mothership* mship, std::string appName, std::string message);
    void (*push_packets)(Mothership* mship, std::vector<P_Addr_Pkt_t>& packets);
    void (*stop_application)(Mothership* mship, std::string appName);
};

#endif
