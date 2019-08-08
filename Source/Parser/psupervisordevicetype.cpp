#include "psupervisordevicetype.h"
#include "pcodefragment.h"
#include "piinputpin.h"
#include "pioutputpin.h"

PSupervisorDeviceType::PSupervisorDeviceType(const QString& name, PIGraphObject *parent) :
    PIGraphBranch(name, "SupervisorDeviceType", QVector<int>({INPIN, OUTPIN, CODE}), parent),
    dev_info_persistent(false), dev_props_valid(false), edge_endpts_exist(false), device_type(0)
{
    valid_elements["InputPin"] = INPIN;
    valid_elements["OutputPin"] = OUTPIN;
    valid_elements["Code"] = CODE;
}

void PSupervisorDeviceType::defineObject(QXmlStreamReader* xml_def)
{
     // first get any flags from the attributes.
     if (xml_def->attributes().hasAttribute("", "requiresPersistentLocalDeviceInfo") && (xml_def->attributes().value("", "requiresPersistentLocalDeviceInfo").toString() == "yes"))
         dev_info_persistent = true;
     if (xml_def->attributes().hasAttribute("", "requiresLocalDeviceProperties") && (xml_def->attributes().value("", "requiresLocalDeviceProperties").toString() == "yes"))
         dev_props_valid = true;
     if (xml_def->attributes().hasAttribute("", "requiresLocalEdgeEndpoints") && (xml_def->attributes().value("", "requiresLocalEdgeEndpoints").toString() == "yes"))
         edge_endpts_exist = true;
     // then usual parsing machinery
     PIGraphBranch::defineObject(xml_def);
}

const PIGraphObject* PSupervisorDeviceType::appendSubObject(QXmlStreamReader* xml_def)
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
    case CODE:
    // Currently the XML supports only a single code tag but there seems no reason not to allow more - possibly e.g. if
    // different fragments are defined in different input tools, and this can be supported seamlessly.
    sub_object = insertSubObject(CODE, new PCodeFragment(QString("SupervisorCode_%2").arg(numSubObjects(CODE)), this));
    sub_object->defineObject(xml_def);
    break;
    default:
    // refer other types of token to the base object. Most of the time this will probably produce an error.
    return PIGraphBranch::appendSubObject(xml_def);
    }
    return sub_object;
    default:
    // non-start elements can be immediately handled in the base (probably producing an error)
    return PIGraphBranch::appendSubObject(xml_def);
    }
}

P_devtyp* PSupervisorDeviceType::elaborateSupervisorDeviceType(P_typdcl* graph_type)
{
    if ((graph_type != NULL) && (device_type == NULL))
    {
       device_type = new P_devtyp(graph_type, name().toStdString());
       // set up the bits of code
       for (QVector<PIGraphObject*>::iterator p_code_frag = beginSubObjects(CODE); p_code_frag < endSubObjects(CODE); p_code_frag++)
       {
           // supervisors may have general code fragments
           CFrag* code_frag = static_cast<PCodeFragment*>(*p_code_frag)->elaborateCodeFragment();
           // add to the vector of general-purpose handlers
           device_type->pHandlv.push_back(code_frag);
       }
       // then deal with pins in the normal way
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
    }
    return device_type;
}
