#include "PMsg_p.hpp"
#include "poets_msg.h"

// OnReceive takes the pin properties and state, the received message and a buffer for any message to send
typedef unsigned (*Sup_OnReceive_t) (const void*, void*, const P_Sup_Msg_t*, PMsg_p*, void*);
// OnSend takes the message buffer and an indication of whether it's a supervisor message
typedef unsigned (*Sup_OnSend_t) (PMsg_p*, void*, unsigned);

class supInputPin
{
public:

      supInputPin(Sup_OnReceive_t, const void*, void*);
      ~supInputPin();
  
      Sup_OnReceive_t OnReceive;
      const void* properties;
      void* state;
};

class supOutputPin
{
public:

      supOutputPin(Sup_OnSend_t);
      ~supOutputPin();

      Sup_OnSend_t OnSend;
};

class Supervisor
{
public:

     static vector<supInputPin*> inputs;
     static vector<supOutputPin*> outputs;
};


