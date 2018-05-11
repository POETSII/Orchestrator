#ifndef __modulo__H
#define __modulo__H

//==============================================================================

class Modulo {

public:
                   Modulo(long,long);
                   Modulo(Modulo &);
virtual ~          Modulo(void);
void               Dump();

long               val;
long               mod;

};

//==============================================================================
   
#endif
