//------------------------------------------------------------------------------

#include "P_addr.h"
#include "flat.h"

//==============================================================================

P_addr_t::P_addr_t()
{
}

//------------------------------------------------------------------------------

P_addr_t::P_addr_t(unsigned bx,unsigned bd,unsigned mb,unsigned co,unsigned th,
                   unsigned de)
{
A_box     = bx;
A_board   = bd;
A_mailbox = mb;
A_core    = co;
A_thread  = th;
A_device  = de;
}

//------------------------------------------------------------------------------

void P_addr_t::Dump(FILE * fp)
{
fprintf(fp,"P_addr_t++++++++\n");
fprintf(fp,"A_box     = %4u\n",A_box);
fprintf(fp,"A_board   = %4u\n",A_board);
fprintf(fp,"A_mailbox = %4u\n",A_mailbox);
fprintf(fp,"A_core    = %4u\n",A_core);
fprintf(fp,"A_thread  = %4u\n",A_thread);
fprintf(fp,"A_device  = %4u\n",A_device);
fprintf(fp,"P_addr_t--------\n");
fflush(fp);
}

//==============================================================================

P_addr::P_addr()
{
    Reset();
}

//------------------------------------------------------------------------------

P_addr::P_addr(unsigned bx,unsigned bd,unsigned mb,unsigned co,unsigned th,
               unsigned de)
{
A_boxV     = 0;                         // Addresses valid
A_boardV   = 0;
A_coreV    = 0;
A_mailboxV = 0;
A_threadV  = 0;
A_deviceV  = 0;
A_box      = bx;
A_board    = bd;
A_core     = co;
A_mailbox  = mb;
A_thread   = th;
A_device   = de;
}

//------------------------------------------------------------------------------

P_addr::~P_addr()
{

}

//------------------------------------------------------------------------------

void P_addr::Dump(FILE * fp)
{
string s("");
fprintf(fp,"P_addr  %35s+++++++++++++++++++++++++++++++++++++\n",s.c_str());
fprintf(fp,"A_boxV     = %4d\n",A_boxV);
fprintf(fp,"A_boardV   = %4d\n",A_boardV);
fprintf(fp,"A_mailboxV = %4d\n",A_coreV);
fprintf(fp,"A_coreV    = %4d\n",A_coreV);
fprintf(fp,"A_threadV  = %4d\n",A_threadV);
fprintf(fp,"A_deviceV  = %4d\n",A_deviceV);
P_addr_t::Dump(fp);
fprintf(fp,"OK       = %c\n",OK()?'T':'F');
fprintf(fp,"Str      = ||%s||\n",Str().c_str());
fprintf(fp,"P_addr  %35s-------------------------------------\n",s.c_str());
fflush(fp);
}

//------------------------------------------------------------------------------

bool P_addr::OK()
{
if (A_boxV!=0)     return false;
if (A_boardV!=0)   return false;
if (A_mailboxV!=0) return false;
if (A_coreV!=0)    return false;
if (A_threadV!=0)  return false;
if (A_deviceV!=0)  return false;
return true;
}

//------------------------------------------------------------------------------

void P_addr::RawA(unsigned & rbx,unsigned & rbo,unsigned & rmb,
                  unsigned & rco,unsigned & rth,unsigned & rde)
{
rbx = A_box;
rbo = A_board;
rmb = A_mailbox;
rco = A_core;
rth = A_thread;
rde = A_device;
}

//------------------------------------------------------------------------------

void P_addr::RawV(int & rbxV,int & rboV, int & rmbV,int & rcoV,int & rthV,
                  int & rdeV)
{
rbxV = A_boxV;
rboV = A_boardV;
rmbV = A_mailboxV;
rcoV = A_coreV;
rthV = A_threadV;
rdeV = A_deviceV;
}

//------------------------------------------------------------------------------

void P_addr::Reset()
{
A_boxV     = 1;                         // Box address not valid
A_boardV   = 1;                         // Board address not valid
A_mailboxV = 1;                         // ...
A_coreV    = 1;
A_threadV  = 1;
A_deviceV  = 1;
A_box      = 0;                         // For the sake of it...
A_board    = 0;
A_mailbox  = 0;
A_core     = 0;
A_thread   = 0;
A_device   = 0;
}

//------------------------------------------------------------------------------

