#ifndef _P_PPARSERTEST_H_
#define _P_PPARSERTEST_H_

#include "P_Pparser.h"
#include <iostream>

class P_PparserTest: public P_Pparser
{
public:

        P_PparserTest(xmlTextReaderPtr, ostream& =cout, string="");
	~P_PparserTest();

    int InsertSubObject(string);
    int SetObjectProperty(string, string);

private:

    ostream& testfile;
    // daft: C++98 doesn't allow initializer lists for stl containers.
    // so we have to define some additional static arrays, then use
    // an iterator constructor initializer to set up searchable lists.
    // This has the further undesirable side effect of creating a
    // static initializer (function), but there is little we can do
    // about that, it would seem. There has to be a better way to do this...
    static const char* conts_array[5];
    static const char* tags_array[34];
    static const char* attrs_array[16];
    static const char* cData_array[7];
    static const char* ttags_array[5];
    static const set<string> containers;
    static const set<string> tags;
    static const set<string> attributes;
    static const set<string> cDatatags;
    static const set<string> texttags;
};

#endif
