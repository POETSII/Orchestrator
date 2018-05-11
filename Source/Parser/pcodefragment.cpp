#include "pcodefragment.h"

PCodeFragment::PCodeFragment(const QString& name, PIGraphObject *parent) : PIGraphLeaf(name, parent)
{

}

void PCodeFragment::defineObject(QXmlStreamReader* xml_def)
{
    // get the next token, which should be either characters or the end of the element.
    // unexpected token types can be handled by the base class.
    // watch for possible lazy-evaluation errors here, this expression relies on order of
    // evaluation to read the next element.
    while (!(xml_def->error() || (xml_def->readNext() == QXmlStreamReader::Characters)) || xml_def->isWhitespace()) PIGraphObject::defineObject(xml_def);
    if (xml_def->isCharacters())
    {
        // see note in pconcretedef.cpp for notes on the curious form needed for the argument to setValue.
        if (xml_def->isCDATA()) setValue(xml_def->text().toString());
        else return signalXmlError(xml_def, QString("Error: expected a C data block for code fragment %1").arg(name()));
    }
    while (!(xml_def->error()  || (xml_def->readNext() == QXmlStreamReader::EndElement))) PIGraphObject::defineObject(xml_def);
    return;
}

CFrag* PCodeFragment::elaborateCodeFragment() const
{
    return new CFrag(value().toStdString());
}
