#include "P_DataDef.h"
// #include "P_datatype.h"
// #include "P_datavalue.h"
// #include "P_JSONParser.h"

// no tags, no attributes for a data definition, these are
// CDATA objects.
const set<string> P_DataDef::tags;
const set<string> P_DataDef::attributes;

P_DataDef::P_DataDef(xmlTextReaderPtr parser, D_graph* graph) : P_Pparser(parser), code(0), parent(graph)
{
      valid_tags=&tags;
      valid_attributes=&attributes;
      isCData = true;
      // isText=true;
      // errorsAreFatal=false; // only during debug when we ignore subtags
}

P_DataDef::~P_DataDef()
{
}

int P_DataDef::InsertSubObject(string subobj)
{
    // stub insert routine swallows the C fragment
    // int error = ERR_SUCCESS;
    // return error;
    if (!parent) return ERR_INVALID_NODE;  // no Orchestrator object means the data initialiser was outside a Graph context. Abort parsing.
    if (!isCData) return ERR_INVALID_NODE; // not CDATA? Can't be a code fragment, which is expected in V4. Abort parsing.
    code = new CFrag(subobj);              // only need to comment this line to stub out functionality
}

int P_DataDef::SetObjectProperty(string prop, string value)
{
    // no properties expected  
    return -ERR_UNEXPECTED_ATTR;
}
