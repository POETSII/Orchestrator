#ifndef __messages1__H
#define __messages1__H

#include <vector>
using namespace std;

#define MESS Messages1::Add

//==============================================================================

class Messages1
{
public:
                Messages1();
virtual ~       Messages1(void);
static void     Add(int ...);
void            Dump();

private:
static vector<int>     messages1;

};

//==============================================================================

#endif
