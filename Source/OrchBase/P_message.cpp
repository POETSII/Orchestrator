//------------------------------------------------------------------------------

#include "P_message.h"
#include "P_typdcl.h"
#include "stdint.h"
// #include "P_datatype.h"

//==============================================================================

P_message::P_message(P_typdcl * _p,string _s):par(_p),MsgSize(0),MsgType(0),pPropsD(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
// pProps = 0;                         // temporary until P_datatype is debugged (V3)
}

//------------------------------------------------------------------------------

P_message::~P_message()
{
  // if (pProps!=0) delete pProps;
if (pPropsD!=0) delete pPropsD;
}

//------------------------------------------------------------------------------

void P_message::Dump(FILE * fp)
{
fprintf(fp,"P_message+++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      %#018lx,%#018lx\n",
        (uint64_t) this, (uint64_t) par);
NameBase::Dump(fp);
DumpChan::Dump(fp);
fprintf(fp,"P_message-----------------------------------\n");
fflush(fp);
}
 
//------------------------------------------------------------------------------

void P_message::MsgDat_cb(P_message * const & m)
// Debug callback for message data
{
if (m!=0) fprintf(dfp,"msg(D): %s",m->Name().c_str());
else fprintf(dfp,"msg(D): ***");
fflush(dfp);
}

//------------------------------------------------------------------------------

void P_message::MsgKey_cb(unsigned const & u)
// Debug callback for message key
{
fprintf(dfp,"msg(K): %03u",u);
fflush(dfp);
}

//==============================================================================








