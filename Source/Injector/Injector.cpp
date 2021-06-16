//------------------------------------------------------------------------------

#include "Injector.h"
#include "CommonBase.h"
#include "PMsg_p.hpp"
#include "mpi.h"
#include "Pglobals.h"
#include "jnj.h"
#include "lex.h"
#include "Cli.h"
#include <stdio.h>

//==============================================================================

Injector::Injector(int argc,char * argv[],string d):
  CommonBase(argc,argv,d,string(__FILE__))
{
                                       // Load the default message map
FnMap[PMsg_p::KEY(Q::INJCT,Q::ACK )] = &Injector::OnInjectAck;
FnMap[PMsg_p::KEY(Q::INJCT,Q::FLAG)] = &Injector::OnInjectFlag;

MPISpinner();                          // Spin on MPI messages; exit only on DIE
}

//------------------------------------------------------------------------------

void Injector::Dump(FILE * fp)
{
fprintf(fp,"\nInjector dump +++++++++++++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"\nI am a 'umble injector. I have no internal state.\n\n");
fprintf(fp,"\nNo-one ever calls me\n");
fprintf(fp,"\nWhy am I here? Why was I written?\n");
CommonBase::Dump(2,fp);
fprintf(fp,"Injector dump -----------------------------------------------\n\n");
}

//------------------------------------------------------------------------------

unsigned Injector::OnInjectFlag(PMsg_p * Z)//,unsigned cIdx)
// Incoming inject message from the monkey
// INJCT|FLAG|-|-| (1:string) originating command
{
string s;
Z->Get(1,s);                           // Unpack command string
Cli cmnd(s);                           // Rebuild command

cmnd.Dump();                           // For want of anything better?
Dump();

// Just for a giggle, inject a "What's the time?" command into the Root:

string query = "system /time";
PMsg_p Msg;
Msg.Put(1,&query);                     // Wrap the command into a message
Msg.Key(Q::INJCT,Q::REQ);              // Message type
Msg.Send(pPmap->U.Root);               // Send it to the injector proces

return 0;
}

//------------------------------------------------------------------------------

unsigned Injector::OnInjectAck(PMsg_p * Z)//,unsigned cIdx)
// Incoming inject message from whatever command has just executed
// INJCT|ACK|   -|   - (???)
{

Z->Dump();                             // For want of anything better?

return 0;
}

//==============================================================================

