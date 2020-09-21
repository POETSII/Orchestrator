//------------------------------------------------------------------------------

#include <stdio.h>
#include "XValid.h"
#include "flat.h"
#include "Stencil_t.h"
#include "attrDecode.h"
#include "pathDecode.h"
#include "Unrec_t.h"

//==============================================================================

// Pretty-print separators. These are direct copies of the constants defined
// elsewhere, but are here to keep XValid et al generic-eligible
const string XValid::sline = string(120,'-');
const string XValid::dline = string(120,'.');
const string XValid::aline = string(120,'*');
const string XValid::eline = string(120,'=');
const string XValid::pline = string(120,'+');

//==============================================================================

XValid::XValid(string s,FILE * ochan)
// Constructor: this establishes a single grammar tree for the definition, which
// is used by future calls to .Validate() and defines the output channel
{
Init(s,ochan);
InitValidation();                      // Build the validator tree
}

//------------------------------------------------------------------------------


XValid::~XValid()
// Bail out time. Because the stencils are hung off a void *, we have to
// explicitly delete them. I know, I know.
{
DeleteTag(Validor.t_root);             // Recursively delete grammar stencils
if (Validee!=0) {                      // If we've got a client....
  DeleteTag(Validee->t_root);          // Recursively delete client stencils
  delete Validee;                      // Delete the client itself
}
}

//------------------------------------------------------------------------------

void XValid::AddStencil(xmlTreeDump::node * n)
// Routine to add the definition stencil to the tag field in the XML definition
// tree of the grammar
{
                                       // Walk the tree (depth first)
WALKVECTOR(xmlTreeDump::node *,n->vnode,i) AddStencil(*i);
                                       // So now we're in business
Stencil_t * ps = new Stencil_t() ;     // Create and attach an (empty) stencil
n->tag = ps;                           // Save a bit of casting later
ps->pn = n;                            // Backpointer FROM stencil TO node
attrDecode aD;                         // Attribute decode functor - need later
vector<attrDecode::a6_t> av;           // Vector of unpacked stencil elements
                                       // Walk the XML attribute vector
for(vector<pair<string,string> >::iterator i=n->attr.begin();
    i<n->attr.end();i++) {
                                       // There should only be one, whose name
                                       // matches the element name, but just in
                                       // case, we iterate
  if (n->ename!=(*i).first) continue;  // Not for us - ignore
                                       // Unpack the stencil definition string
  av = aD((*i).second);
  WALKVECTOR(attrDecode::a6_t,av,k) {  // Look for 'A' stencil elements; these
                                       // attach to node n
    if (k==av.begin()) {               // The first is special - it's the input
      if ((*k).L!=0)                   // Dud string?
        fprintf(fo,"%u. Line %3u : \"%s\"\n%s~^~\n",
                ++g_ecnt,n->lin,(*k).S.c_str(),string(13+(*k).U,' ').c_str());
      continue;                        // NOT the first - it's special
    }
    if ((*k).C!='A') continue;         // Not an attribute (it's an element)
    ps->Add(*k);                       // Attach to the current node
    if ((*k).Ss.empty()) continue;     // Check the inner syntax *name*:
    if ((*k).Ss=="path") continue;     // "path" is OK
                                       // And so might be others.....
    fprintf(fo,"W. Line %3u : Unrecognised inner syntax name \"%s\" ignored\n",
                n->lin,(*k).Ss.c_str());
  }
}
// So far we've decoded the stencil element string to extract the XML attributes
// for the node n. Next we look at the children, to see how many of them are
// expected by the current node. Note we don't bother to even check for
// malformed stencil definitions, because they will be caught by the test in the
// parent when we walk back up the tree. It doesn't actually matter (I think)
// where we do the check, as long as we do it exactly once.
                                       // Walk the children, looking at their
                                       // stencil elements
WALKVECTOR(xmlTreeDump::node *,n->vnode,i) {
  typedef pair<string,string> s2_t;
  vector<s2_t> * pa = &((*i)->attr);   // Get the child attribute list
  WALKVECTOR(s2_t,(*pa),j) {           // Walk the child attribute list
    string _1 = (*j).first;            // The only XML attribute we care about
    string _2 = (*j).second;           // has a name matching n
                                       // XML attribute and element names agree?
    if ((*i)->ename!=(*j).first) continue;   // No - not for us
                                       // Unpack the definition stencil
    av = aD((*j).second);              // Vector of stencil elements
    WALKVECTOR(attrDecode::a6_t,av,k) {
      if (k==av.begin()) continue;     // NOT the first - it's special
      if ((*k).S.empty()) {            // String empty - the one we want
        (*k).S = (*i)->ename;          // Stuff element name back into stencil
        ps->Add(*k);                   // And load it into the stencil of n
      }
    }
  }
}

}

