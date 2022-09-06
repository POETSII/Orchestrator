#ifndef __Pserver_tH__H
#define __Pserver_tH__H

#include "PcsBase_t.h"
#include <vector>
#include <string>
using namespace std;
typedef        unsigned char byte;

//==============================================================================
/* This encapsulates the server-side socket-opening job. I know these exist
already, but one learning curve at a time, eh?
It is tempting to just whack everything into the constructor, but error handling
indicates a more nuanced approach. The use order is:
1. The constructor does very little.
2. The user sets up the error and incoming message callback functions
3. The method DoIt() does all the heavy lifting: If there's a problem, the error
subsystem handles it; if everything is tickety-spong, the message callback is
invoked.
4. When you're done, everything just goes out of scope and closes down sensibly
*/
//==============================================================================

class Pserver_t : public PcsBase_t
{
public:
               Pserver_t();
virtual ~      Pserver_t();
void           Close();
int            Recv(unsigned);
int            Send(int,vector<byte>);
void           SetPCB(void(*)(int,void *,vector<byte>,int)=0);

                                       // Pointer to incoming packet handler
void (*        pPCB)(int,void *,vector<byte>,int);
int            sockfd;                 // Listening socket
int            newsockfd;              // Transient new connection socket
};

//------------------------------------------------------------------------------

void           DefPCB(int,void *,vector<byte>,int);
void *         Recv0(void *);
void *         Recv1(void *);

//==============================================================================

#endif
