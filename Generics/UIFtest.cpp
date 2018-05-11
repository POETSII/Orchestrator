//---------------------------------------------------------------------------

#include <stdio.h>
#include "flat.h"
#include "uif_draw.h"
#include "uif.h"
#include "macros.h"

//==============================================================================

void error(void *);
void DumpV(vector<string>);

//==============================================================================

int main(int argc, char* argv[])
{
printf("Hello, world .. %d arguments\n",argc);

UIF_DRAW c2;

if (argc<=1) {
  printf("No supplied arguments");
  return 0;
}

vector<UIF_DRAW::DC> stuff;

FILE * f = fopen("ping.pong","w");
if (f==0) printf("Cannot open dump target file\n");
c2.SetOFP(f);
for (int i=1;i<argc;i++) {
  printf("Playing with %s.....\n",argv[i]);
  c2.Add(string(argv[1]));
  c2.Geometry();
  c2.Draw(stuff,c2.Root());
//  c2.Dump();
}

WALKVECTOR(UIF_DRAW::DC,stuff,i) (*i).Dump(f);

fclose(f);

/*
UIF cli;
cli.Add(argv[1]);
cli.Save("stuff.txt");

UIF cli0(argc,argv);
cli0.Dump("ping.pong");
cli0.Save("ding.dong");
*/

return 0;

}

//------------------------------------------------------------------------------

void error(void * p)
{
static int count = 0;                  // Count the errors
printf("Error count now %d\n",++count);

UIF * pN = static_cast<UIF *>(p);      // Find and dump the error token
pN->Td.Dump();
printf("-------------------------\n");
}

//------------------------------------------------------------------------------

void DumpV(vector<string> ss)
{
printf("-------------------------\n");
WALKVECTOR(string,ss,i) printf("%s\n",(*i).c_str());
printf("-------------------------\n");
}

//------------------------------------------------------------------------------