//------------------------------------------------------------------------------

void XValid::DeleteTag(xmlTreeDump::node * n)
// See comments in class destructor.
{
Stencil_t * ps = static_cast<Stencil_t *>(n->tag);
if (ps!=0) delete ps;
n->tag = 0;
WALKVECTOR(xmlTreeDump::node *,n->vnode,i) DeleteTag(*i);
}

//------------------------------------------------------------------------------

// NOTE: NOT A CLASS MEMBER; DEFINED HERE SO THE LINKER CAN FIND IT
// WITHOUT A FORWARD DECLARE

void GetTag(FILE * fp,void * p,const char * s)
{
fprintf(fp,"          %s(GetTag %" PTR_FMT ")\n",s,
        OSFixes::getAddrAsUint(p));
if (p==0) return;
Stencil_t * ps = static_cast<Stencil_t *>(p);
ps->Dump(fp,10,s);
}

//------------------------------------------------------------------------------

void XValid::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"%s\n",pline.c_str());
fprintf(fp,"\n%sXValid++++++++++++++++++++++++++++++++++++++\n",os);
xmlTreeDump::node::SetTag_cb(GetTag);  // Set tag callback in xml trees
if (!Validor.Empty()) {
  fprintf(fp,"%sGrammar %s defined :\n",os,Validor.Name().c_str());
  Validor.Dump(off+2,fp);
} else fprintf(fp,"%sGrammar undefined\n\n",os);
if (Validee!=0) {
  fprintf(fp,"%sClient %s defined :\n",os,Validee->Name().c_str());
  Validee->Dump(off+2,fp);
} else fprintf(fp,"%sClient undefined\n\n",os);
fprintf(fp,"%sTotal grammar error count (g_ecnt): %u\n",os,g_ecnt);
fprintf(fp,"%sTotal client error count  (c_ecnt): %u\n",os,c_ecnt);
fprintf(fp,"%sOutput             stream (    fo): %sdefined\n",os,fo==0?"NOT ":"");
fprintf(fp,"%sGrammar file input stream (   gfp): %sdefined\n",os,gfp==0?"NOT ":"");
fprintf(fp,"%sClient  file input stream (   cfp): %sdefined\n",os,cfp==0?"NOT ":"");
fprintf(fp,"%sCurrent wallclock counter (    t0): %lu msec\n\n",os,t0);
fprintf(fp,"%sXValid--------------------------------------\n\n",os);
fprintf(fp,"%s\n",sline.c_str());
fflush(fp);
}

//------------------------------------------------------------------------------

void XValid::ErrCnt(unsigned & rg, unsigned & rc)
// Export the grammar and client cumulative error counts
{
rg = g_ecnt;
rc = c_ecnt;
return;
}

//------------------------------------------------------------------------------

void XValid::Init(string s,FILE * ochan)
// Bits of initialisation that are common to all constructors
{
gfp     = 0;                           // Grammar file input stream
cfp     = 0;                           // Client file input stream
t0      = mTimer();                    // Start wallclock timer
c_ecnt  = 0;                           // So far, so good: no client errors
g_ecnt  = 0;                           // No grammar errors
//fo      = ochan;                       // Output stream for validation tree
Validee = 0;                           // No file to validate yet
Validor.Name(s);                       // Filename of validation grammar
SetOChan(ochan);                       // Set output stream
}

//------------------------------------------------------------------------------

