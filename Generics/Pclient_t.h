#ifndef __Pclient_tH__H
#define __Pclient_tH__H

#include "PcsBase_t.h"
#include <stdio.h>
#include <vector>
#include <string>
using namespace std;
typedef        unsigned char byte;

//==============================================================================

//==============================================================================

class Pclient_t : public PcsBase_t
{
public:
               Pclient_t();
virtual ~      Pclient_t();
void           Close();
void           SetPCB(void(*)(int,void *,vector<byte>,int)=0);
int            Send(vector<byte>);
int            Start(string,unsigned);

                                       // Pointer to incoming packet handler
void (*        pPCB)(int,void *,vector<byte>,int);
int            sockfd;
};

void *         Recv(void *);

//==============================================================================

#endif
