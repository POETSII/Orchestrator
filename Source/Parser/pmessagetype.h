#ifndef PMESSAGETYPE_H
#define PMESSAGETYPE_H

#include "pannotateddef.h"
#include "poetsdatatype.h"
#include "P_message.h"

class PMessageType : public PAnnotatedDef
{
public:
    PMessageType(const QString& name = "", PIGraphObject *parent = 0);

    const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);
    P_message* elaborateMessage(P_typdcl* graph_type = NULL);

private:
    P_message* message_type;
    enum vld_elem_types {OTHER, MSG};
    QHash<QString, vld_elem_types> valid_elements;
};

#endif // PMESSAGETYPE_H
