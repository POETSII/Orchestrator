//---------------------------------------------------------------------------

#include "messages1.h"
#include "macros.h"
#include <stdio.h>

vector<int> Messages1::messages1;        // BORLAND linker bug

//==============================================================================

Messages1::Messages1()
{
printf("Hello, world\n");
}

//------------------------------------------------------------------------------

Messages1::~Messages1()
{
}

//------------------------------------------------------------------------------

void Messages1::Add(int e ...)
// Add a new message report
{
messages1.push_back(e);                 // Store it
printf("\nand yay, the nascent message handler intones %d\n",e);
}

//------------------------------------------------------------------------------

void Messages1::Dump()
{
printf("Message handler dump:\n");
WALKVECTOR(int,messages1,i) printf(" %d\n",*i);
}

//------------------------------------------------------------------------------

