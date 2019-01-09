#include "pigraphinstance.h"
#include "pigraphroot.h"
#include "P_typdcl.h"

PIGraphInstance::PIGraphInstance(const QString& name, PIGraphObject *parent) :
    PConcreteInstance(name, "GraphInstance", QVector<int>({DEVINSTS, EDGEINSTS, SUPERINSTS}), parent), graph_type(NULL), graph_instance(NULL), supervisor(NULL)
{

}

void PIGraphInstance::defineObject(QXmlStreamReader* xml_def)
{
     // no associated graph type is considered an error in the XML
     if (!xml_def->attributes().hasAttribute("", "graphTypeId"))
        return signalXmlError(xml_def, QString("Error:no GraphType defined for GraphInstance %1").arg(name()));
     // otherwise set up the graph type links. No existing graph type as yet is not considered fatal;
     // a later lookup can set the links if necessary.
     graph_type_id = xml_def->attributes().value("", "graphTypeId").toString();
     PIGraphRoot* root_object = dynamic_cast<PIGraphRoot*>(parent());
     if (root_object != NULL) graph_type = static_cast<const PIGraphType*>(root_object->subObject(PIGraphRoot::GTYPE, graph_type_id));
     if (xml_def->attributes().hasAttribute("", "supervisorDeviceTypeId"))
     {
         supervisor_type_id = xml_def->attributes().value("", "supervisorDeviceTypeId").toString();
         supervisor = new PDeviceInstance(true, supervisor_type_id, this);
     }
     else if (supervisor = new PDeviceInstance(true, "", this)) supervisor_type_id = supervisor->name();
     PIGraphBranch::defineObject(xml_def);
}

const PIGraphObject* PIGraphInstance::appendSubObject(QXmlStreamReader* xml_def)
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
    case DEVINSTS:
    // bring in the device instances. If some other type is found the import will abort.
    while (!(xml_def->error() || (xml_def->readNext() == QXmlStreamReader::EndElement)))
    {
          // validate against the legal element types
          if (xml_def->name() == "DevI")
          {
             // Check if the device instance already exists - don't need to create one if it does. This also handles
             // GraphInstanceMetadataPatch declarations which will simply be referred up through the call chain.
             int dev_idx = subObjIndex(DEVINSTS, xml_def->attributes().value("", "id").toString());
             // error if there is no device instance vector - this should never happen.
             if (dev_idx < -1) return PIGraphBranch::appendSubObject(xml_def);
             if (dev_idx == -1)
                sub_object = insertSubObject(DEVINSTS, new PDeviceInstance(false, xml_def->attributes().value("", "id").toString(), this));
             else sub_object = subObject(DEVINSTS, dev_idx);
             sub_object->defineObject(xml_def);
          }
          else PIGraphObject::defineObject(xml_def); // some unexpected element which can be handled generally.
    }
    break;
    case EDGEINSTS:
    while (!(xml_def->error() || (xml_def->readNext() == QXmlStreamReader::EndElement)))
    {
          // validate element. Can only be an EdgeInstance.
          if (xml_def->name() == "EdgeI")
          {
             // no path will create a name which is guaranteed not to parse to a valid edge but which is identifiably unique.
             if (!xml_def->attributes().hasAttribute("", "path"))
                sub_object = insertSubObject(EDGEINSTS, new PEdgeInstance(QString("GraphI_%1_EdgeI_%2:-:").arg(name()).arg(numSubObjects(EDGEINSTS)), this));
             // real paths create a new edge if none exists or has the same handling as devices for pre-existing edges, including the
             // processing of GraphInstanceMetadataPatches.
             else
             {
                int edge_idx = subObjIndex(EDGEINSTS, xml_def->attributes().value("", "path").toString());
                // no vector of edges - a serious error.
                if (edge_idx < -1) return PIGraphBranch::appendSubObject(xml_def);
                if (edge_idx == -1)
                   sub_object = insertSubObject(EDGEINSTS, new PEdgeInstance(xml_def->attributes().value("", "path").toString(), this));
                else sub_object = subObject(EDGEINSTS, edge_idx);
             }
             sub_object->defineObject(xml_def);
          }
          else PIGraphObject::defineObject(xml_def);
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

P_task* PIGraphInstance::elaborateGraphInstance(OrchBase* orch_root)
{
      if (graph_instance == NULL)
      {
         // first set up the task and graph
         graph_instance = new P_task(orch_root, name().toStdString());
         // give it a (dummy) filename equal to the root's name.
         graph_instance->filename = static_cast<PIGraphObject*>(parent())->name().toStdString();
         // add to the task map
         orch_root->P_taskm[name().toStdString()] = graph_instance;
         graph_instance->pD = new D_graph(graph_instance, QString(name()+"_graph").toStdString());
         graph_instance->pP_typdcl = const_cast<PIGraphType*>(graph_type)->elaborateGraphType(orch_root);
         graph_instance->pP_typdcl->P_taskl.push_back(graph_instance);
         graph_instance->pD->pD = graph_instance->pP_typdcl;
         if (properties()) graph_instance->pD->pPropsI = const_cast<PIDataValue*>(properties())->elaborateDataValue();
         // now start adding device instances to the graph
         for (unsigned int dev_inst = 0; dev_inst < numSubObjects(DEVINSTS); dev_inst++)
         {
             P_device* device = static_cast<PDeviceInstance*>(subObject(DEVINSTS, dev_inst))->elaborateDeviceInstance(graph_instance->pD);
             device->idx = dev_inst;
             graph_instance->pD->G.InsertNode(dev_inst, device);
         }
         // insert a supervisor if one is available
         if (supervisor != NULL)
         {
             graph_instance->pSup = static_cast<P_super*>(const_cast<PDeviceInstance*>(supervisor)->elaborateSupervisorInstance(graph_instance->pD));
             graph_instance->pSup->idx = P_device::super_idx;
             graph_instance->pD->G.InsertNode(P_device::super_idx, graph_instance->pSup);
             orch_root->P_superm[graph_instance->pSup->Name()] = graph_instance->pSup;
         }
         // and then add all the edge instances
         for (QVector<PIGraphObject*>::iterator edge = beginSubObjects(EDGEINSTS); edge < endSubObjects(EDGEINSTS); edge++)
             static_cast<PEdgeInstance*>(*edge)->elaborateEdge(graph_instance->pD);
         graph_instance->PoL.IsPoL = false; // set the flag to indicate non-proof-of-life
      }
      return graph_instance;
}

