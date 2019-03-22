#ifndef _P_GRAPHS_H_
#define _P_GRAPHS_H_

#include "P_Pparser.h"
#include "OrchBase.h"

class P_Graphs : public P_Pparser
{
public:
                  P_Graphs(OrchBase* =0, string="");
virtual           ~P_Graphs();	 

        void      Clear();
inline  int       GetMinorVersion() {return minorVersion;};
inline  OrchBase* GetOrch() {return orchDB;};
inline  void      Load(string infile) {if (!infile.empty()) SetParser(xmlNewTextReaderFilename(infile.c_str()));};
virtual int       InsertSubObject(string);
        void      SetOrch(OrchBase* =0);
virtual int       SetObjectProperty(string, string);
    
protected:

        OrchBase* orchDB;

private:
    
             bool        internalOrch;
             int         minorVersion;

static const set<string> tags_init();
static const set<string> attrs_init();
static const set<string>tags;
static const set<string>attributes;
};

#endif
