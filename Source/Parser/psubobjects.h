#ifndef PSUBOBJECTS_H
#define PSUBOBJECTS_H

#include <QObject>
#include "pigraphobject.h"

class PSubObjects : public QObject
{

public:
    explicit PSubObjects(QObject *parent = 0);
    PSubObjects(const QVector<int>& obj_typeids, QObject *parent = 0);
    ~PSubObjects();

    PIGraphObject* insertSubObject(int obj_typeid, PIGraphObject* sub_obj);
    PIGraphObject* removeSubObject(int obj_typeid, PIGraphObject* sub_obj);
    const PIGraphObject* constSubObject(int obj_typeid, const QString& name) const;
    PIGraphObject* subObject(int obj_typeid, const QString& name);
    inline const PIGraphObject* constSubObject(int obj_typeid, int index) const {return sub_objects.contains(obj_typeid) ? sub_objects[obj_typeid]->at(index) : NULL;};
    inline PIGraphObject* subObject(int obj_typeid, int index) {return sub_objects.contains(obj_typeid) ? sub_objects[obj_typeid]->at(index) : NULL;};
    inline const QVector<PIGraphObject*> subObjects(int obj_typeid) const {return sub_objects.contains(obj_typeid) ? *sub_objects[obj_typeid] : QVector<PIGraphObject*>();};
    inline QVector<PIGraphObject*>::iterator beginSubObjects(int obj_typeid) {return sub_objects.contains(obj_typeid) ? sub_objects[obj_typeid]->begin() : NULL;};
    inline QVector<PIGraphObject*>::iterator endSubObjects(int obj_typeid) {return sub_objects.contains(obj_typeid) ? sub_objects[obj_typeid]->end() : NULL;};
    inline QVector<const PIGraphObject*>::const_iterator beginConstSubObjects(int obj_typeid) const {return sub_objects.contains(obj_typeid) ? sub_objects[obj_typeid]->constBegin() : NULL;};
    inline QVector<const PIGraphObject*>::const_iterator endConstSubObjects(int obj_typeid) const {return sub_objects.contains(obj_typeid) ? sub_objects[obj_typeid]->constEnd() : NULL;};
    inline const int numSubObjects(int obj_typeid) const {return sub_objects.contains(obj_typeid) ? sub_objects[obj_typeid]->length() : 0;};
    inline const int subObjIndex(int obj_typeid, const QString& name) const {return sub_obj_indices.contains(obj_typeid) ? sub_obj_indices[obj_typeid]->value(name, -1) : -2;};

protected:
    void insertSubObjClass(int obj_typeid);
    QMap<int, QVector<PIGraphObject*>*> sub_objects;
    QMap<int, QHash<QString, int>*> sub_obj_indices;

};

#endif // PSUBOBJECTS_H
