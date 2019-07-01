#include "P_Types.h"
#include "P_TypeDef.h"


const set<string> P_Types::tags_init()
{
      const char* tags_array[1] = {"TypeDef"};
      set<string> tmp_tags(tags_array,tags_array+1);
      return tmp_tags;
}

const set<string> P_Types::tags(tags_init());
const set<string> P_Types::attributes;

P_Types::P_Types(xmlTextReaderPtr parser, P_typdcl* graphtype) : P_Pparser(parser), parent(graphtype)
{
      valid_tags=&tags;
      valid_attributes=&attributes;
      // errorsAreFatal=false; // only during debug when we ignore subtags
}

P_Types::~P_Types()
{
}

int P_Types::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error;
    if (!parent) return ERR_INVALID_NODE; // no Orchestrator object means the Types had no parent GraphType. Abort parsing.
    // tag not in class-specific subset: abort parsing
    if (valid_tags->find(subobj) == valid_tags->end()) return ERR_UNEXPECTED_TAG;
    if (subobj == "TypeDef")
    {
       P_TypeDef td_subobj(GetParser(), parent);
       if (td_subobj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       // if (!td_subobj.type) return ERR_INVALID_OBJECT;  // no Orchestrator object created
       // parent->P_typdefm[td_subobj.type->Name()] = td_subobj.type); // otherwise add to the type map
       return td_subobj.ParseNextSubObject(subobj);
    }
    return ERR_UNEXPECTED_TAG; // should never reach this line
}
