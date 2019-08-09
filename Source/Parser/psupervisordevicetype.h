#ifndef PSUPERVISORDEVICETYPE_H
#define PSUPERVISORDEVICETYPE_H

#include "pigraphbranch.h"
#include "P_devtyp.h"
#include "P_typdcl.h"

class PSupervisorDeviceType : public PIGraphBranch
{
public:
    PSupervisorDeviceType(const QString& name = "", PIGraphObject *parent = 0);

    void defineObject(QXmlStreamReader* xml_def);
    const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);
    P_devtyp* elaborateSupervisorDeviceType(P_typdcl* graph_type);

    bool dev_info_persistent;
    bool dev_props_valid;
    bool edge_endpts_exist;

    enum vld_elem_types {OTHER, INPIN, OUTPIN, CODE};

private:
    P_devtyp* device_type;
    QHash<QString, vld_elem_types> valid_elements;
};

#endif // PSUPERVISORDEVICETYPE_H
