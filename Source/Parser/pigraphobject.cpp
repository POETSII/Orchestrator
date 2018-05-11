#include "pigraphobject.h"

PIGraphObject::PIGraphObject(const QString& name, const QString& xml_name, QObject *parent) : QObject(parent), _name(name), _xml_name(xml_name), unique_id_counter(0)
{
     if (dynamic_cast<PIGraphObject*>(parent) != 0)
     {
        if (name.isEmpty()) _name = QString("%1_%2").arg(xml_name).arg(static_cast<PIGraphObject*>(parent)->nextUniqueID());
     }
     else
     {
        unique_id_counter = new int;
        *unique_id_counter = 0;
        if (name.isEmpty()) _name = QString("%1_%2").arg(xml_name).arg((*unique_id_counter)++);
     }
}

PIGraphObject::~PIGraphObject()
{
     delete unique_id_counter;
}

QString print_def()
{
     return QString();
}

// convenience function to drive parse loop - could be inline?
QXmlStreamReader* PIGraphObject::nextXmlToken(QXmlStreamReader *xml_def)
{
    xml_def->readNext();
    return xml_def;
}

// the base class' defineObject only needs to handle errors reading tokens, which could be
// of various types.
void PIGraphObject::defineObject(QXmlStreamReader* xml_def)
{
        // immediately switch on the token type because we can't assume anything about what
        // may be in the XML.
        switch (xml_def->tokenType())
        {
        // a few tokens are innocuous and can be ignored.
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Comment:
        return;
        case QXmlStreamReader::Characters:
        // rather stupidly, QXmlStreamReader doesn't have any way to ignore whitespace. So you have to do it manually.
        if (xml_def->isWhitespace()) return;
        return signalXmlError(xml_def, QString("Error: unexpected text in XML definition: %s").arg(xml_def->text().toString()));
        // an internal XML error should return with its error code.
        case QXmlStreamReader::Invalid:
        return signalXmlError(xml_def, "");
        // other (most) types of tokens indicate something wrong in the XML if they've not been handled in their
        // receiving child class.
        case QXmlStreamReader::StartDocument:
        return signalXmlError(xml_def, "Error: unexpected start of XML document");
        case QXmlStreamReader::EndDocument:
        return signalXmlError(xml_def, "Error: unexpected end of XML document");
        case QXmlStreamReader::StartElement:
        case QXmlStreamReader::EndElement:
        return signalXmlError(xml_def, QString("Error: unexpected element %s in XML definition").arg(xml_def->name().toString()));
        default:
        return signalXmlError(xml_def, QString("Error: unexpected token in XML: %d").arg(xml_def->tokenType()));
        }
}

void PIGraphObject::signalXmlError(QXmlStreamReader* xml_err, const QString& err_msg)
{
     if (!err_msg.isEmpty()) xml_err->raiseError(err_msg);
     return;
}
