#include "pconcreteinstance.h"
#include "pigraphroot.h"

PConcreteInstance::PConcreteInstance(const QString& name, const QString& xml_name, const QVector<int>& sub_objects, PIGraphObject *parent) :
    PAnnotatedDef(name, xml_name, sub_objects, parent), _properties(NULL)
{

}

const PIGraphObject* PConcreteInstance::appendSubObject(QXmlStreamReader* xml_def)
{
    // an end element means the entire concrete object has been read from the XML
    if (xml_def->isEndElement()) return this;
    // non-start elements can be immediately referred to the base class.Usually this will result in an error
    if (!xml_def->isStartElement()) return PIGraphBranch::appendSubObject(xml_def);
    if (xml_def->name() == "P" || xml_def->name() == "Properties")
    {
        // set the name to the hierarchical ID chain if available.
        QString prop_name("properties_val");
        PIGraphObject* container = this;
        do
        {
            prop_name.prepend(QString("%1_").arg(container->name()));
            container = dynamic_cast<PIGraphObject*>(container->parent());
        }
        while ((container != NULL) && (dynamic_cast<PIGraphRoot*>(container->parent()) == NULL));
        prop_name.prepend("Inst_");
        _properties = new PIDataValue(prop_name, this);
        setPropsDataType(); // obscure: this is a virtual function - subclasses will replace the default argument with a real data type
        _properties->defineObject(xml_def);
        return _properties;
    }
    // anything other than properties is referred up the inheritance chain.
    else return PAnnotatedDef::appendSubObject(xml_def);
}

QString PConcreteInstance::print_def()
{
    QString expanded_def = QString("Object %1:\n").arg(name());
    if (_properties != NULL) expanded_def.append(QString("Properties: \n%1\n").arg(_properties->print_def()));
    expanded_def.append(PIGraphBranch::print_def());
    return expanded_def;
}
