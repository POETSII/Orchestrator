#include "P_InputPin.h"
#include "P_DataDecl.h"
#include "P_CodeFragment.h"
#include "P_devtyp.h"
#include "P_pintyp.h"
#include "P_typdcl.h"

const set<string> P_InputPin::tags_init()
{
      const char* tags_array[5] = {"Properties","State","OnReceive","Documentation","Metadata"};
      set<string> tmp_tags(tags_array,tags_array+5);
      return tmp_tags;
}

const set<string> P_InputPin::attrs_init()
{
  const char* attrs_array[2] = {"name","messageTypeId"};
      set<string> tmp_attrs(attrs_array,attrs_array+2);
      return tmp_attrs;
}

const set<string> P_InputPin::tags(tags_init());
const set<string> P_InputPin::attributes(attrs_init());

P_InputPin::P_InputPin(xmlTextReaderPtr parser, P_devtyp* devicetype) : P_Pparser(parser), pintyp(0), parent(devicetype)
{
      valid_tags=&tags;
      valid_attributes=&attributes;
      // errorsAreFatal=false; // only during debug when we ignore subtags
}

P_InputPin::~P_InputPin()
{
}

int P_InputPin::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error;
    if (!pintyp) return ERR_INVALID_NODE; // no Orchestrator object means the PinType had no ID. Abort parsing.
    if (isText) return ERR_SUCCESS;       // simply swallow the text of annotations
    // tag not in class-specific subset: refer to parent class.
    if (valid_tags->find(subobj) == valid_tags->end()) return ERR_UNEXPECTED_TAG;
    if (subobj == "Properties")
    {
       string defaultPropsName = parent->par->Name() + "_" + parent->Name() + "_" + pintyp->Name();
       P_DataDecl pinProps(GetParser(), parent->par, defaultPropsName + "_properties_t");
       if (pinProps.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       int error = pinProps.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS) pintyp->pPropsD = pinProps.code;
       // if (error == ERR_SUCCESS) pintyp->pProps = pinProps.type;
       return error;
    }
    if (subobj == "State")
    {
       string defaultStateName = parent->par->Name() + "_" + parent->Name() + "_" + pintyp->Name();
       P_DataDecl pinState(GetParser(), parent->par, defaultStateName + "_state_t");
       if (pinState.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       int error = pinState.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS) pintyp->pStateD = pinState.code;
       // if (error == ERR_SUCCESS) pintyp->pState = pinState.type;
       return error;
    }
    if (subobj == "OnReceive")
    {
       P_CodeFragment recvCode(GetParser());
       int error = recvCode.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS)  pintyp->pHandl = recvCode.code;
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

int P_InputPin::SetObjectProperty(string prop, string value)
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
       if (!msgId.empty())
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
    return -ERR_UNEXPECTED_ATTR;
}
