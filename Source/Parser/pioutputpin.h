#ifndef PIOUTPUTPIN_H
#define PIOUTPUTPIN_H

#include "pannotateddef.h"
#include "pcodefragment.h"
#include "pipin.h"

class PIOutputPin : public PAnnotatedDef, public PIPin
{
public:
    PIOutputPin(const QString& name = "", PIGraphObject *parent = 0);

    void defineObject(QXmlStreamReader* xml_def);
    P_pintyp* elaboratePinType(P_devtyp* device_type = NULL);
    const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);

    int priority;

private:
    enum vld_elem_types {OTHER, ONSEND};
    QHash<QString, vld_elem_types> valid_elements;
};

#endif // PIOUTPUTPIN_H
