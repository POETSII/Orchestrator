#ifndef PIGRAPHROOT_H
#define PIGRAPHROOT_H

#include "pigraphbranch.h"
#include "OrchBase.h"

class PIGraphRoot : public PIGraphBranch
{
public:
    PIGraphRoot(const QString& name = "", QObject *parent = 0);

    void defineObject(QXmlStreamReader* xml_def);
    const PIGraphObject* appendSubObject(QXmlStreamReader* xml_def);
    void elaborateRoot(OrchBase* orch_in);
    inline const int getObjectID() {return obj_id_counter++;};
    inline const QString& xmlHeader() const {return xml_hdr;};

    enum vld_elem_types {OTHER, HEADER, GTYPE, GTREF, GINST, GIREF, METAP};
private:
    OrchBase* orchestrator;
    int obj_id_counter;
    QString xml_hdr;
    const QHash<QString, vld_elem_types> valid_elements = {{"Pheader", HEADER}, {"GraphType", GTYPE}, {"GraphTypeReference", GTREF}, {"GraphInstance", GINST}, {"GraphInstanceReference", GIREF}, {"GraphInstanceMetadataPatch", METAP}};
};

#endif // PIGRAPHROOT_H
