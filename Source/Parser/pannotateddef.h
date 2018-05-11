#ifndef PANNOTATEDDEF_H
#define PANNOTATEDDEF_H

#include "pigraphbranch.h"
#include "pigraphleaf.h"

class PAnnotatedDef : public PIGraphBranch
{
public:
    PAnnotatedDef(const QString& name = "", const QString& xml_name = "AnnotatedObject", const QVector<int>& sub_objects = QVector<int>(), PIGraphObject *parent = 0);

    virtual const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);
    void appendDocumentation(const QString& docs); // functions to be called by a PIMetaDataPatch object.
    void appendMetaData(const QString& meta); // functions to be called by a PIMetaDataPatch object.
    inline const PIGraphLeaf* documentation() const {return _documentation;};
    inline const PIGraphLeaf* metadata() const {return _metadata;};

private:
    PIGraphLeaf* _documentation;
    PIGraphLeaf* _metadata;
};

#endif // PANNOTATEDDEF_H