void XValid::InitValidation()
// Initialise the validator structure. Quid custodiet ipsos custodes, and all
// that.
{
if (Validor.Name().empty()) return;    // No validator file is valid. Ho ho ho
gfp = fopen(Validor.Name().c_str(),"r");
if (gfp==0) g_ecnt++;                  // Specifying a non-existent file isn't
ReportGrammarStart();                  // User-facing
if (gfp!=0) {                          // Try and do something with it
  Validor.Transform(gfp);              // Build the validator tree
  fclose(gfp);                         // Done with grammar input file
}
g_ecnt += Validor.ErrCnt();            // Errors from that?
ValidorCheck(Validor.Root());          // Sanity check on Validor tree
                                       // Decorate grammar tree with stencils ?
if (g_ecnt==0) {                       // .. only if error free
  fprintf(fo,"Checking grammar definition inner syntax... \n\n");
  AddStencil(Validor.t_root);          // ...may add new errors
  if (g_ecnt==0) fprintf(fo,"\n...no inner syntax errors found\n\n");
  else fprintf(fo,"... %u inner syntax errors found\n",g_ecnt);
}
if (g_ecnt==0) ShowP(fo);              // Pretty-print precis
else {
  fprintf(fo,"\n%u errors found - best effort structure :\n\n",g_ecnt);
  ShowV(fo);                           // Pretty-print verbose
}
ReportGrammarEnd();                    // Pretty-print any validator errors
}

//------------------------------------------------------------------------------

unsigned XValid::ReportClientEnd(xmlTreeDump::node * me)
// Routine to report errors in checking the current client file. This is the
// pretty-print that goes out on the user stream, and contains both syntax and
// semantic error messages/
// Like a lot of things, it's recursive
{
if (fo==0) return c_ecnt;              // No O/P channel - return count anyway
if (Validee==0) return c_ecnt;         // No client file
if (me==0) {                           // Special case:
  if (c_ecnt!=0) {
    fprintf(fo,"\nErrors: %u. Best effort client tree after recovery:\n\n",c_ecnt);
    xmlTreeDump::node::SetTag_cb();    // Put ShowV into user-facing mode
    Validee->ShowV(fo);
  }
  else Validee->ShowP(fo);             // Pretty-print it
  me = Validee->t_root;                // Root of client tree
  fprintf(fo,"...Client file %s\n"" exhibits %u syntax errors in %lu msecs\n\n",
             Validee->Name().c_str(),c_ecnt,mTimer(t0));
  fprintf(fo,"%s\n%s %s\n\nChecking client file %s\n",
             dline.c_str(),GetDate(),GetTime(),Validee->Name().c_str());
  if (c_ecnt==0)
    if (g_ecnt==0) fprintf(fo,"against grammar file %s\n\n"
               "(lin,col) refers to element closure location in client file\n\n"
               "Structural analysis:\n\n",Validor.Name().c_str());
    else fprintf(fo,"...no grammar definition available\n\n");
  else fprintf(fo,"Semantic checking suppressed\n");
}
                                       // My stencil ?
Stencil_t * mes = static_cast<Stencil_t *>(me->tag);
                                       // If it's there AND not OK, bleat
if (mes!=0) if (!(mes->OK())) mes->Show(c_ecnt,me,fo);
                                       // Recurse down
WALKVECTOR(xmlTreeDump::node *,me->vnode,i) ReportClientEnd(*i);
if (me==Validee->t_root)               // Final return - epilogue
  fprintf(fo,"\n...Client file %s\n"
             " exhibits %u accumulated (syntax/structure) errors "
             "in %lu msecs\n\n%s\n\n",
             Validee->Name().c_str(),c_ecnt,mTimer(t0),sline.c_str());
fflush(fo);
return c_ecnt;
}

//------------------------------------------------------------------------------

unsigned XValid::ReportClientStart()
{
if (fo==0) return c_ecnt;              // No O/P channel - return count anyway
if (Validee==0) return c_ecnt;         // No client file
fprintf(fo,"%s\n%s %s\n\nChecking client file %s\n\n"
           "   (lin,col) refers to element closure location in client file\n\n"
           "Syntax analysis...\n\n",
           sline.c_str(),GetDate(),GetTime(),Validee->Name().c_str());
fflush(fo);
return c_ecnt;
}

