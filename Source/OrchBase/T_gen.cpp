//------------------------------------------------------------------------------

#include <stdio.h>
#include <sstream>
#include "T_gen.h"
#include "rand.h"
#include "filename.h"
#include "dfprintf.h"
#include "header.h"
#include "P_super.h"
#include "P_devtyp.h"

//==============================================================================

T_gen::T_gen(OrchBase * _p):par(_p)
{
}

//------------------------------------------------------------------------------

T_gen::~T_gen()
{
}

//------------------------------------------------------------------------------

void T_gen::Build(P_task * pT)
// The "way in" from the main datastructure. We look at the PoL structure in the
// task, and if it wants a task (circuit) built, we do it.
{
if (pT==0) {                           // Dud call?
  par->Post(905);
  return;
}
if (!pT->IsPoL()) return;              // Legitimate noop
P_task::PoL_t * pP = &(pT->PoL);
// need a new declaration block for the PoL task.
//string PoL_t_name = static_cast<stringstream*>(&(stringstream("PoL_typ",
//      ios_base::out | ios_base::ate)<<par->P_typdclm.size()))->str();
string PoL_t_name = UniS("PoL_typ");
pT->pP_typdcl = new P_typdcl(par,PoL_t_name);
par->P_typdclm[PoL_t_name] = pT->pP_typdcl;
pT->pP_typdcl->P_taskl.push_back(pT);
if (pP->type == "clique") { BuildClique(pT); return; }
if (pP->type == "random") { BuildRandom(pT); return; }
if (pP->type == "ring")   { BuildRing(pT);   return; }
if (pP->type == "tree")   { BuildTree(pT);   return; }
par->Post(111,pP->type);               // Dud PoL type
}

//------------------------------------------------------------------------------

void T_gen::BuildClique(P_task * pT)
// It's a clique (everything connected to everything else)
// task /pol =   name,  type,size,edge_copies,sources,monitors,synapses
// task /pol = muppet,clique,  10,          2,     no,      no,      no
// We know - and have dealt with - the name and type already. We just unload
// the parameters and build it direct into the datastructure.
{
P_task::PoL_t * pP = &(pT->PoL);
unsigned vs = pP->params.size();
unsigned size = 5;                     // Device count
if (vs>0) size = str2uint(pP->params[0]);
if (size==0) return;
unsigned copy = 2;                     // Edge duplicates
if (vs>1) copy = str2uint(pP->params[1]);
/*                                                     For now.....
bool bsrc = false;                     // Sources ?
if (vs>2) bsrc = str2bool(pP->params[2]);
bool bmon = false;                     // Monitors ?
if (vs>3) bmon = str2bool(pP->params[3]);
bool bsyn = false;                     // Synapses ?
if (vs>4) bsyn = str2bool(pP->params[4]);
*/

FILE * fp = stdout;
fprintf(fp,"\n\n");

// create a new (blank) device type for the PoL devices
//P_devtyp * PoL_devtyp = new P_devtyp(pT->pP_typdcl,
//   static_cast<stringstream*>(&(stringstream("PoL_devtyp", ios_base::out | ios_base::ate)<<pT->pP_typdcl->P_devtypv.size()))->str());
P_devtyp * PoL_devtyp = new P_devtyp(pT->pP_typdcl,UniS("Pol_devtyp"));
                                       // Place it in the device type list
pT->pP_typdcl->P_devtypv.push_back(PoL_devtyp);
                                       // Build a supervisor
P_super * pS = new P_super(UniS("Pol_super"));
par->P_superm[pS->Name()] = pS;        // Store it
pT->pSup = pS;                         // Link it to its task

par->Dump();
vector<unsigned> dkeys;                // Save the ids
for (unsigned i=0;i<size;i++) {        // Shove them in....
  P_device * pD = new P_device();
  pD->pP_devtyp=PoL_devtyp;            // associate each device with a type
  pD->addr.SetDevice(i+1);             // Ordinal number
  pD->Par(pT->pD);                     // Parent
  pD->Npar(pT->pD);                    // Namebase parent
  pD->AutoName("De");                  // Derive the name off the uid
  pT->pD->G.InsertNode(pD->Id(),pD);   // Poke it into the graph
  dkeys.push_back(pD->Id());           // Temporarily save key
  if (i%100==0) {
    fprintf(fp,"%u of %u nodes inserted     \r",i,size);
    fflush(fp);
  }
}
fprintf(fp,"%u of %u nodes inserted      \n",size,size);
fflush(fp);

unsigned tot = size * size * copy;
unsigned AK=0;                         // Arc key
for (unsigned k=0;k<copy;k++) {        // Number of copies
  for (unsigned i=0;i<dkeys.size();i++) {
    for (unsigned j=i;j<dkeys.size();j++) {
//      P_message * pM = new P_message();    // NEED A MESSAGE INSTANTIATION
//      pM->Par(
      pT->pD->G.InsertArc(AK++,dkeys[i],dkeys[j]);   // Arc goes this way.....
      pT->pD->G.InsertArc(AK++,dkeys[j],dkeys[i]);   // ... and back again
    }
    if (AK%100==0) fprintf(fp,"%u of %u arcs inserted      \r",AK,tot);
  }
  fflush(fp);
}
fprintf(fp,"%u of %u arcs inserted      \n",AK,tot);
fflush(fp);

}

