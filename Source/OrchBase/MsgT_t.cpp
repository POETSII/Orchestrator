//------------------------------------------------------------------------------

#include "MsgT_t.h"
#include "GraphT_t.h"

//==============================================================================

MsgT_t::MsgT_t(GraphT_t * _p,string _s):par(_p),pPropsD(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
}

//------------------------------------------------------------------------------

MsgT_t::~MsgT_t()
{
if (pPropsD!=0) delete pPropsD;
}

//------------------------------------------------------------------------------

void MsgT_t::Dump(FILE * fp)
{
fprintf(fp,"MsgT_t+++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      %#018lx,%#018lx\n",
        (uint64_t) this, (uint64_t) par);
NameBase::Dump(fp);
DumpChan::Dump(fp);
fprintf(fp,"MsgT_t-----------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void MsgT_t::MsgDat_cb(MsgT_t * const & m)
// Debug callback for message data
{
if (m!=0) fprintf(dfp,"msg(D): %s",m->Name().c_str());
else fprintf(dfp,"msg(D): ***");
fflush(dfp);
}

//------------------------------------------------------------------------------

void MsgT_t::MsgKey_cb(unsigned const & u)
// Debug callback for message key
{
fprintf(dfp,"msg(K): %03u",u);
fflush(dfp);
}

//==============================================================================








