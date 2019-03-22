#include "P_GraphType.h"
/* to be included once implemented
#include "P_DeviceType.h"
#include "P_ExternalType.h"
#include "P_TypeDef.h"
#include "P_MessageType.h"
#include "P_Properties.h"
#include "P_SupervisorDeviceType.h"
*/

const set<string> P_GraphType::groups_init()
{
      const char* grps_array[3] = {"DeviceTypes","Types","MessageTypes"};
      set<string> tmp_grps(grps_array,grps_array+3);
      return tmp_grps;
}

const set<string> P_GraphType::tags_init()
{
      const char* tags_array[6] = {"DeviceType","ExternalType","SupervisorDeviceType","MessageType","TypeDef","Properties"};
      set<string> tmp_tags(tags_array,tags_array+6);
      tmp_tags.insert(P_GraphType::P_Annotation::tags.begin(), P_GraphType::P_Annotation::tags.end());
      return tmp_tags;
}

const set<string> P_GraphType::attrs_init()
{
      const char* attrs_array[1] = {"id"};
      set<string> tmp_attrs(attrs_array,attrs_array+1);
      return tmp_attrs;
}


const set<string> P_GraphType::groups(groups_init());
const set<string> P_GraphType::tags(tags_init());
const set<string> P_GraphType::attributes(attrs_init());

P_GraphType::P_GraphType(xmlTextReaderPtr parser, OrchBase* orchestrator):P_Annotation(parser)
{
      parent=orchestrator;
      collections=&groups;
      valid_tags=&tags;
      valid_attributes=&attributes;
}

P_GraphType::~P_GraphType()
{
}

int P_GraphType::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    int error = ERR_SUCCESS;
    return error;
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
