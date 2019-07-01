#include "P_MessageType.h"
#include "P_DataDecl.h"
#include "P_message.h"

const set<string> P_MessageType::tags_init()
{
      const char* tags_array[1] = {"Message"};
      set<string> tmp_tags(tags_array,tags_array+1);
      tmp_tags.insert(P_MessageType::P_Annotation::getTags().begin(), P_MessageType::P_Annotation::getTags().end());
      return tmp_tags;
}

const set<string> P_MessageType::attrs_init()
{
      const char* attrs_array[1] = {"id"};
      set<string> tmp_attrs(attrs_array,attrs_array+1);
      return tmp_attrs;
}

const set<string> P_MessageType::tags(tags_init());
const set<string> P_MessageType::attributes(attrs_init());

P_MessageType::P_MessageType(xmlTextReaderPtr parser, P_typdcl* graphtype):P_Annotation(parser), message(0)
{
      parent=graphtype;
      valid_tags=&tags;
      valid_attributes=&attributes;
      // errorsAreFatal=false; // only during debug when we ignore subtags   
}

P_MessageType::~P_MessageType()
{
}

int P_MessageType::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error; 
    if (!message) return ERR_INVALID_NODE; // no Orchestrator object means the MessageType had no ID. Abort parsing.
    if (isText) return ERR_SUCCESS;        // simply swallow the text of annotations
    // tag not in class-specific subset.
    if (valid_tags->find(subobj) == valid_tags->end()) return ERR_UNEXPECTED_TAG;
    if (subobj == "Message")
    {
       // name for the properties is irrelevant to the application, only the Orchestrator cares about it. 
       string defaultMsgName = parent->Name() + "_" + message->Name();
       P_DataDecl msgData(GetParser(), parent, defaultMsgName + "_message_t");
       if (msgData.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       int error = msgData.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS) message->pPropsD = msgData.code;
       //if (error == ERR_SUCCESS) message->pProps = msgData.type;
       return error;
    }
    return  P_Annotation::InsertSubObject(subobj);
}

int P_MessageType::SetObjectProperty(string prop, string value)
{
    if (!parent) return -ERR_UNDEFINED_ORCH; // no associated orchestrator object
    // the only property we care about is the DeviceType id
    if (prop == "id")
    {
       // and we use it to set up its associated Orchestrator P_typdcl object. 
       id = value;
       // messages might have been default-constructed earlier if pins were encountered
       // before their associated message definitions. So we need to look up the possible
       // message and only create it if it's not already there in skeleton form.
       map<string, P_message*>::iterator msg = parent->P_messagem.find(id);
       if (msg == parent->P_messagem.end())
          message = new P_message(parent,id);
       else message = msg->second;
       return ERR_SUCCESS;
    }
    return -ERR_UNEXPECTED_ATTR;
}
