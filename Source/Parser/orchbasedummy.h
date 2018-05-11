#ifndef ORCHBASEDUMMY_H
#define ORCHBASEDUMMY_H

#include "OrchBase.h"

class OrchBaseDummy : public OrchBase
{
public:
    OrchBaseDummy(int, char *[], string, string);
    ~OrchBaseDummy();

#include "Decode.cpp"
};

#endif // ORCHBASEDUMMY_H
