#include "pipin.h"
#include "pigraphtype.h"

PIPin::PIPin(QObject *parent) : QObject(parent), pin_type(NULL), msg_type(NULL), tmp_msg_id(new QString())
{

}

const QString& PIPin::msgID() const
{
    if (msg_type != NULL) return msg_type->name();
    return *tmp_msg_id;
}

void PIPin::setMsgType(const QString& msg_type_id)
{
    PIGraphType* parentGraph = dynamic_cast<PIGraphType*>(parent()->parent());
    if (parentGraph == NULL) return;
    setMsgType(static_cast<const PMessageType*>(parentGraph->constSubObject(PIGraphType::MSGTYPES, msg_type_id)));
    if (!msg_type) *tmp_msg_id = msg_type_id;
}

void PIPin::setMsgType(const PMessageType* message_type)
{
    delete tmp_msg_id;
    msg_type = message_type;
}
