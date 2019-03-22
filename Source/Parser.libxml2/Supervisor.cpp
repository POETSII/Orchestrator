#include "TMoth.h"

// OnReceive takes the pin properties and state, the received message and a buffer for any message to send
typedef unsigned (*Sup_OnReceive_t) (const void*, void*, const P_Sup_Msg_t*, PMsg_p*, void*);
// OnSend takes the message buffer and an indication of whether it's a supervisor message
typedef unsigned (*Sup_OnSend_t) (PMsg_p*, void*, unsigned);

class supInputPin
{
public:

      supInputPin(Sup_OnReceive_t recvHandler, const void* props, void* state);
      ~supInputPin();
  
      Sup_OnReceive_t OnReceive;
      const void* properties;
      void* state;
};

class Supervisor
{
public:

     static vector<supInputPin*> inputs;
     static vector<Sup_OnSend_t> outputs;
}

int TMoth::SupervisorCall(PMsg_p* In, PMsg_p* Out)
{
  int numMsgs;
  char outMsgBuf[P_MSG_MAX_SIZE];
  P_Sup_Msg_t* devMsg = In->Get<P_Sup_Msg_t>(0, numMsgs);
  if (!devMsg)
  {
     Post(530,Name());
     return 0;
  }
  for (int msg=0; msg < numMsgs; msg++)
  {
      supInputPin* dest = inputs[devMsg[msg]->destPin];
      int sentErr = dest->OnReceive(dest->properties, dest->state, devMsg[msg], Out, outMsgBuf);
      if (sentErr < 0) Post(520,int2str(devMsg[msg]->destPin));	  
  }
  return sentErr;
}