//------------------------------------------------------------------------------

void T_gen::BuildRing(P_task * pT)
// It's a ring (guess)
// task /pol =   name,  type,size
// task /pol = muppet,  ring,  10
// We know - and have dealt with - the name and type already. We just unload
// the parameters and build it direct into the datastructure.
{
P_task::PoL_t * pP = &(pT->PoL);
unsigned vs = pP->params.size();
unsigned size = 5;                     // Device count
if (vs>0) size = str2uint(pP->params[0]);
if (size==0) return;
/*unsigned copy = 2;                     // Edge duplicates
if (vs>1) copy = str2uint(pP->params[1]);
bool bsrc = false;                     // Sources ?
if (vs>2) bsrc = str2bool(pP->params[2]);
bool bmon = false;                     // Monitors ?
if (vs>3) bmon = str2bool(pP->params[3]);
bool bsyn = false;                     // Synapses ?
if (vs>4) bsyn = str2bool(pP->params[4]);
*/

FILE * fp = stdout;
fprintf(fp,"\n\n");

// create a new (blank) device type for the PoL devices
P_devtyp* PoL_devtyp = new P_devtyp(pT->pP_typdcl, static_cast<stringstream*>(&(stringstream("PoL_devtyp", ios_base::out | ios_base::ate)<<pT->pP_typdcl->P_devtypv.size()))->str());
pT->pP_typdcl->P_devtypv.push_back(PoL_devtyp); // place it in the device type list
vector<unsigned> dkeys;                // Save the ids
for (unsigned i=0;i<size;i++) {        // Shove them in....
  P_device * pD = new P_device();
  pD->pP_devtyp=PoL_devtyp;            // associate each device with a type
  pD->addr.SetDevice(i+1);             // Ordinal number
  pD->Par(pT->pD);                     // Parent
  pD->Npar(pT->pD);                    // Namebase parent
  pD->AutoName("De");                  // Derive the name off the uid
  pT->pD->G.InsertNode(pD->Id(),pD);   // Poke it into the graph
  dkeys.push_back(pD->Id());           // Temporarily save key
  if (i%100==0) fprintf(fp,"%u of %u nodes inserted     \r",i,size);
  fflush(fp);
}
fprintf(fp,"%u of %u nodes inserted      \n",size,size);
fflush(fp);

unsigned tot = dkeys.size();
unsigned AK=0;                         // Arc key
pT->pD->G.InsertArc(AK++,dkeys.back(),dkeys.front());     //Last to first
if (size!=1) for (unsigned i=0;i<dkeys.size()-1;i++) {
  pT->pD->G.InsertArc(AK++,dkeys[i],dkeys[i+1]);
  fprintf(fp,"%u of %u arcs inserted      \r",AK,tot);
  fflush(fp);
}

fprintf(fp,"%u of %u arcs inserted      \n",AK,tot);
fflush(fp);

}

//------------------------------------------------------------------------------
void T_gen::BuildRandom(P_task * pT){}
void T_gen::BuildTree(P_task * pT){}



