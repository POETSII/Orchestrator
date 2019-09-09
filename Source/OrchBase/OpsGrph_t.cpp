
#include "OpsGrph_t.h"
#include "OrchBase.h"
#include "xmlTreeDump.h"
#include "FileName.h"
#include "BHelper.h"
#include "Apps_t.h"
#include "PinT_t.h"
#include "PinI_t.h"
#include "MsgT_t.h"

//==============================================================================

OpsGrph_t::OpsGrph_t(OrchBase * _p):par(_p)
{
//fd = _p->fd;                           // Copy detail output file from parent
}

//------------------------------------------------------------------------------

OpsGrph_t::~OpsGrph_t()
{
}

//------------------------------------------------------------------------------

unsigned OpsGrph_t::BuildApp(string fn)
// The input file has passed validation - for what it's worth - so now we
// do it all again and (try to) actually build us a datastructure
{
//##string dfs = GetDetailFile(fn);         // Append to existing....
//##fd = fopen(dfs.c_str(),"a");
//##if (fd==0) fd = stdout;
FILE * fd = par->fd;
                                       // Semi-comprehensible header
fprintf(fd,"***************************************************************\n");
fprintf(fd,"'graph /file' internal build log for description \n%s\n"
           "created on %s at %s\n",
           fn.c_str(),GetDate(),GetTime());
fprintf(fd,"---------------------------------------------------------------\n");
BHelper Xb(fn,par);                    // The thing wot does the heavy lifting
Xb.SetOChan(fd);                       // Point the monkey output to a file
unsigned ecnt = 1;
long t0 = mTimer();
ecnt = Xb.BuildApp();                  // Kick it, Winston
                                       // Semi-comprehensible tail
fprintf(fd,"---------------------------------------------------------------\n");
fprintf(fd,"Application build:\n");
fprintf(fd,"\n%u inconsistencies/errors found in %ld ms at about %s\n",
              ecnt,mTimer(t0),GetTime());
fprintf(fd,"***************************************************************\n");
if (ecnt!=0) Show(fd);
//##if (fd!=stdout) fclose(fd);
return ecnt;
}

//------------------------------------------------------------------------------

