#include <iostream>
#include <stdio.h>
#include "P_Graphs.h"
#include "Environment.h"
#include "orchbasedummy.h"

int main(int argc, char** argv)
{
  cout << "Attach debugger to PXMLParser process" << GetPID() << "...\n";
  cout.flush();
  getchar();
  int retval;
  if (argc < 2 || argc > 3)
  {
     cout << "Usage: PGraphsParser <input xml file> [<output file>]";
     cout.flush();
     return 0;
  }
  OrchBase* orchestrator = new OrchBaseDummy(argc,argv,string("OrchBase"),string("OrchBase.cpp"));
  FILE* logfile = stdout;
  P_Graphs parser(orchestrator,argv[1]);
  if (argc == 3) logfile = fopen(argv[2],"w");
  if (!(retval = parser.ParseDocument())) orchestrator->Dump(logfile);
  retval |= parser.CloseParser();
  delete orchestrator;
  return retval*-1;
}
