//------------------------------------------------------------------------------

#include "P_super.h"
#include "P_link.h"
#include "P_port.h"
#include "PoetsEngine.h"

//==============================================================================

P_super::P_super():P_device()
{

}

//------------------------------------------------------------------------------

P_super::P_super(string s):P_device(0,s)
{

}

//------------------------------------------------------------------------------

P_super::~P_super()
{

}

//------------------------------------------------------------------------------

void P_super::Attach(PoetsBox * pB)
// To attach a supervisor device to a box
{
                                         // Already attached?
WALKVECTOR(PoetsBox *,P_boxv,i) if ((*i)==pB) return;
PoetsBoxv.push_back(pB);                    // Forward pointer
                                         // FROM all enclosed boards TO this
                                         // supervisor
WALKVECTOR(PoetsBoard *,pB->P_boardv,i) (*i)->pSup = this;
}

//------------------------------------------------------------------------------

void P_super::Detach(PoetsBox * pB)
// To disconnect this supervisor from a particular box
{
                                         // FROM all enclosed boards TO this
                                         // supervisor
WALKVECTOR(PoetsBoard *,pB->P_boardv,i) (*i)->pSup = 0;
                                         // This supervisor TO the boxes
WALKVECTOR(PoetsBox *,P_boxv,i) if ((*i)==pB) {
  PoetsBoxv.erase(i);                       // Subsequent iteratrs invalid;
  return;                                // no choice but to go now
}
}

//------------------------------------------------------------------------------

void P_super::Detach()
// To disconnect this supervisor from all boxes. We have to get a bit sneaky,
// because we can't - ostensibly - see the box vector from here. But.....
{
if (PoetsBoxv.empty()) return;            // It's not connected to anything anyway
PoetsBox * pB = P_boxv[0];                // Gives us *a* box
PoetsEngine * pG = pB->par;                // Which gives us the topology graph
WALKPDIGRAPHNODES(unsigned,PoetsBox *,unsigned,P_link *,unsigned,P_port *,pG->G,i){
  pB = pG->G.NodeData(i);              // And now we're looping.....
  WALKVECTOR(PoetsBoard *,pB->P_boardv,j) // Walk the boards (ho ho ho)
    if ((*j)->pSup==this) (*j)->pSup = 0;
}
PoetsBoxv.clear();                        // And last of all.....
}

//------------------------------------------------------------------------------

void P_super::Dump(FILE * fp)
{
fprintf(fp,"P_super+++++++++++++++++++++++++++++++++\n");
fprintf(fp,"Supervisor attached to :\n");
WALKVECTOR(PoetsBox *,P_boxv,i) fprintf(fp,"%s\n",(*i)->FullName().c_str());
P_device::Dump(fp);
fprintf(fp,"P_super---------------------------------\n");
fflush(fp);
}

//==============================================================================



