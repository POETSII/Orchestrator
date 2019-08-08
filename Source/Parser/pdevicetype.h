#ifndef PDEVICETYPE_H
#define PDEVICETYPE_H

#include "pconcretedef.h"
#include "P_devtyp.h"
#include "P_typdcl.h"
#include "pidatatype.h"

class PDeviceType : public PConcreteDef
{
public:
    PDeviceType(const QString& name = "", PIGraphObject *parent = 0);

    const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);
    P_devtyp* elaborateDeviceType(P_typdcl* graph_type = NULL);
    inline P_devtyp* deviceType() const {return device_type;};

     enum vld_elem_types {OTHER, INPIN, OUTPIN, STATE, CODE};

private:
    P_devtyp* device_type;
    QHash<QString, vld_elem_types> valid_elements;
};

#endif // PDEVICETYPE_H
