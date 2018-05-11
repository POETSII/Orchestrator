#ifndef PIGRAPHTYPE_H
#define PIGRAPHTYPE_H

#include "pconcretedef.h"
#include "P_typdcl.h"


class PIGraphType : public PConcreteDef
{
public:
    PIGraphType(const QString& name = "", PIGraphObject *parent = 0);

    const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);
    P_typdcl* elaborateGraphType(OrchBase* orch_root = NULL);

    enum vld_elem_types {OTHER, DEVTYPES, MSGTYPES, SHAREDCODE, SUPERDEVTYPES};

private:

    P_typdcl* graph_type; // need to store the graph type here for GraphInstances to recover.
    const QHash<QString, vld_elem_types> valid_elements = {{"DeviceTypes", DEVTYPES}, {"MessageTypes", MSGTYPES}, {"SharedCode", SHAREDCODE}};
};

#endif // PIGRAPHDEF_H
