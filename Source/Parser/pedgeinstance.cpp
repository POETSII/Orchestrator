#include "pedgeinstance.h"
#include "pigraphinstance.h"
#include "pidatavalue.h"
#include "build_defs.h"

PEdgeInstance::PEdgeInstance(const QString& name, PIGraphObject *parent) : PConcreteInstance(name, "EdgeI", QVector<int>({STATE}), parent), containing_graph(NULL)
{

}

void PEdgeInstance::defineObject(QXmlStreamReader* xml_def)
{
    parsePath();
    PIGraphBranch::defineObject(xml_def);
}

const PIGraphObject* PEdgeInstance::appendSubObject(QXmlStreamReader* xml_def)
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
    case STATE:
    // only one State object is expected.
    if (numSubObjects(STATE)) xml_def->raiseError(QString("Error: more than one State definition for EdgeInstance %1").arg(name()));
    else
    {
       // set the name to the hierarchical ID chain if available.
       if (path.dst.pin && path.dst.pin->numSubObjects(PIInputPin::STATE))
       {
           sub_object = insertSubObject(STATE, new PIDataValue(QString("%1_EdgeInst_%2_%3_val").arg(static_cast<PIGraphObject*>(parent())->name()).arg(name()).arg(path.dst.pin->subObject(PIInputPin::STATE, 0)->name()), this));
           static_cast<PIDataValue*>(sub_object)->data_type = static_cast<const PIDataType*>(path.dst.pin->subObject(PIInputPin::STATE, 0));
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

void PEdgeInstance::elaborateEdge(D_graph* graph_rep)
{
     if (containing_graph == NULL) // no need to do anything if the edge has already been elaborated
     {
        PIGraphInstance* parent_instance = dynamic_cast<PIGraphInstance*>(parent());
        if (parent_instance) // no parent instance would be a serious elaboration error
        {
           // generate or retrieve all the objects required to insert the node into the graph
           P_pin* src_pin = new P_pin(graph_rep, path.src.pin->name().toStdString());
           P_pin* dst_pin = new P_pin(graph_rep, path.dst.pin->name().toStdString());
           P_message* msg_type = const_cast<PMessageType*>(path.src.pin->msgType())->elaborateMessage();
           path_index.msg_i = parent_instance->subObjIndex(PIGraphInstance::EDGEINSTS, name());
           path_index.src_i = parent_instance->subObjIndex(PIGraphInstance::DEVINSTS, path.src.device->name());
           path_index.dst_i = parent_instance->subObjIndex(PIGraphInstance::DEVINSTS, path.dst.device->name());
           // set up the pins. Input pins may have properties and state.
           src_pin->pP_pintyp = const_cast<PIOutputPin*>(path.src.pin)->elaboratePinType();
           dst_pin->pP_pintyp = const_cast<PIInputPin*>(path.dst.pin)->elaboratePinType();
           if (properties() != NULL) dst_pin->pPropsI = const_cast<PIDataValue*>(properties())->elaborateDataValue();
           if (numSubObjects(STATE)) dst_pin->pStateI = static_cast<PIDataValue*>(subObject(STATE, 0))->elaborateDataValue();
           // pin indices are keys into a multimap, so duplicates are OK.
           src_pin->idx = path_index.opin_i = src_pin->pP_pintyp->idx;
           // the destination index needs to capture both the pin number and what will be the local array index for the edge at the pin.
           // searching the pin lists would be wildly inefficient, so we store a value in the DeviceInstance for each pin giving the
           // current count of internal edges.
           P_device* dest_device = path.dst.device->getDevice();
           dst_pin->idx = path_index.ipin_i = ((dst_pin->pP_pintyp->idx << PIN_POS) | dest_device->ipin_idxs[dst_pin->pP_pintyp->idx]++);
           //src_pin->idx = path_index.opin_i = 2*path_index.msg_i;
           //dst_pin->idx = path_index.ipin_i = 2*path_index.msg_i+1;
           // now we are ready to insert into the graph, updating the internal representation if the insertion succeeds.
           if (graph_rep->G.InsertArc(path_index.msg_i, path_index.src_i, path_index.dst_i, msg_type, path_index.opin_i, src_pin, path_index.ipin_i, dst_pin)) containing_graph = graph_rep;
        }
     }
}

void PEdgeInstance::parsePath()
{
    QVector<QStringRef> dst_src = name().splitRef("-");
    QVector<QStringRef> dst_id_pin = dst_src[DST].split(":");
    QVector<QStringRef> src_id_pin = dst_src[SRC].split(":");
    PIGraphInstance* parent_instance = dynamic_cast<PIGraphInstance*>(parent());
    if (parent_instance)
    {
        if (dst_id_pin[0].isEmpty())
        {
            path.dst.device = parent_instance->supervisor;
            if (path.dst.device)
            {
               const PSupervisorDeviceType* supervisor_device = dynamic_cast<const PSupervisorDeviceType*>(path.dst.device->deviceType());
               if (supervisor_device) path.dst.pin = static_cast<const PIInputPin*>(supervisor_device->subObject(PSupervisorDeviceType::INPIN, dst_id_pin[1].toString()));
            }
        }
        else
        {
            path.dst.device = static_cast<const PDeviceInstance*>(parent_instance->subObject(PIGraphInstance::DEVINSTS, dst_id_pin[0].toString()));
            if (path.dst.device)
            {
                const PDeviceType* dst_devicetype = dynamic_cast<const PDeviceType*>(path.dst.device->deviceType());
                if (dst_devicetype) path.dst.pin = static_cast<const PIInputPin*>(dst_devicetype->subObject(PDeviceType::INPIN, dst_id_pin[1].toString()));
            }
        }
        if (src_id_pin[0].isEmpty())
        {
            path.src.device = parent_instance->supervisor;
            if (path.src.device)
            {
                const PSupervisorDeviceType* supervisor_device = dynamic_cast<const PSupervisorDeviceType*>(path.src.device->deviceType());
                if (supervisor_device) path.src.pin = static_cast<const PIOutputPin*>(supervisor_device->subObject(PSupervisorDeviceType::OUTPIN, src_id_pin[1].toString()));
            }
        }
        else
        {
            path.src.device = static_cast<const PDeviceInstance*>(parent_instance->subObject(PIGraphInstance::DEVINSTS, src_id_pin[0].toString()));
            if (path.src.device)
            {
                const PDeviceType* src_devicetype = dynamic_cast<const PDeviceType*>(path.src.device->deviceType());
                if (src_devicetype) path.src.pin = static_cast<const PIOutputPin*>(src_devicetype->subObject(PDeviceType::OUTPIN, src_id_pin[1].toString()));
            }
        }
    }
}
