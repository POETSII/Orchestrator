#ifndef PIDATAVALUE_H
#define PIDATAVALUE_H

#include "pigraphleaf.h"
#include "pidatatype.h"
#include "CFrag.h"

class PIDataValue : public PIGraphLeaf
{
public:
     PIDataValue(const QString& name = "", PIGraphObject *parent = 0);

     // turns the raw XML into valid JSON to set up for elaborating the value.
     inline void setValue(const QString& new_value) {PIGraphLeaf::setValue(QString("{%1}").arg(new_value));};
     const QString& elaborateDataValueStr(void);
     inline QString print_def() {return QString(elaborateDataValueStr());};
     inline CFrag* elaborateDataValue() {return new CFrag(elaborateDataValueStr().toStdString());};

     const PIDataType* data_type;

private:
     // helper functions to descend the data structure recursively and fill in the initialiser list
     QString elaborateSubValue(const QJsonObject& in_obj, const PIDataType* sub_type);
     // and determine whether the JSON definition even has the subfield mentioned. (If not the default value will be used)
     QString subFieldInObj(const QJsonObject& in_obj, const PIDataType* sub_type) const;

     QString elaborated_value;
};

#endif // PIDATAVALUE_H
