#include "psubobjects.h"

PSubObjects::PSubObjects(QObject *parent) : QObject(parent), sub_objects(), sub_obj_indices()
{

}

PSubObjects::PSubObjects(const QVector<int> &obj_typeids, QObject *parent) : QObject(parent), sub_objects()
{
    for (QVector<int>::const_iterator subclass = obj_typeids.begin(); subclass < obj_typeids.end(); subclass++)
    {
        sub_objects.insert(*subclass, new QVector<PIGraphObject*>());
        sub_obj_indices.insert(*subclass, new QHash<QString, int>());
    }
}

PSubObjects::~PSubObjects()
{
    // slightly dangerous: QMap iterators apparently don't support operator < so one must test using !=. Therefore there's some possibility we may run over the end of the map.
    for (QMap<int, QVector<PIGraphObject*>*>::iterator s_o = sub_objects.begin(); s_o != sub_objects.end(); s_o++) delete s_o.value();
    for (QMap<int, QHash<QString, int>*>::iterator s_i = sub_obj_indices.begin(); s_i != sub_obj_indices.end(); s_i++) delete s_i.value();
    sub_objects.clear();
    sub_obj_indices.clear();
}

PIGraphObject* PSubObjects::insertSubObject(int obj_typeid, PIGraphObject* sub_obj)
{
    if (!sub_objects.contains(obj_typeid)) return NULL;
    sub_objects[obj_typeid]->append(sub_obj);
    sub_obj_indices[obj_typeid]->insert(sub_obj->name(), sub_objects[obj_typeid]->length()-1);
    return sub_obj;
}

PIGraphObject* PSubObjects::removeSubObject(int obj_typeid, PIGraphObject* sub_obj)
{
    if (!sub_objects.contains(obj_typeid)) return NULL;
    if (sub_obj_indices[obj_typeid]->remove(sub_obj->name()))
    {
       if (sub_objects[obj_typeid]->removeAll(sub_obj)) return sub_obj;
    }
    return NULL;
}

const PIGraphObject* PSubObjects::subObject(int obj_typeid, const QString& name) const
{
    if (sub_objects.contains(obj_typeid))
    {
       int idx = subObjIndex(obj_typeid, name);
       if (idx >= 0) return sub_objects[obj_typeid]->at(idx);
    }
    return NULL;
}

void PSubObjects::insertSubObjClass(int obj_typeid)
{
    sub_objects.insert(obj_typeid, new QVector<PIGraphObject*>());
    sub_obj_indices.insert(obj_typeid, new QHash<QString, int>());
}
