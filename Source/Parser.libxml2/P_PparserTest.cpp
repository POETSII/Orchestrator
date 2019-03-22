#include "P_PparserTest.h"

// lists of allowable tag values. Unfortunately C++98 syntax for initializing
// static const STL containers is extremely oblique. C++11 improves upon this,
// although it's not clear that it improves upon the problem of generating
// a static initializer (function). Surely this shouldn't be this hard?
// However as long as the lists are small this probably isn't too terribly bad.

const char* P_PparserTest::conts_array[5] = {"DeviceTypes", "Types", "MessageTypes", "DeviceInstances", "EdgeInstances"};
const char* P_PparserTest::tags_array[34] = {"Graphs", "GraphTypeReference", "GraphType", "GraphInstance", "GraphInstanceReference",
		                          "GraphInstanceMetadataPatch","SupervisorDeviceType","ExternalType","DeviceType","TypeDef",
		                          "MessageType","DevI","ExtI","EdgeI","Message","M","P","S","OutputPin","InputPin",
		                          "OnCompute","SharedCode","DeviceSharedCode","ReadyToSend","OnSend","OnReceive","State",
			                  "Properties","Documentation","MetaData","Union", "Tuple","Scalar","Array"};
const char* P_PparserTest::attrs_array[16] = {"formatMinorVersion","id","cTypeName","name","messageTypeId","application",
					   "priority","src","graphTypeId","supervisorDeviceTypeId","sorted","type",
                                           "path","default","length","tagName"};
const char* P_PparserTest::cData_array[7] = {"Code","OnSend","OnReceive","OnCompute","SharedCode","DeviceSharedCode","ReadyToSend"};
const char* P_PparserTest::ttags_array[5] = {"M","P","S","Documentation","MetaData"};
const set<string> P_PparserTest::containers(conts_array, conts_array+5);
const set<string> P_PparserTest::tags(tags_array, tags_array+34);
const set<string> P_PparserTest::attributes(attrs_array, attrs_array+16);
const set<string> P_PparserTest::cDatatags(cData_array, cData_array+7);
const set<string> P_PparserTest::texttags(ttags_array, ttags_array+5);

P_PparserTest::P_PparserTest(xmlTextReaderPtr h_parser, ostream& outfile, string tag):P_Pparser(h_parser),testfile(outfile)
{
   collections=&containers;
   valid_tags=&tags;
   valid_attributes=&attributes;
   if (texttags.find(tag) != texttags.end()) isText = true;
   if (cDatatags.find(tag) != cDatatags.end()) isCData = true;
}

P_PparserTest::~P_PparserTest()
{
}

int P_PparserTest::InsertSubObject(string subtag)
{
   int error = 0;
   if (isText || isCData)
   {
      testfile << "\n";
      testfile << subtag.c_str();
      testfile << "\n";
      testfile.flush();
   }
   else
   {
      P_PparserTest subobj(GetParser(), testfile, subtag);
      testfile << "-------------------------------------------------------------------------------\n";
      testfile << "Object " << subtag.c_str() << ":\n";
      testfile << "with properties: \n";
      // subobj.ParseObjectProperties();
      testfile << "total number of properties: " << subobj.ParseObjectProperties() << "\n";
      // testfile << "\n";
      testfile << "and with subobjects: \n";
      error = subobj.ParseNextSubObject(subtag);
      testfile << "End object " << subtag.c_str() << "\n";
      testfile << "________________________________________________________________________________\n";
      testfile << "\n";
      testfile.flush();
   }
   return error;
}

int P_PparserTest::SetObjectProperty(string prop, string value)
{
    testfile << prop.c_str() << ": " << value.c_str() << "\n";
    return 0;
}
