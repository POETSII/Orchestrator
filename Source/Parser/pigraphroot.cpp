#include "pigraphroot.h"
#include "pigraphtype.h"
#include "pigraphinstance.h"
#include "pmetadatapatch.h"
#include <QFile>

PIGraphRoot::PIGraphRoot(const QString& name, QObject *parent) : PIGraphBranch(name, "Graphs", QVector<int>({HEADER, GTYPE, GINST, METAP}), parent), orchestrator(NULL), obj_id_counter(0), xml_hdr("")
{
     valid_elements["Pheader"] = HEADER;
     valid_elements["GraphType"] = GTYPE;
     valid_elements["GraphTypeReference"] = GTREF;
     valid_elements["GraphInstance"] = GINST;
     valid_elements["GraphInstanceReference"] = GIREF;
     valid_elements["GraphInstanceMetadataPatch"] = METAP;
}

void PIGraphRoot::defineObject(QXmlStreamReader* xml_def)
{
     if (xml_def->tokenType()) return signalXmlError(xml_def, "Error: tried to read a Graphs definition, but XML parsing has already started on the document");
     const PIGraphObject* curr_obj = this;
     while (!xml_def->atEnd())
     {
           // handle the possible XML elements.
           switch (xml_def->readNext())
           {
           // start and end of document are innocuous.
           case QXmlStreamReader::EndDocument:
           case QXmlStreamReader::StartDocument:
           break;
           // start and end of elements should invoke the appropriate subobject processing:
           // compare the element type found against the current one being processed. If they
           // differ, add a new subobject (which will conveniently error if an unexpected end-element
           // is found).
           case QXmlStreamReader::EndElement:
           case QXmlStreamReader::StartElement:
           if (xml_def->name().toString() != curr_obj->xmlName()) curr_obj = appendSubObject(xml_def);
           break;
           // otherwise invoke default processing for the object. Mostly this should be comments.
           default:
           PIGraphObject::defineObject(xml_def);
           }
           if (xml_def->error()) return signalXmlError(xml_def);
     }
}

const PIGraphObject* PIGraphRoot::appendSubObject(QXmlStreamReader* xml_def)
{
     // we should usually get a subelement. Non-start elements can be immediately handled in the base
     if (!xml_def->isStartElement()) return PIGraphBranch::appendSubObject(xml_def);
     PIGraphObject* sub_object = this;
     // deal with the various different possible types of subelement.
     switch (valid_elements.value(xml_def->name().toString(), OTHER))
     {
     // the header can just be swallowed whole
     case HEADER:
     xml_hdr = xml_def->readElementText(QXmlStreamReader::IncludeChildElements);
     break;
     // a GraphType. Create and descend into the subparser.
     case GTYPE:
     sub_object = insertSubObject(GTYPE, new PIGraphType(xml_def->attributes().value("", "id").toString(), this));
     sub_object->defineObject(xml_def);
     break;
     // a GraphTypeReference can be read in by spawning a separate reader for its associated file and simply ignoring all elements except for the
     // one desired. Then one proceeds as for a GraphType.
     case GTREF:
     {
        QFile ref_def_file(xml_def->attributes().value("", "src").toString());
        QXmlStreamReader ref_def(&ref_def_file);
        while (!ref_def.atEnd())
        {
             while (!(ref_def.error() || (ref_def.readNext() != QXmlStreamReader::StartElement)));
             if ((ref_def.name().toString() != "Graphs") || (ref_def.name().toString() != "GraphType")) ref_def.skipCurrentElement();
             else if ((ref_def.name() == "GraphType") && (ref_def.attributes().value("", "id").toString() != xml_def->attributes().value("", "id").toString())) ref_def.skipCurrentElement();
             else
             {
                 sub_object = insertSubObject(GTYPE, new PIGraphType(ref_def.attributes().value("", "id").toString(), this));
                 sub_object->defineObject(&ref_def);
                 break;
             }
        }
        if (ref_def.atEnd())
        {
           xml_def->raiseError(QString("Error: GraphType %s not found in reference file %s").arg(xml_def->attributes().value("", "id").toString()).arg(xml_def->attributes().value("", "src").toString()));
           return sub_object;
        }
     }
     break;
     // a GraphInstance. Handled similarly to a GraphType
     case GINST:
     sub_object = insertSubObject(GINST, new PIGraphInstance(xml_def->attributes().value("", "id").toString(), this));
     sub_object->defineObject(xml_def);
     break;
     // a GraphInstanceReference. As per a GraphTypeReference.
     case GIREF:
     {
        QFile iref_def_file(xml_def->attributes().value("", "src").toString());
        QXmlStreamReader iref_def(&iref_def_file);
        while(!iref_def.atEnd())
        {
             while (!(iref_def.error() || (iref_def.readNext() != QXmlStreamReader::StartElement)));
             if ((iref_def.name().toString() != "Graphs") || (iref_def.name().toString() != "GraphInstance")) iref_def.skipCurrentElement();
             else if ((iref_def.name() == "GraphInstance") && (iref_def.attributes().value("", "id").toString() != xml_def->attributes().value("", "id").toString())) iref_def.skipCurrentElement();
             else
             {
                 sub_object = insertSubObject(GINST, new PIGraphInstance(iref_def.attributes().value("", "id").toString(), this));
                 sub_object->defineObject(&iref_def);
                 break;
             }
        }
        if (iref_def.atEnd())
        {
           xml_def->raiseError(QString("Error: GraphInstance %1 not found in reference file %2").arg(xml_def->attributes().value("", "id").toString()).arg(xml_def->attributes().value("", "src").toString()));
           return sub_object;
        }
     }
     break;
     // a GraphInstanceMetadataPatch. We don't really need to handle this but it's easy enough to do it just like GraphInstances.
     case METAP:
     sub_object = insertSubObject(METAP, new PMetadataPatch(xml_def->attributes().value("", "id").toString(), this));
     sub_object->defineObject(xml_def);
     break;
     default:
     // unexpected objects are handled by the base class. (probably producing an error)
     return PIGraphBranch::appendSubObject(xml_def);
     }
     return sub_object;
}

void PIGraphRoot::elaborateRoot(OrchBase* orch_in)
{
    if (orchestrator != NULL) return; // abort if the elaboration has already been done.
    orchestrator = orch_in;
    string t_name;
    // first elaborate the GraphTypes
    for (QVector<PIGraphObject*>::iterator graph_type = beginSubObjects(GTYPE); graph_type < endSubObjects(GTYPE); graph_type++)
    {
        t_name =static_cast<PIGraphType*>(*graph_type)->name().toStdString();
        if (orchestrator->P_typdclm.find(t_name)==orchestrator->P_typdclm.end()) // does the graphType already exist?
        {
           orchestrator->P_typdclm[t_name] = static_cast<PIGraphType*>(*graph_type)->elaborateGraphType(orchestrator); // if not set it up
        }
        else orchestrator->Post(45,t_name);
    }
    // then GraphInstances
    for (QVector<PIGraphObject*>::iterator graph_inst = beginSubObjects(GINST); graph_inst < endSubObjects(GINST); graph_inst++)
    {
        t_name = static_cast<PIGraphInstance*>(*graph_inst)->name().toStdString();
        if (orchestrator->P_taskm.find(t_name)==orchestrator->P_taskm.end()) // does the task already exist?
        {
           P_task* task = 0;
           if ((task = static_cast<PIGraphInstance*>(*graph_inst)->elaborateGraphInstance(orchestrator)) != NULL) orchestrator->P_taskm[t_name] = task; //if not set it up
        }
        else orchestrator->Post(49,t_name);
    }
}
