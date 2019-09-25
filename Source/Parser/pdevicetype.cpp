#include "pdevicetype.h"
#include "piinputpin.h"
#include "pioutputpin.h"
#include "pcodefragment.h"

PDeviceType::PDeviceType(const QString& name, PIGraphObject *parent) :
    PConcreteDef(name, "DeviceType", QVector<int>({INPIN, OUTPIN, STATE, CODE}), parent), device_type(NULL)
{
    valid_elements["InputPin"] = INPIN;
    valid_elements["OutputPin"] = OUTPIN;
    valid_elements["State"] = STATE;
    valid_elements["SharedCode"] = CODE;
    valid_elements["DeviceSharedCode"] = CODE;
    valid_elements["ReadyToSend"] = CODE;
    valid_elements["OnCompute"] = CODE;
}

const PIGraphObject* PDeviceType::appendSubObject(QXmlStreamReader* xml_def)
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
    case INPIN:
    sub_object = insertSubObject(INPIN, new PIInputPin(xml_def->attributes().value("", "name").toString(), this));
    sub_object->defineObject(xml_def);
    break;
    case OUTPIN:
    sub_object = insertSubObject(OUTPIN, new PIOutputPin(xml_def->attributes().value("", "name").toString(), this));
    sub_object->defineObject(xml_def);
    break;
    case STATE:
    // only one State object is expected.
    if (numSubObjects(STATE)) xml_def->raiseError(QString("Error: more than one State definition for DeviceType %1").arg(name()));
    else
    {
       // set the name to the C type name if available
       if (xml_def->attributes().hasAttribute("", "cTypeName"))
               sub_object = insertSubObject(STATE, new PIDataType(xml_def->attributes().value("", "cTypeName").toString(), this));
       // otherwise the default name is {GraphTypeID}_{DeviceTypeID}_state_t
       else sub_object = insertSubObject(STATE, new PIDataType(QString("%1_%2_state_t").arg(static_cast<PIGraphObject*>(parent())->name()).arg(name()), this));
       sub_object->defineObject(xml_def);
    }
    break;
    case CODE:
    // Code objects could either be specific named handlers or general code.
    // For general code we insert a number to put it in sequence; these may not be contiguous but they are guaranteed
    // to be montonically increasing, which is all we require.
    if ((xml_def->name() == "OnCompute") || (xml_def->name() == "ReadyToSend"))
       sub_object = insertSubObject(CODE, new PCodeFragment(xml_def->name().toString(), this));
    else
       sub_object = insertSubObject(CODE, new PCodeFragment(QString("%1_code_%2").arg(xml_def->name().toString()).arg(numSubObjects(CODE)), this));
    sub_object->defineObject(xml_def);
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

P_devtyp* PDeviceType::elaborateDeviceType(P_typdcl* graph_type)
{
   if ((graph_type != NULL) && (device_type == NULL))
   {
      device_type = new P_devtyp(graph_type, name().toStdString());
      // set up the bits of code
      for (QVector<PIGraphObject*>::iterator p_code_frag = beginSubObjects(CODE); p_code_frag < endSubObjects(CODE); p_code_frag++)
      {
          // ready-to-send and onIdle have special pointers
          CFrag* code_frag = static_cast<PCodeFragment*>(*p_code_frag)->elaborateCodeFragment();
          if (static_cast<PCodeFragment*>(*p_code_frag)->name() == "OnCompute") device_type->pOnIdle = code_frag;
          else if (static_cast<PCodeFragment*>(*p_code_frag)->name() == "ReadyToSend") device_type->pOnRTS = code_frag;
          // otherwise add to the vector of general-purpose handlers
          else device_type->pHandlv.push_back(code_frag);
      }
      // then deal with pins
      for (QVector<PIGraphObject*>::iterator p_inpin = beginSubObjects(INPIN); p_inpin < endSubObjects(INPIN); p_inpin++)
      {
          device_type->P_pintypIv.push_back(static_cast<PIInputPin*>(*p_inpin)->elaboratePinType(device_type));
          device_type->P_pintypIv.back()->idx = device_type->P_pintypIv.size()-1;
      }
      for (QVector<PIGraphObject*>::iterator p_outpin = beginSubObjects(OUTPIN); p_outpin < endSubObjects(OUTPIN); p_outpin++)
      {
          device_type->P_pintypOv.push_back(static_cast<PIOutputPin*>(*p_outpin)->elaboratePinType(device_type));
          device_type->P_pintypOv.back()->idx = device_type->P_pintypOv.size()-1;
      }
      // and then the various device variables
      if (numSubObjects(STATE))
      {
          device_type->pStateD = static_cast<PIDataType*>(subObject(STATE, 0))->elaborateDataType();
          device_type->pStateI = static_cast<PIDataType*>(subObject(STATE, 0))->elaborateDataDefault();

      }
      if (properties() != NULL)
      {
          device_type->pPropsD = const_cast<PIDataType*>(properties())->elaborateDataType();
          device_type->pPropsI = const_cast<PIDataType*>(properties())->elaborateDataDefault();
      }
   }
   return device_type;
}