string P_addr::Str()
{
string xxxx = "xxxx";
string ans;
ans += (A_boxV==0) ? uint2str(A_box,4) : xxxx;
ans = ans + ".";
ans += (A_boardV==0) ? uint2str(A_board,4) : xxxx;
ans = ans + ".";
ans += (A_mailboxV==0) ? uint2str(A_mailbox,4) : xxxx;
ans = ans + ".";
ans += (A_coreV==0) ? uint2str(A_core,4) : xxxx;
ans = ans + ".";
ans += (A_threadV==0) ? uint2str(A_thread,4) : xxxx;
ans = ans + ".";
ans += (A_deviceV==0) ? uint2str(A_device,4) : xxxx;
return ans;
}

//------------------------------------------------------------------------------

P_addr P_addr::operator|(P_addr & RHS)
// The logic: if one of the fields is unset and the other set, the result is
// the set field.
// If they're both unset OR both set, the answer is unset.
{
P_addr ans;
const int   SET = 0;
const int UNSET = 1;

if ((RHS.A_boxV==  SET)&&(A_boxV==  SET)) ans.A_boxV = UNSET;
if ((RHS.A_boxV==UNSET)&&(A_boxV==UNSET)) ans.A_boxV = UNSET;
if ((RHS.A_boxV==  SET)&&(A_boxV==UNSET)){ans.A_boxV=SET; ans.A_box=RHS.A_box;}
if ((RHS.A_boxV==UNSET)&&(A_boxV==  SET)){ans.A_boxV=SET; ans.A_box=A_box;    }

if ((RHS.A_boardV==  SET)&&(A_boardV==  SET)) ans.A_boardV = UNSET;
if ((RHS.A_boardV==UNSET)&&(A_boardV==UNSET)) ans.A_boardV = UNSET;
if ((RHS.A_boardV==  SET)&&(A_boardV==UNSET)){ans.A_boardV=SET; ans.A_board=RHS.A_board;}
if ((RHS.A_boardV==UNSET)&&(A_boardV==  SET)){ans.A_boardV=SET; ans.A_board=A_board;  }

if ((RHS.A_mailboxV==  SET)&&(A_mailboxV==  SET)) ans.A_mailboxV = UNSET;
if ((RHS.A_mailboxV==UNSET)&&(A_mailboxV==UNSET)) ans.A_mailboxV = UNSET;
if ((RHS.A_mailboxV==  SET)&&(A_mailboxV==UNSET)){ans.A_mailboxV=SET; ans.A_mailbox=RHS.A_mailbox;}
if ((RHS.A_mailboxV==UNSET)&&(A_mailboxV==  SET)){ans.A_mailboxV=SET; ans.A_mailbox=A_mailbox;    }

if ((RHS.A_coreV==  SET)&&(A_coreV==  SET)) ans.A_coreV = UNSET;
if ((RHS.A_coreV==UNSET)&&(A_coreV==UNSET)) ans.A_coreV = UNSET;
if ((RHS.A_coreV==  SET)&&(A_coreV==UNSET)){ans.A_coreV=SET; ans.A_core=RHS.A_core;}
if ((RHS.A_coreV==UNSET)&&(A_coreV==  SET)){ans.A_coreV=SET; ans.A_core=A_core;    }

if ((RHS.A_threadV==  SET)&&(A_threadV==  SET)) ans.A_threadV = UNSET;
if ((RHS.A_threadV==UNSET)&&(A_threadV==UNSET)) ans.A_threadV = UNSET;
if ((RHS.A_threadV==  SET)&&(A_threadV==UNSET)){ans.A_threadV=SET; ans.A_thread=RHS.A_thread;}
if ((RHS.A_threadV==UNSET)&&(A_threadV==  SET)){ans.A_threadV=SET; ans.A_thread=A_thread;    }

if ((RHS.A_deviceV==  SET)&&(A_deviceV==  SET)) ans.A_deviceV = UNSET;
if ((RHS.A_deviceV==UNSET)&&(A_deviceV==UNSET)) ans.A_deviceV = UNSET;
if ((RHS.A_deviceV==  SET)&&(A_deviceV==UNSET)){ans.A_deviceV=SET; ans.A_device=RHS.A_device;}
if ((RHS.A_deviceV==UNSET)&&(A_deviceV==  SET)){ans.A_deviceV=SET; ans.A_device=A_device;    }

return ans;
}

//------------------------------------------------------------------------------

P_addr P_addr::operator|=(P_addr & RHS)
{
*this = *this | RHS;                   // No, I don't know why I need the
return *this;                          // assignment
}

//==============================================================================
