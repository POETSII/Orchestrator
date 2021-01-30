#include "Supervisor.h"


extern "C"
{
    SupervisorApi* GetSupervisorApi(){return &(Supervisor::__api);}

    int SupervisorInit(){return Supervisor::OnInit();}

    int SupervisorCall(std::vector<P_Pkt_t>& In, std::vector<P_Addr_Pkt_t> Out)
    {
#ifdef _APPLICATION_SUPERVISOR_
        int sentErr = 0;
        uint8_t op;

        WALKVECTOR(P_Pkt_t, In, pkt)
        {
            op = (pkt->header.swAddr & P_SW_OPCODE_MASK) >> P_SW_OPCODE_SHIFT;
            if (op == P_CNC_IMPL) sentErr += Supervisor::OnImplicit(&*pkt, Out);
            else sentErr += Supervisor::OnPkt(&*pkt, Out);
        }
        return sentErr;
#endif
        return -1;
    }

    int SupervisorIdle(){return Supervisor::OnIdle();}

    int SupervisorCtl(){return Supervisor::OnCtl();}

    int SupervisorRTCL(){return Supervisor::OnRTCL();}

    int SupervisorExit(){return Supervisor::OnStop();}
    
    // Return the full symbolic address of the given index
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
    
    // "return" a copy of the device vector.
    void SupervisorGetAddresses(std::vector<SupervisorDeviceInstance_t>& devV)
    {
        devV = Supervisor::DeviceVector;
    }

}

#include "supervisor_generated.cpp"
