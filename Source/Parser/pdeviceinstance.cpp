#include "pdeviceinstance.h"
#include "pigraphtype.h"
#include "pigraphinstance.h"

PDeviceInstance::PDeviceInstance(bool is_supervisor, const QString &name, PIGraphObject *parent) :
    PConcreteInstance(name, is_supervisor ? QString("SDevI") : QString("DevI"), is_supervisor ? QVector<int>() : QVector<int>({STATE}), parent), device(NULL), supervisor(NULL), device_type_id(""), device_type(NULL), supervisor_type(NULL)
{
    valid_elements["S"] = STATE;

    if (is_supervisor && parent)
    {
        device_type_id = name;
        setDeviceType();
        setName(QString("%1_%2_inst").arg(parent->name()).arg(supervisor_type ? supervisor_type->name() : "sup_unknown"));
    }
}


void PDeviceInstance::defineObject(QXmlStreamReader* xml_def)
{
    if (xmlName() == "SDevI") return; // supervisors have no internal structure.
    // no associated device type is considered an error in the XML.
    if (!xml_def->attributes().hasAttribute("", "type"))
       return signalXmlError(xml_def, QString("Error:no DeviceType defined for DeviceInstance %1").arg(name()));
    // otherwise set up the device type link. No existing device type as yet is not considered fatal;
    // a later lookup can set the link if necessary.
    device_type_id = xml_def->attributes().value("", "type").toString();
    setDeviceType();
    PIGraphBranch::defineObject(xml_def);
}

const PIGraphObject* PDeviceInstance::appendSubObject(QXmlStreamReader* xml_def)
{
    // if we got to this point with a supervisor something has gone wrong with the parsing.
    if (xmlName() == "SDevI") return PIGraphBranch::appendSubObject(xml_def);
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
    case STATE:
    // only one State object is expected.
    if (numSubObjects(STATE)) xml_def->raiseError(QString("Error: more than one State definition for DeviceInstance %1").arg(name()));
    else
    {
       // set the name to the hierarchical ID chain if available.
       if (device_type && device_type->numSubObjects(PDeviceType::STATE))
       {
           sub_object = insertSubObject(STATE, new PIDataValue(QString("%1_DevInst_%2_%3_val").arg(static_cast<PIGraphObject*>(parent())->name()).arg(name()).arg(device_type->constSubObject(PDeviceType::STATE, 0)->name()), this));
           static_cast<PIDataValue*>(sub_object)->data_type = static_cast<const PIDataType*>(device_type->constSubObject(PDeviceType::STATE, 0));
       }
       // otherwise the default name is DevInst_{DeviceInstanceID}_state_t_val
       else sub_object = insertSubObject(STATE, new PIDataValue(QString("DevInst_%1_state_t_val").arg(name()), this));
       sub_object->defineObject(xml_def);
    }
    break;
    default:
    // refer other types of token up the inheritance chain.
    return PConcreteInstance::appendSubObject(xml_def);
    }
    return sub_object;
    default:
    // non-start elements can be immediately handled in the base (probably producing an error)
    return PIGraphBranch::appendSubObject(xml_def);
    }
}

P_device* PDeviceInstance::elaborateDeviceInstance(D_graph* graph_instance)
{
    if ((graph_instance != NULL) && (device == NULL)) // don't rebuild an existing device
    {
       // device instances belong to the D_graph and not the P_task, although P_task operates the machinery to create device instances.
       device = new P_device(graph_instance, name().toStdString());
       // once created, set up the associated device type, state, and properties as appropriate.
       if (device_type == NULL) setDeviceType(); // no device type means it may have been defined later
       if (device_type != NULL) // but it still could turn out to be empty
       {
           device->pP_devtyp = const_cast<PDeviceType*>(device_type)->elaborateDeviceType();
           if (numSubObjects(STATE))
           {
              PIDataValue* devState = static_cast<PIDataValue*>(subObject(STATE, 0));
              if (!devState->data_type) devState->data_type = static_cast<const PIDataType*>(device_type->constSubObject(PDeviceType::STATE, 0));
              device->pStateI = static_cast<PIDataValue*>(subObject(STATE, 0))->elaborateDataValue();
           }
          if (properties() != NULL)
          {
             if (!(properties()->data_type)) setPropsDataType(device_type->properties());
             device->pPropsI = const_cast<PIDataValue*>(properties())->elaborateDataValue();
          }
       }
    }
    return device;
}

P_super* PDeviceInstance::elaborateSupervisorInstance(D_graph* graph_instance)
{
    if ((graph_instance != NULL) && (supervisor == NULL)) // don't rebuild an existing device
    {
       // supervisor instances belong to the top-level orchestrator
       supervisor = new P_super(name().toStdString());
       // once created, set up the associated device type, state, and properties as appropriate.
       if (supervisor_type != NULL)
       {
           supervisor->pP_devtyp = const_cast<PSupervisorDeviceType*>(supervisor_type)->elaborateSupervisorDeviceType(graph_instance->par->pP_typdcl);
       }
    }
    return supervisor;
}

void PDeviceInstance::setDeviceType()
{
     if (parent())
     {
        PIGraphInstance* instParent = dynamic_cast<PIGraphInstance*>(parent());
        if (instParent && (instParent->graph_type != NULL))
        {
           if (isSupervisorInstance())
           {
               if (device_type_id.isEmpty()) // no supervisor specified
               {
                   // default to the first available supervisor if none specified
                   if (instParent->graph_type->numSubObjects(PIGraphType::SUPERDEVTYPES))
                      supervisor_type = static_cast<const PSupervisorDeviceType*>(instParent->graph_type->constSubObject(PIGraphType::SUPERDEVTYPES, 0));
                   // or to the default supervisor if there are no defined supervisor types.
                   else
                      supervisor_type = static_cast<const PSupervisorDeviceType*>(instParent->graph_type->insertSubObject(PIGraphType::SUPERDEVTYPES, new PSupervisorDeviceType("_DEFAULT_SUPERVISOR_", instParent->graph_type)));
                   device_type_id = supervisor_type->name();
               }
               // otherwise use the supervisor specified
               else supervisor_type = static_cast<const PSupervisorDeviceType*>(instParent->graph_type->constSubObject(PIGraphType::SUPERDEVTYPES, device_type_id));
           }
           else device_type = static_cast<const PDeviceType*>(instParent->graph_type->constSubObject(PIGraphType::DEVTYPES, device_type_id));
        }
     }
}
