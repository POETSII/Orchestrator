#include "P_TypeDef.h"
/*
#include "P_datatype.h"
#include "P_Scalar.h"
#include "P_Tuple.h"
#include "P_Array.h"
#include "P_Union.h"
*/

const set<string> P_TypeDef::tags_init()
{
      const char* tags_array[5] = {"Documentation","Scalar","Tuple","Union","Array"};
      set<string> tmp_tags(tags_array,tags_array+5);
      return tmp_tags;
}

const set<string> P_TypeDef::attrs_init()
{
      const char* attrs_array[1] = {"id"};
      set<string> tmp_attrs(attrs_array,attrs_array+1);
      return tmp_attrs;
}

const set<string> P_TypeDef::tags(tags_init());
const set<string> P_TypeDef::attributes(attrs_init());

P_TypeDef::P_TypeDef(xmlTextReaderPtr parser, P_typdcl* graphtype) : P_Pparser(parser), parent(graphtype)
{
      parent=graphtype;
      valid_tags=&tags;
      valid_attributes=&attributes;
      errorsAreFatal=false; // only during debug when we ignore subtags      
}

P_TypeDef::~P_TypeDef()
{
}

int P_TypeDef::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    int error = ERR_SUCCESS;
    return error;
    /*
    if (!type) return ERR_INVALID_NODE; // no Orchestrator object means the type had no name. Abort parsing.
    if (isText) return ERR_SUCCESS; // simply swallow the text of documentation
    // tag not in class-specific subset
    if (valid_tags.find(subobj) == valid_tags.end()) return ERR_UNEXPECTED_TAG;
    if (subobj == "Scalar")
    {
       // when a data type subobject such as P_Scalar, P_Tuple, etc. gets
       // a type which returns IsTypeDef() == true, it will set up that
       // type directly. If IsTypeDef() == false, it will interpret that
       // it's a subtype of a higher-level type and set itself up as
       // a submember.
       P_Scalar msgData(GetParser(), type);
       if (msgData.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;   
       return msgData.ParseNextSubObject(subobj);
    }
    if (subobj == "Tuple")
    {
       P_Tuple msgData(GetParser(), type);
       if (msgData.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;   
       return msgData.ParseNextSubObject(subobj);
    }
    if (subobj == "Union")
    {
       P_Union msgData(GetParser(), type);
       if (msgData.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;   
       return msgData.ParseNextSubObject(subobj);
    }
    if (subobj == "Array")
    {
       P_Array msgData(GetParser(), type);
       if (msgData.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;   
       return msgData.ParseNextSubObject(subobj);
    }
    if (subobj == "Documentation")
    {       
       // descend immediately into the subtags; no need to recur.
       isText = true;
       int error = ParseNextSubObject(subobj);
       isText = false;
       return error;
    }
    return ERR_UNEXPECTED_TAG; // should never reach this line
    */
}

int P_TypeDef::SetObjectProperty(string prop, string value)
{
    if (!parent) return -ERR_UNDEFINED_ORCH; // no associated orchestrator object
    // the only property we care about is the TypeDef id
    if (prop == "id")
    {
       // and we use it to set up its associated Orchestrator P_datatype object. 
       id = value;
       // type = new P_datatype(parent,id,id);
       // parent->P_typdefm[id] = type;
       return ERR_SUCCESS;
    }
    return -ERR_UNEXPECTED_ATTR;
}
