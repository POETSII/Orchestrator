#include "Supervisor.h"


extern "C"
{
    SupervisorApi* GetSupervisorApi(){return &(Supervisor::api);}

    int SupervisorInit(){return Supervisor::OnInit();}

    int SupervisorCall(PMsg_p* In, PMsg_p* Out)
    {
#ifdef _APPLICATION_SUPERVISOR_
        int sentErr = 0;
        uint8_t op;

        std::vector<P_Pkt_t> pkts; // packets are packed in Tinsel packet format
        In->Put<P_Pkt_t>();
        In->Get(1, pkts);
        WALKVECTOR(P_Pkt_t, pkts, pkt)
        {
            op = (pkt->header.swAddr & P_SW_OPCODE_MASK) >> P_SW_OPCODE_SHIFT;
            if (op == P_CNC_IMPL) sentErr += Supervisor::OnImplicit(&*pkt);
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

    uint64_t SupervisorIdx2Addr(uint32_t idx)
    {
        uint64_t ret = 0;

        if(idx >= Supervisor::DeviceVector.size())
        {
            //out of range Idx
            return UINT64_MAX;
        }

        ret = Supervisor::DeviceVector[idx].HwAddr;
        ret = ret << 32;
        ret |= Supervisor::DeviceVector[idx].SwAddr;

        return ret;
    }

}

#include "supervisor_generated.cpp"
