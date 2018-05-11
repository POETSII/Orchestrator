#include "pmessagetype.h"
#include "pidatatype.h"

PMessageType::PMessageType(const QString& name, PIGraphObject *parent) : PAnnotatedDef(name, "MessageType", QVector<int>({MSG}), parent), message_type(NULL)
{

}

const PIGraphObject* PMessageType::appendSubObject(QXmlStreamReader* xml_def)
{
    PIGraphObject* sub_object = NULL;
    // reading the next element should get a subelement.
    switch (xml_def->tokenType())
    {
    // If we found an EndElement this should be the end of the DeviceType definition.
    case QXmlStreamReader::EndElement:
    return this;
    case QXmlStreamReader::StartElement:
    switch (valid_elements.value(xml_def->name().toString(), OTHER))
    {
    case MSG:
    // only one Message object is expected.
    if (numSubObjects(MSG)) xml_def->raiseError(QString("Error: more than one Message definition for MessageType %1").arg(name()));
    else
    {
        // use the specified type name if available
        if (xml_def->attributes().hasAttribute("", "cTypeName"))
            sub_object = insertSubObject(MSG, new PIDataType(xml_def->attributes().value("", "cTypeName").toString(), this));
        // otherwise the default name is {GraphTypeID}_{MessageTypeID}_message_t.
        else sub_object = insertSubObject(MSG, new PIDataType(QString("%1_%2_message_t").arg(static_cast<PIGraphObject*>(parent())->name()).arg(name()), this));
        sub_object->defineObject(xml_def);
    }
    break;
    default:
    // refer other types of token up the inheritance chain.
    return PAnnotatedDef::appendSubObject(xml_def);
    }
    return sub_object;
    default:
    // non-start elements can be immediately handled in the base (probably producing an error)
    return PIGraphBranch::appendSubObject(xml_def);
    }
}

P_message* PMessageType::elaborateMessage(P_typdcl* graph_type)
{
    if ((graph_type != NULL) && (message_type == NULL))
    {
       message_type = new P_message(graph_type, name().toStdString());
       if (numSubObjects(MSG))
       {
          PIDataType* msg_pyld = static_cast<PIDataType*>(subObject(MSG, 0));
          message_type->pPropsD = msg_pyld->elaborateDataType();
          message_type->MsgSize = msg_pyld->size();
       }
    }
    return message_type;
}
