//------------------------------------------------------------------------------

#include "MsgT_t.h"
#include "GraphT_t.h"
#include "CFrag.h"

//==============================================================================

MsgT_t::MsgT_t(GraphT_t * _p,string _s):par(_p),pPropsD(0)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent

pPropsD = 0;
}

//------------------------------------------------------------------------------

MsgT_t::~MsgT_t()
{
if (pPropsD!=0) delete pPropsD;
}

//------------------------------------------------------------------------------

void MsgT_t::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sMsgT_t++++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sNameBase      %s\n",os,FullName().c_str());
fprintf(fp,"%sMe,Parent     %#018lx,%#018lx\n",os,(uint64_t)this,(uint64_t)par);
fprintf(fp,"%sProperties    %#018lx\n",os,(uint64_t)pPropsD);
if (pPropsD!=0) pPropsD->Dump(off+2,fp);
NameBase::Dump(off+2,fp);
DefRef::Dump(off+2,fp);
DumpChan::Dump(off+2,fp);
fprintf(fp,"%sMsgT_t--------------------------------------------------\n\n",os);
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

//------------------------------------------------------------------------------
/*
void MsgT_t::preDS_XML(DS_XML::xnode * pn, DS_XML * pDS)
// XML DS -> Message Type
//
{
printf("MsgT_t::preDS_XML(%s)\n",pn->ename.c_str());
pn->Dump();

fflush(stdout);
}

//------------------------------------------------------------------------------

void MsgT_t::pstDS_XML(DS_XML::xnode * pn,DS_XML *)
{
printf("MsgT_t::pstDS_XML(%s)\n",pn->ename.c_str());
}
  */
//==============================================================================
