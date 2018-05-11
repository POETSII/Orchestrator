#include "pigraphbranch.h"

PIGraphBranch::PIGraphBranch(const QString& name, const QString& xml_name, const QVector<int>& sub_objects, QObject *parent) :
    PSubObjects(sub_objects, parent), PIGraphObject(name, xml_name, parent)
{

}

/* left out for debug
const PIGraphBranch::PGraphMap* map()
{

}
*/

void PIGraphBranch::defineObject(QXmlStreamReader* xml_def)
{
    if (xml_def->namespaceUri().isEmpty()) return signalXmlError(xml_def, "Error: no POETS namespace defined");
    if (xml_def->name().toString() != xmlName()) return signalXmlError(xml_def, QString("Error: tried to instantiate a %1, but found an element %2").arg(xmlName()).arg(xml_def->name().toString()));
    while (!(xml_def->error() || (appendSubObject(nextXmlToken(xml_def)) == this)));
}

const PIGraphObject* PIGraphBranch::appendSubObject(QXmlStreamReader* xml_def)
{
    // End elements must match the current object. If they don't, this means it's an unexpected end element and an error should be generated.
    if (xml_def->isEndElement() && (xml_def->name() == xmlName())) return this;
    // A few other element types may be innocuous and can be handled using default processing
    if (!xml_def->isStartElement())
    {
        PIGraphObject::defineObject(xml_def);
        return NULL;
    }
    // Mostly we expect start elements. If we got here it means some subelement not usually found in the object was read.
    xml_def->raiseError(QString("Error: unexpected subelement %1 found for element type %2").arg(xml_def->name().toString()).arg(this->xmlName()));
    return NULL;
}

QString PIGraphBranch::print_def()
{
    QString expanded_def("");
    for (QMap<int, QVector<PIGraphObject*>*>::iterator sub_classes = sub_objects.begin(); sub_classes != sub_objects.end(); sub_classes++)
    {
        if (sub_classes.value()->length()) expanded_def.append(QString("\nObjects of class %1:\n").arg(QString(typeid(sub_classes.value()->at(0)).name())));
        for (QVector<PIGraphObject*>::iterator s_c_obj = sub_classes.value()->begin(); s_c_obj < sub_classes.value()->end(); s_c_obj++)
        {
            expanded_def.append(QString("%1:\n%2").arg((*s_c_obj)->name()).arg((*s_c_obj)->print_def()));
        }
    }
    return expanded_def;
}