//------------------------------------------------------------------------------

unsigned XValid::ReportGrammarEnd()
// Routine to report errors in the grammar definition phase.
{
if (fo==0) return g_ecnt;              // No output channel; return count anyway
fprintf(fo,"...Grammar file %s\n exhibits %u syntax errors in %lu msecs\n\n"
           "THE GRAMMAR FILE IS NOT CHECKED FOR STRUCTURAL "
           "OR SEMANTIC INTEGRITY\n\n%s\n\n",
           Validor.Name().c_str(),g_ecnt,mTimer(t0),sline.c_str());
fflush(fo);
return g_ecnt;
}

//------------------------------------------------------------------------------

void XValid::ReportGrammarStart()
// Routine to report errors in the grammar definition phase.
{
if (fo==0) return;                     // No output channel
fprintf(fo,"%s\n%s %s\n\nChecking XValidator grammar file %s",
           sline.c_str(),GetDate(),GetTime(),Validor.Name().c_str());
if (gfp==0) fprintf(fo," - not accessible / not specified\n");
else fprintf(fo,"\n"
           "   (lin,col) refers to element closure location in grammar file\n\n"
           "Syntax analysis...\n\n");
fflush(fo);
return;
}

//------------------------------------------------------------------------------

void XValid::SetOChan(FILE * _fp)
// Setter for the output channel for this object. It's not entirely trivial:
// we have to set the two xmlparser output channels as well
// This is a - compromise. Hell or high water, _fp should be !=0 and valid for
// write, but I just thought I'd check, and throw() if there was a problem.
// And it SEEMS that if you throw () over a stack frame that contains
// MPI_Finalize() in the destructor, then it (MPI_Finalize) hangs. I got nowhere
// for a couple of days, then went round it with the below. Not ideal, but
// choose your battles....
{
fo = _fp;                              // Cosmic output stream
if (_fp==0) {                          // ... not there ?
  printf("XValid::SetOChan has no valid output channel; switch to stdout\n");
  fflush(stdout);
  fo = stdout;
}
Validor.SetOChan(fo);                  // Auto subclass
if (Validee!=0) Validee->SetOChan(fo); // Dynamic subclass
}

//------------------------------------------------------------------------------

void XValid::ShowP(FILE * fp)
// Pretty-print Show precis of the entire class
{
Validor.ShowP(fp);
if (Validee!=0) Validee->ShowP(fp);
}

//------------------------------------------------------------------------------

void XValid::ShowV(FILE * fp)
// Pretty-print Show verbose of the entire class
{
xmlTreeDump::node::SetTag_cb();
Validor.ShowV(fp);
if (Validee!=0) Validee->ShowV(fp);
}

//------------------------------------------------------------------------------

void XValid::Validate(string s)
// Way in WITHOUT overwriting the output channel
// The client input to the validator. We assume that the grammar definition is
// in place and correct. If there's an (old) client file still around, we kick
// it out. Then we check that the validee file exists, and so on, and check the
// syntax. If there are problems, we report them, but we still attempt a
// validation against the grammar of whatever made sense before the syntax
// error.
{
if (g_ecnt!=0)
  fprintf(fo,"\nGrammar definition %s corrupt - cannot validate %s\n"
             "...syntax check only\n\n",
              Validor.Name().c_str(),s.c_str());

t0 = mTimer();                         // Start wallclock timer
if (Validee!=0) {                      // One already in place? then kill it
  DeleteTag(Validee->t_root);          // Delete the stencils
  delete Validee;                      // Delete the node tree
  Validee = 0;                         // Reset everything else
}
c_ecnt = 0;                            // No client errors
FILE * fp = fopen(s.c_str(),"r");      // Try to open it
if (fp==0) {                           // Nope
  if (fo!=0) fprintf(fo,"%s\n\nClient file %s cannot be opened\n\n%s\n",
                         sline.c_str(),s.c_str(),sline.c_str());
  fflush(fo);
  c_ecnt++;                            // Failure counts as an error
  return;                              // Nothing can be done
}
Validee = new xmlTreeDump();           // Check client syntax
Validee->SetOChan(fo);
Validee->Name(s);                      // Save the new client name
ReportClientStart();
c_ecnt += Validee->Transform(fp);      // Build the client tree - more errors?
fclose(fp);                            // Shut down input source stream
                                       // (Only) if still OK, do structure check
if ((c_ecnt==0)&&(g_ecnt==0)) Validate2();
fprintf(fo,"\n");
ReportClientEnd();
DeleteTag(Validee->t_root);            // Delete the stencils -> clean structure
}

