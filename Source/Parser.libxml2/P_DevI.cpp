#include "P_DevI.h"
#include "P_DataDecl.h"
#include "P_DataDef.h"
#include "P_typdcl.h"
#include "P_devtyp.h"
#include "CFrag.h"
// #include <iostream>

const set<string> P_DevI::tags_init()
{
  const char* tags_array[3] = {"P","S","M"};
      set<string> tmp_tags(tags_array,tags_array+3);
      return tmp_tags;
}

const set<string> P_DevI::attrs_init()
{
  const char* attrs_array[4] = {"id","type","P","S"};
      set<string> tmp_attrs(attrs_array,attrs_array+4);
      return tmp_attrs;
}

const set<string> P_DevI::tags(tags_init());
const set<string> P_DevI::attributes(attrs_init());

P_DevI::P_DevI(xmlTextReaderPtr parser, P_task* graph) : P_Pparser(parser), id(""), devtype(""), device(0), properties(""), state(""), parent(graph), isMetaData(false)
{
      valid_tags=&tags;
      valid_attributes=&attributes;
      // errorsAreFatal=false; // only during debug when we ignore subtags
}

P_DevI::~P_DevI()
{
}

int P_DevI::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error;
    if (!device) return ERR_INVALID_NODE; // no Orchestrator object means the device had no ID. Abort parsing.
    if (isMetaData) return ERR_SUCCESS;
    if (subobj == "P")
    {
       if (!properties.empty()) return ERR_UNEXPECTED_TAG; // expect either a P tag or attribute, not both
       // by this point everything should have been set up to be able to generate a value
       P_DataDef propsVal(GetParser(), device->par); // V4 P_DataDef construction
       // P_DataDef propsVal(GetParser(), device->pP_devtyp->pProps); // V3 P_DataDef construction
       int error = propsVal.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS) device->pPropsI = propsVal.code;
       // if (error == ERR_SUCCESS) device->pProps = propsVal.value;
       return error;
    }
    if (subobj == "S")
    {
       if (!state.empty()) return ERR_UNEXPECTED_TAG; // expect either an S tag or attribute, not both
       // by this point everything should have been set up to be able to generate a value
       P_DataDef stateVal(GetParser(), device->par);
       // P_DataDef stateVal(GetParser(), device->pP_devtyp->pState);
       int error = stateVal.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS) device->pStateI = stateVal.code;
       // if (error == ERR_SUCCESS) device->pState = stateVal.value;
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

int P_DevI::SetObjectProperty(string prop, string value)
{
    if (!parent) return -ERR_UNDEFINED_ORCH; // no associated orchestrator object
    // set up the new device instance
    if (prop == "id")
    {
       id = value;
       device = new P_device(parent->pD,id);
       if (!properties.empty()) device->pPropsI = new CFrag(properties);
       if (!state.empty()) device->pStateI = new CFrag(state);
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
    if (prop == "P")
    {
       properties = value;
       if (device) device->pPropsI = new CFrag(properties);
       return ERR_SUCCESS;
    }
    if (prop == "S")
    {
       state = value;
       if (device) device->pStateI = new CFrag(state);
       return ERR_SUCCESS;
    }
    return -ERR_UNEXPECTED_ATTR;
}
