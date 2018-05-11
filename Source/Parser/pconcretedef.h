#ifndef PCONCRETEDEF_H
#define PCONCRETEDEF_H

#include "pannotateddef.h"
#include "pidatatype.h"

class PConcreteDef : public PAnnotatedDef
{
public:
    PConcreteDef(const QString& name = "", const QString& xml_name = "ConcreteObject", const QVector<int>& sub_objects = QVector<int>(), PIGraphObject *parent = 0);

    virtual const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);
    inline const PIDataType* properties() const {return _properties;};
    virtual QString print_def();

private:
    PIDataType* _properties;

};

#endif // PCONCRETEDEF_H
