#ifndef __SupervisorH__H
#define __SupervisorH__H

#include "PMsg_p.hpp"
#include "poets_pkt.h"
#include "OSFixes.hpp"
#include "supervisor_generated.h";

#include <iostream>
#include <vector>
#include <map>

typedef struct SupervisorProperties_t;
typedef struct SupervisorState_t;

typedef struct SupervisorDeviceInstance_t
{
    uint32_t HwAddr;
    uint32_t SwAddr;
    std::string Name;   // Temporary until we have Nameserver
} SupervisorDeviceInstance_t;

class Supervisor
{
public:
   static bool __SupervisorInit;
   
   static SupervisorProperties_t* __SupervisorProperties;
   static SupervisorState_t* __SupervisorState;
   
   
   static const std::vector<SupervisorDeviceInstance_t> DeviceVector;
   
   static int OnImplicit(P_Pkt_t*);
   static int OnPkt(P_Pkt_t*);
   
   static int OnInit();
   static int OnStop();
   static int OnCtl();
   static int OnIdle();
   static int OnRTCL();
   
   
}


#endif
