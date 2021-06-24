#ifndef __LogServerH__H
#define __LogServerH__H

#include "CommonBase.h"
#include "OSFixes.hpp"

//==============================================================================

class LogServer : public CommonBase
{

public:
                    LogServer(int,char **,string);
virtual ~           LogServer();

typedef unsigned    (LogServer::*pMeth)(PMsg_p *);
map<unsigned,pMeth> FnMap;

private:
string              Assemble(int,vector<string> &);
#include            "Decode.cpp"
void                Dump(unsigned = 0,FILE * = stdout);
void                InitFile();
void                LoadMessages(string);
bool                HaveIdleWork();
void                OnIdle();
unsigned            OnLoad(PMsg_p *);
unsigned            OnLogP(PMsg_p *);

struct M {                             // Message file extracted
  M(){}                                // Keep the compiler happy
  M(char c,string & s):typ(c),Ms(s){}
  unsigned args();                     // Count the arguments in the template
  char typ;                            // Message category
  string Ms;                           // Message string
};
map<int,M>          Msgs;              // Message map
map<char,unsigned>  Mcount;            // Counter for each type
FILE *              logfp;             // Log file pointer
string              logfn;             // Log file name

};

//==============================================================================

#endif
