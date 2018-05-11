//------------------------------------------------------------------------------

#include "P_pintyp.h"
#include "P_message.h"

//==============================================================================

P_pintyp::P_pintyp(P_devtyp * _p,string _s):par(_p)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
pPropsD = 0;
pPropsI = 0;
pStateD = 0;
pStateI = 0;
pHandl = 0;
pMsg = 0;
PinPropsSize = 0;
PinStateSize = 0;
}

//------------------------------------------------------------------------------

P_pintyp::~P_pintyp()
{
if(pPropsD!=0) delete pPropsD;
if(pPropsI!=0) delete pPropsI;
if(pStateD!=0) delete pStateD;
if(pStateI!=0) delete pStateI;
if(pHandl!=0) delete pHandl;
if(pMsg!=0) delete pMsg;
}

//------------------------------------------------------------------------------

void P_pintyp::Dump(FILE * fp)
{
fprintf(fp,"P_pintyp++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      0x%#08p,0x%#08p\n",this,par);
NameBase::Dump(fp);
fprintf(fp,"P_pintyp------------------------------------\n");
fflush(fp);
}

//==============================================================================



