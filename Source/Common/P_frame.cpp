
#include "P_frame.h"

//==============================================================================
/* A light-weight container-of-containers that allows the user to parcel up
multiple messages for insertion into the POETS ecosystem.
From the user perspective, you have a stack of strings (which UserI will
interpret as machine name, user name and so on) and a stack of PMsg_p (in
streamed form). Depending on their keys, these PMsg_ps will be forwarded to
diverse components within the POETS network.
The strings are only ever seen/used by UserI and UserO. We could play nested
encapsulation games here but life is too short.
The stacks are true stacks (the user can push and pop and that's it) but they
are internally represented as stl vectors, so this is easily changed if we want.
All of which means..... you don't get random access.
*/
//==============================================================================

P_frame::P_frame()
{
}

//------------------------------------------------------------------------------

P_frame::P_frame(byte * pb)
// Unstream from a byte vector that appears from somewhere.
{
if (pb==0) return;                     // Sanity check 1.
if (Msg_p::Sizeof(pb)==0) return;      // Sanity check 2.


}

//------------------------------------------------------------------------------

P_frame::~P_frame(void)
{
WALKVECTOR(Msg_p *,Msg_pv,i) delete *i;
}

//------------------------------------------------------------------------------

void P_frame::AddS(string & rs)
// Save a copy of the string
{
stringv.push_back(rs);
}

//------------------------------------------------------------------------------

void P_frame::AddM(Msg_p & rm)
// Save a copy of the message
{
Msg_pv.push_back(new Msg_p(rm));
}

//------------------------------------------------------------------------------

unsigned P_frame::Crc()
{
return 0;
}

//------------------------------------------------------------------------------

void P_frame::Dump(FILE * fp)
{
fprintf(fp,"P_frame+++++++++++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"String vector  (%u entries):\n",stringv.size());
WALKVECTOR(string,stringv,i) fprintf(fp,"%s\n",(*i).c_str());
fprintf(fp,"Message vector (%u entries):\n",Msg_pv.size());
WALKVECTOR(Msg_p *,Msg_pv,i) (*i)->Dump(fp);
fprintf(fp,"P_frame---------------------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void P_frame::PopS()
// Chuck out the last string in
{
stringv.pop_back();
}

//------------------------------------------------------------------------------

void P_frame::PopM()
// Chuck out the last message in
{
if (Msg_pv.empty()) return;            // Nothing to do
delete Msg_pv.back();                  // Delete the message
Msg_pv.pop_back();                     // Hose the carrier
}

//------------------------------------------------------------------------------

unsigned P_frame::PullU(byte * pb,unsigned off)
{
return 0;
}

//------------------------------------------------------------------------------

void P_frame::PushS(vector<byte> & rv,string str)
// Routine to attach the string str to the end of the byte vector rv as a
// Hollerith literal
{
unsigned len = str.size();
PushU(rv,len);
for (unsigned i=0;i<len;i++) rv.push_back(str[i]);
}

//------------------------------------------------------------------------------

void P_frame::PushU(vector<byte> & rv,unsigned u)
// Routine to attach the the unsigned u to the end of the byte vector rv
// We spit in the face of strong typing.
{
const int s = sizeof(u);
byte buff[s];                          // It's all compile-time....
new(buff) unsigned(u);
for(unsigned i=0;i<s;i++) rv.push_back(buff[i]);
}

//------------------------------------------------------------------------------

unsigned P_frame::Sizeof(byte * pb)
{
if (pb==0) return 0;
return PullU(pb,0);
}

//------------------------------------------------------------------------------

byte * P_frame::Stream()
{
static vector<byte> vm;
unsigned tmp = 0;
PushU(vm,tmp);                         // Placeholder for stream length

printf("\n");
WALKVECTOR(byte,vm,i) printf("%02x ",*i);
printf("\n");

PushU(vm,tmp);                         // Placeholder for CRC
printf("\n");
WALKVECTOR(byte,vm,i) printf("%02x ",*i);
printf("\n");

PushU(vm,stringv.size());              // Number of strings to come
printf("\n");
WALKVECTOR(byte,vm,i) printf("%02x ",*i);
printf("\n");

WALKVECTOR(string,stringv,i) PushS(vm,*i);           // One string at a time
printf("\n");
WALKVECTOR(byte,vm,i) printf("%02x ",*i);
printf("\n");

PushU(vm,Msg_pv.size());               // Number of messages to come
printf("\n");
WALKVECTOR(byte,vm,i) printf("%02x ",*i);
printf("\n");

WALKVECTOR(Msg_p *,Msg_pv,i) {         // One message at a time
  byte * pb = (*i)->Stream();          // Stream it
  unsigned lb = Msg_p::Sizeof(pb);     // Measure it
  for(unsigned i=0;i<lb;i++) vm.push_back(*(pb+i));  // Copy to frame stream
printf("\n");
WALKVECTOR(byte,vm,i) printf("%02x ",*i);
printf("\n");

}
new(&vm[0]) unsigned(vm.size());       // Overwrite stream length
printf("\n");
WALKVECTOR(byte,vm,i) printf("%02x ",*i);
printf("\n");

new(&vm[1]) unsigned(Crc());           // Overwrite frame CRC
printf("\n");
WALKVECTOR(byte,vm,i) printf("%02x ",*i);
printf("\n");

return &vm[0];                         // And we're done
}

//==============================================================================

