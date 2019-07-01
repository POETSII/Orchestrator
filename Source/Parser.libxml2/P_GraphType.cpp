#include "P_GraphType.h"
#include "P_DeviceTypes.h"
#include "P_MessageTypes.h"
#include "P_Types.h"
#include "P_DataDecl.h"
#include "P_CodeFragment.h"
// #include <iostream>

const set<string> P_GraphType::tags_init()
{
      const char* tags_array[5] = {"DeviceTypes","MessageTypes","Types","Properties","SharedCode"};
      set<string> tmp_tags(tags_array,tags_array+5);
      tmp_tags.insert(P_GraphType::P_Annotation::getTags().begin(), P_GraphType::P_Annotation::getTags().end());
      return tmp_tags;
}

const set<string> P_GraphType::attrs_init()
{
      const char* attrs_array[1] = {"id"};
      set<string> tmp_attrs(attrs_array,attrs_array+1);
      return tmp_attrs;
}

const set<string> P_GraphType::tags(tags_init());
const set<string> P_GraphType::attributes(attrs_init());

P_GraphType::P_GraphType(xmlTextReaderPtr parser, OrchBase* orchestrator) : P_Annotation(parser), typdcl(0)
{
      parent=orchestrator;
      valid_tags=&tags;
      valid_attributes=&attributes;
}

P_GraphType::~P_GraphType()
{
}

int P_GraphType::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error;
    if (!typdcl) return ERR_INVALID_NODE; // no Orchestrator object means the GraphType had no ID. Abort parsing.
    if (isText) return ERR_SUCCESS;       // simply swallow the text of annotations
    // tag not in class-specific subset
    if (valid_tags->find(subobj) == valid_tags->end()) return ERR_UNEXPECTED_TAG;  
    if (subobj == "DeviceTypes")
    {
       P_DeviceTypes dt_subobj(GetParser(), typdcl);
       return dt_subobj.ParseNextSubObject(subobj);
    }
    if (subobj == "MessageTypes")
    {
       P_MessageTypes mt_subobj(GetParser(), typdcl);
       return mt_subobj.ParseNextSubObject(subobj);
    }
    if (subobj == "Types")
    {
       P_Types tt_subobj(GetParser(), typdcl);
       return tt_subobj.ParseNextSubObject(subobj);
    }
    if (subobj == "Properties")
    {
       string defaultPropsName = typdcl->Name();
       P_DataDecl p_subobj(GetParser(), typdcl, defaultPropsName + "_properties_t");
       if (p_subobj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       int error = p_subobj.ParseNextSubObject(subobj);
       // if (error == ERR_SUCCESS) typdcl->pProps = p_subobj.type;
       if (error == ERR_SUCCESS) typdcl->pPropsD = p_subobj.code;
       return error;
    }
    if (subobj == "SharedCode")
    {
       P_CodeFragment sc_subobj(GetParser());
       int error = sc_subobj.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS && (sc_subobj.code != 0)) typdcl->General.push_back(sc_subobj.code);
       return error;
    }
    return P_Annotation::InsertSubObject(subobj); 
}

int P_GraphType::SetObjectProperty(string prop, string value)
{
    if (!parent) return -ERR_UNDEFINED_ORCH; // no associated orchestrator
    // the only property we care about is the GraphType id
    if (prop == "id")
    {
       // and we use it to set up its associated Orchestrator P_typdcl object. 
       id = value;
       // this immediately creates an object in the type declare map. We might like to
       // change this so that the object is created only if the entire parse of its
       // associated subtree succeeds.
       if (!(typdcl = parent->P_typdclm[id])) typdcl = parent->P_typdclm[id] = new P_typdcl(parent,id);
       return ERR_SUCCESS;
    }
    return -ERR_UNEXPECTED_ATTR;
}