/*




vH recds,valus;                        // Get the type-specific parameters
P.FndRecdVari(sect,"size",recds);      // Base clique size
                                       // Not there ?
if (recds.size()!=1)pLog->Post(208,pCommon->name);
else {
  P.GetValu(P.FndRecd(recds[0]),valus);// Values associated with "size"
                                       // Pull the device count
  if (valus.size()>=1) pCommon->clique.csize = str2int(valus[0]->str);
                                       // Pull the edge repetition count
  if (valus.size()> 1) pCommon->clique.crept = str2int(valus[1]->str);
}

P.FndRecdVari(sect,"monitors",recds);  // Monitors?
if (recds.size()!=0) {                 // There ?
  P.GetValu(P.FndRecd(recds[0]),valus);// Value associated with "monitors"
  if ((valus.size()>0)&&(valus[0]->str=="yes")) pCommon->clique.mons = true;
}

P.FndRecdVari(sect,"sources",recds);   // Sources?
if (recds.size()!=0) {                 // There ?
  P.GetValu(P.FndRecd(recds[0]),valus);// Value associated with "sources"
  if ((valus.size()>0)&&(valus[0]->str=="yes")) pCommon->clique.srcs = true;
}

// .........and do it.........

FILE *fp = fopen(pCommon->savefile.c_str(),"w");
fprintf(fp,"\n");                      // All the header junk
fprintf(fp,"[Circuit(%s)]\n",pCommon->name.c_str());
fprintf(fp,"#device  :\n#synapse :\n\n");
fprintf(fp,"*device  : SOU\n*device  : MON\n");
fprintf(fp,"*device  : DEV(\"\",\"\")\n");
fprintf(fp,"*synapse : Sy1(\"\",\"\")\n");
fprintf(fp,"#device  : DEV\n");
fprintf(fp,"#synapse : Sy1\n");
fprintf(fp,"\n");
                                       // Devices per line
int dpl = pCommon->clique.csize * pCommon->clique.crept;
//int sn = 0;                            // Synapse count
for (int i=0;i<pCommon->clique.csize;i++) {
  fprintf(fp,"D%04d := ",i);
  int dcnt = 1;                        // Line wrap counter ?
  if (pCommon->clique.mons) fprintf(fp,"M%04d, ",i,dcnt++);
  for (int j=0;j<pCommon->clique.csize;j++) {
    for (int k=0;k<pCommon->clique.crept;k++,dcnt++) {
      fprintf(fp,"D%04d%c ",j,j+(k*pCommon->clique.csize)<dpl-1?',':' ');
                                       // Wrap the line?
      if (dcnt>9)fprintf(fp,"\\ \n         ",dcnt=0);
    } //k
  } // j
  fprintf(fp,"\n");
} // i
fprintf(fp,"\n");

                                       // Declare the monitors
if (pCommon->clique.mons) {
  int dcnt = 1;                        // Wrap line counter
  for (int i=0;i<pCommon->clique.csize;i++,dcnt++) {
    fprintf(fp,"M%04d%s ",i,i<pCommon->clique.csize-1 ? ",":" : MON\n");
                                       // Wrap the line?
    if (dcnt>10)fprintf(fp,"\\ \n",dcnt=0);
  }
}

fprintf(fp,"\n#device  : SOU\n");      // Declare (and connect) the sources
if (pCommon->clique.srcs) for (int i=0;i<pCommon->clique.csize;i++)
  fprintf(fp,"S%04d := D%04d\n",i,i);

fprintf(fp,"\n//-----------------------------------------\n");
fclose(fp);
return;
}

//------------------------------------------------------------------------------

void BuildParameters(JNJ & P,UIF::Node * & sect)
// Now we *know* it's a parameter-testing-circuit... but there are different
// types.
{
vH recds,valus,names,sname;            // Get the type-specific parameters
P.FndRecdVari(sect,"pattern",recds);   // We know there's (at least) one, or we
                                       // wouldn't be here
P.GetValu(recds[0],names);             // Variable (at least one...)
P.GetSub(names[0],sname);
if (sname.empty()&&pLog->Post(401)) return;// No circuit subtype specified?

P.FndRecdVari(sect,"extents",recds);   // "Extents" record ?
                                       // Not there - bail
if ((recds.size()!=1) && pLog->Post(302,pCommon->name)) return;
P.GetValu(P.FndRecd(recds[0]),valus);  // Values associated with "extents" ?
unsigned devcnt = 1;                   // Default device count
if (valus.size()==1) devcnt = str2int(valus[0]->str);

// ................and do it................

FILE *fp = fopen(pCommon->savefile.c_str(),"w");
Header H;
H.Author("CGenerator");


if (sname[0]->str == string("UoM")) {
  H.Name(pCommon->savefile);
  H.SaveA(fp);
  fprintf(fp,"\n");                      // All the header junk
  fprintf(fp,"[Circuit(%s)]\n",pCommon->name.c_str());
  fprintf(fp,"// Compressed synapses, fixed width device parameters\n");
//  fprintf(fp,"#device  :\n#synapse :\n\n");
  fprintf(fp,"*device  : Dev(\"%%4c%%4c%%4c\",\"%%32b %%32b %%16b\")\n");
  fprintf(fp,"*synapse : Sy1(\"%%4c%%4c\",\"#\")\n");
  fprintf(fp,"#device  : Dev\n");
  fprintf(fp,"#synapse : Sy1\n");
  fprintf(fp,"\n");
  unsigned syp = 0;
  unsigned syd = 0;
  unsigned i;
  for(i=0;i<devcnt;i++) {
    fprintf(fp,"D%04d : Dev(%2d,%2d,%2d) = D%04d(Sy1(%2d,%2d))\n",
               i,syd,syd+1,syd+2,i+1,syp,syp+1);
    syp += 2;
    syd += 3;
  }
  fprintf(fp,"D%04d : Dev(%2d,%2d,%2d) = \n",i,syd,syd+1,syd+2);
  fclose(fp);
  return;
}

if (sname[0]->str == string("generic")) {
  const unsigned dLEN = 10;
  const unsigned dlen[dLEN] = {0,1,2,3,5,7,9,11,13,19};
  const unsigned sLEN = 7;
  const unsigned slen[sLEN] = {0,1,3,5,7,11,13};
  H.Name(pCommon->savefile);
  H.SaveA(fp);
  fprintf(fp,"\n");                      // All the header junk
  fprintf(fp,"[Circuit(%s)]\n",pCommon->name.c_str());
  fprintf(fp,"// Variable width synapses/device parameters, no compression\n");
//  fprintf(fp,"#device  :\n#synapse :\n\n");
  for (unsigned i=0;i<dLEN;i++) {
    fprintf(fp,"*device  : Dev%02u(\"",i);
    for (unsigned j=0;j<dlen[i];j++) fprintf(fp,"%%4c ");
    fprintf(fp,"\", \\\n                 \"");
    for (unsigned j=0;j<dlen[i];j++) fprintf(fp,"%%32b");
    fprintf(fp,"\")\n");
  }
  for (unsigned i=0;i<sLEN;i++) {
    fprintf(fp,"*synapse : Sy%02u(\"",i);
    for (unsigned j=0;j<slen[i];j++) fprintf(fp,"%%4c ");
    fprintf(fp,"\", \\\n                 \"");
    for (unsigned j=0;j<slen[i];j++) fprintf(fp,"%%32b");
    fprintf(fp,"\")\n");
  }
//  fprintf(fp,"#device  : Dev\n");
//  fprintf(fp,"#synapse : Sy1\n");
  fprintf(fp,"\n");
  unsigned syp = 0;                      // Start synapse param value
  unsigned syd = 0;                      // Start device param value
  unsigned i;                            // Need it out of the loop
  for (i=0;i<devcnt;i++) {
    fprintf(fp,"D%04u : Dev%02u(",i,i%dLEN);      // Driver....
    for (unsigned k=0;k<dlen[i%dLEN];k++)
      fprintf(fp,"%2u%s",syd++,k<dlen[i%dLEN]-1? ",":"");
    fprintf(fp,") = D%04u(Sy%02u(",i+1,i%sLEN);   // Drivee....
    for (unsigned k=0;k<slen[i%sLEN];k++)
      fprintf(fp,"%2u%s",syp++,k<slen[i%sLEN]-1? ",":"");
    fprintf(fp,"))\n");
  }
  fprintf(fp,"D%04u : Dev%02u =\n",devcnt,i%dLEN);  // Declare the final drivee
  fclose(fp);
  return;
}

P.FndRecdVari(sect,"extents",recds);   // "Extents" record
                                       // Not there - bail
if ((recds.size()!=1) && pLog->Post(302,pCommon->name)) return;
P.GetValu(P.FndRecd(recds[0]),valus);  // Values associated with "extents"
                                       // If not exactly three, go
if ((valus.size()!=3) && pLog->Post(303,pCommon->name)) return;
                                       // Pull them out...
                                       // Size of first ring
pCommon->ring.rSize0   = str2int(valus[0]->str);
if (pCommon->ring.rSize0 < 2) {
  pCommon->ring.rSize0 = 2;
  pLog->Post(203,pCommon->name);
}
                                       // How big is the next one?
pCommon->ring.stride   = str2int(valus[1]->str);
                                       // How many rings?
pCommon->ring.rCnt     = str2int(valus[2]->str);
if (pCommon->ring.rCnt < 1) {
  pCommon->ring.rCnt = 1;
  pLog->Post(204,pCommon->name);
}
switch (valus[1]->qop) {
  case Lex::Sy_cat  : pCommon->ring.str_op = '^'; break;
  case Lex::Sy_mult : pCommon->ring.str_op = '*'; break;
  default           : pCommon->ring.str_op = '+'; break;
}

pCommon->ring.Enumerate();             // Evaluate the ring sizes

// .........and do it.........

FILE *fp = fopen(pCommon->savefile.c_str(),"w");
fprintf(fp,"\n");                      // All the header junk
fprintf(fp,"type            = asset(circuit)\n");
fprintf(fp,"version         = No_Version_Control\n");
fprintf(fp,"create          = \"%s(%s)\"\n",
           pCommon->start_d.c_str(),pCommon->start_t.c_str());
fprintf(fp,"name(saved)     = \"%s\"\n",pCommon->savefile.c_str());
fprintf(fp,"\n");
fprintf(fp,"[generator(%s)]\n",pCommon->name.c_str());
fprintf(fp,"pattern         = %s\n",pCommon->pattern.c_str());
fprintf(fp,"extents         = %d,%c%d,%d\n",
           pCommon->ring.rSize0,pCommon->ring.str_op,
           pCommon->ring.stride,pCommon->ring.rCnt);
for (unsigned int i=0;i<pCommon->ring.rSizes.size();i++)
  fprintf(fp,"  %02d            : %04d\n",i,pCommon->ring.rSizes[i]);
fprintf(fp,"save            = %s\n",pCommon->name.c_str());
fprintf(fp,"synapses        = %s\n\n",pCommon->synapses ? "yes" : "no");

fprintf(fp,"[Circuit(%s)]\n",pCommon->name.c_str());
//fprintf(fp,"<graph vov>\n");
fprintf(fp,"#device  :\n#synapse :\n");
fprintf(fp,"*device  : DEV(\"\",\"\")\n");
fprintf(fp,"*device  : SOU(\"\",\"\")\n");
fprintf(fp,"*device  : MON(\"\",\"\")\n");
fprintf(fp,"*synapse : Sy1(\"\",\"\")\n");
fprintf(fp,"#device  : DEV\n");
fprintf(fp,"#synapse : Sy1\n");
fprintf(fp,"\n");
int dev = 0;                           // Device count in file
int dcnt = 0;                          // Device count in line
int sn = 0;                            // Synapse count
                                       // Start device
fprintf(fp,"DS0      : SOU = ",dev++);
for (int i=0;i<pCommon->ring.rCnt;i++) {
  fprintf(fp,"DR%02dP000",i);
  if (i==pCommon->ring.rCnt-1) fprintf(fp,"\n");
  else {
    fprintf(fp,",");
    if (++dcnt>4)fprintf(fp,"\\ \n                             ",dcnt=0);
  }
}
                                       // Stop device
fprintf(fp,"DS1      : SOU = ",dev++);
dcnt = 0;
for (int i=0;i<pCommon->ring.rCnt;i++) {
  fprintf(fp,"DR%02dP001",i);
  if (i==pCommon->ring.rCnt-1) fprintf(fp,"\n");
  else {
    fprintf(fp,",");
    if (++dcnt>4)fprintf(fp,"\\ \n                             ",dcnt=0);
  }
}
                                       // Build each ring
for (int r=0;r<pCommon->ring.rCnt;r++) {
                                       // Declare the monitor
  fprintf(fp,"DM%02d     : MON = \n",r);
  dev++;
                                       // Each device in the ring
  for (int p=0;p<pCommon->ring.rSizes[r];p++) {
    int tgt = p+1;                     // Get driven device position
                                       // Close the ring
    if (p==int(pCommon->ring.rSizes[r])-1) tgt = 0;
// driven = "DR" + int2str(r) + "P" + int2str(tgt);
// driver = "DR" + int2str(r) + "P" + int2str(p);
    fprintf(fp,"DR%02dP%03d := DR%02dP%03d%s",
               r,p,r,tgt,S_syn(sn++).c_str());
    dev++;
    if (p==1) fprintf(fp,", DM%02d",r);
    fprintf(fp,"\n");
  } // for (each device in the ring)
} // for(build each ring)

//fprintf(fp,"<!graph>\n");
fprintf(fp,"//-----------------------------------------\n");
fclose(fp);
return;    */ /*
}

//------------------------------------------------------------------------------

void BuildRandom(JNJ & P,UIF::Node * & sect)
// Now we *know* it's a random mess; pull in the rest of the parameters and
//build it.
{
pLog->Post(110,pCommon->savefile);
vH recds,valus,names,names2,namesv;    // Get the type-specific parameters
P.FndRecdVari(sect,"seed",recds);      // PRNG seed value
                                       // Not there ?
if ((recds.size()!=1) && pLog->Post(106,pCommon->name))
  pCommon->random.seed = Time2long(pCommon->start_t);
else {
  P.GetValu(P.FndRecd(recds[0]),valus);  // Values associated with "seed"
                                       // If not exactly one, go
  if ((valus.size()!=1) && pLog->Post(310,pCommon->name)) return;
                                       // So get the value
  pCommon->random.seed = str2long(valus[0]->str);
}

P.FndRecdVari(sect,"cluster",recds);   // How many clusters?
                                       // None - bail
if ((recds.size()==0) && pLog->Post(107,pCommon->name)) return;

WALKVECTOR(hJNJ,recds,i) {             // Walk the cluster records
  P.GetVari(*i,names);                 // Get the names (s.b. "cluster")
  WALKVECTOR(hJNJ,names,j) {
    P.GetSub(*j,names2);               // Get the subnames
    if ((names2.size()!=1)&& pLog->Post(311,pCommon->name)) return;
    Common::Cluster cx;                // So far, so good. Assemble....
    P.GetValu(*i,namesv);              // Get the record values
    WALKVECTOR(hJNJ,namesv,k) {
      vH valu1,valu2;
      if((*k)->str=="devices") {       // Name is "value"
        P.GetSub(*k,valu1);
        if (valu1.size()==1) {         // Pull in device average
          cx.devAve = str2int(valu1[0]->str);
          P.GetSub(valu1[0],valu2);    // Pull in device tolerance
          if (valu2.size()==1) cx.devTol = str2int(valu2[0]->str);
                                       // Wanker trap
          if (cx.devTol>cx.devAve) cx.devTol=cx.devAve;
        }
      } else
      if((*k)->str=="fanout") {
        P.GetSub(*k,valu1);
        if (valu1.size()==1) {
          cx.fanAve = str2int(valu1[0]->str);
          P.GetSub(valu1[0],valu2);
          if (valu2.size()==1) cx.fanTol = str2int(valu2[0]->str);
                                       // Wanker trap
          if (cx.fanTol>cx.fanAve) cx.fanTol=cx.fanAve;
        }
      } else
      if((*k)->str=="source") {
        P.GetSub(*k,valu1);            // Vector of source targets
        WALKVECTOR(hJNJ,valu1,l) cx.src.push_back((*l)->str);
      } else
      if((*k)->str=="monitor") {
        P.GetSub(*k,valu1);            // Vector of monitor targets
        WALKVECTOR(hJNJ,valu1,l) cx.mon.push_back((*l)->str);
      } else
      pLog->Post(207,pCommon->name);
    }
//  cx.Dump();
    pCommon->random.mc[names2[0]->str] = cx;
//  pCommon->random.Dump();
  }
}

// Having unpacked the source and monitor clauses, I decided they didn't
// actually bring much to the party - if you do simulate it it's unlikely you'll
// get anything interesting - so I don't actually do anything with them.


// .........and do it.........

FILE *fp = fopen(pCommon->savefile.c_str(),"w");
fprintf(fp,"\n");                      // All the header junk
fprintf(fp,"type            = asset(circuit)\n");
fprintf(fp,"version         = No_Version_Control\n");
fprintf(fp,"create          = \"%s(%s)\"\n",
           pCommon->start_d.c_str(),pCommon->start_t.c_str());
fprintf(fp,"name(saved)     = \"%s\"\n",pCommon->savefile.c_str());
fprintf(fp,"\n");
fprintf(fp,"[generator(%s)]\n",pCommon->name.c_str());
fprintf(fp,"pattern         = %s\n",pCommon->pattern.c_str());
fprintf(fp,"seed            = %ld\n",pCommon->random.seed);
fprintf(fp,"name            = %s\n",pCommon->name.c_str());
fprintf(fp,"synapses        = %s\n",pCommon->synapses ? "yes" : "no");
WALKMAP(string,Common::Cluster,pCommon->random.mc,i) {
  fprintf(fp,"cluster(%s)      = devices(%4d(%4d)), fanout(%4d(%4d)), source(",
             (*i).first.c_str(),(*i).second.devAve,(*i).second.devTol,
                                (*i).second.fanAve,(*i).second.fanTol);
  WALKVECTOR(string,(*i).second.src,j) {
    fprintf(fp,"%s",(*j).c_str());
    if (j!=(*i).second.src.end()-1) fprintf(fp,",");
  }
  fprintf(fp,"), monitor(");
  WALKVECTOR(string,(*i).second.mon,j) {
    fprintf(fp,"%s",(*j).c_str());
    if (j!=(*i).second.mon.end()-1) fprintf(fp,",");
  }
  fprintf(fp,")\n");
}
fprintf(fp,"\n");

fprintf(fp,"[Circuit(%s)]\n",pCommon->name.c_str());
fprintf(fp,"#device  :\n#synapse :\n");
fprintf(fp,"*device  : DEV(\"\",\"\")\n");
fprintf(fp,"*synapse : Sy1(\"\",\"\")\n");
fprintf(fp,"#device  : DEV\n");
fprintf(fp,"#synapse : Sy1\n");
fprintf(fp,"\n");
//fprintf(fp,"<graph vov>\n");

int devTOT = 0;                        // Total device count
int sn = 0;                            // Synapse count
int dstart=0;
vector<string> kc;
                                       // For each cluster....
WALKMAP(string,Common::Cluster,pCommon->random.mc,i) {
  int dTol = (*i).second.devTol;       // Device count in cluster
  int dAve = (*i).second.devAve;       // Tolerance
  int fTol = (*i).second.fanTol;       // Unpack fanout parameters
  int fAve = (*i).second.fanAve;
  int seed = pCommon->random.seed;
  Urand dPRNG(dTol*2);                 // Wake up the PRNGs
  Urand fPRNG(fTol*2);
  dPRNG.seed(seed);                    // Seed them
  fPRNG.seed(seed);
  int csize = dPRNG() - dTol + dAve;   // Get the size of the cluster
  if (dTol==0) csize = dAve;           // PRNG breaks when |range| = 0
  fprintf(fp,"// Start of cluster %s ... %d+/-%d (=%d)\n",
             (*i).first.c_str(),dAve,dTol,csize);
  Urand cPRNG(csize);
  cPRNG.seed(seed);
  for (int j=0;j<csize;j++) kc.push_back(string("D"+int2str(j+dstart)));
  int devIN=0;                         // Devices in this cluster
                                       // Walk the devices in this cluster
  for (int d=dstart;d<int(kc.size());d++) {
    devTOT++;
    devIN++;
    int fo = fPRNG() - fTol + fAve;    // Fanout for this device
    if (fTol==0) fo = fAve;            // Fix PRNG
    fprintf(fp,"D%04d := ",d);
    int dcnt = 0;                      // Line wrap counter
    for (int k=0;k<fo;k++) {           // Walk the driven devices
      int driven = cPRNG() + dstart;
      fprintf(fp,"D%04d%s%c ",driven,S_syn(sn++).c_str(),k<fo-1?',':' ');
                                       // Wrap the line?
      if (++dcnt>11)fprintf(fp,"\\ \n         ",dcnt=0);
    }
    fprintf(fp,"\\ \n         {fanout = %d}\n",fo);
  }
  fprintf(fp,"// End of cluster %s : %d devices\n\n",(*i).first.c_str(),devIN);
  dstart += csize;
}
fprintf(fp,"// Total devices : %d\n\n",devTOT);


//fprintf(fp,"<!graph>\n");
fprintf(fp,"//-----------------------------------------\n");
fclose(fp);
return;
}

//------------------------------------------------------------------------------

void BuildRing(JNJ & P,UIF::Node * & sect)
// Now we *know* it's a (set of) ring(s); pull in the rest of the parameters
// and build them.
{
vH recds,valus;                        // Get the type-specific parameters
P.FndRecdVari(sect,"extents",recds);   // "Extents" record
                                       // Not there - bail
if ((recds.size()!=1) && pLog->Post(302,pCommon->name)) return;
P.GetValu(P.FndRecd(recds[0]),valus);  // Values associated with "extents"
                                       // If not exactly three, go
if ((valus.size()!=3) && pLog->Post(303,pCommon->name)) return;
                                       // Pull them out...
                                       // Size of first ring
pCommon->ring.rSize0   = str2int(valus[0]->str);
if (pCommon->ring.rSize0 < 2) {
  pCommon->ring.rSize0 = 2;
  pLog->Post(203,pCommon->name);
}
                                       // How big is the next one?
pCommon->ring.stride   = str2int(valus[1]->str);
                                       // How many rings?
pCommon->ring.rCnt     = str2int(valus[2]->str);
if (pCommon->ring.rCnt < 1) {
  pCommon->ring.rCnt = 1;
  pLog->Post(204,pCommon->name);
}
switch (valus[1]->qop) {
  case Lex::Sy_cat  : pCommon->ring.str_op = '^'; break;
  case Lex::Sy_mult : pCommon->ring.str_op = '*'; break;
  default           : pCommon->ring.str_op = '+'; break;
}

pCommon->ring.Enumerate();             // Evaluate the ring sizes

// .........and do it.........

FILE *fp = fopen(pCommon->savefile.c_str(),"w");
fprintf(fp,"\n");                      // All the header junk
fprintf(fp,"type            = asset(circuit)\n");
fprintf(fp,"version         = No_Version_Control\n");
fprintf(fp,"create          = \"%s(%s)\"\n",
           pCommon->start_d.c_str(),pCommon->start_t.c_str());
fprintf(fp,"name(saved)     = \"%s\"\n",pCommon->savefile.c_str());
fprintf(fp,"\n");
fprintf(fp,"[generator(%s)]\n",pCommon->name.c_str());
fprintf(fp,"pattern         = %s\n",pCommon->pattern.c_str());
fprintf(fp,"extents         = %d,%c%d,%d\n",
           pCommon->ring.rSize0,pCommon->ring.str_op,
           pCommon->ring.stride,pCommon->ring.rCnt);
for (unsigned int i=0;i<pCommon->ring.rSizes.size();i++)
  fprintf(fp,"  %02d            : %04d\n",i,pCommon->ring.rSizes[i]);
fprintf(fp,"save            = %s\n",pCommon->name.c_str());
fprintf(fp,"synapses        = %s\n\n",pCommon->synapses ? "yes" : "no");

fprintf(fp,"[Circuit(%s)]\n",pCommon->name.c_str());
//fprintf(fp,"<graph vov>\n");
fprintf(fp,"#device  :\n#synapse :\n");
fprintf(fp,"*device  : DEV(\"\",\"\")\n");
fprintf(fp,"*device  : SOU(\"\",\"\")\n");
fprintf(fp,"*device  : MON(\"\",\"\")\n");
fprintf(fp,"*synapse : Sy1(\"\",\"\")\n");
fprintf(fp,"#device  : DEV\n");
fprintf(fp,"#synapse : Sy1\n");
fprintf(fp,"\n");
int dev = 0;                           // Device count in file
int dcnt = 0;                          // Device count in line
int sn = 0;                            // Synapse count
                                       // Start device
fprintf(fp,"DS0      : SOU = ",dev++);
for (int i=0;i<pCommon->ring.rCnt;i++) {
  fprintf(fp,"DR%02dP000",i);
  if (i==pCommon->ring.rCnt-1) fprintf(fp,"\n");
  else {
    fprintf(fp,",");
    if (++dcnt>4)fprintf(fp,"\\ \n                             ",dcnt=0);
  }
}
                                       // Stop device
fprintf(fp,"DS1      : SOU = ",dev++);
dcnt = 0;
for (int i=0;i<pCommon->ring.rCnt;i++) {
  fprintf(fp,"DR%02dP001",i);
  if (i==pCommon->ring.rCnt-1) fprintf(fp,"\n");
  else {
    fprintf(fp,",");
    if (++dcnt>4)fprintf(fp,"\\ \n                             ",dcnt=0);
  }
}
                                       // Build each ring
for (int r=0;r<pCommon->ring.rCnt;r++) {
                                       // Declare the monitor
  fprintf(fp,"DM%02d     : MON = \n",r);
  dev++;
                                       // Each device in the ring
  for (int p=0;p<pCommon->ring.rSizes[r];p++) {
    int tgt = p+1;                     // Get driven device position
                                       // Close the ring
    if (p==int(pCommon->ring.rSizes[r])-1) tgt = 0;
// driven = "DR" + int2str(r) + "P" + int2str(tgt);
// driver = "DR" + int2str(r) + "P" + int2str(p);
    fprintf(fp,"DR%02dP%03d := DR%02dP%03d%s",
               r,p,r,tgt,S_syn(sn++).c_str());
    dev++;
    if (p==1) fprintf(fp,", DM%02d",r);
    fprintf(fp,"\n");
  } // for (each device in the ring)
} // for(build each ring)

//fprintf(fp,"<!graph>\n");
fprintf(fp,"//-----------------------------------------\n");
fclose(fp);
return;
}

//------------------------------------------------------------------------------

void BuildTree(JNJ & P,UIF::Node * & sect)
// Now we *know* it's a tree; pull in the rest of the parameters and build it.
{
vH recds,valus;                        // Get the type-specific parameters
P.FndRecdVari(sect,"fanout",recds);    // "Fanout" record
                                       // Not there - bail
if ((recds.size()!=1) && pLog->Post(306,pCommon->name)) return;
P.GetValu(P.FndRecd(recds[0]),valus);  // Values associated with "fanout"
                                       // If not exactly one, go
if ((valus.size()!=1) && pLog->Post(307,pCommon->name)) return;
                                       // So get the value
pCommon->tree.fanout   = str2int(valus[0]->str);
if (pCommon->tree.fanout < 1) {
  pCommon->tree.fanout = 1;
  pLog->Post(205,pCommon->name);
}

P.FndRecdVari(sect,"depth",recds);     // How many levels?
                                       // Not there - bail
if ((recds.size()!=1) && pLog->Post(308,pCommon->name)) return;
P.GetValu(P.FndRecd(recds[0]),valus);  // Values associated with "depth"
                                       // If not exactly one, go
if ((valus.size()!=1) && pLog->Post(309,pCommon->name)) return;
                                       // So get the value
pCommon->tree.depth = str2int(valus[0]->str);
if (pCommon->tree.depth < 1) {
  pCommon->tree.depth = 1;
  pLog->Post(206,pCommon->name);
}

// .........and do it.........

FILE *fp = fopen(pCommon->savefile.c_str(),"w");
fprintf(fp,"\n");                      // All the header junk
fprintf(fp,"type            = asset(circuit)\n");
fprintf(fp,"version         = No_Version_Control\n");
fprintf(fp,"create          = \"%s(%s)\"\n",
           pCommon->start_d.c_str(),pCommon->start_t.c_str());
fprintf(fp,"name(saved)     = \"%s\"\n",pCommon->savefile.c_str());
fprintf(fp,"\n");
fprintf(fp,"[generator(%s)]\n",pCommon->name.c_str());
fprintf(fp,"pattern         = %s\n",pCommon->pattern.c_str());
fprintf(fp,"fanout          = %d\n",pCommon->tree.fanout);
fprintf(fp,"depth           = %d\n",pCommon->tree.depth);
fprintf(fp,"save            = %s\n",pCommon->name.c_str());
fprintf(fp,"synapses        = %s\n\n",pCommon->synapses ? "yes" : "no");

fprintf(fp,"[Circuit(%s)]\n",pCommon->name.c_str());
//fprintf(fp,"<graph vov>\n");
fprintf(fp,"#device  :\n#synapse :\n");
fprintf(fp,"*device  : DEV(\"\",\"\")\n");
fprintf(fp,"*device  : SOU(\"\",\"\")\n");
fprintf(fp,"*device  : MON(\"\",\"\")\n");
fprintf(fp,"*synapse : Sy1(\"\",\"\")\n");
fprintf(fp,"#device  : DEV\n");
fprintf(fp,"#synapse : Sy1\n");
fprintf(fp,"\n");

int dev = 0;                           // Device count
dev++;                                 // Start device drives tree root
fprintf(fp,"DS0        := DL000P0000\n");

vector<string> devices;
devices.push_back("DL000P0000");        // Push in the root
int Lb = 0;                            // Address of leaves in vector
int Le = 0;
int sn = 0;                            // Synapse count

for (int lev = 1;lev<=pCommon->tree.depth;lev++) {
  int p = 0;                           // Offset in row
  for (int pos = Lb;pos<=Le;pos++) {   // From start leaf to end leaf
//    printf("\nDriver %s\n-------------\n",devices[pos].c_str());
    fprintf(fp,"%s := ",devices[pos].c_str());
    dev++;
    int dcnt = 0;                       // Continuation line counter
    for (int j=0;j<pCommon->tree.fanout;j++) {
      string buf;
      dprintf(buf,"DL%03dP%04d%s",lev,p++,S_syn(sn++).c_str());
      devices.push_back(buf);
//      printf("Driven %s\n",(*(devices.end()-1)).c_str());
      fprintf(fp,"%s%c ",
                 (*(devices.end()-1)).c_str(),j<pCommon->tree.fanout-1?',':' ');
                                       // Wrap the line?
      if (++dcnt>5)fprintf(fp,"\\ \n                       ",dcnt=0);
    }
    fprintf(fp,"\n");
  }
  Lb = Le + 1;
  Le = (int)devices.size()-1;
//printf("End of level %d build:\ndevices:\n",lev);
//for(unsigned int i = 0;i<devices.size();i++) printf("%3d : %s\n",i,devices[i].c_str());
//printf("Lb = %d\nLe = %d\n",Lb,Le);
}
// Now the definition of the last tree row (leaves) and the chain of drivers to
// the final monitor
int off = 0;
for (int pos=Lb;pos<=Le;pos++) {
  fprintf(fp,"%s := DLf%04d \n",devices[pos].c_str(),off);
  dev++;
  if (pos<Le) {
    fprintf(fp,"DLf%04d    := DLf%04d\n",off,off+1);
    dev++;
  }
  else {
    fprintf(fp,"DLf%04d    := DM0\n",off);
    dev++;
  }
  off++;
}
                                       // Declare the final monitor
fprintf(fp,"DM0        : MON = \n");
dev++;

//fprintf(fp,"<!graph>\n");
fprintf(fp,"//-----------------------------------------\n");
fclose(fp);
return;
}

//------------------------------------------------------------------------------

string FindQualifier(int argc,char * argv[],string qual)
// Parse the command line arguments to see if there is a qualifier called "qual"
// and return the first argument if there is.
{
JNJ A(argc,argv);                      // UIF parser for command-line arguments
vH sects,recds,varis;
A.FndSect(qual,sects);                 // Hunt the qualifier
if (sects.empty()) return string();    // No sections of the right name?
A.FndRecdLabl(sects[0],qual,recds);    // Yup - find records with right label
if (recds.empty()) return string();    // No records?
A.GetVari(recds[0],varis);             // Yup - find the corresponding variables
if (varis.empty()) return string();    // None there?
return varis[0]->str;                  // At last! We only want the first one 
}

//------------------------------------------------------------------------------

string S_syn(int i)
// Build a synapse label
{
                                       // Don't want to
if (!pCommon->synapses) return string();
string ans;
dprintf(ans,"(s%03d($%04x))",i,i);
return ans;
}

//==============================================================================

         */


