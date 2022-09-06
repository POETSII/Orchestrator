#ifndef __PcsBase_tH__H
#define __PcsBase_tH__H

#include <vector>
#include <string>
using namespace std;
typedef        unsigned char byte;

//==============================================================================
/* Client-server base class. It has in it the things that are common to both:
1. The skyhook and its setter
2. The error subsystem and the reporting code
3. Non-windoze utilities (array->vector)
*/
//==============================================================================

class PcsBase_t
{
public:
               PcsBase_t();
virtual ~      PcsBase_t();

int            PsMessage(int,string);
int            PsMessage(int,string,int);
void           SetMCB(void(*)(void *,string)=0);
void           SetSkyHook(void * _S) { pSkyHook = _S; }
int            Status(vector<string> &);

void *         pSkyHook;               // Callback skyhook
void (*        pMCB)(void *,string);   // Pointer to message callback
vector<string> messBuf_v;              // Message strings
};

void           DefMCB(void *,string);  // Default message callback

//==============================================================================

#endif
