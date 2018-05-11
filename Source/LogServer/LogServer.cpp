//------------------------------------------------------------------------------

#include "LogServer.h"
#include "CommonBase.h"
#include "PMsg_p.hpp"
#include "mpi.h"
#include "Pglobals.h"
#include "jnj.h"
#include <stdio.h>

//==============================================================================

LogServer::LogServer(int argc,char * argv[],string d):
  CommonBase(argc,argv,d,string(__FILE__))
{
string txt = "OrchestratorMessages.txt";
LoadMessages(txt);                     // Load message templates
logfn = "POETS_Logfile.log";           // Output log file name
Mcount['I']=0;                         // Initialise message type counts:
Mcount['W']=0;                         // Not sure why this is necessary
Mcount['E']=0;
Mcount['S']=0;
Mcount['F']=0;
Mcount['U']=0;
Mcount['X']=0;
Mcount['D']=0;
InitFile();                            // Stuff the boilerplate into the logfile

FnMapx.push_back(new FnMap_t);    // create a new event map in the derived class

// Load the message map
(*FnMapx[0])[PMsg_p::KEY(Q::LOG,Q::POST,Q::N000,Q::N000)] = &LogServer::OnLogP;


MPISpinner();                          // Spin on MPI messages; exit only on DIE

printf("********* LogServer rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

LogServer::~LogServer()
{
WALKVECTOR(FnMap_t*, FnMapx, F)
    delete *F;
//printf("********* LogServer rank %d destructor\n",Urank); fflush(stdout);
fprintf(logfp,"\n");
char c;
c='I'; fprintf(logfp,"%4u (%c)nformation   messages\n",Mcount[c],c);
c='W'; fprintf(logfp,"%4u (%c)arning       messages\n",Mcount[c],c);
c='E'; fprintf(logfp,"%4u (%c)rror         messages\n",Mcount[c],c);
c='S'; fprintf(logfp,"%4u (%c)evere        messages\n",Mcount[c],c);
c='F'; fprintf(logfp,"%4u (%c)atal         messages\n",Mcount[c],c);
c='U'; fprintf(logfp,"%4u (%c)nrecoverable messages\n",Mcount[c],c);
c='X'; fprintf(logfp,"%4u E(%c)ternal      messages\n",Mcount[c],c);
c='D'; fprintf(logfp,"%4u (%c)ebug         messages\n",Mcount[c],c);
unsigned total = 0;
WALKMAP(char,unsigned,Mcount,i)total += (*i).second;
fprintf(logfp,"     Total %3u       messages\n",total);

fprintf(logfp,"\nPOETS execution log closed at %s\n"
              "======================================"
              "======================================\n",GetTime());
fclose(logfp);                         // Close the logfile
}

//------------------------------------------------------------------------------

string LogServer::Assemble(int id,vector<string> & rvargs)
// Routine to assemble the template with id "id" and the vector of strings into
// one humungous string. This is effectively a bodgette of a format decoder that
// only knows about strings. We make a copy of the message descriptor, then loop
// through it, replacing each "%s" with the strings in the argument vector. But
// then it is only one line. How simple is that?
// Or it was only one line. There is a problem if the message proforma string is
// malformed, for example with less "%s" substrings than user-supplied
// arguments. The problem is that text.find("%s") returns -1 (FFFFFFFF) if it
// can't find the "%s", although the spec says it's an unsigned.
// Fix: just test to see if text.find(...) returns > text.size() .....
{
string text = Msgs[id].Ms;             // Copy of message descriptor
string D2 = "..";                      // Let us substitute:
WALKVECTOR(string,rvargs,i)
  if (text.find("%s")>text.size()) text += D2 + *i;
  else text.replace(text.find("%s"),2,*i);
return text;                           // Simples.
}

//------------------------------------------------------------------------------

unsigned LogServer::Connect(bool server,string svc)
{
unsigned err = CommonBase::Connect(server,svc);
FnMapx.push_back(new FnMap_t);    // create a new event map in the derived class
// Logserver should be able to log messages from remote groups as well.
(*FnMapx[FnMapx.size()-1])[PMsg_p::KEY(Q::LOG,Q::POST,Q::N000,Q::N000)] = &LogServer::OnLogP;
return err;
}

void LogServer::Dump(FILE * fp)
{
fprintf(fp,"LogServer dump+++++++++++++++++++++++++++++++++++\n");
printf("Message handler function map:\n");
printf("Key        Method\n");
WALKVECTOR(FnMap_t*,FnMapx,F)
{
WALKMAP(unsigned,pMeth,(**F),i)
  fprintf(fp,"%#010x %#010p\n",(*i).first,(*i).second);
}
printf("\nMessage map:\n");
printf("Key(id)    Data(id : format string)\n");
WALKMAP(int,M,Msgs,i)
  fprintf(fp,"%3d  %c %s\n",(*i).first,(*i).second.typ,(*i).second.Ms.c_str());
printf("\nMessage counters:\n");
printf("Type  Instance count\n");
WALKMAP(char,unsigned,Mcount,i)
  fprintf(fp,"%c : %3d\n",(*i).first,(*i).second);
fprintf(fp,"LogServer dump-----------------------------------\n");
CommonBase::Dump(fp);
}

//------------------------------------------------------------------------------

void LogServer::InitFile()
// Write all the header stuff into the logfile
{
logfp = fopen(logfn.c_str(),"w");
if (logfp==0) return;                  // Paranoia
fprintf(logfp,"\nPOETS execution log\n"
              "======================================"
              "======================================\n");
fprintf(logfp,"Created by %s\nFile %s opened at %s on %s\n",
               Sbinary.c_str(),logfn.c_str(),GetTime(),GetDate());
//pPmap->Dump(logfp);


}

//------------------------------------------------------------------------------

void LogServer::LoadMessages(string smessage)
{
if (!Msgs.empty()) return;             // It's static; maybe another instance
                                       // has loaded it?

JNJ P;                                 // Parser for message file
P.Add(smessage);                       // Do it
if (P.ErrCnt()!=0) {                   // Find out if the message file loaded OK
  // look for the comm with the root
  int cIdx=RootCIdx();
  if (cIdx < 0)
  {
     // bad initialisation 
     fprintf(logfp,"Error: no Root process running\n");
     return;
  }
  PMsg_p Z;                            // No; talk to the console the hard way
  Z.comm = Comms[cIdx];
  Z.Key(Q::LOG,Q::FULL,Q::N000,Q::N000);
  string m = "Logserver message file " + smessage + " corrupt or inaccessible";
  Z.Put(1,&m);
  Z.Send(pPmap[cIdx]->U.Root);
  return;
}
                                       // OK, here if we're clear to go
hJNJ sect = P.LocSect(0);              // Only one section
hJNJ recd = P.LocRecd(sect);           // Initialise the 'current record'
while (recd!=0) {                      // Walk the records in the section
  vH labls,types,varis;                // ID, type, message string vectors
  P.GetLabl(recd,labls);               // Get 'em while they're 'ot
  if (!labls.empty()) {                // Something to do?
    P.GetSub(labls[0],types);          // Message type
    P.GetVari(recd,varis);             // So we've got everything we need
                                       // (It may not be sensible, but...)
    int m = str2int(labls[0]->str);    // Message ID
    char t = '_';                      // Message type
    if (!types.empty()) t=types[0]->str[0];
    string s;                          // Message string
    if(!varis.empty()) s = varis[0]->str;
    Msgs[m] = M(t,s);                  // Shove it into the map
  }
  recd = P.LocRecd(sect,recd);
}
}

//------------------------------------------------------------------------------

void LogServer::OnIdle()
{
fflush(stdout);
static vector<bool> flags;             // All the processes registered?
// look for new comms
for (unsigned i = flags.size(); i < Comms.size(); i++)
{
  if (pPmap[i]->vPmap.size()==unsigned(Usize[i])) {
  fprintf(logfp,"For comm %d:\n", i);
  pPmap[i]->Dump(logfp);
  flags.push_back(true);       // Make sure we just do this once for each comm
}
}
CommonBase::OnIdle();                  // Any base actions?
}

//------------------------------------------------------------------------------

unsigned LogServer::OnLogP(PMsg_p * Z,unsigned cIdx)
// Incoming abbreviated message for the server
// LOG|POST|   -|   - (1:int)message_id,(1:vector<string>)arguments
{
int rIdx=RootCIdx();
if (rIdx < 0)
{
   fprintf(logfp,"Error: no Root process running\n");
   return -1;
}
int cnt;                               // Unpack message id
int * pid = Z->Get<int>(1,cnt);
int id = -1;
if (pid!=0) id = *pid;                 // Corrupt message (-1) OK here
vector<string> vstr;                   // Unpack string vector
Z->GetX(1,vstr);
char typ = Msgs[id].typ;               // Unpack message type
//printf("LogServer::OnLogP id=%d\n",id);
//WALKVECTOR(string,vstr,i) printf("||%s||\n",(*i).c_str());
string sfull = Assemble(id,vstr);
PMsg_p Z2;                             // Build the full message
Z2.comm = Comms[rIdx];
Z2.Key(Q::LOG,Q::FULL,Q::N000,Q::N000);
Z2.Put<int>(1,&id);                    // Message id
Z2.Put<char>(2,&typ);                  // Message type
Z2.Put(3,&sfull);                      // Load assembled full string
Z2.Send(pPmap[cIdx]->U.Root);          // Send it back to the console
                                       // And write it into the log file.....
fprintf(logfp,"%s: %3d(%c) %s\n\n",GetTime(),id,typ,sfull.c_str());
Mcount[typ]++;                         // Increment the type counter
return 0;
}

//==============================================================================

