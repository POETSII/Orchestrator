#ifndef PIGRAPHOBJECT_H
#define PIGRAPHOBJECT_H

#include <QtCore>
#include <QXmlStreamReader>
#include <typeinfo>

class PIGraphObject : public QObject
{

public:
    explicit PIGraphObject(const QString& name = "", const QString& xml_name = "", QObject *parent = 0);
    ~PIGraphObject();

    virtual void defineObject(QXmlStreamReader* xml_def);
    inline virtual QString print_def() {return QString("%1 %2\n").arg(typeid(this).name()).arg(_name);};
    QXmlStreamReader* nextXmlToken(QXmlStreamReader* xml_def);
    void signalXmlError(QXmlStreamReader* xml_err, const QString& err_msg = "");
    inline const QString& name() const {return _name;};
    inline const QString& xmlName() const {return _xml_name;};
    inline void setName(const QString& new_name) {_name = new_name;};
    inline void setXmlName(const QString& new_xml_name) {_xml_name = new_xml_name;};
    inline const int uniqueID() {return (unique_id_counter? *unique_id_counter : static_cast<PIGraphObject*>(parent())->uniqueID());};
    // hopefully we won't ever exceed the need for 2^31-1 IDs! If we do this will break.
    inline const int nextUniqueID() {return (unique_id_counter? (*unique_id_counter)++ : static_cast<PIGraphObject*>(parent())->nextUniqueID());};

private:
    // keeping the names private prevents them from being inadvertently transformed by string-manipulation methods.
    QString _name;
    QString _xml_name;
    int* unique_id_counter;
};

#endif // PIGRAPHOBJECT_H
