//------------------------------------------------------------------------------

#include "CmGrph_t.h"
#include "OrchBase.h"

//==============================================================================

CmGrph_t::CmGrph_t(OrchBase * par) : OpsGrph_t(par)
{

}

//------------------------------------------------------------------------------

CmGrph_t::~CmGrph_t()
{
}

//------------------------------------------------------------------------------

unsigned CmGrph_t::operator()(Cli * pC)
// Handle "graph" command from the monkey
{
//printf("CmGrph_t operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
//pC->Dump();
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string scl = (*i).Cl;                // Pull out clause name
  if (strcmp(scl.c_str(),"buil")==0) {                          continue; }


  if (strcmp(scl.c_str(),"dela")==0) { GrDela(*i);              continue; }
  if (strcmp(scl.c_str(),"deli")==0) {                          continue; }
  if (strcmp(scl.c_str(),"delt")==0) {                          continue; }
  if (strcmp(scl.c_str(),"depl")==0) {                          continue; }
  if (strcmp(scl.c_str(),"doit")==0) { /*hOutp = (*i).GetP(0);*/continue; }
  if (strcmp(scl.c_str(),"dump")==0) { Dump();                  continue; }
  if (strcmp(scl.c_str(),"file")==0) { GrFile(*i);              continue; }
  if (strcmp(scl.c_str(),"init")==0) {                          continue; }
  if (strcmp(scl.c_str(),"inst")==0) {                          continue; }
  if (strcmp(scl.c_str(),"inte")==0) {                          continue; }
  if (strcmp(scl.c_str(),"outp")==0) { GrOutp(*i);              continue; }
  if (strcmp(scl.c_str(),"pol" )==0) {                          continue; }
  if (strcmp(scl.c_str(),"rena")==0) {                          continue; }
  if (strcmp(scl.c_str(),"rety")==0) { GrRety(*i);              continue; }
  if (strcmp(scl.c_str(),"run" )==0) {                          continue; }
  if (strcmp(scl.c_str(),"show")==0) { GrShow(*i);              continue; }
  if (strcmp(scl.c_str(),"stop")==0) {                          continue; }
  if (strcmp(scl.c_str(),"tlin")==0) {                          continue; }
  if (strcmp(scl.c_str(),"type")==0) {                          continue; }
  par->Post(25,scl,"graph");           // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//------------------------------------------------------------------------------

//==============================================================================

