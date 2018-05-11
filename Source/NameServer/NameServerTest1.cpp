//------------------------------------------------------------------------------

#include <stdio.h>
#include "string.h"
#include "Ns_el.h"

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{

printf("Hello, world\n");
string s1 = string("one");         string s2 = string("two");
string s3 = string("three");       string s4 = string("four");
string s5 = string("five");        string s6 = string("six");
string s7 = string("seven");       string s8 = string("eight");
string s9 = string("nine");        string s0 = string("ten");
string sD = string("DEVICE");
string sS = string("SUPERVISOR");
string sT = string("TASK");
string sO = string("OWNER");
string sX = string("THING");


Ns_el * pNs_el = new Ns_el;

vector<string> vin,vint,vou,vout;
vin.push_back(s1);
vin.push_back(s2);
vin.push_back(s3);
vint.push_back(s0);
vint.push_back(s0);
vint.push_back(s0);
vou.push_back(s4);
vou.push_back(s5);
vout.push_back(s8);
vout.push_back(s0);


pNs_el->PutD(sD,sX,sS,sT,sO,vin,vint,vou,vout,101,P_addr_t(1,2,3,4,5),0x123,22);
pNs_el->PutD(sD,sX,sS,sT,sO,vin,vint,vou,vout,100,P_addr_t(1,2,3,4,5),0x123,22);
pNs_el->PutS(sS,sT,sO,199,P_addr_t(51,52,53,54,55),0xfff,44);
pNs_el->PutS(sD,sX,sS,98,P_addr_t(61,62,63,64,65),777,55);
pNs_el->PutT(sT,sO,97);
pNs_el->PutT(sT,sO+sT,96);
pNs_el->PutO(sO,95);

pNs_el->Dump();

Ns_el N2(pNs_el->Stream(),pNs_el->Length());

N2.Dump();

vector<Ns_0 *> * vE = N2.Construct();

WALKVECTOR(Ns_0 *,(*vE),i) (*i)->Dump();

WALKVECTOR(Ns_0 *,(*vE),i) delete *i;

int cnt;
unsigned char * pEntity = N2.Get<unsigned char>(99,cnt);
unsigned char Entity = 'X';
if (pEntity!=0) Entity = *pEntity;
Ns_el::Dindex_t * pDindex = N2.Get<Ns_el::Dindex_t>(99,cnt);
vector<string> vs;
N2.GetX(99,vs);

WALKVECTOR(string,vs,i)printf("%s\n",(*i).c_str());
if (pDindex!=0) {
  pDindex->addr.Dump();
  printf("Entity=%c\nkey=%u\nattr=%u\nbin=%u\nsizeof(inpins)=%u\nsizeof(oupins)=%u\n",
  Entity,pDindex->key,pDindex->attr,pDindex->bin,pDindex->inP/2,pDindex->ouP/2);
}

delete pNs_el;

getchar();
return 0;
}

//------------------------------------------------------------------------------