//------------------------------------------------------------------------------

void XValid::Validate(string s,FILE * fc)
// Way in VIA overwriting the output channel
{
SetOChan(fc);                          // Change output channel
Validate(s);                           // Do the hard bit
}

//------------------------------------------------------------------------------

void XValid::Validate2(xmlTreeDump::node * me)
// Everything based around walking the client tree. The entire XValid class
// is centered around the logic contained in this method. Pay attention.
// We have two trees: grammar definition (rooted on Validor.t_root) and the
// client (rooted on Validee->t_root).
// We recursively walk the client tree (note recursive call to Validate2 at the
// end of this method definition). At each node in the client tree:
// Create and attach a Stencil_t object. This contains a set of stencil
// elements, which are a copy of the relevant stencil on the definition tree.
// Each of the stencil elements contains a condition that must be satisfied for
// the client to be valid. The stencil is attached to the node tree by the
// void * tag field, which is a pain in the bum and I should have used
// polymorphism and turned the TreeDumper into a template, but I didn't. Bad
// decision; hence the static_casts all over the place.
// Links FROM client TO definition trees are embodied in Stencil_t.pn.
// Then we look for the corresponding node in the definition tree:
// Go to client node parent (must exist).
// Go to client node parent stencil (must exist).
// Go to definition node (may not exist).
// Search children of parent definition to find match (definition) for me.
// ...Oh, read the damn code - or even the documentation. It's fiddly.
{
if (me==0) {                           // We're just starting
  Stencil_t * Rs = new Stencil_t();    // Create a new stencil
  Validee->t_root->tag = Rs;           // Add the stencil to the root
  Rs->Refpn() = Validor.t_root;        // Back link client root -> defn root
                                       // Corner case - empty client
  if (Validee->t_root->vnode.empty()) return;
  Validate2(Validee->t_root->vnode[0]);// Start validation proper on client tree
  return;
}
                                       // Here we're in recursion land
Stencil_t * mes = new Stencil_t();     // Create new empty client stencil for me
me->tag = mes;                         // Attach it to current client node
                                       // Locate the relevant definition node:
xmlTreeDump::node * mp = me->par;      // My (client) parent
                                       // My (client) parent stencil
Stencil_t * mps = static_cast<Stencil_t *>(mp->tag);
xmlTreeDump::node * pd = mps->Refpn(); // My (grammar) parent definition node
xmlTreeDump::node * md = 0;
                                       // Grammar node for me (may not exist)
if (pd!=0) md = pd->FindChild(me->ename);
                                       // Find 'me' stencil element in client
                                       // parent
attrDecode::a6_t * pstel = mps->FindStel(me->ename);

if (md==0) {                           // If no grammar node for 'me'...
                                       // If no pre-existing stencil element,
                                       // add one in to the client stencil
  if (pstel==0) mps->Add(attrDecode::a6_t('E',me->ename,0,0,1));
  else pstel->V++;                     // Otherwise increment the 'actual' count
} else {                               // Yes, there is a definition node
  mes->Refpn() = md;                   // Link me to my definition
                                       // My definition stencil (may not exist)
  Stencil_t * mds = static_cast<Stencil_t *>(md->tag);
  mes->CopyOf(mds);                    // Copy my defn stencil -> client (me)
  mes->Update(me);                     // See if my stencil matches reality
}
// That's the fiddly bit *building* the datastucture for the stencil done; now
// actually *do* something with it.
// 1. Check any attribute value inner syntaxes
                                       // Walk attribute defn list in stencil
WALKVECTOR(attrDecode::a6_t,mes->a6_v,i) {
  if ((*i).Ss.empty()) continue;       // No attribute grammar? ignore
  vector<string> strv;                 // ** Need to make this grammar-specific?
  if ((*i).Ss=="path") {               // Now locate actual attribute(s)
    pathDecode pD;                     // Syntax decoder for "path"
                                       // Walk the client attribute list
    for(vector<pair<string,string> >::iterator j=me->attr.begin();
        j!=me->attr.end();j++){
      if ((*j).first==(*i).S) {        // Is there a grammar for this attribute?
        strv = pD((*j).second);        // Yes - syntax check the string
        if (!strv[0].empty()) {        // Errors found? Bleat mightily
          string Os(str2uint(strv[0]),' ');
          Os += "~^~";                 // Pretty-print stuff
          fprintf(fo,"%u. (%3d,%2d): Inner syntax (\"%s\") failure in attribute"
                     " \"%s\"\n             %s\n           %s\n",
                    ++c_ecnt,me->lin,me->col,(*i).Ss.c_str(),(*j).first.c_str(),
                    (*j).second.c_str(),Os.c_str());
        } // if (!strv[
      } // if ((*j).
    } // for(vector<pair,
    continue;                          // Done checking inner syntax "path"
  } // if ((*i).
//  fprintf(fo,"W. (%3d,%2d): Unknown inner syntax \"%s\" ignored\n",
//             me->lin,me->col,(*i).Ss.c_str());
} // WALKVECTOR(attrDecode::

if (me->cfrag.empty()) me->cfrag = "// " + me->FullName() + " not defined\n";

// 2. If the grammar says there's a CFRAG, there has to be a CFRAG
//if (md!=0) {                           // Maybe no grammar node anyway?
//  string gCDATA = md->cfrag;           // Go get grammar CFRAG
//  string cCDATA = me->cfrag;           // Go get client CFRAG
                                       // If grammar=no and client=yes, warn
//  if (!(cCDATA.empty())&&gCDATA.empty()) {
//    fprintf(fo,"W. (%3d,%2d): Element %s has unexpected "
//            "CDATA \"%s...\" (%lu chars) - ignored in subsequent processing\n",
//            me->lin,me->col,me->FullName().c_str(),
//            string(cCDATA.c_str(),10).c_str(),cCDATA.size());
//    me->cfrag.clear();                 // It didn't orta be there
//  }
                                       // If grammar=yes, prepend g to c
//  if (!gCDATA.empty())me->cfrag = gCDATA + string("\n") + cCDATA;
//}

//mes->Dump();
fflush(fo);                            // Walk on down
WALKVECTOR(xmlTreeDump::node *,me->vnode,i) Validate2(*i);
}

