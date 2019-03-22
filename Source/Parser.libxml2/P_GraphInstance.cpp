#include "P_GraphInstance.h"
#include "P_super.h"
/* to be included once implemented
#include "P_DeviceInstance.h"
#include "P_EdgeInstance.h"
#include "P_Properties.h"
*/

const set<string> P_GraphInstance::groups_init()
{
      const char* grps_array[2] = {"DeviceInstances","EdgeInstances"};
      set<string> tmp_grps(grps_array,grps_array+2);
      return tmp_grps;
}

const set<string> P_GraphInstance::tags_init()
{
      const char* tags_array[4] = {"DevI","ExtI","EdgeI","Properties"};
      set<string> tmp_tags(tags_array,tags_array+4);
      tmp_tags.insert(P_GraphInstance::P_Annotation::tags.begin(), P_GraphInstance::P_Annotation::tags.end());
      return tmp_tags;
}

const set<string> P_GraphInstance::attrs_init()
{
      const char* attrs_array[3] = {"id","graphTypeId","supervisorDeviceTypeId"};
      set<string> tmp_attrs(attrs_array,attrs_array+3);
      return tmp_attrs;
}

const set<string> P_GraphInstance::groups(groups_init());
const set<string> P_GraphInstance::tags(tags_init());
const set<string> P_GraphInstance::attributes(attrs_init());

P_GraphInstance::P_GraphInstance(xmlTextReaderPtr parser, OrchBase* orchestrator):P_Annotation(parser)
{
      parent=orchestrator;
      collections=&groups;
      valid_tags=&tags;
      valid_attributes=&attributes;
}

P_GraphInstance::~P_GraphInstance()
{
}

int P_GraphInstance::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    int error = ERR_SUCCESS;
    return error;
}

int P_GraphInstance::SetObjectProperty(string prop, string value)
{
    if (!parent) return -ERR_UNDEFINED_ORCH; // no associated orchestrator
    // with the ID we can create the task
    if (prop == "id")
    {
       id = value;
       if (!(task = parent->P_taskm[id])) task = parent->P_taskm[id] = new P_task(parent,value);
       // if we have already read a graph type for the instance we can associate it.
       // note here that if the graph type itself has not been read in, this line
       // will create a new entry in the P_typdclm map for a to-be-seen type. When
       // we encounter that later in the XML, the value will be filled in
       if (!graphTypeID.empty() && !(task->pP_typdcl = parent->P_typdclm[graphTypeID])) task->pP_typdcl = parent->P_typdclm[graphTypeID] = new P_typdcl(parent,graphTypeID);
       // similarly for the supervisor type
       if (!supervisorID.empty() && !(task->pSup = parent->P_superm[supervisorID])) task->pSup = parent->P_superm[supervisorID] = new P_super(supervisorID);      
       return ERR_SUCCESS;
    }
    // once we have a graph type we can associate it with the task
    if (prop == "graphTypeId")
    {
       graphTypeID = value;
       // provided the task already exists
       if (task && !(task->pP_typdcl = parent->P_typdclm[graphTypeID])) task->pP_typdcl = parent->P_typdclm[graphTypeID] = new P_typdcl(parent, graphTypeID);
       return ERR_SUCCESS;
    }
    // and once we have a supervisor type we can likewise associate it with the task
    if (prop == "supervisorDeviceTypeId")
    {
       supervisorID = value;
       // under the same constraint that the task exists.
       if (task && !(task->pSup = parent->P_superm[supervisorID])) task->pSup = parent->P_superm[supervisorID] = new P_super(supervisorID); 
       return ERR_SUCCESS;	 
    }
    return -ERR_UNEXPECTED_ATTR;
}
