#include "P_OutputPin.h"
#include "P_CodeFragment.h"
#include "P_devtyp.h"
#include "P_pintyp.h"
#include "stdlib.h"
#include "P_typdcl.h"

const set<string> P_OutputPin::tags_init()
{
      const char* tags_array[3] = {"OnSend","Documentation","Metadata"};
      set<string> tmp_tags(tags_array,tags_array+3);
      return tmp_tags;
}

const set<string> P_OutputPin::attrs_init()
{
  const char* attrs_array[4] = {"name","messageTypeId","indexed","priority"};
      set<string> tmp_attrs(attrs_array,attrs_array+4);
      return tmp_attrs;
}

const set<string> P_OutputPin::tags(tags_init());
const set<string> P_OutputPin::attributes(attrs_init());

P_OutputPin::P_OutputPin(xmlTextReaderPtr parser, P_devtyp* devicetype) : P_Pparser(parser), indexedSend(false), priority(-1), pintyp(0), parent(devicetype)
{
      valid_tags=&tags;
      valid_attributes=&attributes;
      // errorsAreFatal=false; // only during debug when we ignore subtags
}

P_OutputPin::~P_OutputPin()
{
}

int P_OutputPin::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error;
    if (!pintyp) return ERR_INVALID_NODE; // no Orchestrator object means the PinType had no ID. Abort parsing.
    if (isText) return ERR_SUCCESS;       // simply swallow the text of annotations
    // tag not in class-specific subset: refer to parent class.
    if (valid_tags->find(subobj) == valid_tags->end()) return ERR_UNEXPECTED_TAG;
    if (subobj == "OnSend")
    {
       P_CodeFragment sendCode(GetParser());
       int error = sendCode.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS)  pintyp->pHandl = sendCode.code;
       return error; 
    }
    if (subobj == "Documentation" || subobj == "Metadata")
    {
       // descend immediately into the subtags; no need to recur.
       isText = true;
       int error = ParseNextSubObject(subobj);
       isText = false;
       return error;
    }
    return ERR_UNEXPECTED_TAG; // should never reach this line
}

int P_OutputPin::SetObjectProperty(string prop, string value)
{
    if (!parent) return -ERR_UNDEFINED_ORCH; // no associated orchestrator object
    if (prop == "name")
    {
       // sets the pin name
       id = value;
       // this immediately creates a pin type object. We might like to
       // change this so that the object is created only if the entire parse of its
       // associated subtree succeeds.
       pintyp = new P_pintyp(parent,id);
       if (msgId.empty())
       {
	  map<string, P_message*>::iterator msgType = parent->par->P_messagem.find(msgId);
          // this immediately creates a message type object. We might like to
          // change this so that the object is created only if the entire parse of its
          // associated subtree succeeds.
          if (msgType == parent->par->P_messagem.end())
          {
	     P_message* msg = new P_message(parent->par,msgId);
             parent->par->P_messagem[msgId] = pintyp->pMsg = msg;	     
          }
	  else pintyp->pMsg = msgType->second;
       }
       return ERR_SUCCESS;
    }
    if (prop == "messageTypeId")
    {
       msgId = value;
       // if we have the pin type, its associated message type may need to be set up
       if (pintyp) 
       {
          map<string, P_message*>::iterator msgType = parent->par->P_messagem.find(msgId);
          // this immediately creates a message type object. We might like to
          // change this so that the object is created only if the entire parse of its
          // associated subtree succeeds.
          if (msgType == parent->par->P_messagem.end())
          {
	     P_message* msg = new P_message(parent->par,msgId);
             parent->par->P_messagem[msgId] = pintyp->pMsg = msg;	     
          }
	  else pintyp->pMsg = msgType->second;
       }
       return ERR_SUCCESS;
    }
    // indexed attribute not used for the moment but we capture it anyway
    if (prop == "indexed")
    {
       if (value == "true" || value == "True" || (atoi(value.c_str()) != 0)) indexedSend = true;
       return ERR_SUCCESS;
    }
    // priority can be set and determines the position in the devicetype's pin vector.
    if (prop == "priority")
    {
       priority = atoi(value.c_str());
       return ERR_SUCCESS;
    }   
    return -ERR_UNEXPECTED_ATTR;
}
