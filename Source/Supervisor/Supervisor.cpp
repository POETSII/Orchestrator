#include "Supervisor.h"

#ifdef _APPLICATION_SUPERVISOR_

/* note that by enclosing the entire extern declaration in braces, variables
   still have internal linkage. The alternative, no braces around the extern
   declaration, makes the variables external as well. May need some
   experimentation if this doesn't work.
*/
extern "C"
{
    int SupervisorCall(PMsg_p* In, PMsg_p* Out)
    {
        int sentErr = 0;
        char outPktBuf[P_PKT_MAX_SIZE];

        std::vector<P_Pkt_t> pkts; // packets are packed in Tinsel packet format
        In->Put<P_Pkt_t>();
        In->Get(1, pkts);
        WALKVECTOR(P_Pkt_t, pkts, pkt) // and they're sent blindly
        {
            uint8_t pin = (((pkt->header.pinAddr) & P_HD_TGTPIN_MASK)
                           >> P_HD_TGTPIN_SHIFT);
            if (pin >= Supervisor::inputs.size()) return -1; // invalid pin
            supInputPin* dest = Supervisor::inputs[pin];
            sentErr += dest->OnReceive(dest->properties, dest->state, &*pkt,
                                       Out, outPktBuf);
        }
        return sentErr;
    }

    int SupervisorExit()
    {
        for (std::vector<supInputPin*>::iterator ipin = Supervisor::inputs.begin();
             ipin != Supervisor::inputs.end(); ipin++) delete *ipin;
        for (std::vector<supOutputPin*>::iterator opin = Supervisor::outputs.begin();
             opin != Supervisor::outputs.end(); opin++) delete *opin;
        return 0;
    }
}

#else
extern "C"
{
    int SupervisorCall(PMsg_p* In, PMsg_p* Out){return -1;}
    int SupervisorExit(){return 0;}
    int SupervisorInit(){return 0;}
}

#endif

supInputPin::supInputPin(Sup_OnReceive_t recvHandler,
                         Sup_PinTeardown_t pinTeardown,
                         const void* props, void* st)
{
    OnReceive = recvHandler;
    PinTeardown = pinTeardown;
    properties = props;
    state = st;
}

supInputPin::~supInputPin(){if(PinTeardown) PinTeardown(properties, state);}
supOutputPin::supOutputPin(Sup_OnSend_t sendHandler){OnSend = sendHandler;}
supOutputPin::~supOutputPin(){}