//------------------------------------------------------------------------------

void XValid::ValidorCheck(xmlTreeDump::node * n)
// Encapsulates validity checks on the grammar tree.
// It's passed syntax and structure, now we look for daftness.
// Nothing here causes errors - just warnings, plus remedial action
// Having said that, there's only one check....
{
// Ensure that the nodes on a given level all have unique names.
// It's quadratic, but this is the grammar tree, so it's quite small.
WALKVECTOR(xmlTreeDump::node *,n->vnode,i) {
  if ((*i)==0) continue;
  string iname = (*i)->ename;
  for(vector<xmlTreeDump::node *>::iterator j=i+1;j!=n->vnode.end();j++) {
    if ((*j)==0) continue;
    if (iname!=(*j)->ename) continue;  // All good ?
    fprintf(fo,"W. (%3d,%2d): Element \"%s\" duplicated in grammar definition "
            "- only the first instance is significant\n",
            n->lin,n->col,(*j)->FullName().c_str());
    delete (*j);                       // Kill the tree branch
    *j = 0;                            // Set the child pointer to NULL
  }
}
// Structure is untouched, but there may be null children in it
vector<xmlTreeDump::node *> tmp = n->vnode;   // Safe copy
n->vnode.clear();                      // Clear it all
                                       // Copy non-nulls back
WALKVECTOR(xmlTreeDump::node *,tmp,i) if ((*i)!=0) n->vnode.push_back(*i);
                                       // And down we go
WALKVECTOR(xmlTreeDump::node *,n->vnode,i) ValidorCheck(*i);
if (n==Validor.Root()) fprintf(fo,"\n");
}

//------------------------------------------------------------------------------
