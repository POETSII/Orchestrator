#include "dumpchan.h"

FILE * DumpChan::dfp = stdout;

//==============================================================================

void DumpChan::Dump(FILE * fp)
{
fprintf(fp,"DumpChan++++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"Dump file channel = 0x%p\n",dfp);
fprintf(fp,"DumpChan------------------------------------\n");
fflush(fp);
}

//==============================================================================
