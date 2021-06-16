//------------------------------------------------------------------------------

#include "Apps_t.h"
#include "P_super.h"
#include "PinT_t.h"
#include "Pglobals.h"
#include "DS_XML.h"
#include "MsgT_t.h"
#include "GraphT_t.h"
#include "CFrag.h"

//==============================================================================

map<string,Apps_t *> Apps_t::Apps_m;   // Skyhook: Application map
unsigned Apps_t::PoLCnt = 0;           // Of which PoLCnt are proof-of-life
unsigned Apps_t::ExtCnt = 0;           // And the others are externally loaded

//==============================================================================

Apps_t::Apps_t(OrchBase * _p,string _s):par(_p)
{
Name(_s);                              // Save name
Npar(_p);                              // Namebase parent
Apps_m[_s] = this;                     // Store in anchor structure
pPoL       = 0;                        // Proof of life (PoL) application?
ExtCnt++;
sTime      = string(GetTime());        // Housekeeping
sDate      = string(GetDate());
}

//------------------------------------------------------------------------------

Apps_t::~Apps_t()
{
if (pPoL==0) ExtCnt--;
else PoLCnt--;
par->Post(214,Name());
WALKVECTOR(GraphI_t *,GraphI_v,i) delete *i;
WALKVECTOR(GraphT_t *,GraphT_v,i) delete *i;
}

//------------------------------------------------------------------------------

void Apps_t::DelAll()
// Without fear or favour, kill everything
{
WALKMAP(string,Apps_t *,Apps_t::Apps_m,i) delete (*i).second;
Apps_t::Apps_m.clear();
Apps_t::PoLCnt = 0;
Apps_t::ExtCnt = 0;
}

//------------------------------------------------------------------------------

void Apps_t::DelApp(string s)
// Delete an App from the map. We assume all the error reporting has been done:
// We check to see if it exists, if it doesn't, we bail silently
{
Apps_t * pA = Apps_t::FindApp(s);
if (pA==0) return;
delete pA;
Apps_t::Apps_m.erase(s);
}

//------------------------------------------------------------------------------

void Apps_t::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n\n%sApps_t++++++++++++++++++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sNameBase                  %s\n",os,FullName().c_str());
fprintf(fp,"%sMe,Parent                 %#018lx,%#018lx\n",
            os,(uint64_t)this,(uint64_t)par);
if (par!=0) fprintf(fp,"%s...(0x%#018lx) %s\n",
            os,(uint64_t)par,par->FullName().c_str());
fprintf(fp,"%s%lu instantiation graphs:\n",os,GraphI_v.size());
WALKVECTOR(GraphI_t *,GraphI_v,i) (*i)->Dump(off+2,fp);
fprintf(fp,"%s%lu type declare trees:\n",os,GraphT_v.size());
WALKVECTOR(GraphT_t *,GraphT_v,i) (*i)->Dump(off+2,fp);
fprintf(fp,"%sSource file  %s\n",os,filename.c_str());
fprintf(fp,"%s PoL applications  :      %u\n",os,PoLCnt);
fprintf(fp,"%s User applications :      %u\n",os,ExtCnt);
fprintf(fp,"%sTimestamps         :      %s %s\n",os,sTime.c_str(),sDate.c_str());
NameBase::Dump(off+2,fp);
DefRef::Dump(off+2,fp);
fprintf(fp,"\n%sApps_t------------------------------------------------\n\n",os);
fflush(fp);
}

//------------------------------------------------------------------------------

void Apps_t::DumpAll(FILE * fp)
// The static version that dumps *everything* in the application database
{
fprintf(fp,"%s\n",Q::eline.c_str());
fprintf(fp,"POETS application database dump\n");
fprintf(fp,"Statics:\n Proof-of-life (PoLCnt) %u\n External (ExtCnt) %u\n",
            PoLCnt,ExtCnt);
WALKMAP(string,Apps_t *,Apps_t::Apps_m,i) ((*i).second)->Dump(0,fp);
fprintf(fp,"%s\n",Q::eline.c_str());
}

//------------------------------------------------------------------------------

Apps_t * Apps_t::FindApp(string name)
// Search the map by application name.....
{
if (Apps_m.find(name)!=Apps_m.end()) return Apps_m[name];
return 0;
}

//------------------------------------------------------------------------------

Apps_t * Apps_t::FindFile(string name)
// Search the map by definition file name.....
{
WALKMAP(string,Apps_t *,Apps_t::Apps_m,i)
  if ((*i).second->filename==name) return (*i).second;
return 0;
}

//------------------------------------------------------------------------------

GraphI_t * Apps_t::FindGrph(string s)
// Locate the graph instance object held in here with the name "s" (if any)
{
WALKVECTOR(GraphI_t *,GraphI_v,i) if ((*i)->Name()==s) return *i;
return 0;
}

