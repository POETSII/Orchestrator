#include "P_ExternalType.h"
#include "P_DataDecl.h"
#include "P_devtyp.h"
#include "P_pintyp.h"
#include "P_InputPin.h"
#include "P_OutputPin.h"
#include "P_DataDecl.h"

const set<string> P_ExternalType::tags_init()
{
      const char* tags_array[3] = {"Properties","InputPin","OutputPin"};
      set<string> tmp_tags(tags_array,tags_array+3);
      tmp_tags.insert(P_ExternalType::P_Annotation::getTags().begin(), P_ExternalType::P_Annotation::getTags().end());
      return tmp_tags;
}

const set<string> P_ExternalType::attrs_init()
{
      const char* attrs_array[1] = {"id"};
      set<string> tmp_attrs(attrs_array,attrs_array+1);
      return tmp_attrs;
}

const set<string> P_ExternalType::tags(tags_init());
const set<string> P_ExternalType::attributes(attrs_init());

P_ExternalType::P_ExternalType(xmlTextReaderPtr parser, P_typdcl* graphtype):P_Annotation(parser), devtyp(0)
{
      parent=graphtype;
      valid_tags=&tags;
      valid_attributes=&attributes;
      // errorsAreFatal=false; // only during debug when we ignore subtags
}

P_ExternalType::~P_ExternalType()
{
}

int P_ExternalType::InsertSubObject(string subobj)
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
       // name for the properties is irrelevant to the application, only the Orchestrator cares about it. 
       string defaultPropsName = "External_";
       defaultPropsName += devtyp->Name();
       P_DataDecl devProps(GetParser(), devtyp->par, defaultPropsName + "_properties_t");
       if (devProps.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       int error = devProps.ParseNextSubObject(subobj);
       // if (error == ERR_SUCCESS) devtyp->pProps = devProps.type;
       if (error == ERR_SUCCESS) devtyp->pPropsD = devProps.code;
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
    return P_Annotation::InsertSubObject(subobj);
}

int P_ExternalType::SetObjectProperty(string prop, string value)
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
       devtyp->isExt = true;
       return ERR_SUCCESS;
    }
    return -ERR_UNEXPECTED_ATTR;
}
