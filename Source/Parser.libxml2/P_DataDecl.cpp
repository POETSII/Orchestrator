#include "P_DataDecl.h"
// #include "P_datatype.h"
// #include "P_datavalue.h"
// #include "P_Scalar.h"
// #include "P_Tuple.h"
// #include "P_Union.h"
// #include "P_Array.h"

/*
const set<string> P_DataDecl::tags_init()
{
      const char* tags_array[4] = {"Scalar","Tuple","Array","Union"};
      set<string> tmp_tags(tags_array,tags_array+4);
      return tmp_tags;
}
*/

const set<string> P_DataDecl::attrs_init()
{
      const char* attrs_array[1] = {"cTypeName"};
      set<string> tmp_attrs(attrs_array,attrs_array+1);
      return tmp_attrs;
}

// const set<string> P_DataDecl::tags(tags_init());
const set<string> P_DataDecl::tags;
const set<string> P_DataDecl::attributes(attrs_init());

P_DataDecl::P_DataDecl(xmlTextReaderPtr parser, P_typdcl* graphtype, string name) : P_Pparser(parser), typName(name), code(0), parent(graphtype) 
{
      // parent->P_typdefm[name] = type = new P_datatype(graphtype, name, "", STRUCT); // add to the type map
      isCData = true;
      valid_tags=&tags;
      valid_attributes=&attributes;
      // errorsAreFatal=false;           // only during debug when we ignore subtags
}

P_DataDecl::~P_DataDecl()
{
}

int P_DataDecl::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error;
    if (!parent) return ERR_INVALID_NODE;  // no Orchestrator object means the data declaration was outside a GraphType context. Abort parsing.
    if (!isCData) return ERR_INVALID_NODE; // not CDATA? Can't be a code fragment. Abort parsing.
    code = new CFrag(subobj);              // only need to comment this line to stub out functionality (V4)
    string preamble("struct " + typName + " {");  // append the name to create a complete declaration
    code->c_src = preamble + code->c_src + " };\n";
    return ERR_SUCCESS;
    /*
    // tag not in class-specific subset
    if (valid_tags->find(subobj) == valid_tags->end()) return ERR_UNEXPECTED_TAG;
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
    {;
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
    */
}

int P_DataDecl::SetObjectProperty(string prop, string value)
{
    if (!parent) return -ERR_UNDEFINED_ORCH; // no associated orchestrator object
    // set up the new device instance
    if (prop == "cTypeName")
    {
       typName = value;
       // if (type) type->Name(typName); 
       return ERR_SUCCESS;
    }
    return -ERR_UNEXPECTED_ATTR;
}
