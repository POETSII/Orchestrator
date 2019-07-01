//------------------------------------------------------------------------------

#include "P_pintyp.h"
#include "P_message.h"
#include "stdint.h"
// #include "P_datatype.h"

//==============================================================================

P_pintyp::P_pintyp(P_devtyp * _p,string _s):par(_p)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
// pProps = 0;
// pState = 0;
pPropsD = 0;
pPropsI = 0;
pStateD = 0;
pStateI = 0;
pHandl = 0;
pMsg = 0;
PinPropsSize = 0; // how should these be determined with V4 input?
PinStateSize = 0;
}

//------------------------------------------------------------------------------

P_pintyp::~P_pintyp()
{
// if (pProps!=0) delete pProps;
// if (pState!=0) delete pState;
if(pPropsD!=0) delete pPropsD;
if(pPropsI!=0) delete pPropsI;
if(pStateD!=0) delete pStateD;
if(pStateI!=0) delete pStateI;
if(pHandl!=0) delete pHandl;
}

//------------------------------------------------------------------------------

void P_pintyp::Dump(FILE * fp)
{
fprintf(fp,"P_pintyp++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"NameBase       %s\n",FullName().c_str());
fprintf(fp,"Me,Parent      %#018lx,%#018lx\n",
        (uint64_t) this, (uint64_t) par);
NameBase::Dump(fp);
fprintf(fp,"P_pintyp------------------------------------\n");
fflush(fp);
}

//==============================================================================



