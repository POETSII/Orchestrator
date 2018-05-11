#ifndef PMETADATAPATCH_H
#define PMETADATAPATCH_H

#include "pannotateddef.h"
#include "pigraphinstance.h"

class PMetadataPatch : public PAnnotatedDef
{
public:
    PMetadataPatch(const QString& name = "", PIGraphObject *parent = 0);

    void defineObject(QXmlStreamReader* xml_def);
    const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);

    PIGraphInstance* graph_to_patch;
};

#endif // PMETADATAPATCH_H
