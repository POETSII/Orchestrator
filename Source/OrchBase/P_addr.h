#ifndef __P_addrH__H
#define __P_addrH__H

#include <stdio.h>
#include <string>
using namespace std;

//==============================================================================
// Class to encapsulate the concept of a device address. The address as such
// has six subfields (Box.Board.Mailbox.Core.Thread.Device), any of which can
// be valid or not at any time (indicated by the int A_???V variable). By and
// large, A_???V == 0 implies that subfield has been set.  The individual
// hardware components have a P_addr (with the appropriate fields set ot not),
// and these can be set (merged) with the operators provided. Once the devices
// are mapped and crosslinked, the full device P_addr is derived from the
// mapped hardware hierarchy.  The whole thing has a rather banal access
// interface to allow us to do it differently if we ever want to.

// It's made of two classes, P_addr and P_addr_t, the latter being the subset of
// the class state that can be packed into packets and ferried about in messages
//==============================================================================

struct P_addr_t
{
                   P_addr_t();
                   P_addr_t(unsigned,unsigned,unsigned,unsigned,unsigned,
                            unsigned);
void               Dump(unsigned = 0,FILE * = stdout);
unsigned           A_box;              // Box address
unsigned           A_board;            // Board address
unsigned           A_mailbox;          // And so on....
unsigned           A_core;
unsigned           A_thread;
unsigned           A_device;
};

//==============================================================================

class P_addr : public P_addr_t
{
public:
                   P_addr();
                   P_addr(unsigned,unsigned,unsigned,unsigned,unsigned,
                          unsigned);
virtual ~          P_addr();

void               Dump(unsigned = 0,FILE * = stdout);

int                GetBoard   (unsigned & a){ a=A_board;   return A_boardV;  }
int                GetBox     (unsigned & a){ a=A_box;     return A_boxV;    }
int                GetMailbox (unsigned & a){ a=A_mailbox; return A_mailboxV;}
int                GetCore    (unsigned & a){ a=A_core;    return A_coreV;   }
int                GetDevice  (unsigned & a){ a=A_device;  return A_deviceV; }
int                GetThread  (unsigned & a){ a=A_thread;  return A_threadV; }
bool               OK();
void               RawA(unsigned &,unsigned &,unsigned &,unsigned &,unsigned &,
                        unsigned &);
void               RawV(int &,int &,int &,int &,int &, int &);
void               Reset();
void               SetBoard  (unsigned a){ A_board   = a; A_boardV   = 0; }
void               SetBox    (unsigned a){ A_box     = a; A_boxV     = 0; }
void               SetMailbox(unsigned a){ A_mailbox = a; A_mailboxV = 0; }
void               SetCore   (unsigned a){ A_core    = a; A_coreV    = 0; }
void               SetDevice (unsigned a){ A_device  = a; A_deviceV  = 0; }
void               SetThread (unsigned a){ A_thread  = a; A_threadV  = 0; }
string             Str();
P_addr             operator|(P_addr &);
P_addr             operator|=(P_addr &);

private:
int                A_boxV;             // Box address valid?
int                A_boardV;           // Board address valid?
int                A_mailboxV;         // And so on....
int                A_coreV;
int                A_threadV;
int                A_deviceV;

};

//==============================================================================

#endif
