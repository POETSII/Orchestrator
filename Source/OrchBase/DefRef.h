#ifndef __DefRefH__H
#define __DefRefH__H
//==============================================================================
/* Base class to handle referencing/definitipon counting for its derived ....
... sibling? parent? ancestor? derivative? Whatever.
*/
//==============================================================================

#include <stdio.h>

class DefRef
{
public:
              DefRef();
virtual ~     DefRef();

void          ClrD() { def = 0; }      // Clear definition count
void          ClrR() { ref = 0; }      // Clear reference count
void          Def(int);                // Increment def (may be -ve); clamp to 0
unsigned      Def() { return def; }    // Guess
void          Dump(FILE * = stdout);
void          Ref(int);                // Increment ref (may be -ve); clamp to 0
unsigned      Ref() { return ref; }

private:
unsigned      def;
unsigned      ref;

};

//==============================================================================
   
#endif
