#include "pigraphtype.h"
#include "pdevicetype.h"
#include "pmessagetype.h"
#include "psupervisordevicetype.h"
#include "pcodefragment.h"

PIGraphType::PIGraphType(const QString& name, PIGraphObject *parent) :
    PConcreteDef(name, "GraphType", QVector<int>({DEVTYPES, MSGTYPES, SHAREDCODE}), parent), graph_type(NULL)
{

}

const PIGraphObject* PIGraphType::appendSubObject(QXmlStreamReader* xml_def)
{
    PIGraphObject* sub_object = NULL;
    // reading the next element should get a subelement.
    switch (xml_def->tokenType())
    {
    // If we found an EndElement this should be the end of the GraphType definition.
    case QXmlStreamReader::EndElement:
    return this;
    case QXmlStreamReader::StartElement:
    switch (valid_elements.value(xml_def->name().toString(), OTHER))
    {
    case DEVTYPES:
    // bring in the device types. If some other type is found the import will abort.
    while (!(xml_def->error() || (xml_def->readNext() == QXmlStreamReader::EndElement)))
    {
          // validate against the legal element types
          if (xml_def->name().contains("DeviceType"))
          {
             if (xml_def->name() == "SupervisorDeviceType")
                sub_object = insertSubObject(SUPERDEVTYPES, new PSupervisorDeviceType(xml_def->attributes().value("", "id").toString(), this));
             else
                sub_object = insertSubObject(DEVTYPES, new PDeviceType(xml_def->attributes().value("", "id").toString(), this));
             sub_object->defineObject(xml_def);
          }
          else PIGraphObject::defineObject(xml_def); // some unexpected element which can be handled generally.
    }
    break;
    case MSGTYPES:
    while (!(xml_def->error() || (xml_def->readNext() == QXmlStreamReader::EndElement)))
    {
          // validate element. Can only be a MessageType in this case.
          if (xml_def->name() == "MessageType")
          {
             sub_object = insertSubObject(MSGTYPES, new PMessageType(xml_def->attributes().value("", "id").toString(), this));
             sub_object->defineObject(xml_def);
          }
          else PIGraphObject::defineObject(xml_def);
    }
    break;
    case SHAREDCODE:
    sub_object = insertSubObject(SHAREDCODE, new PCodeFragment(QString("SharedCodeFragment_%1").arg(numSubObjects(SHAREDCODE)), this));
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

// elaboration of a GraphType must be partial because GraphInstances will need to link to it to set up the P_taskl list.
P_typdcl* PIGraphType::elaborateGraphType(OrchBase* orch_root)
{
    // only elaborate if this has not already been done.
    if ((orch_root != NULL) && (graph_type == NULL))
    {
       graph_type =  new P_typdcl(orch_root, name().toStdString());
       // note here: order is critical because the XML schema is built in such a way as to expect a particular
       // declaration sequence. Order is properties>global code>messages>device types, or internal
       // references may be broken.
       // First get properties
       if (properties() != NULL)
       {
           graph_type->pPropsD = const_cast<PIDataType*>(properties())->elaborateDataType();
           graph_type->pPropsI = const_cast<PIDataType*>(properties())->elaborateDataDefault();
       }
       // then global code
       for (QVector<PIGraphObject*>::iterator code_frag = beginSubObjects(SHAREDCODE); code_frag < endSubObjects(SHAREDCODE); code_frag++)
       {
           graph_type->General.push_back(static_cast<PCodeFragment*>(*code_frag)->elaborateCodeFragment());
       }
       // then messages
       for (QVector<PIGraphObject*>::iterator message_type = beginSubObjects(MSGTYPES); message_type < endSubObjects(MSGTYPES); message_type++)
       {
           graph_type->P_messagev.push_back(static_cast<PMessageType*>(*message_type)->elaborateMessage(graph_type));
           graph_type->P_messagev.back()->MsgType = graph_type->P_messagev.size()-1;
       }
       // and finally the various device types
       for (QVector<PIGraphObject*>::iterator device_type = beginSubObjects(DEVTYPES); device_type < endSubObjects(DEVTYPES); device_type++)
       {
           graph_type->P_devtypv.push_back(static_cast<PDeviceType*>(*device_type)->elaborateDeviceType(graph_type));
           graph_type->P_devtypv.back()->idx = graph_type->P_devtypv.size()-1;
       }
    }
    return graph_type;
}
