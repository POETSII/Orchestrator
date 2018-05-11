#ifndef PCODEFRAGMENT_H
#define PCODEFRAGMENT_H

#include "pigraphleaf.h"
#include "CFrag.h"

class PCodeFragment : public PIGraphLeaf
{
public:
    PCodeFragment(const QString& name = "", PIGraphObject *parent = 0);

    void defineObject(QXmlStreamReader* xml_def);    
    CFrag* elaborateCodeFragment() const;
};

#endif // PCODEFRAGMENT_H
