#include "P_GraphInstanceReference.h"
#include "P_GraphInstance.h"

// For a GraphInstance reference the valid attributes are the minor version from
// the parent P_Graphs object, and the id/path fields from the P_GraphInstanceReference
// itself.
const char* P_GraphInstanceReference::attrs_array[3] = {"formatMinorVersion","id","src"};
const set<string> P_GraphInstanceReference::attributes(attrs_array,attrs_array+3);

P_GraphInstanceReference::P_GraphInstanceReference(OrchBase* orchestrator):P_Graphs(orchestrator)
{
      valid_attributes = &attributes;
}

P_GraphInstanceReference::~P_GraphInstanceReference()
{
}

int P_GraphInstanceReference::InsertSubObject(string subobj)
{
    // ignore everything but GraphInstance objects since that's all we care about for a
    // GraphInstanceReference
    int error = ERR_SUCCESS;
    if (subobj != "GraphInstance") return error;
    P_GraphInstance subGraphInstance(GetParser(), orchDB);
    subGraphInstance.ParseObjectProperties();
    // only add the GraphInstance matching the one we asked for.
    if (subGraphInstance.id == graphInstanceID)
    {
       error =  subGraphInstance.ParseNextSubObject(subobj);
       if (!error)
       {
	  // and only if its contained XML is valid
	  orchDB->P_taskm[graphInstanceID] = subGraphInstance.task;
	  return error;
       }
    }
    // otherwise free the temporary orchestrator object created.
    delete subGraphInstance.task;
    return error;
}

int P_GraphInstanceReference::SetObjectProperty(string prop, string value)
{

    if (prop == "formatMinorVersion") return P_Graphs::SetObjectProperty(prop, value);
    if (prop == "id")
    {
       graphInstanceID = value;
       return 0;
    }   
    if (prop == "src")
    {
       Load(value);
       return 0;
    }
    return -ERR_UNEXPECTED_ATTR;
}
