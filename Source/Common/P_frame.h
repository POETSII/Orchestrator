#ifndef __P_frame__H
#define __P_frame__H

#include "Msg_p.hpp"
#include <string>
using namespace std;

//==============================================================================

typedef unsigned char byte;

class P_frame {
public :
                 P_frame();            // Empty constructor
                 P_frame(byte *);      // Unstreaming constructor
virtual ~        P_frame(void);        // Kill! Maim! Annihilate"

void             AddS(string &);       // Push a string
void             AddM(Msg_p &);        // Push a message
unsigned         Crc();
void             Dump(FILE * = stdout);
void             PopS();               // Pop a string
void             PopM();               // Pop a message
static unsigned  PullU(byte *,unsigned);
void             PushS(vector<byte> &,string);
void             PushU(vector<byte> &,unsigned);
static unsigned  Sizeof(byte *);
byte *           Stream();             // Stream the whole thing

private:
vector<string>   stringv;              // String store
vector<Msg_p *>  Msg_pv;               // Message store
//vector<byte *>   vm;                   // Streamed version
};

//==============================================================================

#endif

