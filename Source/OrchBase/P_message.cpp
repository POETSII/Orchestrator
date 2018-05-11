//------------------------------------------------------------------------------

#include "P_message.h"
#include "P_typdcl.h"

//==============================================================================

P_message::P_message(P_typdcl * _p,string _s):par(_p),pPropsD(0),MsgSize(0),MsgType(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
}

//------------------------------------------------------------------------------

P_message::~P_message()
{
if (pPropsD!=0) delete pPropsD;
}

//------------------------------------------------------------------------------

void P_message::Dump(FILE * fp)
{
fprintf(fp,"P_message+++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      0x%#08p,0x%#08p\n",this,par);
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








