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

#define GRAPHPROPERTIES(a)  graphProperties->a
#define SUPPROPERTIES(a)    supervisorProperties->a
#define SUPSTATE(a)         supervisorState->a
#define MSG(a)              message->a
#define PKT(a)              message->a
#define REPLY(a)            reply->a
#define BCAST(a)            bcast->a
#define RTSREPLY()          __rtsReply=true
#define RTSBCAST()          __rtsBcast=true

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

   //static const std::map<std::string,const SupervisorDeviceInstance_t*> DeviceNameMap;

   static int OnImplicit(P_Pkt_t*, std::vector<P_Addr_Pkt_t>&);
   static int OnPkt(P_Pkt_t*, std::vector<P_Addr_Pkt_t>&);

   static int OnInit();
   static int OnStop();
   static int OnCtl();
   static int OnIdle();
   static int OnRTCL();

};

#include "SupervisorApiEntrypoints.h"

#endif
