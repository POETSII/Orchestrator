#ifndef POETSDATATYPE_H
#define POETSDATATYPE_H

#include <QObject>
#include <QVariant>
#include "OSFixes.hpp"

class PoetsDataType : public QObject, public QVariant
{
public:
    PoetsDataType(QObject* parent = 0);

    enum dtype_classes {ISCALAR, USCALAR, FSCALAR, SSCALAR, ARRAY, STRUCT, UNION};

    QHash<QString, dtype_classes> vld_types;
    QHash<QString, size_t> type_sizes;

/*    QHash<int, QVector<QString>*> vld_typ_classes = {{ISCALAR, &int_scalars}, {FSCALAR, &float_scalars}, {SSCALAR, &string_scalars}, {STRUCT, &_unions}, {UNION, &_structs}};

private:
    const QVector<QString> int_scalars = {"char", "short", "int", "long", "long long", "unsigned char", "unsigned short", "unsigned int", "unsigned long", "unsigned long long", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t"};
    const QVector<QString> float_scalars = {"float", "double", "long double"};
    const QVector<QString> string_scalars = {"char*", "string"};
    const QVector<QString> _unions = {"union"};
    const QVector<QString> _structs = {"struct"};
*/
};

#endif // POETSDATATYPE_H
