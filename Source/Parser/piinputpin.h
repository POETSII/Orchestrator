#ifndef PIINPUTPIN_H
#define PIINPUTPIN_H

#include "pconcretedef.h"
#include "pcodefragment.h"
#include "pidatatype.h"
#include "pipin.h"

class PIInputPin : public PConcreteDef, public PIPin
{
public:
    PIInputPin(const QString& name = "", PIGraphObject *parent = 0);

    void defineObject(QXmlStreamReader* xml_def);
    P_pintyp* elaboratePinType(P_devtyp* device_type = NULL);
    const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);

    enum vld_elem_types {OTHER, ONRECEIVE, STATE};
private:
    const QHash<QString, vld_elem_types> valid_elements = {{"OnReceive", ONRECEIVE}, {"State", STATE}};
};

#endif // PIINPUTPIN_H
