#ifndef __P_deviceH__H
#define __P_deviceH__H

#include <stdio.h>
#include "NameBase.h"
#include "pdigraph.hpp"
#include "P_addr.h"
class D_graph;
class PoetsThread;
class P_devtyp;
class CFrag;

//==============================================================================

class P_device : public NameBase, protected DumpChan
{
public:
                   P_device(D_graph * =0, string = string());
virtual ~          P_device();
void               Dump(FILE * = stdout);
static void        DevDat_cb(P_device * const &);
static void        DevKey_cb(unsigned const &);
vector<string>     NSGetinpn();
vector<string>     NSGetinpt();
vector<string>     NSGetoupn();
vector<string>     NSGetoupt();
void               Par(D_graph * _p);
void               Unlink();

D_graph *          par;
PoetsThread *      pP_thread;
P_devtyp *         pP_devtyp;
CFrag *            pPropsI;            // Device properties initialiser code
CFrag *            pStateI;            // Device state initialiser code
unsigned int       idx;                // Index in the graph
unsigned           attr;               // The eponymous attribute
/* this looks like a hack, and maybe it is. When creating edges, in the
   actual on-chip implementation, messages are to be sent using a header
   which encodes a (destDevice, destPin, destEdgeIndex) triplet. The catch
   is, the sending device needs to know this, and when we are generating
   output files, we need to be able to access information that would (normally)
   only be available at the receiving device. The easiest way to achieve
   this is to embed the (destPin, destEdgeIndex) pair into the edge (i.e. arc)
   involved in the graph. This can be done by making the destination pin index
   (the fani key for the pin) a concatenation of the 2 indices - we can break
   apart the indices whilst doing the file generation and conveniently by
   appending the destEdgeIndex onto the key we ensure that the sort order of
   pins with identical pin indices will come out in the order we want. Generating
   these keys *could* be done by counting through the number of pins within
   the bounded range set by
   [G.index_n[dest_device].fani.lower_bound(pin << PIN_POS),
    G.index_n[dest_device].fani.upper_bound(pin << PIN_POS | 0xFFFFFFFF >> (32-PIN_POS)))
   but this is not scalable (we have to do num_indices traversals through the
   pin list for each pin on each device). Much easier simply to store an index
   count and grab the next available index. But if you have a better solution,
   by all means put it in here, or whereever...
*/
map<unsigned int, unsigned int> ipin_idxs;

};

//==============================================================================

#endif
