#include "P_GraphTypeReference.h"
#include "P_GraphType.h"

const set<string> P_GraphTypeReference::tags_init()
{
      set<string> tmp_tags;
      tmp_tags.insert(P_GraphTypeReference::P_Graphs::getTags().begin(), P_GraphTypeReference::P_Graphs::getTags().end());
      return tmp_tags;
}

// For a GraphTypeReference the valid attributes are the minor version from
// the parent P_Graphs object, and the id/path fields from the P_GraphTypeReference
// itself.
const set<string> P_GraphTypeReference::attrs_init()
{
      const char* attrs_array[2] = {"id","src"};
      set<string> tmp_attrs(attrs_array,attrs_array+2);
      tmp_attrs.insert(P_GraphTypeReference::P_Graphs::getAttributes().begin(), P_GraphTypeReference::P_Graphs::getAttributes().end());
      return tmp_attrs;
}

const set<string> P_GraphTypeReference::tags(tags_init());
const set<string> P_GraphTypeReference::attributes(attrs_init());

P_GraphTypeReference::P_GraphTypeReference(OrchBase* orchestrator):P_Graphs(orchestrator)
{
      valid_tags = &tags;
      valid_attributes = &attributes;
}

P_GraphTypeReference::~P_GraphTypeReference()
{
}

int P_GraphTypeReference::InsertSubObject(string subobj)
{
    // ignore everything but GraphType objects since that's all we care about for a
    // GraphTypeReference
    int error = ERR_SUCCESS;
    if (subobj != "GraphType") return error;
    P_GraphType subGraphType(GetParser(), orchDB);
    subGraphType.ParseObjectProperties();
    // only add the GraphType matching the one we asked for.
    if (subGraphType.id == graphTypeID)
    {
       error =  subGraphType.ParseNextSubObject(subobj);
       if (!error)
       {
	  // and only if its contained XML is valid
	  orchDB->P_typdclm[graphTypeID] = subGraphType.typdcl;
	  return error;
       }
    }
    // otherwise free the temporary orchestrator object created.
    delete subGraphType.typdcl;
    return error;
}

int P_GraphTypeReference::SetObjectProperty(string prop, string value)
{

    if (prop == "formatMinorVersion") return P_Graphs::SetObjectProperty(prop, value);
    if (prop == "id")
    {
       graphTypeID = value;
       return ERR_SUCCESS;
    }   
    if (prop == "src")
    {
       Load(value);
       return ERR_SUCCESS;
    }
    return -ERR_UNEXPECTED_ATTR;
}
