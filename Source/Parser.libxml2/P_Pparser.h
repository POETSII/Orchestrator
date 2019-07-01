#ifndef _P_PPARSER_H_
#define _P_PPARSER_H_

#include "xmlreader.h"
#include <string>
#include <set>
using namespace std;

/* ----------------------------------------------------------------------------- 

A base class to contain the machinery for the parser. The parser grabs one
object (tag) at a time, then descends into any subtags the enclosing object
may have. It is up to derived classes to determine how to handle the subtags,
through the methods InsertSubObject and SetObjectProperty. Derived classes also 
contain constant sets containing the valid tags - these are located by the base
class through the 'valid_tags', 'valid_attributes' and 'collections' pointers.
_____________________________________________________________________________ */

class P_Pparser
{
public:

                         P_Pparser(xmlTextReaderPtr=0, bool=true);
virtual                  ~P_Pparser();

virtual int              ParseDocument(string="Graphs");            
        int              ParseNextSubObject(string); // high-level parser step
	int              ParseObjectProperties();    // translates attributes to properties
inline  void             ClearParser() {parser=0;};
inline  int              CloseParser() {if (parser) return xmlTextReaderClose(parser);};
inline  xmlTextReaderPtr GetParser() {return parser;};
inline  void             SetParser(xmlTextReaderPtr p_reader) {parser = p_reader;};
virtual int              InsertSubObject(string) = 0;
virtual int              SetObjectProperty(string, string) = 0;

protected:

	bool               isCData;        // CData and text tags
	bool               isText;         // should just be copied verbatim
	bool               errorsAreFatal; // abort parsing if an error occurs
	const set<string>* valid_tags;
	const set<string>* valid_attributes;
static  const int ERR_SUCCESS = 0x0;
static  const int ERR_UNEXPECTED_TAG = 0x1;
static  const int ERR_UNEXPECTED_ATTR = 0x2;
static  const int ERR_UNEXPECTED_TEXT = 0x4;
static  const int ERR_UNEXPECTED_CDATA = 0x8;
static  const int ERR_INVALID_NODE = 0x10;
static  const int ERR_INVALID_NODE_TYPE = 0x20;
static  const int ERR_INVALID_ATTR = 0x40;
static  const int ERR_INVALID_XML_DOC = 0x80;
static  const int ERR_UNPARSEABLE_PROPS = 0x100;
static  const int ERR_PARSER_NOT_OPEN = 0x200;
static  const int ERR_NOT_AT_DOC_TAG = 0x400;
static  const int ERR_INVALID_OBJECT = 0x800;
static  const int ERR_UNDEFINED_ORCH = 0x1000;
static  const int ERR_INVALID_JSON_DOC = 0x2000;
static  const int ERR_INVALID_JSON_INIT = 0x4000;

private:

	xmlTextReaderPtr   parser;
        string& GetNextTag(int*,string&); // the low-level parser step
	// ugly function to place an xmlChar* into a string. Awkwardly xmlChar*
        // is typedef'd as unsigned char*, so you have to use reinterpret_cast
        // because of certain C++ standards.
	inline string&     xmlChartoStr(xmlChar* xmlstr, string& str) 
	  {return (str = reinterpret_cast<char*>(xmlstr));};
	int                err;
   
};

#endif
