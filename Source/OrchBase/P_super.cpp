//------------------------------------------------------------------------------

#include "P_super.h"
#include "P_link.h"
#include "P_port.h"
#include "P_graph.h"

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

void P_super::Attach(P_box * pB)
// To attach a supervisor device to a box
{
                                         // Already attached?
WALKVECTOR(P_box *,P_boxv,i) if ((*i)==pB) return;
P_boxv.push_back(pB);                    // Forward pointer
                                         // FROM all enclosed boards TO this
                                         // supervisor
WALKVECTOR(P_board *,pB->P_boardv,i) (*i)->pSup = this;
}

//------------------------------------------------------------------------------

void P_super::Detach(P_box * pB)
// To disconnect this supervisor from a particular box
{
                                         // FROM all enclosed boards TO this
                                         // supervisor
WALKVECTOR(P_board *,pB->P_boardv,i) (*i)->pSup = 0;
                                         // This supervisor TO the boxes
WALKVECTOR(P_box *,P_boxv,i) if ((*i)==pB) {
  P_boxv.erase(i);                       // Subsequent iteratrs invalid;
  return;                                // no choice but to go now
}
}

//------------------------------------------------------------------------------

void P_super::Detach()
// To disconnect this supervisor from all boxes. We have to get a bit sneaky,
// because we can't - ostensibly - see the box vector from here. But.....
{
if (P_boxv.empty()) return;            // It's not connected to anything anyway
P_box * pB = P_boxv[0];                // Gives us *a* box
P_graph * pG = pB->par;                // Which gives us the topology graph
WALKPDIGRAPHNODES(unsigned,P_box *,unsigned,P_link *,unsigned,P_port *,pG->G,i){
  pB = pG->G.NodeData(i);              // And now we're looping.....
  WALKVECTOR(P_board *,pB->P_boardv,j) // Walk the boards (ho ho ho)
    if ((*j)->pSup==this) (*j)->pSup = 0;
}
P_boxv.clear();                        // And last of all.....
}

//------------------------------------------------------------------------------

void P_super::Dump(FILE * fp)
{
fprintf(fp,"P_super+++++++++++++++++++++++++++++++++\n");
fprintf(fp,"Supervisor attached to :\n");
WALKVECTOR(P_box *,P_boxv,i) fprintf(fp,"%s\n",(*i)->FullName().c_str());
P_device::Dump(fp);
fprintf(fp,"P_super---------------------------------\n");
fflush(fp);
}

//==============================================================================



