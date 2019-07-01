#include "P_DeviceInstances.h"
#include "P_DevI.h"
#include "P_ExtI.h"


const set<string> P_DeviceInstances::tags_init()
{
  const char* tags_array[2] = {"DevI","ExtI"};
      set<string> tmp_tags(tags_array,tags_array+2);
      return tmp_tags;
}

const set<string> P_DeviceInstances::attrs_init()
{
      const char* attrs_array[1] = {"sorted"};
      set<string> tmp_attrs(attrs_array,attrs_array+1);
      return tmp_attrs;
}

const set<string> P_DeviceInstances::tags(tags_init());
const set<string> P_DeviceInstances::attributes(attrs_init());

P_DeviceInstances::P_DeviceInstances(xmlTextReaderPtr parser, P_task* graphinstance):P_Pparser(parser), parent(graphinstance)
{
      valid_tags=&tags;
      valid_attributes=&attributes;
      // errorsAreFatal=false; // only during debug when we ignore subtags
}

P_DeviceInstances::~P_DeviceInstances()
{
}

int P_DeviceInstances::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error;
    if (!parent) return ERR_INVALID_NODE; // no Orchestrator task means the parent GraphInstance was badly defined. Abort parsing.
    // tag not in class-specific subset: abort parsing
    if (valid_tags->find(subobj) == valid_tags->end()) return ERR_UNEXPECTED_TAG;
    if (subobj == "DevI")
    {
       P_DevI di_subobj(GetParser(), parent);
       if (di_subobj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       if (!di_subobj.device) return ERR_INVALID_OBJECT;   // no Orchestrator object created
       unsigned deviceIdx = parent->pD->G.SizeNodes()+1;
       parent->pD->G.InsertNode(deviceIdx,di_subobj.device);               // otherwise add to the device graph
       parent->pD->deviceMap[di_subobj.device->Name()] = di_subobj.device; // and place in the quick-lookup map
       di_subobj.device->idx = deviceIdx;                                  // updating the back index also
       return di_subobj.ParseNextSubObject(subobj);
    }
    if (subobj == "ExtI")
    {
       P_ExtI ei_subobj(GetParser(), parent);
       if (ei_subobj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       if (!ei_subobj.device) return ERR_INVALID_OBJECT;     // no Orchestrator object created
       unsigned deviceIdx = parent->pD->G.SizeNodes()+1;
       parent->pD->G.InsertNode(deviceIdx,ei_subobj.device); // otherwise add to the device graph
       ei_subobj.device->idx = deviceIdx; // updating the back index also
       return ei_subobj.ParseNextSubObject(subobj);
    }
    return ERR_UNEXPECTED_TAG; // should never reach this line
}

int P_DeviceInstances::SetObjectProperty(string prop, string value)
{
    // Only one property is expected: whether the list is sorted
    if (prop == "sorted") return ERR_SUCCESS; // (which we don't care about)
    return -ERR_UNEXPECTED_ATTR;
}
