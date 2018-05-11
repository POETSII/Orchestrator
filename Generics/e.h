#ifndef __EH__H
#define __EH__H

//==============================================================================

class E
// Exception wrapper
{
public:
E(char * s_,int i_):s(s_),i(i_){}
void Dump() {printf("\n\nE() exception from %s(line %d) ...\n",s,i);}
char * s;
int i;
};

//==============================================================================

#endif