//------------------------------------------------------------------------------

GraphT_t * Apps_t::FindTree(string s)
// Locate the graph type object held in here with the name "s" (if any)
{
WALKVECTOR(GraphT_t *,GraphT_v,i) if ((*i)->Name()==s) return *i;
return 0;
}

//------------------------------------------------------------------------------

void Apps_t::Show(FILE * fo)
// Pretty-print for the application database. It's just a thundering great
// printf statement
{
if (Apps_t::Apps_m.empty()) {
  fprintf(fo,"Application database contains no entries\n\n");
  return;
}
string s1,s2,s3,s4,s5;
map<string,unsigned> sMap;
fprintf(fo,"\n%s\n",Q::pline.c_str());
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

fprintf(fo,"\n%s\n\n",Q::eline.c_str());

// File/application precis:
fprintf(fo,"Table of LOADED APPLICATIONS []\n\n"
           "  Application file   Application name     "
           "   Timestamp         Graphs   Type trees \n"
           "+------------------+------------------+---"
           "-------------------+--------+------------+\n");
WALKMAP(string,Apps_t *,Apps_t::Apps_m,i) {
  Apps_t * a = (*i).second;
  s1 = sBank(sMap,18,a->filename);
  s2 = sBank(sMap,18,a->Name());
  fprintf(fo,"|%18s|%18s|%11s %10s|%8lu|%12lu|\n",
          s1.empty()?"*":s1.c_str(),s2.c_str(),
          a->sTime.c_str(),a->sDate.c_str(),
          a->GraphI_v.size(),a->GraphT_v.size());
}
fprintf(fo,"+------------------+------------------+---"
           "-------------------+--------+------------+\n\n");
sBankShow(fo,sMap);

fprintf(fo,"\n%s\n\n",Q::eline.c_str());

// For each application
WALKMAP(string,Apps_t *,Apps_t::Apps_m,AP) {
  Apps_t * a = (*AP).second;
  // Walk the Instance graphs
  fprintf(fo,"For each loaded application (%s)\n\n"
             "....Table of INSTANCE GRAPHS []\n\n",
             a->Name().c_str());
  fprintf(fo,"File %s \nApplication %s\n\n"
             " Graphs (%2lu)    Type     Devices  Edges        Type target\n"
             "+-----------+-----------+-------+-------+"
             "------------------------+\n",
             a->filename.empty()?"*":a->filename.c_str(),
             a->Name().c_str(),a->GraphI_v.size());
  WALKVECTOR(GraphI_t *,a->GraphI_v,GI) {
    s1 = sBank(sMap,11,(*GI)->Name());
    s2 = sBank(sMap,11,(*GI)->tyId);
    if ((*GI)->pT!=0) {
      s3 = sBank(sMap,11,(*GI)->pT->Name());
      s4 = sBank(sMap,11,(*GI)->pT->par->Name());
    } else s3 = s4 = "---";
    fprintf(fo,"|%11s|%11s|%7u|%7u|%11s::%-11s|\n",
           s1.c_str(),s2.c_str(),
           (*GI)->G.SizeNodes(),(*GI)->G.SizeArcs(),
           s3.c_str(),s4.c_str());
  }
  fprintf(fo,"+-----------+-----------+-------+-------+"
             "------------------------+\n\n");
  sBankShow(fo,sMap);
  // Walk the Type trees
  fprintf(fo,"For each loaded application (%s)\n"
             "....Table of TYPE TREES []\n\n",
             a->Name().c_str());
  fprintf(fo,"  Type       Device  Message\n"
             "  trees(%lu)   types   types\n"
             "+-----------+-------+-------+\n",
              a->GraphT_v.size());
  WALKVECTOR(GraphT_t *,a->GraphT_v,GT) {
    s1 = sBank(sMap,11,(*GT)->Name());
    fprintf(fo,"|%11s|%7lu|%7lu|\n",
               s1.c_str(),(*GT)->DevT_v.size(),(*GT)->MsgT_v.size());
  }
  fprintf(fo,"+-----------+-------+-------+\n\n");
  sBankShow(fo,sMap);
  if (AP!=Apps_t::Apps_m.end())
    fprintf(fo,"\n%s\n\n",Q::dline.c_str());
}

fprintf(fo,"\n%s\n\n",Q::eline.c_str());

WALKMAP(string,Apps_t *,Apps_t::Apps_m,AP){
  Apps_t * a = (*AP).second;
  fprintf(fo,"For each loaded application (%s)\n\n"
             "....For each instance graph\n"
             "........BINARY FILE needing dedicated browser []\n",
             a->Name().c_str());
  WALKVECTOR(GraphI_t *,a->GraphI_v,I) {
    fprintf(fo,"Writing binary file for %s...\n",(*I)->Name().c_str());
    WriteBinaryGraph(*I);
  }

  fprintf(fo,"\n%s\n\n",Q::dline.c_str());

  fprintf(fo,"For each loaded application - message types (%s)\n\n",
             a->Name().c_str());
  WALKVECTOR(GraphT_t *,a->GraphT_v,T) {
    fprintf(fo,"....For each type tree (%s)\n"
               "........Table of MESSAGE TYPES []\n\n",
             (*T)->Name().c_str());
    fprintf(fo,"   Message   References  Props..CFrag\n"
               "+-----------+-----------+-----------+\n");
    WALKVECTOR(MsgT_t *,(*T)->MsgT_v,M) {
      s1 = sBank(sMap,11,(*M)->Name());
      fprintf(fo,"|%11s|%11u|",s1.c_str(),(*M)->Ref());
      if ((*M)->pPropsD==0) fprintf(fo,"No message | !\n");
      else if ((*M)->pPropsD->C_src().empty()) fprintf(fo,"0 chars| !\n");
      else fprintf(fo,"%5lu chars|\n",(*M)->pPropsD->C_src().size());
    }
  }
  if (a->GraphT_v.empty()) fprintf(fo,"None present\n");
  else fprintf(fo,"+-----------+-----------+-----------+\n\n");
  sBankShow(fo,sMap);
  fprintf(fo,"For each loaded application - device types (%s)\n\n",
             a->Name().c_str());
  WALKVECTOR(GraphT_t *,a->GraphT_v,T) {
    fprintf(fo,"....For each type tree (%s)\n"
               "........Table of DEVICE TYPES []\n\n",
             (*T)->Name().c_str());
    fprintf(fo,"   Device type    Inpin type (message)   "
               "  Outpin type (message)   \n"
               "+--------------+------------------------+"
               "------------------------+\n");
    WALKVECTOR(DevT_t *,(*T)->DevT_v,D) {
      DevT_t * pDt = *D;
      s1 = sBank(sMap,14,pDt->Name());
      fprintf(fo,"|%14s|",s1.c_str());
      for (unsigned k=0;(k<pDt->PinTI_v.size())||(k<pDt->PinTO_v.size());k++) {
        if (k!=0) fprintf(fo,"|              |");
        if (!(pDt->PinTI_v.empty())) {
          if (k<pDt->PinTI_v.size()) {
            PinT_t * p = pDt->PinTI_v[k];
            s2 = sBank(sMap,11,p->Name());
            s3 = sBank(sMap,11,p->tyId);
            fprintf(fo,"%11s(%11s)|",s2.c_str(),s3.c_str());
          }
          else fprintf(fo,"                        |");
        } else fprintf(fo,"                        |");
        if (!(pDt->PinTO_v.empty())) {
          if (k<pDt->PinTO_v.size()) {
            PinT_t * p = pDt->PinTO_v[k];
            s4 = sBank(sMap,11,p->Name());
            s5 = sBank(sMap,11,p->tyId);
            fprintf(fo,"%11s(%11s)|\n",s4.c_str(),s5.c_str());
          }
          else fprintf(fo,"                        |\n");
        } else fprintf(fo,"                        |\n");
      }
      if ((pDt->PinTI_v.empty())&&(pDt->PinTO_v.empty()))
        fprintf(fo,"                        |                        |\n");
    }
  }
  if (a->GraphT_v.empty()) fprintf(fo,"None present\n");
  else fprintf(fo,"+--------------+------------------------+"
                  "------------------------+\n");
  sBankShow(fo,sMap);
  if (AP!=Apps_t::Apps_m.end()) fprintf(fo,"\n%s\n\n",Q::dline.c_str());
}
fprintf(fo,"\n%s\n\n",Q::eline.c_str());
fflush(fo);
}

//------------------------------------------------------------------------------

void Apps_t::WriteBinaryGraph(GraphI_t * pI)
{
string fn = pI->Name() + "BINARY.log";
FILE * fx = fopen(fn.c_str(),"w");

pI->G.DumpChan(fx);
pI->G.Dump();

fclose(fx);
}

//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

Apps_t::PoL_t::PoL_t(): IsPoL(false){}

//------------------------------------------------------------------------------

void Apps_t::PoL_t::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sApps_t::PoL_t + + + + + + + + + + + + + + + + + + + + + \n",os);
fprintf(fp,"%sIsPoL      : %s\n",os,IsPoL ? "TRUE" : "FALSE");
fprintf(fp,"%sPoL type   : %s\n",os,type.c_str());
fprintf(fp,"%sParameters :\n",os);
WALKVECTOR(string,params,i) fprintf(fp,"%s  %s\n",os,(*i).c_str());
fprintf(fp,"%sApps_t::PoL_t - - - - - - - - - - - - - - - - - - - - - \n\n",os);
fflush(fp);
}

//==============================================================================
