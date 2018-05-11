#ifndef PIPIN_H
#define PIPIN_H

#include <QObject>
#include "pmessagetype.h"
#include "P_pintyp.h"

class PIPin : public QObject
{

public:
    explicit PIPin(QObject *parent = 0);

    inline virtual P_pintyp* elaboratePinType(P_devtyp* device_type = NULL) {return ((device_type != NULL) && (pin_type == NULL)) ? pin_type = new P_pintyp(device_type, QString("virtual_pin").toStdString()) : pin_type;};
    const QString& msgID() const;
    void setMsgType(const QString& msg_type_id);
    void setMsgType(const PMessageType* message_type);
    inline const PMessageType* msgType() const {return msg_type;};

protected:
     P_pintyp* pin_type; // needs to be a protected member so that classes derived from this class can use the virtual method elaboratePinType

private:
     const PMessageType* msg_type;
     QString* tmp_msg_id;

};

#endif // PIPIN_H
