#include "P_SupervisorType.h"
#include "P_DataDecl.h"
#include "P_CodeFragment.h"
#include "P_devtyp.h"
#include "P_pintyp.h"
#include "P_InputPin.h"
#include "P_OutputPin.h"

const set<string> P_SupervisorType::tags_init()
{
      const char* tags_array[8] = {"Properties","State","InputPin","OutputPin","SharedCode","OnInit","OnStop","SupervisorCompute"};
      set<string> tmp_tags(tags_array,tags_array+8);
      tmp_tags.insert(P_SupervisorType::P_Annotation::getTags().begin(), P_SupervisorType::P_Annotation::getTags().end());
      return tmp_tags;
}

const set<string> P_SupervisorType::attrs_init()
{
  const char* attrs_array[4] = {"id","requiresPersistentLocalDeviceInfo","requiresPersistentLocalDeviceProperties","requiresLocalEdgeEndpoints"};
      set<string> tmp_attrs(attrs_array,attrs_array+4);
      return tmp_attrs;
}

const set<string> P_SupervisorType::tags(tags_init());
const set<string> P_SupervisorType::attributes(attrs_init());

P_SupervisorType::P_SupervisorType(xmlTextReaderPtr parser, P_typdcl* graphtype):P_Annotation(parser), devtyp(0)
{
      parent=graphtype;
      valid_tags=&tags;
      valid_attributes=&attributes;
      // errorsAreFatal=false; // only during debug when we ignore subtags
}

P_SupervisorType::~P_SupervisorType()
{
}

int P_SupervisorType::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error;
    if (!devtyp) return ERR_INVALID_NODE; // no Orchestrator object means the DeviceType had no ID. Abort parsing.
    if (isText) return ERR_SUCCESS;       // simply swallow the text of annotations
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
       if (error == ERR_SUCCESS && (inputPin.pintyp != 0)) devtyp->P_pintypIm[inputPin.pintyp->Name()] = inputPin.pintyp;
       return error;
    }
    if (subobj == "OutputPin")
    {
       P_OutputPin outputPin(GetParser(), devtyp);
       if (outputPin.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       int error = outputPin.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS && (outputPin.pintyp != 0)) devtyp->P_pintypOm[outputPin.pintyp->Name()] = outputPin.pintyp;
       return error;
    }
    if (subobj == "SharedCode")
    {
       P_CodeFragment sharedCode(GetParser());
       int error = sharedCode.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS)  devtyp->pHandlv.push_back(sharedCode.code);
       return error; 
    }
    if (subobj == "OnInit")
    {
       P_CodeFragment initCode(GetParser());
       int error = initCode.ParseNextSubObject(subobj);
       // will need to be modified for Supervisor init
       if (error == ERR_SUCCESS)  devtyp->pOnCtl = initCode.code;
       return error; 
    }
    if (subobj == "OnStop")
    {
       P_CodeFragment stopCode(GetParser());
       int error = stopCode.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS) devtyp->pOnRTS = stopCode.code;
       return error;
    }
    if (subobj == "SupervisorCompute")
    {
       P_CodeFragment supComputeCode(GetParser());
       int error = supComputeCode.ParseNextSubObject(subobj);
       // co-opting onIdle here as the most obvious option given that pOnCtl
       // is used for OnInit.
       if (error == ERR_SUCCESS) devtyp->pOnIdle = supComputeCode.code;
       return error;
    }
    return P_Annotation::InsertSubObject(subobj);
}

int P_SupervisorType::SetObjectProperty(string prop, string value)
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
       devtyp->isSuper=true;
       return ERR_SUCCESS;
    }
    // these properties exist but we don't know what to do with them yet. TBD.
    if (prop == "requiresPersistentLocalDeviceInfo") return ERR_SUCCESS;
    if (prop == "requiresPersistentLocalDeviceProperties") return ERR_SUCCESS;
    if (prop == "requiresLocalEdgeEndpoints") return ERR_SUCCESS;
    return -ERR_UNEXPECTED_ATTR;
}
