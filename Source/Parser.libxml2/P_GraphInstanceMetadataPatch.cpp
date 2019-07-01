#include "P_GraphInstanceMetadataPatch.h"
/* to be included once implemented
#include "P_DeviceType.h"
#include "P_ExternalType.h"
#include "P_TypeDef.h"
#include "P_MessageType.h"
#include "P_Properties.h"
#include "P_SupervisorDeviceType.h"
*/

const set<string> P_GraphInstanceMetadataPatch::tags_init()
{
      const char* tags_array[2] = {"DeviceInstances","EdgeInstances"};
      set<string> tmp_tags(tags_array,tags_array+2);
      tmp_tags.insert(P_GraphInstanceMetadataPatch::P_Annotation::getTags().begin(), P_GraphInstanceMetadataPatch::P_Annotation::getTags().end());
      return tmp_tags;
}

const set<string> P_GraphInstanceMetadataPatch::attrs_init()
{
      const char* attrs_array[1] = {"id"};
      set<string> tmp_attrs(attrs_array,attrs_array+1);
      return tmp_attrs;
}

const set<string> P_GraphInstanceMetadataPatch::tags(tags_init());
const set<string> P_GraphInstanceMetadataPatch::attributes(attrs_init());

P_GraphInstanceMetadataPatch::P_GraphInstanceMetadataPatch(xmlTextReaderPtr parser, OrchBase* orchestrator):P_Annotation(parser), graph(0)
{
      parent=orchestrator;
      valid_tags=&tags;
      valid_attributes=&attributes;
      errorsAreFatal=false; // only during debug when we ignore subtags
}

P_GraphInstanceMetadataPatch::~P_GraphInstanceMetadataPatch()
{
}

int P_GraphInstanceMetadataPatch::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    int error = ERR_SUCCESS;
    return error;
}

int P_GraphInstanceMetadataPatch::SetObjectProperty(string prop, string value)
{
    if (!parent) return -ERR_UNDEFINED_ORCH; // no associated orchestrator
    // the only property we care about is the GraphInstance id
    if (prop == "id")
    {
       // and we use it to set up its associated Orchestrator P_task object. 
       graphInstanceID = value;
       // inserting if it isn't already there.
       if (!(graph = parent->P_taskm[graphInstanceID])) graph = parent->P_taskm[graphInstanceID] = new P_task(parent, graphInstanceID);
       return ERR_SUCCESS;
    }
    return -ERR_UNEXPECTED_ATTR;
}
