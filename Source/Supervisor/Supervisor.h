#ifndef __SupervisorH__H
#define __SupervisorH__H

#include "poets_pkt.h"
#include "OSFixes.hpp"
#include "supervisor_generated.h"
#include "SupervisorApi.h"
#include "macros.h"

#include <cstdint>
#include <string>
#include <iostream>
#include <vector>
#include <map>

typedef struct SupervisorDeviceInstance_t
{
    uint32_t HwAddr;
    uint32_t SwAddr;
    std::string Name;   // Temporary until we have Nameserver
} SupervisorDeviceInstance_t;

class SupervisorApi;

class Supervisor
{
public:
   static bool __SupervisorInit;
   static SupervisorApi __api;

   static SupervisorProperties_t* __SupervisorProperties;
   static SupervisorState_t* __SupervisorState;

   static const std::vector<SupervisorDeviceInstance_t> DeviceVector;
   static const std::vector<uint32_t> ThreadVector;

   static int OnImplicit(P_Pkt_t*, std::vector<std::pair<uint32_t, P_Pkt_t> >&);
   static int OnPkt(P_Pkt_t*, std::vector<std::pair<uint32_t, P_Pkt_t> >&);

   static int OnInit();
   static int OnStop();
   static int OnCtl();
   static int OnIdle();
   static int OnRTCL();

};

#include "SupervisorApiEntrypoints.h"

#endif
