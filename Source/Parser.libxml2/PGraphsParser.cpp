#include <iostream>
#include <stdio.h>
#include "P_Graphs.h"
#include "Environment.h"
#include "OrchBase.h"

int main(int argc, char** argv)
{
  cout << "Attach debugger to OrchParser process " << GetPID() << "...\n";
  cout.flush();
  getchar();
  int retval = -1;
  if (argc < 2 || argc > 3)
  {
     cout << "Usage: OrchParser <input xml file> [<output file>]";
     cout.flush();
     return 0;
  }
  OrchBase* orchestrator = new OrchBase(); //(argc,argv,string("OrchBase"),string("OrchBase.cpp"));
  FILE* logfile = stdout;
  P_Graphs parser(orchestrator,argv[1]);
  if (argc == 3) logfile = fopen(argv[2],"w");
  retval = parser.ParseDocument();
  //
  if (retval)
  {
     cout << "Parser error at line " << xmlTextReaderGetParserLineNumber(parser.GetParser()) << " column " << xmlTextReaderGetParserColumnNumber(parser.GetParser()) << ": " << retval << "\n";
     cout << "Node type encountered was " << xmlTextReaderNodeType(parser.GetParser()) << "\n";
     cout.flush();
  }
  else orchestrator->Dump(logfile);
  // if (!(retval = parser.ParseDocument())) orchestrator->Dump(logfile);
  retval |= parser.CloseParser();
  delete orchestrator;
  return retval*-1;
}
