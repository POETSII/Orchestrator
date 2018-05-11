#ifndef PIGRAPHLEAF_H
#define PIGRAPHLEAF_H

#include "pigraphobject.h"

class PIGraphLeaf : public PIGraphObject
{
public:
    PIGraphLeaf(const QString& name = "", PIGraphObject *parent = 0);

    virtual void defineObject(QXmlStreamReader* xml_def);
    virtual QString print_def();
    inline virtual void setValue(const QString& new_value) {_value = new_value;};
    inline const QString& value() const {return _value;};

private:
    QString _value;
};

#endif // PIGRAPHLEAF_H
