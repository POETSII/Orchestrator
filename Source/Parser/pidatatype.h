#ifndef PIDATATYPE_H
#define PIDATATYPE_H

#include "pigraphbranch.h"
#include "poetsdatatype.h"
#include "CFrag.h"
#include <cstdio>

class PIDataType : public PIGraphBranch
{
public:

    PIDataType(const QString& name = "", PIGraphObject *parent = 0);

    ~PIDataType();
    void defineObject(QXmlStreamReader* xml_def);
    const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);
    // temporary method simply builds the data declaration by string concatenation.
    // later on we will use the PoetsDataType class to define the data structure and
    // it has a method that spits out the declaration.
    const QString& elaborateDataTypeStr(const QString& indent = "");
    inline void setValue(const QString& new_value) {value = new_value;};
    inline int numElements() const {return num_replications;};
    const QString& defaultValue() const;
    const size_t size() const;
    // by not recursing down we ensure that the printed definition is only applied to top-level data types.
    inline QString print_def() {return QString("%1").arg(elaborateDataTypeStr(""));};
    // rather an expensive operation to give an initialised data type but each new
    // data definition with initialiser needs a new string.
    inline QString initDataType(const QString& new_value = "") {return QString(elaborateDataTypeStr()).insert(expanded_type.size()-2, QString(" = %1").arg(new_value.isEmpty() ? defaultValue() : new_value));};
    // these methods can be implemented later
    inline const QString& dataTypeStr() const {return base_type;};
    inline const PoetsDataType* dataType() const {return data_type;};
    inline CFrag* elaborateDataType() {return new CFrag(elaborateDataTypeStr().toStdString());};
    inline CFrag* elaborateDataDefault() {return new CFrag(defaultValue().toStdString());};

    enum vld_elem_types {DTYPE, SCALAR, ARRAY, STRUCT, UNION};

private:
    // not defined yet. Will hold a data type in native form.
    PoetsDataType* data_type;
    // temporary: these members can go away once the PoetsDataType class is fully defined.
    QString base_type;
    QString expanded_type;
    QString value; // a better name ought to be found for this.
    mutable QString default_value;
    QString _documentation;
    int num_replications;

    QHash<QString, vld_elem_types> valid_elements;
};

#endif // PIDATATYPE_H
