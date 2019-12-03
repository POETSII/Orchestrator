#include "Supervisor.h"

#ifdef _APPLICATION_SUPERVISOR_

extern "C"
{
    /* note that by enclosing the entire extern declaration in braces, variables
still have internal linkage. The alternative, no braces around the extern
declaration, makes the variables external as well. May need some experimentation
if this doesn't work.
*/
int SupervisorCall(PMsg_p* In, PMsg_p* Out)
{
    int sentErr = 0;
    char outMsgBuf[P_MSG_MAX_SIZE];

    vector<P_Super_Msg_t> msgs; // messages are packed in Tinsel message format
    In->Get(0, msgs);      // We assume they're directly placed in the message
    WALKVECTOR(P_Super_Msg_t, msgs, msg) // and they're sent blindly
    {
        uint8_t pin = (((msg->msg.header.pinAddr) & P_HD_TGTPIN_MASK)
                        >> P_HD_TGTPIN_SHIFT);
        supInputPin* dest = Supervisor::inputs[pin];
        sentErr += dest->OnReceive(dest->properties, dest->state, &(msg->msg), Out, outMsgBuf);
    }
    return sentErr;
}

int SupervisorExit()
{
    for (vector<supInputPin*>::iterator ipin = Supervisor::inputs.begin(); ipin != Supervisor::inputs.end(); ipin++)
    delete *ipin;
    for (vector<supOutputPin*>::iterator opin = Supervisor::outputs.begin(); opin != Supervisor::outputs.end(); opin++)
    delete *opin;
    return 0;
} 
}

#else
extern "C"
{
    int SupervisorCall(PMsg_p* In, PMsg_p* Out)
    {
        return -1;
    }

    int SupervisorExit()
    {
        return 0;
    }

    int SupervisorInit()
    {
        return 0;
    }
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

supInputPin::~supInputPin()
{
    if(PinTeardown) PinTeardown(properties, state);
    //if (properties) delete properties;
    //if (state) delete state;
}

supOutputPin::supOutputPin(Sup_OnSend_t sendHandler)
{
    OnSend = sendHandler;
}

supOutputPin::~supOutputPin()
{
}
