#ifndef __DUMPCHAN__H
#define __DUMPCHAN__H

#include <stdio.h>
#include <stdint.h>

//==============================================================================

class DumpChan
// DumpChan is a base class of the digraph, and intended to be the base of all
// the classes stored in it. This is so that the pretty-printer can find an
// output stream. G.DumpChan() sets the static channel value, and the static
// callbacks in the stored classes pick it up, and write to it when G.Dump()
// is called.
{
public:
              DumpChan(){}
virtual ~     DumpChan(){}
void          Dump(FILE * = stdout);
static FILE * dfp;                     // Dumpfile channel
};

//==============================================================================

#endif
