#include "P_DeviceType.h"
#include "P_DataDecl.h"
#include "P_CodeFragment.h"
#include "P_devtyp.h"
#include "P_pintyp.h"
#include "P_InputPin.h"
#include "P_OutputPin.h"

const set<string> P_DeviceType::tags_init()
{
      const char* tags_array[10] = {"Properties","State","InputPin","OutputPin","SharedCode","OnInit","ReadyToSend","OnDeviceIdle","OnThreadIdle","OnHardwareIdle"};
      set<string> tmp_tags(tags_array,tags_array+10);
      tmp_tags.insert(P_DeviceType::P_Annotation::getTags().begin(), P_DeviceType::P_Annotation::getTags().end());
      return tmp_tags;
}

const set<string> P_DeviceType::attrs_init()
{
      const char* attrs_array[1] = {"id"};
      set<string> tmp_attrs(attrs_array,attrs_array+1);
      return tmp_attrs;
}

const set<string> P_DeviceType::tags(tags_init());
const set<string> P_DeviceType::attributes(attrs_init());

P_DeviceType::P_DeviceType(xmlTextReaderPtr parser, P_typdcl* graphtype):P_Annotation(parser)
{
      parent=graphtype;
      valid_tags=&tags;
      valid_attributes=&attributes;
      devtyp=0;
      // errorsAreFatal=false; // only during debug when we ignore subtags
}

P_DeviceType::~P_DeviceType()
{
}

int P_DeviceType::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error;
    if (!devtyp) return ERR_INVALID_NODE; // no Orchestrator object means the DeviceType had no ID. Abort parsing.
    if (isText) return ERR_SUCCESS; // simply swallow the text of annotations
    // tag not in class-specific subset: refer to parent class.
    if (valid_tags->find(subobj) == valid_tags->end()) return ERR_UNEXPECTED_TAG;
    if (subobj == "Properties")
    {
       string defaultPropsName = parent->Name() + "_" + devtyp->Name();
       P_DataDecl devProps(GetParser(), devtyp->par, defaultPropsName + "_properties_t");
       if (devProps.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       int error = devProps.ParseNextSubObject(subobj);
       // if (error == ERR_SUCCESS) devtyp->pProps = devProps.type;
       if (error == ERR_SUCCESS) devtyp->pPropsD = devProps.code;
       return error;
    }
    if (subobj == "State")
    {
       string defaultStateName = parent->Name() + "_" + devtyp->Name();
       P_DataDecl devState(GetParser(), devtyp->par, defaultStateName + "_state_t");
       if (devState.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       int error = devState.ParseNextSubObject(subobj);
       // if (error == ERR_SUCCESS) devtyp->pState = devState.type;
       if (error == ERR_SUCCESS) devtyp->pStateD = devState.code;
       return error;
    }
    if (subobj == "InputPin")
    {
       P_InputPin inputPin(GetParser(), devtyp);
       if (inputPin.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       int error = inputPin.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS && (inputPin.pintyp != 0))
       {
	  // need the ability to look up pin types by name or parsing will be SLOW for large files
	  devtyp->P_pintypIm[inputPin.pintyp->Name()] = inputPin.pintyp;
       }
       return error;
    }
    if (subobj == "OutputPin")
    {
       P_OutputPin outputPin(GetParser(), devtyp);
       if (outputPin.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       int error = outputPin.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS && (outputPin.pintyp != 0))
       {
	  devtyp->P_pintypOm[outputPin.pintyp->Name()] = outputPin.pintyp;
       }
       return error;
    }
    if (subobj == "SharedCode")
    {
       P_CodeFragment sharedCode(GetParser());
       int error = sharedCode.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS && (sharedCode.code != 0)) devtyp->pHandlv.push_back(sharedCode.code);
       return error; 
    }
    if (subobj == "OnInit")
    {
       P_CodeFragment initCode(GetParser());
       int error = initCode.ParseNextSubObject(subobj);
       // we co-opt pOnCtl for init - needs some Softswitch modifications_
       if (error == ERR_SUCCESS) devtyp->pOnCtl = initCode.code;
       return error; 
    }
    if (subobj == "ReadyToSend")
    {
       P_CodeFragment rtsCode(GetParser());
       int error = rtsCode.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS) devtyp->pOnRTS = rtsCode.code;
       return error;
    }
    if (subobj == "OnDeviceIdle")
    {
       P_CodeFragment devIdleCode(GetParser());
       int error = devIdleCode.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS) devtyp->pOnIdle = devIdleCode.code;
       return error;
    }
    if (subobj == "OnThreadIdle")
    {
       P_CodeFragment thrIdleCode(GetParser());
       int error = thrIdleCode.ParseNextSubObject(subobj);
       // how do we want to deal with the threadIdle handler?
       if (error == ERR_SUCCESS && (thrIdleCode.code != 0))  devtyp->pHandlv.push_back(thrIdleCode.code);
       return error;
    }
    if (subobj == "OnHardwareIdle")
    {
       P_CodeFragment hwIdleCode(GetParser());
       int error = hwIdleCode.ParseNextSubObject(subobj);
       // hardwareIdle needs some special handling - discuss with Matt?
       if (error == ERR_SUCCESS && (hwIdleCode.code != 0))  devtyp->pHandlv.push_back(hwIdleCode.code);
       return error;
    }
    return P_Annotation::InsertSubObject(subobj); // should never reach this line
}

int P_DeviceType::SetObjectProperty(string prop, string value)
{
    if (!parent) return -ERR_UNDEFINED_ORCH; // no associated orchestrator object
    // the only property we care about is the DeviceType id
    if (prop == "id")
    {
       // and we use it to set up its associated Orchestrator P_devtyp object. 
       id = value;
       // only create a device type if one hasn't been created by a previous
       // encounter with a device instance
       map<string, P_devtyp*>::iterator devTypIt = parent->P_devtypm.find(id);
       if (devTypIt == parent->P_devtypm.end())
       {
          devtyp = new P_devtyp(parent,id);
	  parent->P_devtypm[id] = devtyp;
       }
       else devtyp = devTypIt->second;
       return ERR_SUCCESS;
    }
    return -ERR_UNEXPECTED_ATTR;
}
