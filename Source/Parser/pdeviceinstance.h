#ifndef PDEVICEINSTANCE_H
#define PDEVICEINSTANCE_H

#include "pconcreteinstance.h"
#include "pdevicetype.h"
#include "psupervisordevicetype.h"
#include "P_device.h"
#include "P_super.h"

class PDeviceInstance : public PConcreteInstance
{
public:
    PDeviceInstance(bool is_supervisor = false, const QString& name = "", PIGraphObject *parent = 0);

    void defineObject(QXmlStreamReader* xml_def);
    const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);
    P_device* elaborateDeviceInstance(D_graph* graph_instance = NULL);
    P_super* elaborateSupervisorInstance(D_graph* graph_instance = NULL);
    inline P_device* getDevice() const {return device;};
    inline bool isSupervisorInstance() {return xmlName() == "SDevI";};
    inline const PIGraphBranch* deviceType() const {return xmlName() == "SDevI" ? static_cast<const PIGraphBranch*>(supervisor_type) : static_cast<const PIGraphBranch*>(device_type);};
    inline void setPropsDataType(const PIDataType* data_type = 0) {PConcreteInstance::setPropsDataType(xmlName() == "PDevI" ? NULL : device_type->properties());};

private:
    P_device* device;
    P_super* supervisor;
    QString device_type_id;
    const PDeviceType* device_type;
    const PSupervisorDeviceType* supervisor_type;

    // 'officially' state is not supported but this seems unlikely.
    enum vld_elem_types {OTHER, STATE};
    const QHash<QString, vld_elem_types> valid_elements = {{"S", STATE}};
};

#endif // PDEVICEINSTANCE_H
