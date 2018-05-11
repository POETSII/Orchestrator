#ifndef PCONCRETEINSTANCE_H
#define PCONCRETEINSTANCE_H

#include "pannotateddef.h"
#include "pconcretedef.h"
#include "pidatavalue.h"

class PConcreteInstance : public PAnnotatedDef
{
public:
    PConcreteInstance(const QString& name = "", const QString& xml_name = "ConcreteInstance", const QVector<int>& sub_objects = QVector<int>(), PIGraphObject *parent = 0);

    virtual const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);
    virtual QString print_def();
    inline virtual void setPropsDataType(const PIDataType* data_type = 0) {_properties->data_type = data_type;};
    inline const PConcreteDef* dataDef() const {return dynamic_cast<const PConcreteDef*>(_properties->data_type->parent());};
    inline const PIDataValue* properties() const {return _properties;};

private:
    PIDataValue* _properties; // the instantiated properties
};

#endif // PCONCRETEINSTANCE_H
