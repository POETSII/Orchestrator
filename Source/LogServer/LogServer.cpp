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
logfp = 0;                             // No output channel yet
Mcount['I']=0;                         // Initialise message type counts:
Mcount['W']=0;                         // Not sure why this is necessary
Mcount['E']=0;
Mcount['S']=0;
Mcount['F']=0;
Mcount['U']=0;
Mcount['X']=0;
Mcount['D']=0;
                                       // Load the message map
FnMap[PMsg_p::KEY(Q::LOG,Q::POST,Q::N000,Q::N000)] = &LogServer::OnLogP;
FnMap[PMsg_p::KEY(Q::LOG,Q::LOAD,Q::N000,Q::N000)] = &LogServer::OnLoad;

MPISpinner();                          // Spin on MPI messages; exit only on DIE

//printf("********* LogServer rank %d on the way out\n",Urank); fflush(stdout);
}

//------------------------------------------------------------------------------

LogServer::~LogServer()
{
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
if (logfp != stdout) fclose(logfp);       // Close the logfile
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

void LogServer::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%sLogServer dump +++++++++++++++++++++++++++++++++++++++++++\n",os);
printf("%sMessage handler function map:\n",os);
printf("%sKey        Method\n",os);
WALKMAP(unsigned,pMeth,FnMap,i)
{
  fprintf(fp,"%s%#010x %" PTR_FMT "\n",os,(*i).first,
                        OSFixes::getAddrAsUint((*i).second));
}
printf("\n%sMessage map:\n",os);
printf("%sKey(id)    Data(id : format string)\n",os);
WALKMAP(int,M,Msgs,i)
  fprintf(fp,"%s%3d  %c %s\n",
          os,(*i).first,(*i).second.typ,(*i).second.Ms.c_str());
printf("\n%sMessage counters:\n",os);
printf("%sType  Instance count\n",os);
WALKMAP(char,unsigned,Mcount,i)
  fprintf(fp,"%s%c : %3d\n",os,(*i).first,(*i).second);
CommonBase::Dump(off+2,fp);
fprintf(fp,"%sLogServer dump -------------------------------------------\n",os);

}

//------------------------------------------------------------------------------

void LogServer::InitFile()
// Write all the header stuff into the logfile
{
if (logfp==0) return;                  // Paranoia
fprintf(logfp,"\nPOETS execution log\n"
              "======================================"
              "======================================\n");
fprintf(logfp,"Created by %s\nFile %s opened at %s on %s\n",
               Sbinary.c_str(),logfn.c_str(),GetTime(),GetDate());
}

//------------------------------------------------------------------------------

void LogServer::LoadMessages(string smessage)
{
if (!Msgs.empty()) return;             // It's static; maybe another instance
                                       // has loaded it?

JNJ P;                                 // Parser for message file
P.Add(smessage);                       // Do it
if (P.ErrCnt()!=0) {                   // Find out if the message file loaded OK
  // no; best we can do is write a message to the logfile and hope the user
  // sees it, because at this point we don't have the process map loaded to
  // send a message to the Root.
  fprintf(logfp, "Logserver message file %s corrupt or inaccessible\n", smessage.c_str());
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
if (logfp==0) return;                  // May not yet have an output channel
static bool flag0 = false;             // All the processes registered?
if ((flag0==false)&&(pPmap->vPmap.size()>=unsigned(Usize))) {
  if (pPmap->vPmap.size()>unsigned(Usize))Post(113);
  pPmap->Show(logfp);
  flag0 = true;                        // Make sure we just do this once
}
CommonBase::OnIdle();                  // Any base actions?
}

//------------------------------------------------------------------------------

unsigned LogServer::OnLoad(PMsg_p * Z)
// Initialisation:
// Where to go to get the elaboration file
// Where to write the output
{
string Oloc;                           // LogServer output file
Z->Get(1,Oloc);                        // Unload from message
logfp = fopen(Oloc.c_str(),"w");       // Can only not work if the path is wrong
if (logfp==0) {                        // Open an emergency stream
  string Elog = "Emergency_LogServer.log";
  logfp = fopen(Elog.c_str(),"w");
  fprintf(logfp,"\n\n****\nCannot open designated file %s for LogServer log\n"
                "LogServer writing to %s\n\n",
                 Oloc.c_str(),Elog.c_str());
}
InitFile();                            // Stuff the boilerplate into the logfile

string Mloc;                           // Elaboration file
Z->Get(0,Mloc);                        // Unload from message
LoadMessages(Mloc);                    // Load message template
   
return 0;
}

//------------------------------------------------------------------------------

unsigned LogServer::OnLogP(PMsg_p * Z)
// Incoming abbreviated message for the server
// LOG|POST|   -|   - (1:int)message_id,(1:vector<string>)arguments
{
int cnt;                               // Unpack message id
int * pid = Z->Get<int>(1,cnt);
int id = -1;
if (pid!=0) id = *pid;                 // Corrupt message (-1) OK here
vector<string> vstr;                   // Unpack string vector
Z->GetX(1,vstr);
char typ = Msgs[id].typ;               // Unpack message type
string sfull = Assemble(id,vstr);
PMsg_p Z2;                             // Build the full message
Z2.Key(Q::LOG,Q::FULL,Q::N000,Q::N000);
Z2.Put<int>(1,&id);                    // Message id
Z2.Put<char>(2,&typ);                  // Message type
Z2.Put(3,&sfull);                      // Load assembled full string
Z2.Send(pPmap->U.Root);                // Send it back to the console
                                       // And write it into the log file.....
fprintf(logfp,"%s: %3d(%c) %s\n",GetTime(),id,typ,sfull.c_str());
Mcount[typ]++;                         // Increment the type counter

return 0;
}

//==============================================================================

