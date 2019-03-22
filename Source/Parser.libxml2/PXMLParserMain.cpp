#include <fstream>
#include "P_PparserTest.h"

int main(int argc, char** argv)
{
  int retval;
  if (argc < 2 || argc > 3)
  {
     cout << "Usage: PXMLParser <input xml file> [<output file>]";
     return 0;
  }
  xmlTextReaderPtr inXML = xmlNewTextReaderFilename(argv[1]);
  if (argc == 3)
  {
     ofstream logfile(argv[2]);
     P_PparserTest parser(inXML,logfile);
     retval = parser.ParseDocument();
  }
  else
  {
     P_PparserTest parser(inXML);
     retval = parser.ParseDocument();
  }
  if (!xmlTextReaderClose(inXML)) return retval;
  else return retval*-1;
}
