#include "piinputpin.h"
#include "pcodefragment.h"
#include <stdio.h>

PIInputPin::PIInputPin(const QString& name, PIGraphObject *parent) : PConcreteDef(name, "InputPin", QVector<int>({ONRECEIVE, STATE}), parent), PIPin(parent)
{
    valid_elements["OnReceive"] = ONRECEIVE;
    valid_elements["State"] = STATE;
}

void PIInputPin::defineObject(QXmlStreamReader* xml_def)
{
    // first get pin attributes.
    if (!xml_def->attributes().hasAttribute("", "messageTypeId"))
       // no message type ID is considered to be an error in the XML.
       return signalXmlError(xml_def, QString("Error: no message type ID specified for input pin %1").arg(name()));
    setMsgType(xml_def->attributes().value("", "messageTypeId").toString());
    // as always refer to the base object to grind through the subobjects
    PIGraphBranch::defineObject(xml_def);
}

const PIGraphObject* PIInputPin::appendSubObject(QXmlStreamReader* xml_def)
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
    case ONRECEIVE:
    // only one OnReceive object is expected. More means there is some error in the XML. This might change in future
    // if definitions allow the OnReceive fragment to be assembled from multiple subfragments.
    if (numSubObjects(ONRECEIVE)) xml_def->raiseError(QString("Error: more than one OnReceive definition for Pin %1").arg(name()));
    else
    {
        sub_object = insertSubObject(ONRECEIVE, new PCodeFragment("OnReceive", this));
        sub_object->defineObject(xml_def);
    }
    break;
    case STATE:
    // likewise only one State object is expected and this is unlikely to change.
    if (numSubObjects(STATE)) xml_def->raiseError(QString("Error: more than one State definition for Pin %1").arg(name()));
    else
    {
        // use the specified type name if available
        if (xml_def->attributes().hasAttribute("", "cTypeName"))
            sub_object = insertSubObject(STATE, new PIDataType(xml_def->attributes().value("", "cTypeName").toString(), this));
        // otherwise the default name is {GraphTypeID}_{DeviceTypeID}_{PinName}_state_t.
        else sub_object = insertSubObject(STATE, new PIDataType(QString("%1_%2_%3_state_t").arg(static_cast<PIGraphObject*>(PConcreteDef::parent()->parent())->name()).arg(static_cast<PIGraphObject*>(PConcreteDef::parent())->name()).arg(name()), this));
        sub_object->defineObject(xml_def);
    }
    break;
    default:
    // refer other types of token up the inheritance chain.
    return PConcreteDef::appendSubObject(xml_def);
    }
    return sub_object;
    default:
    // non-start elements can be immediately handled in the base (probably producing an error)
    return PIGraphBranch::appendSubObject(xml_def);
    }
}

P_pintyp* PIInputPin::elaboratePinType(P_devtyp* device_type)
{
    if ((device_type != NULL) && (pin_type == NULL))
    {
       pin_type = new P_pintyp(device_type, name().toStdString());
       if (properties() != NULL)
       {
           pin_type->pPropsD = const_cast<PIDataType*>(properties())->elaborateDataType();
           pin_type->pPropsI = const_cast<PIDataType*>(properties())->elaborateDataDefault();
           // pin_type->PinPropsSize = properties()->size();
       }
       if (numSubObjects(STATE))
       {
           pin_type->pStateD = static_cast<PIDataType*>(subObject(STATE, 0))->elaborateDataType();
           pin_type->pStateI = static_cast<PIDataType*>(subObject(STATE, 0))->elaborateDataDefault();
           // pin_type->PinStateSize = static_cast<PIDataType*>(subObject(STATE,0))->size();
       }
       if (numSubObjects(ONRECEIVE)) pin_type->pHandl = static_cast<PCodeFragment*>(subObject(ONRECEIVE, 0))->elaborateCodeFragment();
       pin_type->pMsg = const_cast<PMessageType*>(msgType())->elaborateMessage();
    }
    return pin_type;

}
