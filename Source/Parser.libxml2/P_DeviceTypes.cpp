#include "P_DeviceTypes.h"
#include "P_DeviceType.h"
#include "P_ExternalType.h"
#include "P_SupervisorType.h"


const set<string> P_DeviceTypes::tags_init()
{
      const char* tags_array[3] = {"DeviceType","ExternalType","SupervisorType"};
      set<string> tmp_tags(tags_array,tags_array+3);
      return tmp_tags;
}

const set<string> P_DeviceTypes::tags(tags_init());
const set<string> P_DeviceTypes::attributes;

P_DeviceTypes::P_DeviceTypes(xmlTextReaderPtr parser, P_typdcl* graphtype) : P_Pparser(parser), parent(graphtype)
{
      valid_tags=&tags;
      valid_attributes=&attributes;
      // errorsAreFatal=false; // only during debug when we ignore subtags
}

P_DeviceTypes::~P_DeviceTypes()
{
}

int P_DeviceTypes::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error;
    if (!parent) return ERR_INVALID_NODE; // no Orchestrator object means the DeviceTypes had no parent GraphType. Abort parsing.
    // tag not in class-specific subset: abort parsing
    if (valid_tags->find(subobj) == valid_tags->end()) return ERR_UNEXPECTED_TAG;
    if (subobj == "DeviceType")
    {
       P_DeviceType dt_subobj(GetParser(), parent);
       if (dt_subobj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       if (!dt_subobj.devtyp) return ERR_INVALID_OBJECT;   // no Orchestrator object created
       parent->P_devtypm[dt_subobj.id] = dt_subobj.devtyp; // otherwise add to the device type map
       // dt_subobj.devtyp->idx = parent->P_devtypv.size()-1; // updating the back index also
       return dt_subobj.ParseNextSubObject(subobj);
    }
    if (subobj == "ExternalType")
    {
       P_ExternalType et_subobj(GetParser(), parent);
       if (et_subobj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       if (!et_subobj.devtyp) return ERR_INVALID_OBJECT;
       parent->P_devtypm[et_subobj.id] = et_subobj.devtyp;
       // et_subobj.devtyp->idx = parent->P_devtypv.size()-1;
       return et_subobj.ParseNextSubObject(subobj);
    }
    if (subobj == "SupervisorType")
    {
       P_SupervisorType st_subobj(GetParser(), parent);
       if (st_subobj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       if (!st_subobj.devtyp) return ERR_INVALID_OBJECT;
       parent->P_devtypm[st_subobj.id] = st_subobj.devtyp;
       // st_subobj.devtyp->idx = parent->P_devtypv.size()-1;
       return st_subobj.ParseNextSubObject(subobj);
    }
    return ERR_UNEXPECTED_TAG; // should never reach this line
}