void OpsGrph_t::Dump(FILE * fp)
{
fprintf(fp,"OpsGrph_t++++++++++++\n");
if (par==0) fprintf(fp,"OrchBase parent not defined\n");
else fprintf(fp,"Orchbase parent : %s\n",par->FullName().c_str());
fprintf(fp,"OpsGrph_t------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

string OpsGrph_t::GetDetailFile(string fn)
// Find (user) detail log file. fn = full symbolic input file path
{
FileName f(fn);                        // Take input filename to bits
                                       // Build output
string sdf = par->CmPath->pathOutp + "POETSDetail" + f.FNBase() + ".log";
FileName x(sdf);                       // Just checking
if (x.Err()) par->Post(240,fn,sdf);    // Should never happen
else return sdf;                       // What we expect
return string();
}

//------------------------------------------------------------------------------

void OpsGrph_t::GrDela(Cli::Cl_t cl)
// Got a "Delete application" command
// POETS> graph /dela = *|app_name
{
string an = cl.GetP();                 // Not optional application name
if (an.empty()) {                      // Name not supplied?
  par->Post(250,an);
  return;
}
if (an!="*") {                         // Just the one, then. Is it there?
  Apps_t * pA = Apps_t::FindApp(an);
  if (pA==0) par->Post(244,an);        // No ?
  else {                               // Yes?
    Apps_t::Apps_m.erase(an);          // Modify the map
    delete pA;
    Apps_t::ExtCnt -= 1;               // Application count
  }
  return;                              // Deleted one specific application
}
                                       // It was "*": delete the lot
WALKMAP(string,Apps_t *,Apps_t::Apps_m,i) delete (*i).second;
Apps_t::Apps_m.clear();                // Clear the map
Apps_t::ExtCnt = 0;                    // Application count

}

//------------------------------------------------------------------------------

void OpsGrph_t::GrFile(Cli::Cl_t cl)
// Got a "Load application" command
// POETS> graph /file=file_name
// Load the graph, the type tree and cross link them
{
string op = cl.GetO();                 // Optional operator
string fn = cl.GetP();                 // Not optional filename
if (fn.empty()) {                      // Filename not supplied?
  par->Post(250,cl.Cl);
  return;
}

FileName Fn(fn);                       // Is it the application there already?
string apName = Fn.FNBase();
if (Apps_t::Apps_m.find(apName)!=Apps_t::Apps_m.end()) {
  par->Post(242,apName);
  return;
}
                                       // Append pathname?
if (op==string("+")) fn = par->CmPath->pathGrap + fn;
if (!file_readable(fn.c_str())) {      // Can we read it?
  par->Post(112,fn);
  return;
}
                                       // OK, good to go. fn==full i/p filename
par->Post(235,fn);
long t0 = mTimer();                    // ... start wallclock
if (Validate(fn)!=0) {                 // Validation
  par->Post(245,"graph /file","validation",long2str(mTimer(t0)));
  return;
}
if (BuildApp(fn)!=0) {                 // Build skeleton datastructure
  par->Post(245,"graph /file","skeleton build",long2str(mTimer(t0)));
  return;
}
if (TypeLink(fn)!=0) {                 // Type link entire file local contents
  par->Post(245,"graph /file","type link",long2str(mTimer(t0)));
  return;
}
if (XLink(fn)!=0) {                    // Cross link graph instance & type trees
  par->Post(245,"graph /file","cross link",long2str(mTimer(t0)));
  return;
}
Apps_t::ExtCnt++;                      // We got one more application in
par->Post(206,"Complete graph /file",long2str(mTimer(t0)));
}

//------------------------------------------------------------------------------

void OpsGrph_t::GrRety(Cli::Cl_t cl)
// Command to re-type an already loaded graph instance
{
unsigned pcnt = cl.Pa_v.size();        // Parameter count
if (pcnt!=3) {                         // Wrong number?
  par->Post(62,uint2str(3));
  return;
}
string app = cl.GetP();                // Application identifier
FileName Fn(app);                      // All we want is the base
app = Fn.FNBase();                     // It's now in the form we want
Apps_t * pA = Apps_t::FindApp(app);    // Is it there?
if (pA==0) {
  par->Post(244,app);
  return;
}                                      // OK, we got it
string sGr = cl.GetP(1);               // Graph name
GraphI_t * pGI = pA->FindGrph(sGr);    // Find the instance
if (pGI==0) {
  par->Post(243,sGr);
  return;
}
string sp3 = cl.GetP(2);               // Either a new type or "~" (reset)
if (sp3=="~") {                        // Reset?
  if (pGI->tyId2==pGI->tyId) return;   // It already was
  pGI->tyId2 = pGI->tyId;              // It wasn't: reset it
} else if (pGI->tyId2==sp3) return;    // It's already that
                                       // Here iff something has changed
Xunlink(pGI);                          // Remove the x-links
}

//------------------------------------------------------------------------------

void OpsGrph_t::GrShow(Cli::Cl_t cl)
// Monkey command for graph / show=xxxx
// Decode file (if any)
{
string fn = cl.GetP();                 // Get putative filename
if (fn.empty()) {                      // Nothing there?
  Show(par->fd);
  return;
}
                                       // Append long pathname?
if (cl.GetO()==string("+")) fn = par->CmPath->pathOutp + fn;

FileName Fn(fn);                       // Paranoia
if (Fn.Err()) {
  par->Post(241,fn);                   // Invalid filename
  return;
}

FILE * fp = fopen(fn.c_str(),"w");     // Finally - good to go
Show(fp);
fclose(fp);
}

//------------------------------------------------------------------------------

void OpsGrph_t::GrOutp(Cli::Cl_t cl)
// Rather long-winded way of deleting chosen output files. The default access
// mode for everything is "append", but when you're hacking, this gets to be a
// pain in the bum
{
string fn = cl.GetP();                 // Which file to delete?
if (fn.empty()) return;                // Finger trouble?
if (cl.GetO()==string("+")) fn = par->CmPath->pathGrap + fn;
string ftg;                            // (F)ile (T)o (G)o...
                                       // Note this loop will compare the first
                                       // parameter (the filename) against the
                                       // keywords, but that'll fail silently
                                       // so we don't care
WALKVECTOR(Cli::Pa_t,cl.Pa_v,i) {      // Walk the parameter list
  string spa = (*i).Val;               // Pull out parameter value
  if ((spa=="resetdetaillog")||(spa=="reseteverylog")) {
    ftg = GetDetailFile(fn);
    if (remove(ftg.c_str())==0) par->Post(236,fn,ftg);
  }
//  if ((spa=="resettypelinklog")||(spa=="reseteverylog")){
//    ftg = GetDetailFile(fn);
//    if (remove(ftg.c_str())==0) par->Post(237,fn,ftg);
//  }
}

}

//------------------------------------------------------------------------------

string OpsGrph_t::sBank(map<string,unsigned> & rm,unsigned i,string s)
// String bank for Show() routine below
// If the input string is less than i characters, it goes straight out again
// Otherwise it gets aliassed onto a unique unsigned.
// Just helps the pretty-print
{
if (s.size()<i) return s;
rm[s] = UniU(1001);
return "{" + uint2str(rm[s]) + "}";
}

//------------------------------------------------------------------------------

void OpsGrph_t::sBankShow(FILE * fo,map<string,unsigned> & rm)
{
if (rm.empty()) return;
WALKMAP(string,unsigned,rm,i)
  fprintf(fo,"{%u} == %s\n",(*i).second,(*i).first.c_str());
rm.clear();
fprintf(fo,"\n");
}

//------------------------------------------------------------------------------

void OpsGrph_t::Show(FILE * fo)
// Pretty-print for the application database. It's just a thundering great
// printf statement
{
string s1,s2,s3,s4,s5;
map<string,unsigned> sMap;
fprintf(fo,"\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
fprintf(fo,"\nLoaded application status at %s on %s\n\n",GetTime(),GetDate());
fprintf(fo,"Table of LOADED APPLICATIONS []\n"
           "For each loaded application\n"
           "....Table of INSTANCE GRAPHS []\n"
           "....Table of TYPE TREES []\n"
           "For each loaded application\n"
           "....For each instance graph\n"
           "........BINARY FILE needing dedicated browser []\n"
           "....For each type tree\n"
           "........Table of MESSAGE TYPES []\n"
           "........Table of DEVICE TYPES []\n");
fprintf(fo,"\nApplications loaded: %u external, %u proof-of-life\n",
           Apps_t::ExtCnt,Apps_t::PoLCnt);

fprintf(fo,"\n==============================================================\n\n");

// File/application precis:
fprintf(fo,"Table of LOADED APPLICATIONS []\n");
fprintf(fo," Application Application       Timestamp        Graphs Type\n"
           "    File        Name                                   trees\n"
           "+-----------+-----------+----------------------+------+------+\n");
WALKMAP(string,Apps_t *,Apps_t::Apps_m,i) {
  Apps_t * a = (*i).second;
  s1 = sBank(sMap,11,a->filename);
  s2 = sBank(sMap,11,a->Name());
  fprintf(fo,"|%11s|%11s|%11s %10s|%6u|%6u|\n",
          s1.empty()?"*":s1.c_str(),s2.c_str(),
          a->sTime.c_str(),a->sDate.c_str(),
          a->GraphI_v.size(),a->GraphT_v.size());
}
fprintf(fo,"+-----------+-----------+----------------------+------+------+\n\n");
sBankShow(fo,sMap);

fprintf(fo,"\n==============================================================\n\n");

// For each application
WALKMAP(string,Apps_t *,Apps_t::Apps_m,AP) {
  Apps_t * a = (*AP).second;
  // Walk the Instance graphs
  fprintf(fo,"For each loaded application (%s)\n"
             "....Table of INSTANCE GRAPHS []\n",
             a->Name().c_str());
  fprintf(fo,"File %s \napplication %s\n"
             " Graphs (%2u)    Type     Devices  Edges        Type target\n"
             "+-----------+-----------+-------+-------+------------------------+\n",
             a->filename.empty()?"*":a->filename.c_str(),
             a->Name().c_str(),a->GraphI_v.size());
  WALKVECTOR(GraphI_t *,a->GraphI_v,GI) {
    s1 = sBank(sMap,11,(*GI)->Name());
    s2 = sBank(sMap,11,(*GI)->tyId);
    s3 = sBank(sMap,11,(*GI)->pT->Name());
    s4 = sBank(sMap,11,(*GI)->pT->par->Name());
    fprintf(fo,"|%11s|%11s|%7u|%7u|%11s::%-11s|\n",
           s1.c_str(),s2.c_str(),
           (*GI)->G.SizeNodes(),(*GI)->G.SizeArcs(),
           ((*GI)->pT==0)?"---":s3.c_str(),
           ((*GI)->pT==0)?"---":s4.c_str());
  }
  fprintf(fo,"+-----------+-----------+-------+-------+------------------------+\n\n");
  sBankShow(fo,sMap);
  // Walk the Type trees
  fprintf(fo,"For each loaded application (%s)\n"
             "....Table of TYPE TREES []\n",
             a->Name().c_str());
  fprintf(fo,"  Type       Device Message\n"
             "  trees(%u)   types   types\n"
             "+-----------+-------+-------+\n",
              a->GraphT_v.size());
  WALKVECTOR(GraphT_t *,a->GraphT_v,GT) {
    s1 = sBank(sMap,11,(*GT)->Name());
    fprintf(fo,"|%11s|%7u|%7u|\n",
               s1.c_str(),(*GT)->DevT_v.size(),(*GT)->MsgT_v.size());
  }
  fprintf(fo,"+-----------+-------+-------+\n\n");
  sBankShow(fo,sMap);
  if (AP!=Apps_t::Apps_m.end())
    fprintf(fo,"\n= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = \n\n");
}

fprintf(fo,"\n==============================================================\n\n");

WALKMAP(string,Apps_t *,Apps_t::Apps_m,AP){
  Apps_t * a = (*AP).second;
  fprintf(fo,"For each loaded application (%s)\n"
             "....For each instance graph\n"
             "........BINARY FILE needing dedicated browser []\n",
             a->Name().c_str());
  WALKVECTOR(GraphI_t *,a->GraphI_v,I) {
    fprintf(fo,"Writing binary file for %s...\n",(*I)->Name().c_str());
    WriteBinaryGraph(*I);
  }

  fprintf(fo,"\n= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = \n\n");

  fprintf(fo,"For each loaded application (%s)\n",a->Name().c_str());
  WALKVECTOR(GraphT_t *,a->GraphT_v,T) {
  fprintf(fo,"....For each type tree (%s)\n"
             "........Table of MESSAGE TYPES []\n",
             (*T)->Name().c_str());
    fprintf(fo,"   Message   References   Props..CFrag\n"
               "+-----------+-----------+-----------+\n");
    WALKVECTOR(MsgT_t *,(*T)->MsgT_v,M) {
      s1 = sBank(sMap,11,(*M)->Name());
      fprintf(fo,"|%11s|%11u|",s1.c_str(),(*M)->Ref());
      if ((*M)->pPropsD==0) fprintf(fo,"No message | !\n",(*M)->Name().c_str());
      else if ((*M)->pPropsD->c_src.empty()) fprintf(fo,"0 chars| !\n");
      else fprintf(fo,"%5u chars|\n",(*M)->pPropsD->c_src.size());
    }
  }
  fprintf(fo,"+-----------+-----------+-----------+\n");
  sBankShow(fo,sMap);
  fprintf(fo,"For each loaded application (%s)\n",a->Name().c_str());
  WALKVECTOR(GraphT_t *,a->GraphT_v,T) {
  fprintf(fo,"....For each type tree (%s)\n"
             "........Table of DEVICE TYPES []\n",
             (*T)->Name().c_str());
    fprintf(fo,"   Device       Inpin                    Outpin\n"
               "    type         type   (message)         type (message)\n"
               "+-----------+------------------------+------------------------+\n");
    WALKVECTOR(DevT_t *,(*T)->DevT_v,D) {
      DevT_t * pDt = *D;
      s1 = sBank(sMap,11,pDt->Name());
      fprintf(fo,"|%11s|",s1.c_str());
      for (unsigned k=0;(k<pDt->PinTI_v.size())||(k<pDt->PinTO_v.size());k++) {
        if (k!=0) fprintf(fo,"|           |");
        if (!(pDt->PinTI_v.empty())) {
          if (k<pDt->PinTI_v.size()) {
            PinT_t * p = pDt->PinTI_v[k];
            s2 = sBank(sMap,11,p->Name());
            s3 = sBank(sMap,11,p->tyIdM);
            fprintf(fo,"%11s(%11s)|",s2.c_str(),s3.c_str());
          }
          else fprintf(fo,"                        |");
        } else fprintf(fo,"                        |");
        if (!(pDt->PinTO_v.empty())) {
          if (k<pDt->PinTO_v.size()) {
            PinT_t * p = pDt->PinTO_v[k];
            s4 = sBank(sMap,11,p->Name());
            s5 = sBank(sMap,11,p->tyIdM);
            fprintf(fo,"%11s(%11s)|\n",s4.c_str(),s5.c_str());
          }
          else fprintf(fo,"                        |\n");
        } else fprintf(fo,"                        |\n");
      }
      if ((pDt->PinTI_v.empty())&&(pDt->PinTO_v.empty()))
        fprintf(fo,"         |           |\n");
    }
  }
  fprintf(fo,"+-----------+------------------------+------------------------+\n");
  sBankShow(fo,sMap);
  if (AP!=Apps_t::Apps_m.end())
    fprintf(fo,"\n= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = \n\n");
}

fprintf(fo,"\n==============================================================\n\n");
fprintf(fo,"-------------------------------------------------------------\n");
fflush(fo);
}

//------------------------------------------------------------------------------

unsigned OpsGrph_t::TypeLink(string fn)
// fn: full symbolic pathname, which reduces to an unambiguous application name
// Here we build the internal type links inside ALL the type trees it contains
// between the pin type definitions (PinT_t) and the message vector (MsgT_t)
{
//##string sdf = GetDetailFile(fn);        // Append to existing....
//##fd = fopen(sdf.c_str(),"a");
//##if (fd==0) fd = stdout;
FILE * fd = par->fd;
                                       // Semi-comprehensible header
fprintf(fd,"***************************************************************\n");
fprintf(fd,"'graph /file' type tree build and typelink log "
           "for description \n%s\ncreated on %s at %s\n",
           fn.c_str(),GetDate(),GetTime());
fprintf(fd,"---------------------------------------------------------------\n");
unsigned ecnt = 0;                     // So far, so good
FileName Fn(fn);                       // Extract the application name
string apName = Fn.FNBase();           // (We know it's OK by now)
Apps_t * pA = Apps_t::FindApp(apName); // Find the application
long t0 = mTimer();
if (pA==0) {                           // Not there?
  ecnt++;
  fprintf(fd,"Application %s cannot be found\n",apName.c_str());
} else {
  WALKVECTOR(GraphT_t *,pA->GraphT_v,i) {          // Walk the type trees
    ecnt = 0;                                      // No errors in type tree yet
    WALKVECTOR(DevT_t *,(*i)->DevT_v,j) {          // Walk the device types
      for(int n=0;n<2;n++) {                       // Input and output pins
        vector<PinT_t *> vP = (n==0) ? (*j)->PinTI_v : (*j)->PinTO_v;
        WALKVECTOR(PinT_t *,vP,k) {                // Walk the pins
          MsgT_t * pM = (*i)->FindMsg((*k)->tyIdM);// Find message for pin
          if (pM==0) {                             // Not there?
            fprintf(fd,"Application ||%s||\n...graph type ||%s||\n"
                       "...device type ||%s||\n...pin ||%s||\n"
                       "...message ||%s|| cannot be found\n",
                       apName.c_str(),(*i)->Name().c_str(),(*j)->Name().c_str(),
                       (*k)->Name().c_str(),(*k)->tyIdM.c_str());
            ecnt++;
          } else {
            (*k)->pMsg = pM;           // Build type link
            pM->Ref(1);                // Increment message reference count
          } // if (pM==0)
        } // WALKVECTOR(PinT_t
      } // for(int n
    } // WALKVECTOR(DevT_t
    WALKVECTOR(MsgT_t *,(*i)->MsgT_v,j) {          // Unreferenced messages?
      if ((*j)->Ref()==0)
        fprintf(fd,"Application ||%s||\n...graph type ||%s||\n"
                   "Message type ||%s|| defined but not referenced\n",
                   apName.c_str(),(*i)->Name().c_str(),(*j)->Name().c_str());
      if ((*j)->pPropsD==0)
        fprintf(fd,"Application ||%s||\n...graph type ||%s||\n"
                   "Message type ||%s|| defined but no message body supplied\n",
                   apName.c_str(),(*i)->Name().c_str(),(*j)->Name().c_str());
    } // WALKVECTOR(MsgT_t
    if (ecnt!=0) {                     // Errors - tear Tlink out
      fprintf(fd,"%u errors - tear down partial typelink\n",ecnt);
      WALKVECTOR(DevT_t *,(*i)->DevT_v,j) {        // Walk devices
        for(int n=0;n<2;n++) {                     // Input and output pins
          vector<PinT_t *> vP = (n==0) ? (*j)->PinTI_v : (*j)->PinTO_v;
          WALKVECTOR(PinT_t *,vP,k)(*k)->pMsg = 0; // Walk the pins
        } // for(int n
        WALKVECTOR(MsgT_t *,(*i)->MsgT_v,k) (*k)->ClrR(); // Walk message types
      } // WALKVECTOR(DevT_t
    } else (*i)->TyFlag = true;        // No errors - assert flag
  } // WALKVECTOR(GraphT_t
} // else
                                       // Semi-comprehensible tail
fprintf(fd,"---------------------------------------------------------------\n");
fprintf(fd,"Type link:\n");
fprintf(fd,"\n%u inconsistencies/errors found in %ld ms at about %s\n",
           ecnt,mTimer(t0),GetTime());
fprintf(fd,"***************************************************************\n");
if (ecnt!=0) Show(fd);
//##if (fd!=stdout) fclose(fd);
return ecnt;
}

//------------------------------------------------------------------------------

unsigned OpsGrph_t::Validate(string fn)
// fn: full symbolic pathname
// Validation of XML input file. Except it isn't, this incantation is just a
// syntax check: it's a placeholder for jam tomorrow. What it does achieve in
// its current form is to detect sytax errors (obviously) so the system has a
// chance to bail before trying to build anything, which saves a complicated
// partial tear-down if errors are discovered. Having said that, we still allow
// the *possibility* of errors during build...
{
//##string sdf = GetDetailFile(fn);        // Append to existing....
//##fd = fopen(sdf.c_str(),"a");
//##if (fd==0) fd = stdout;
FILE * fd = par->fd;
                                       // Semi-comprehensible header
fprintf(fd,"***************************************************************\n");
fprintf(fd,"'graph /file' validation log for description \n%s\n"
           "created on %s at %s\n",
           fn.c_str(),GetDate(),GetTime());
fprintf(fd,"---------------------------------------------------------------\n");
xmlTreeDump Xp;                        // Treedumper::parser
Xp.SetOChan(fd);                       // Point the monkey output to a file
unsigned ecnt = 1;
long t0 = mTimer();
FILE * fi = fopen(fn.c_str(),"r");
if (fi!=0) {
  ecnt = Xp.Transform(fi);             // Kick it, Winston
  fclose(fi);
}
else {
  fprintf(fd,"Cannot open %s\n",fn.c_str());
  par->Post(238,fn);
}
                                       // Semi-comprehensible tail
fprintf(fd,"---------------------------------------------------------------\n");
fprintf(fd,"\n%u inconsistencies/errors found in %ld ms at about %s\n",
              ecnt,mTimer(t0),GetTime());
fprintf(fd,"***************************************************************\n");
if (ecnt!=0) Show(fd);
//##if (fd!=stdout) fclose(fd);
return ecnt;
}

//------------------------------------------------------------------------------

void OpsGrph_t::WriteBinaryGraph(GraphI_t * pI)
{
string fn = pI->Name() + "BINARY.log";
FILE * fx = fopen(fn.c_str(),"w");

pI->G.DumpChan(fx);
pI->G.Dump();

fclose(fx);
}

//------------------------------------------------------------------------------

unsigned OpsGrph_t::XLink(string fn)
// The application graphs and type trees are in place; here we cross-link *all*
// the graph instances.
{
//##string dfs = GetDetailFile(fn);         // Append to existing....
//##fd = fopen(dfs.c_str(),"a");
//##if (fd==0) fd = stdout;
FILE * fd = par->fd;
                                       // Semi-comprehensible header
fprintf(fd,"***************************************************************\n");
fprintf(fd,"'graph /file' graph/type tree crosslink log for description \n%s\n"
           "created on %s at %s\n",
           fn.c_str(),GetDate(),GetTime());
fprintf(fd,"---------------------------------------------------------------\n");

unsigned ecnt = 1;
long t0 = mTimer();
ecnt = XLink2(fn);                     // Kick it, Winston
                                       // Semi-comprehensible tail
fprintf(fd,"---------------------------------------------------------------\n");
fprintf(fd,"Cross-link:\n");
fprintf(fd,"\n%u inconsistencies/errors found in %ld ms at about %s\n",
              ecnt,mTimer(t0),GetTime());
fprintf(fd,"***************************************************************\n");
Show(fd);
//##if (fd!=stdout) fclose(fd);
return ecnt;
}

//------------------------------------------------------------------------------

unsigned OpsGrph_t::XLink2(string fn)
// Cross-link ALL the graph instances in the application
{
FileName Fn(fn);                       // Derive the application name
string apName = Fn.FNBase();
                                       // Is it there?
if (Apps_t::Apps_m.find(apName)==Apps_t::Apps_m.end()) {
  par->Post(925,apName);
  return 1;
}
unsigned ecnt = 0;                     // So far, so good
Apps_t * pA = Apps_t::Apps_m[apName];  // Go get the application itself
WALKVECTOR(GraphI_t *,pA->GraphI_v,i) {
  GraphI_t * pGI = *i;
  GraphT_t * pGT = pA->FindTree(pGI->tyId2);
  if (pGT==0) {
    fprintf(fd,"Application ||%s||\n...graph instance ||%s||\n"
               "...can't find type definition ||%s||\n",
               pA->Name().c_str(),pGI->Name().c_str(),pGI->tyId2.c_str());
    ecnt++;
    continue;
  }
  bool Xgood = true;                   // In case it goes weird on us later
  pGI->pT = pGT;                       // Cross-link graph instance -> tree
  pGT->GraphI_v.push_back(pGI);        //      ....  tree -> graph instance
                                       // Walk the devices
  WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,P_link *,
                    unsigned,PinI_t *,pGI->G,i) {
    DevI_t * pDi = pGI->G.NodeData(i); // Get the device
    string sTy = pDi->tyId;            // Device type name
    DevT_t * pDt = pGT->FindDev(sTy);  // Device type class
    pDi->pT = pDt;                     // Cross-link device instance -> tree
    if (pDt==0) {                      // Can't find device type definition
      fprintf(fd,"Application ||%s||\n...graph instance ||%s||\n"
                 "...device instance ||%s||\n"
                 "...can't find type definition ||%s||\n",
                 pA->Name().c_str(),pGI->Name().c_str(),
                 pDi->Name().c_str(),sTy.c_str());
      ecnt++;                          // Overall error count
      Xgood = false;                   // Graph gone bad
      continue;                        // Skip to next graph
    }
                                       // Walk the input pins
    WALKPDIGRAPHINPINS(unsigned,DevI_t *,unsigned,P_link *,
                    unsigned,PinI_t *,pGI->G,pGI->G.NodeKey(i),j) {
      PinI_t * pPi = pGI->G.PinData(j);// Get the pin
      if (pPi->pT!=0) continue;        // Already linked - it's a virtual pin
      string sTy = pPi->tyId;          // Pin type name
      PinT_t * pPt = pGT->FindPin(pDt,sTy);   // Pin type class
      pPi->pT = pPt;                   // Cross-link pin instance -> tree
      if (pPt==0) {                    // Can't find pin type definition
      fprintf(fd,"Application ||%s||\n...graph instance ||%s||\n"
                 "...device ||%s||\n...pin name ||%s||\n"
                 "...can't find type definition ||%s||\n",
                 pA->Name().c_str(),pGI->Name().c_str(),
                 pDi->Name().c_str(),pPi->Name().c_str(),sTy.c_str());
        ecnt++;                        // Overall error count
        Xgood = false;                 // Graph gone bad
        continue;                      // Skip to next graph
      } // if (pPt
    } // WALKPDIGRAPHINPINS
                                      // Walk the output pins
    WALKPDIGRAPHOUTPINS(unsigned,DevI_t *,unsigned,P_link *,
                    unsigned,PinI_t *,pGI->G,pGI->G.NodeKey(i),j) {
      PinI_t * pPi = pGI->G.PinData(j);// Get the pin
      if (pPi->pT!=0) continue;        // Already linked - it's a virtual pin
      string sTy = pPi->tyId;          // Pin type name
      PinT_t * pPt = pGT->FindPin(pDt,sTy);   // Pin type class
      pPi->pT = pPt;                   // Cross-link pin instance -> tree
      if (pPt==0) {                    // Can't find pin type definition
      fprintf(fd,"Application ||%s||\n...graph instance ||%s||\n"
                 "...device ||%s||\n...pin name ||%s||\n"
                 "...can't find type definition ||%s||\n",
                 pA->Name().c_str(),pGI->Name().c_str(),
                 pDi->Name().c_str(),pPi->Name().c_str(),sTy.c_str());
        ecnt++;                        // Overall error count
        Xgood = false;                 // Graph gone bad
        continue;                      // Skip to next graph
      } // if (pPt
    } // WALKPDIGRAPHOUTPINS
  } // WALKPDIGRAPHNODES
  if (!Xgood) Xunlink(pGI);            // Problem - undo all the good work
} // WALKVECTOR(GraphI_t

return ecnt;                           // Return error count

}

//------------------------------------------------------------------------------

void OpsGrph_t::Xunlink(GraphI_t * pGI)
// Can be here for several reasons:
// (1) A problem occured in xlinking the graph pGI, so we have to tear down any
// cross-links that were successful
// (2) The monkey has changed the type of the graph instance, so the xlinks
// are no longer valid
// (3) The monkey wants to remove the application
{
fprintf(fd,"Tearing out the hardware cross-links in graph instance %s\n...",
           pGI->Name().c_str());

GraphT_t * pGT = pGI->pT;              // Graph level: tree -> instance link
if (pGT!=0) WALKVECTOR(GraphI_t *,pGT->GraphI_v,i) if((*i)==pGI) {
    pGT->GraphI_v.erase(i);            // Safe: we exit the loop immediately
    break;
}
pGI->pT = 0;                           // Graph level: instance -> tree link
                                       // Walk the devices
WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,P_link *,
                  unsigned,PinI_t *,pGI->G,i) {
  pGI->G.NodeData(i)->pT = 0;
                                       // Walk the input pins
  WALKPDIGRAPHINPINS(unsigned,DevI_t *,unsigned,P_link *,
                     unsigned,PinI_t *,pGI->G,pGI->G.NodeKey(i),j)
    pGI->G.PinData(j)->pT = 0;
                                       // Walk the output pins
  WALKPDIGRAPHOUTPINS(unsigned,DevI_t *,unsigned,P_link *,
                      unsigned,PinI_t *,pGI->G,pGI->G.NodeKey(i),j)
    pGI->G.PinData(j)->pT = 0;

} // WALKPDIGRAPHNODES
}

//------------------------------------------------------------------------------

  /*
FILE * OpsGrph_t::GetOutFile(string fn)
// Find (user) detailed output file and open it for append
{
FileName f(fn);                        // Take the input filename to bits
f.FNBase("POETSValidation"+f.FNBase());// Append Orchestrator prefix
f.FNExtn("log");                       // Change extension
if (f.Err()) return stdout;
FILE * ft = fopen(f.FNComplete().c_str(),"a"); // Open the damn thing
if (ft==0) return stdout;              // Cockup?
return ft;                             // What we expect
}
       */
//------------------------------------------------------------------------------
                       /*
string OpsGrph_t::GetTypeLinkFile(string fn)
// Find (user) detailed typelink log file. The reason for this and subsequent
// (very similar) routines is that they allow the structure of the various
// fine detail output files to be changed arbitrarily and independently in the
// future. Should you want to, in some bizarre set of circumstances.
{
FileName f(fn);                        // Take the input filename to bits
f.FNBase("POETSTypeLink"+f.FNBase());  // Append Orchestrator prefix
f.FNExtn("log");                       // Change extension
if (f.Err()) return string();          // Cockup?
return f.FNComplete();                 // What we expect
}
                         */
//------------------------------------------------------------------------------


//==============================================================================

