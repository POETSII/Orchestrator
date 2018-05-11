#include "pigraphleaf.h"

PIGraphLeaf::PIGraphLeaf(const QString& name, PIGraphObject *parent) : PIGraphObject(name, "", parent), _value("")
{

}

void PIGraphLeaf::defineObject(QXmlStreamReader* xml_def)
{
     // get the next token, which should be either characters or the end of the element.
     // unexpected token types can be handled by the base class.
     while (!(xml_def->error() || (xml_def->readNext() == QXmlStreamReader::Characters))) PIGraphObject::defineObject(xml_def);
     if (xml_def->isCharacters()) setValue(xml_def->text().toString());
     while (!(xml_def->error() || (xml_def->readNext() == QXmlStreamReader::EndElement))) PIGraphObject::defineObject(xml_def);
     return;
}

QString PIGraphLeaf::print_def()
{
    return QString("%1: %2\n").arg(name()).arg(_value);
}
