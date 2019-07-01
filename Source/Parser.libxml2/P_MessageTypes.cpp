#include "P_MessageTypes.h"
#include "P_MessageType.h"


const set<string> P_MessageTypes::tags_init()
{
      const char* tags_array[1] = {"MessageType"};
      set<string> tmp_tags(tags_array,tags_array+1);
      return tmp_tags;
}

const set<string> P_MessageTypes::tags(tags_init());
const set<string> P_MessageTypes::attributes;

P_MessageTypes::P_MessageTypes(xmlTextReaderPtr parser, P_typdcl* graphtype) : P_Pparser(parser), parent(graphtype)
{
      valid_tags=&tags;
      valid_attributes=&attributes;
      // errorsAreFatal=false; // only during debug when we ignore subtags
}

P_MessageTypes::~P_MessageTypes()
{
}

int P_MessageTypes::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error;
    if (!parent) return ERR_INVALID_NODE; // no Orchestrator object means the MessageTypes had no parent GraphType. Abort parsing.
    // tag not in class-specific subset: abort parsing
    if (valid_tags->find(subobj) == valid_tags->end()) return ERR_UNEXPECTED_TAG;
    if (subobj == "MessageType")
    {
       P_MessageType mt_subobj(GetParser(), parent);
       if (mt_subobj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       if (!mt_subobj.message) return ERR_INVALID_OBJECT;  // no Orchestrator object created
       if (parent->P_messagem.find(mt_subobj.id) == parent->P_messagem.end()) // need a message map in P_typdcl
          parent->P_messagem[mt_subobj.id] = mt_subobj.message;    // add to the message type map if not already there
       return mt_subobj.ParseNextSubObject(subobj);
    }
    return ERR_UNEXPECTED_TAG; // should never reach this line
}
