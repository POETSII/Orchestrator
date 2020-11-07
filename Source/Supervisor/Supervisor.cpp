#include "Supervisor.h"


extern "C"
{
    int SupervisorInit()
    {
        // Call Supervisor::OnInit
        return Supervisor::OnInit()
    }

    int SupervisorCall(PMsg_p* In, PMsg_p* Out)
    {
#ifdef _APPLICATION_SUPERVISOR_
        int sentErr = 0;
        uint8_t opcode;
        std::vector<P_Pkt_t> pkts;  // packets are packed in Tinsel packet
                                    // format
        In->Put<P_Pkt_t>();
        In->Get(1, pkts);
        WALKVECTOR(P_Pkt_t, pkts, pkt)
        {
            opcode = (pkt->header.swAddr & P_SW_OPCODE_MASK) >>
                P_SW_OPCODE_SHIFT;
            if (opcode == P_CNC_IMPL) sentErr += Supervisor::OnImplicit(&*pkt);
            else sentErr += Supervisor::OnPkt(&*pkt);
        }
        return sentErr;
#endif
        return -1;
    }

    int SupervisorIdle(){return Supervisor::OnIdle();}

    int SupervisorCtl(){return Supervisor::OnCtl();}

    int SupervisorRTCL(){return Supervisor::OnRTCL();}

    int SupervisorExit(){return Supervisor::OnStop();}

    SupervisorDeviceInstance_t SupervisorIdx2Addr(uint32_t idx)
    {
        if(idx >= Supervisor::DeviceVector.size())
        {
            //out of range Idx

            SupervisorDeviceInstance_t fail;
            fail.HwAddr = 0;
            fail.SwAddr = 0;
            fail.Name = "";

            return fail;
        }

        return Supervisor::DeviceVector[idx];
    }

}
