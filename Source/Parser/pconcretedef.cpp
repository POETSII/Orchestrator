#include "pconcretedef.h"
#include "pigraphroot.h"

PConcreteDef::PConcreteDef(const QString& name, const QString& xml_name, const QVector<int>& sub_objects, PIGraphObject *parent) :
    PAnnotatedDef(name, xml_name, sub_objects, parent), _properties(NULL)
{

}

const PIGraphObject* PConcreteDef::appendSubObject(QXmlStreamReader* xml_def)
{
    // an end element means the entire concrete object has been read from the XML
    if (xml_def->isEndElement()) return this;
    // non-start elements can be immediately referred to the base class.Usually this will result in an error
    if (!xml_def->isStartElement()) return PIGraphBranch::appendSubObject(xml_def);
    if (xml_def->name() == "Properties")
    {
        // set the name to the C type name if available
        /* rather awkwardly, the QXmlStreamReader methods provide a QStringRef which is not implicitly convertible
           to a QString - thus one must use the .toString() method to get a string from the XML text to run setValue.
           But the attributes methods use plain QStrings and don't have an interface to QStringRefs
           for their own internal methods, so as can be seen here a conversion-dereferencing chain is
           necessary. Rather roundabout...
        */
        if (xml_def->attributes().hasAttribute("", "cTypeName"))
                _properties = new PIDataType(xml_def->attributes().value("", "cTypeName").toString(), this);
        else
        {
            QString prop_name("properties_t");
            PIGraphObject* container = this;
            do
            {
                prop_name.prepend(QString("%1_").arg(container->name()));
                container = dynamic_cast<PIGraphObject*>(container->parent());
            }
            while ((container != NULL) && (dynamic_cast<PIGraphRoot*>(container) == NULL));
            //while ((container != NULL) && (dynamic_cast<PIGraphRoot*>(container->parent()) == NULL));
            _properties = new PIDataType(prop_name, this);
        }
        _properties->defineObject(xml_def);
        return _properties;
    }
    // anything other than properties is referred up the inheritance chain.
    else return PAnnotatedDef::appendSubObject(xml_def);
}

QString PConcreteDef::print_def()
{
    QString expanded_def = QString("Object %1:\n").arg(name());
    if (_properties != NULL) expanded_def.append(QString("Properties: \n%1\n").arg(_properties->print_def()));
    expanded_def.append(PIGraphBranch::print_def());
    return expanded_def;
}

