#include "pmetadatapatch.h"
#include "pigraphroot.h"

PMetadataPatch::PMetadataPatch(const QString& name, PIGraphObject *parent) : PAnnotatedDef(name, "GraphInstanceMetadataPatch", QVector<int>(), parent), graph_to_patch(NULL)
{

}

void PMetadataPatch::defineObject(QXmlStreamReader* xml_def)
{
     PIGraphRoot* g_def = dynamic_cast<PIGraphRoot*>(parent());
     if (g_def != NULL) graph_to_patch = static_cast<PIGraphInstance*>(g_def->subObject(PIGraphRoot::GINST, g_def->subObjIndex(PIGraphRoot::GINST, name())));
     if (graph_to_patch) PIGraphBranch::defineObject(xml_def);
}

const PIGraphObject* PMetadataPatch::appendSubObject(QXmlStreamReader* xml_def)
{
     return graph_to_patch->appendSubObject(xml_def);
}
