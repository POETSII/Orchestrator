#ifndef PIGRAPHBRANCH_H
#define PIGRAPHBRANCH_H

#include <QVector>
#include "pigraphobject.h"
#include "psubobjects.h"

class PIGraphBranch : public PIGraphObject, public PSubObjects
{
public:
    PIGraphBranch(const QString& name = "", const QString& xml_name = "GraphBranchObject", const QVector<int>& sub_objects = QVector<int>(), QObject *parent = 0);

    // const PGraphMap* map(); // left out for debug
    virtual void defineObject(QXmlStreamReader* xml_def);
    virtual const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);
    virtual QString print_def();

    inline const bool isMapped() const {return mapped;};
    // overloads of the parent(), setParent() functions because of diamond-shaped inheritance
    inline QObject* parent() const {return PIGraphObject::parent();};
    inline void setParent(QObject *parent) {PIGraphObject::setParent(parent);};

private:
    // PGraphMap* _map;
    bool mapped;
};

#endif // PIGRAPHBRANCH_H
