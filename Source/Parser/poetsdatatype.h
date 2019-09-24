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

    const QHash<QString, dtype_classes> vld_types = {{"char", ISCALAR}, {"short", ISCALAR}, {"int", ISCALAR}, {"long", ISCALAR}, {"long long", ISCALAR}, {"unsigned char", USCALAR}, {"unsigned short", USCALAR}, {"unsigned int", USCALAR}, {"unsigned long", USCALAR}, {"unsigned long long", USCALAR}, {"int8_t", ISCALAR},
                                                            {"int16_t", ISCALAR}, {"int32_t", ISCALAR}, {"int64_t", ISCALAR}, {"uint8_t", USCALAR}, {"uint16_t", USCALAR}, {"uint32_t", USCALAR}, {"uint64_t", USCALAR},
                                                            {"float", FSCALAR}, {"double", FSCALAR}, {"long double", FSCALAR}, {"char*", SSCALAR}, {"string", SSCALAR}, {"union", UNION}, {"struct", STRUCT}};
    const QHash<QString, size_t> type_sizes = {{"char", sizeof(char)}, {"short", sizeof(short)}, {"int", sizeof(int)}, {"long", sizeof(long)}, {"long long", sizeof(long long)}, {"unsigned char", sizeof(unsigned char)}, {"unsigned short", sizeof(unsigned short)}, {"unsigned int", sizeof(unsigned int)},
                                               {"unsigned long", sizeof(unsigned long)}, {"unsigned long long", sizeof(unsigned long long)}, {"int8_t", sizeof(int8_t)}, {"int16_t", sizeof(int16_t)}, {"int32_t", sizeof(int32_t)}, {"int64_t", sizeof(int64_t)}, {"uint8_t", sizeof(uint8_t)},
                                               {"uint16_t", sizeof(uint16_t)}, {"uint32_t", sizeof(uint32_t)}, {"uint64_t", sizeof(uint64_t)}, {"float", sizeof(float)}, {"double", sizeof(double)}, {"long double", sizeof(long double)}, {"char*", sizeof(char*)}};

/*    const QHash<int, QVector<QString>*> vld_typ_classes = {{ISCALAR, &int_scalars}, {FSCALAR, &float_scalars}, {SSCALAR, &string_scalars}, {STRUCT, &_unions}, {UNION, &_structs}};

private:
    const QVector<QString> int_scalars = {"char", "short", "int", "long", "long long", "unsigned char", "unsigned short", "unsigned int", "unsigned long", "unsigned long long", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t"};
    const QVector<QString> float_scalars = {"float", "double", "long double"};
    const QVector<QString> string_scalars = {"char*", "string"};
    const QVector<QString> _unions = {"union"};
    const QVector<QString> _structs = {"struct"};
*/
};

#endif // POETSDATATYPE_H
