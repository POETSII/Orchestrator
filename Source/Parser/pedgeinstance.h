#ifndef PEDGEINSTANCE_H
#define PEDGEINSTANCE_H

#include "pconcreteinstance.h"
#include "P_message.h"
#include "pdeviceinstance.h"
#include "piinputpin.h"
#include "pioutputpin.h"
#include "P_pin.h"
#include "pdigraph.hpp"

class PEdgeInstance : public PConcreteInstance
{
public:
    typedef enum pin_dir {DST, SRC} pin_dir_t;

    PEdgeInstance(const QString& name = "", PIGraphObject *parent = 0);

    void defineObject(QXmlStreamReader* xml_def);
    const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);
    int elaborateEdge(D_graph* graph_rep);
    inline void setPropsDataType(const PIDataType* data_type = 0) {PConcreteInstance::setPropsDataType(path.dst.pin ? path.dst.pin->properties() : NULL);};

private:

    void parsePath();

    typedef struct
    {
            const PDeviceInstance* device;
            const PIOutputPin* pin;
    } EdgeBegin;

    typedef struct
    {
            const PDeviceInstance* device;
            const PIInputPin* pin;
    } EdgeEnd;

    struct path_t
    {
        EdgeBegin src;
        EdgeEnd dst;
    } path;

    struct path_idx_t
    {
        unsigned int src_i;
        unsigned int dst_i;
        unsigned int msg_i;
        unsigned int ipin_i;
        unsigned int opin_i;
    } path_index;

    static const unsigned int INVALID_INDEX = 0xFFFFFFFF;
    D_graph* containing_graph;
    enum vld_elem_types {OTHER, STATE};
    QHash<QString, vld_elem_types> valid_elements;
};

#endif // PEDGEINSTANCE_H
