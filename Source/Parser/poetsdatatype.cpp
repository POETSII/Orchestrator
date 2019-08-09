#include "poetsdatatype.h"

PoetsDataType::PoetsDataType(QObject* parent) : QObject(parent), QVariant()
{
    vld_types["char"] = ISCALAR;
    vld_types["short"] = ISCALAR;
    vld_types["int"] = ISCALAR;
    vld_types["long"] = ISCALAR;
    vld_types["long long"] = ISCALAR;
    vld_types["unsigned char"] = USCALAR;
    vld_types["unsigned short"] = USCALAR;
    vld_types["unsigned int"] = USCALAR;
    vld_types["unsigned long"] = USCALAR;
    vld_types["unsigned long long"] = USCALAR;
    vld_types["int8_t"] = ISCALAR;
    vld_types["int16_t"] = ISCALAR;
    vld_types["int32_t"] = ISCALAR;
    vld_types["int64_t"] = ISCALAR;
    vld_types["uint8_t"] = USCALAR;
    vld_types["uint16_t"] = USCALAR;
    vld_types["uint32_t"] = USCALAR;
    vld_types["uint64_t"] = USCALAR;
    vld_types["float"] = FSCALAR;
    vld_types["double"] = FSCALAR;
    vld_types["long double"] = FSCALAR;
    vld_types["char*"] = SSCALAR;
    vld_types["string"] = SSCALAR;
    vld_types["union"] = UNION;
    vld_types["struct"] = STRUCT;

    type_sizes["char"] = sizeof(char);
    type_sizes["short"] = sizeof(short);
    type_sizes["int"] = sizeof(int);
    type_sizes["long"] = sizeof(long);
    type_sizes["long long"] = sizeof(long long);
    type_sizes["unsigned char"] = sizeof(unsigned char);
    type_sizes["unsigned short"] = sizeof(unsigned short);
    type_sizes["unsigned int"] = sizeof(unsigned int);
    type_sizes["unsigned long"] = sizeof(unsigned long);
    type_sizes["unsigned long long"] = sizeof(unsigned long long);
    type_sizes["int8_t"] = sizeof(int8_t);
    type_sizes["int16_t"] = sizeof(int16_t);
    type_sizes["int32_t"] = sizeof(int32_t);
    type_sizes["int64_t"] = sizeof(int64_t);
    type_sizes["uint8_t"] = sizeof(uint8_t);
    type_sizes["uint16_t"] = sizeof(uint16_t);
    type_sizes["uint32_t"] = sizeof(uint32_t);
    type_sizes["uint64_t"] = sizeof(uint64_t);
    type_sizes["float"] = sizeof(float);
    type_sizes["double"] = sizeof(double);
    type_sizes["long double"] = sizeof(long double);
    type_sizes["char*"] = sizeof(char*);
}
