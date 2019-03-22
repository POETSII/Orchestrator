#include "P_GraphTypeReference.h"
#include "P_GraphType.h"

// For a GraphType reference the valid attributes are the minor version from
// the parent P_Graphs object, and the id/path fields from the P_GraphTypeReference
// itself.
const char* P_GraphTypeReference::attrs_array[3] = {"formatMinorVersion","id","src"};
const set<string> P_GraphTypeReference::attributes(attrs_array,attrs_array+3);

P_GraphTypeReference::P_GraphTypeReference(OrchBase* orchestrator):P_Graphs(orchestrator)
{
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
