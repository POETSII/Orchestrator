#ifndef PIGRAPHINSTANCE_H
#define PIGRAPHINSTANCE_H

#include "pconcreteinstance.h"
#include "pigraphtype.h"
#include "psupervisordevicetype.h"
#include "pdeviceinstance.h"
#include "pedgeinstance.h"
#include "P_task.h"

class PIGraphInstance : public PConcreteInstance
{
public:
    PIGraphInstance(const QString& name = "", PIGraphObject *parent = 0);

    void defineObject(QXmlStreamReader* xml_def);
    const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);
    inline int deviceInstIndex(const PDeviceInstance* d_instance) const {return numSubObjects(DEVINSTS) ? subObjIndex(DEVINSTS, d_instance->name()) : 0;};
    inline int edgeInstIndex(const PEdgeInstance* e_instance) const {return numSubObjects(EDGEINSTS) ? subObjIndex(EDGEINSTS, e_instance->name()) : 0;};
    inline void setPropsDataType(const PIDataType* data_type = 0) {PConcreteInstance::setPropsDataType(graph_type->properties());};
    P_task* elaborateGraphInstance(OrchBase* orch_root);

    PIGraphType* graph_type;
    const PDeviceInstance* supervisor;

    enum vld_elem_types {OTHER, DEVINSTS, EDGEINSTS, SUPERINSTS};

private:

    P_task* graph_instance;
    QString graph_type_id;
    QString supervisor_type_id;

    const QHash<QString, vld_elem_types> valid_elements = {{"DeviceInstances", DEVINSTS}, {"EdgeInstances", EDGEINSTS}};

};

#endif // PIGRAPHINSTANCE_H
