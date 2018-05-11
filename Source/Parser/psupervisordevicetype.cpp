#include "psupervisordevicetype.h"
#include "pcodefragment.h"
#include "piinputpin.h"
#include "pioutputpin.h"

PSupervisorDeviceType::PSupervisorDeviceType(const QString& name, PIGraphObject *parent) :
    PIGraphBranch(name, "SupervisorDeviceType", QVector<int>({INPIN, OUTPIN, CODE}), parent),
    dev_info_persistent(false), dev_props_valid(false), edge_endpts_exist(false)
{

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
    return new P_devtyp(graph_type, name().toStdString());
}
