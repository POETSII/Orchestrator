//------------------------------------------------------------------------------

#include "A_builder.h"
#include "filename.h"
#include "xmlTreeDump.h"
#include "BHelper.h"

//==============================================================================

A_builder::A_builder(OrchBase * _p):par(_p)
{

}

//------------------------------------------------------------------------------

A_builder::~A_builder()
{

}

//------------------------------------------------------------------------------

void A_builder::Build(Apps_t * pT)
// Generates the application binaries - virtually mapped to a single board.
{
//if (!pT) pT = par->P_filem.begin()->second;
//par->Post(801,pT->Name(),pT->filename);
}

//------------------------------------------------------------------------------

unsigned A_builder::DoIt(Apps_t * pF)
// The description (nominal XML) file has passed validation, so we can at
// least hope it'll load cleanly-ish.
{
par->Post(207);
long t0 = mTimer();

BHelper Xh(this);                      // Attach the helper object
Xh.DoIt(pF);                           // ..and away we go
par->Post(206,long2str(mTimer(t0)));
pF->Show(fl);                          // Pretty-print to the load log

return 0;
}

//------------------------------------------------------------------------------

void A_builder::Dump(FILE * fp)
{
fprintf(fp,"A_builder+++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"Parent         %#08p\n",par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());

fprintf(fp,"A_builder-----------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

unsigned A_builder::Integ(Apps_t * pT)
{
par->Post(208);
long t0 = mTimer();




par->Post(206,long2str(mTimer(t0)));
return 0;
}

//------------------------------------------------------------------------------

bool A_builder::Load(Apps_t * pF)
// The file object (*pF) exists. Here we go get the file with the source in it
// (we know it exists, but not what's in it), so we open it and load everything.
// In slightly more detail:
// 1. Create detailed log infrastructure (i.e open the file)
// 2. Validate file contents
// 3. Build datastructure (can't do this in any known order, 'cos the contents
// of the definition file is unordered)
// 4. Integrity check
// ...and we're good to go
{
if (pF==0) return par->Post(920);      // Non-existent file?????
pF->Dump();                            // We're about as sure as we can be that
                                       // we're in business
string file = pF->filename;            // Extract filename
FileName Fn(file);                     // Take it to bits
loadlog = pF->Name()+"_"+Fn.FNBase()+"_Load.log";  // Build log file name
fl = fopen(loadlog.c_str(),"w");       // Guess
if (fl==0) return par->Post(921,loadlog);// No valid reason why this should fail
unsigned ecnt = Validate(file);        // Have a look at it
fclose(fl);                            // We may open it again if we keep going
par->Post(202,pF->Name(),file,uint2str(ecnt));
if (ecnt!=0) {
  par->Post(201,pF->Name(),file,loadlog);
  return true;
}
                                       // So here - at last - we build
fl = fopen(loadlog.c_str(),"a");       // Guess
ecnt = DoIt(pF);                       // Possibly more error....
fclose(fl);
if (ecnt!=0) {
  par->Post(203,pF->Name(),file,uint2str(ecnt));
  UnDoIt(pF);                          // Unpick it all
  return true;
}
ecnt = Integ(pF);                      // Check structural integrity
if (ecnt!=0) {
  par->Post(204,pF->Name(),file,uint2str(ecnt));
  UnDoIt(pF);                          // Unpick it all
  return true;
}

return false;
}

//------------------------------------------------------------------------------

void A_builder::UnDoIt(Apps_t * pT)
{
par->Post(209);
long t0 = mTimer();





par->Post(206,long2str(mTimer(t0)));
return;
}

//------------------------------------------------------------------------------

unsigned A_builder::Validate(string file)
// Validate the incoming file.
// *************************************************
// TEMPORARY MEASURE: We just parse it, build the tree and throw the whole lot
// into the load log file. Such "validation" as there is consists of seeing
// if the *XML* parser chokes on anything.
// *************************************************
// ftAoD:
// 'file' is the name of the XML file
// 'fx' is the file stream associated with it
// 'fl' is the file stream associated with the text output stream
{
FILE * fx = fopen(file.c_str(),"r");
if (fx==0) {
  par->Post(922,file);
  return 1;
}
par->Post(205);
long t0 = mTimer();
// Stick a semi-comprehensible header in the load log file
fprintf(fl,"***************************************************************\n");
fprintf(fl,"File %s created %s %s: load log for description %s\n",
  loadlog.c_str(),GetDate(),GetTime(),file.c_str());
fprintf(fl,"***************************************************************\n");
xmlTreeDump Xp;                        // Treedumper::parser
Xp.SetOChan(fl);                       // Point the output to a file
unsigned ecnt = Xp.Transform(fx);      // Kick it, Winston
// ...and a semi-comprehensible tail in the load log file

fprintf(fl,"***************************************************************\n");
fprintf(fl,"\n%u inconsistencies/errors found in %ld ms at about %s\n",
              ecnt,long2str(mTimer(t0)),string(GetTime()).c_str());
fprintf(fl,"***************************************************************\n");
fflush(stdout);                        // You can't be too careful
fflush(fl);                            // And again
fclose(fx);                            // Close the XML file
fx = 0;
par->Post(206,long2str(mTimer(t0)));
return ecnt;
}

//==============================================================================

