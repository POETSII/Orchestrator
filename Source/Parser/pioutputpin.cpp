#include "pioutputpin.h"

PIOutputPin::PIOutputPin(const QString& name, PIGraphObject *parent) : PAnnotatedDef(name, "OutputPin", QVector<int>({ONSEND}), parent), PIPin(parent)
{
    valid_elements["OnSend"] = ONSEND;
}

void PIOutputPin::defineObject(QXmlStreamReader* xml_def)
{
    // first get pin attributes.
    if (!xml_def->attributes().hasAttribute("", "messageTypeId"))
       // no message type ID is considered to be an error in the XML.
       return signalXmlError(xml_def, QString("Error: no message type ID specified for output pin %1").arg(name()));
    setMsgType(xml_def->attributes().value("", "messageTypeId").toString());
    if (xml_def->attributes().hasAttribute("", "priority"))
        priority = xml_def->attributes().value("", "priority").toString().toInt();
    // and the same old machinery can be reused, even though only one other tag is expected.
    PIGraphBranch::defineObject(xml_def);
}

const PIGraphObject* PIOutputPin::appendSubObject(QXmlStreamReader* xml_def)
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
    case ONSEND:
    // only one OnSend object is expected. More means there is some error in the XML. This might change in future
    // if definitions allow the OnSend fragment to be assembled from multiple subfragments.
    if (numSubObjects(ONSEND)) xml_def->raiseError(QString("Error: more than one OnSend definition for Pin %1").arg(name()));
    else
    {
        sub_object = insertSubObject(ONSEND, new PCodeFragment("OnSend", this));
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

P_pintyp* PIOutputPin::elaboratePinType(P_devtyp* device_type)
{
    if ((device_type != NULL) && (pin_type == NULL))
    {
       pin_type = new P_pintyp(device_type, name().toStdString());
       if (numSubObjects(ONSEND)) pin_type->pHandl = static_cast<PCodeFragment*>(subObject(ONSEND, 0))->elaborateCodeFragment();
       pin_type->pMsg = const_cast<PMessageType*>(msgType())->elaborateMessage();
    }
    return pin_type;
}
