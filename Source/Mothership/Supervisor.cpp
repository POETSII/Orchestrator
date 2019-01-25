#include "Supervisor.h"

//#undef _APPLICATION_SUPERVISOR_
// when this is defined application supervisors will be built.
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
  int numMsgs = 0;
  int sentErr = 0;
  char outMsgBuf[P_MSG_MAX_SIZE];
  P_Sup_Msg_t* devMsg = In->Get<P_Sup_Msg_t>(0, numMsgs); // decode the message
  if (!devMsg) return -1;                                 // bad message
  for (int msg=0; msg < numMsgs; msg++)
  {
      // run as necessary any OnReceive handler 
      supInputPin* dest = Supervisor::inputs[devMsg[msg].header.destPin];
      // more thought needed about accumulation of error messages vs. send indications
      sentErr += dest->OnReceive(dest->properties, dest->state, &devMsg[msg], Out, outMsgBuf);
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

supInputPin::supInputPin(Sup_OnReceive_t recvHandler, const void* props, void* st)
{
      OnReceive = recvHandler;
      properties = props;
      state = st;
}

supInputPin::~supInputPin()
{
     if (properties) delete properties;
     if (state) delete state;
}

supOutputPin::supOutputPin(Sup_OnSend_t sendHandler)
{
      OnSend = sendHandler;
}

supOutputPin::~supOutputPin()
{
}
