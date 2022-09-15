//------------------------------------------------------------------------------

#include <stdio.h>
#include "OSFixes.hpp"
#include "flat.h"
#include "PcsBase_t.h"

//==============================================================================

PcsBase_t::PcsBase_t()
// Can't use message subsystem in the constructor/destructor because we don't
// know the order in which things are built/torn down. u$oft and linux do it in
// different ways. But then, they would.....
{
// printf("********* PcsBase_t constructor\n");
// fflush(stdout);
pMCB = DefMCB;
}

//------------------------------------------------------------------------------

PcsBase_t::~PcsBase_t()
{
// printf("********* PcsBase_t destructor\n");
// fflush(stdout);
}

//------------------------------------------------------------------------------

int PcsBase_t::PsMessage(int sev,string mess)
// Message buffer
{
messBuf_v.push_back(mess);             // Preserve in class-local storage
                                       // Call user supplied message handler (?)
if (pMCB!=0)(*pMCB)(pSkyHook,messBuf_v.back());
return sev;                            // Return the severity
}

//------------------------------------------------------------------------------

int PcsBase_t::PsMessage(int sev,string mess,int arg)
// Quick, dirty .....
{
mess += int2str(arg);
return PsMessage(sev,mess);
}

//------------------------------------------------------------------------------

void PcsBase_t::SetMCB(void (* _pMCB)(void *,string))
// Connect a third party message handler
{
//printf("PcsBase_t::SetMCB() Just arrived\n");
//fflush(stdout);
pMCB = (_pMCB==0) ? DefMCB : _pMCB;    // If null, use the default
}

//------------------------------------------------------------------------------

int PcsBase_t::Status(vector<string> & rmessBuf)
{
rmessBuf = messBuf_v;
return (int)messBuf_v.size();
}

//==============================================================================

void DefMCB(void * pSkyHook,string mess)
// Default message handler - may be disconnected and overridden by the user
{
printf("\nDefault message callback\nMessage [%s]\n",mess.c_str());
printf("SkyHook: %" PTR_FMT "\n\n",reinterpret_cast<uint64_t>(pSkyHook));
fflush(stdout);
}

//------------------------------------------------------------------------------
