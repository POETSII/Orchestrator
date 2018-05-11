#include "pannotateddef.h"

PAnnotatedDef::PAnnotatedDef(const QString& name, const QString& xml_name, const QVector<int>& sub_objects, PIGraphObject *parent) :
    PIGraphBranch(name, xml_name, sub_objects, parent), _documentation(NULL), _metadata(NULL)
{

}

const PIGraphObject* PAnnotatedDef::appendSubObject(QXmlStreamReader* xml_def)
{
    // an end element means the entire concrete object has been read from the XML
    if (xml_def->isEndElement()) return this;
    // non-start elements can be immediately referred to the base class.Usually this will result in an error
    if (!xml_def->isStartElement()) return PIGraphBranch::appendSubObject(xml_def);
    if (xml_def->name() == "Documentation")
    {
        _documentation = new PIGraphLeaf("Documentation", this);
        // documentation could be literally anything. As a result, use readElementText
        // to swallow all up to the closing end-element without further parsing.
        _documentation->setValue(xml_def->readElementText(QXmlStreamReader::IncludeChildElements));
        return _documentation;
    }
    else if (xml_def->name() == "MetaData" || xml_def->name() == "M")
    {
        _metadata = new PIGraphLeaf("MetaData", this);
        // metadata can be handled in the usual way because we expect untagged JSON text.
        _metadata->defineObject(xml_def);
        return _metadata;
    }
    // any other element type is handled in the base class, probably producing an error.
    else return PIGraphBranch::appendSubObject(xml_def);
}

void PAnnotatedDef::appendDocumentation(const QString& docs)
{
    if (!_documentation) _documentation = new PIGraphLeaf("Documentation", this);
    QString curr_docs = _documentation->value();
    _documentation->setValue(curr_docs.append(docs));
}

void PAnnotatedDef::appendMetaData(const QString& meta)
{
    if (!_metadata) _metadata = new PIGraphLeaf("MetaData", this);
    QString curr_meta = _metadata->value();
    _metadata->setValue(curr_meta.append(meta));
}
