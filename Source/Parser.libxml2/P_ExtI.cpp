#include "P_ExtI.h"
// #include "P_DataDecl.h"
#include "P_DataDef.h"
#include "P_typdcl.h"
#include "P_devtyp.h"

const set<string> P_ExtI::tags_init()
{
      const char* tags_array[2] = {"P","M"};
      set<string> tmp_tags(tags_array,tags_array+2);
      return tmp_tags;
}

const set<string> P_ExtI::attrs_init()
{
      const char* attrs_array[2] = {"id","type"};
      set<string> tmp_attrs(attrs_array,attrs_array+2);
      return tmp_attrs;
}

const set<string> P_ExtI::tags(tags_init());
const set<string> P_ExtI::attributes(attrs_init());

P_ExtI::P_ExtI(xmlTextReaderPtr parser, P_task* graph): P_Pparser(parser), id(""), devtype(""), device(0), isMetaData(false)
{
      parent=graph;
      valid_tags=&tags;
      valid_attributes=&attributes;
      errorsAreFatal=false; // only during debug when we ignore subtags
}

P_ExtI::~P_ExtI()
{
}

int P_ExtI::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error;
    if (!device) return ERR_INVALID_NODE; // no Orchestrator object means the MessageType had no ID. Abort parsing.
    if (isMetaData) return ERR_SUCCESS;   // For the moment ignore metadata. This will have to do something useful in future.
    if (subobj == "P")
    {
       // by this point everything should have been set up to be able to generate a value
       P_DataDef propsVal(GetParser(), 0);
       // P_DataDef propsVal(GetParser(), device->pP_devtyp->pProps);
       int error = propsVal.ParseNextSubObject(subobj);
       // if (error == ERR_SUCCESS) device->pProps = propsVal.value;
       return error;
    }
    if (subobj == "M")
    {
       // for the moment just swallow metadata
       isMetaData = true;
       int error = ParseNextSubObject(subobj);
       isMetaData = false;
       return error;
    }
    return ERR_UNEXPECTED_TAG; // should never reach this line
}

int P_ExtI::SetObjectProperty(string prop, string value)
{
    if (!parent) return -ERR_UNDEFINED_ORCH; // no associated orchestrator object
    // set up the new device instance
    if (prop == "id")
    {
       id = value;
       device = new P_device(parent->pD,id);
       if (devtype != "")
       {
	  map<string, P_devtyp*>::iterator devTypIt = parent->pP_typdcl->P_devtypm.find(id);
	  if (devTypIt == parent->pP_typdcl->P_devtypm.end())
	  {
	     device->pP_devtyp = new P_devtyp(parent->pP_typdcl, devtype);
	     parent->pP_typdcl->P_devtypm[devtype] = device->pP_devtyp;
	  }
	  else device->pP_devtyp = devTypIt->second;
       }
       return ERR_SUCCESS;
    }
    if (prop == "type")
    {
       devtype = value;
       if (device)
       {
	   map<string, P_devtyp*>::iterator devTypIt = parent->pP_typdcl->P_devtypm.find(id);
	   if (devTypIt == parent->pP_typdcl->P_devtypm.end())
	   {
	      device->pP_devtyp = new P_devtyp(parent->pP_typdcl, devtype);
	      parent->pP_typdcl->P_devtypm[devtype] = device->pP_devtyp;
	   }
	   else device->pP_devtyp = devTypIt->second;	 
       }
       return ERR_SUCCESS;
    }
    return -ERR_UNEXPECTED_ATTR;
}
