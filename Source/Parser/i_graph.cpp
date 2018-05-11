#include "i_graph.h"
#include "orchbasedummy.h"

I_Graph::I_Graph(QObject *parent) : QObject(parent), app_xml_def(), app_graph_def(NULL), app_text_def(NULL), orchestrator(NULL), internal_orch(false)
{

}

I_Graph::I_Graph(OrchBase* orchestrator, QObject *parent) : QObject(parent), app_xml_def(), app_graph_def(NULL), app_text_def(NULL), orchestrator(orchestrator), internal_orch(false)
{

}

I_Graph::I_Graph(const QString& input_filename, OrchBase* input_orchestrator, QObject *parent) : QObject(parent), app_xml_def(new QFile(input_filename, parent)), orchestrator(input_orchestrator), app_graph_def(NULL), app_text_def(NULL), internal_orch(false)
{

}

I_Graph::~I_Graph()
{
    delete app_graph_def;
    delete app_text_def;
    if (internal_orch) delete orchestrator;
}

const QVector<const PIGraphType*> I_Graph::graphTypes() const
{
    QVector<const PIGraphType*> graph_types;
    if (app_graph_def == NULL) return graph_types;
    for (QVector<const PIGraphObject*>::const_iterator obj = app_graph_def->beginConstSubObjects(PIGraphRoot::GTYPE); obj < app_graph_def->endConstSubObjects(PIGraphRoot::GTYPE); obj++)
        graph_types.append(static_cast<const PIGraphType*>(*obj));
    return graph_types; // copy semantics - potentially expensive, but we're not expecting to return many graphTypes!
}

const QVector<const PIGraphInstance*> I_Graph::graphInstances() const
{
    QVector<const PIGraphInstance*> graph_instances;
    if (app_graph_def == NULL) return graph_instances;
    for (QVector<const PIGraphObject*>::const_iterator obj = app_graph_def->beginConstSubObjects(PIGraphRoot::GINST); obj < app_graph_def->endConstSubObjects(PIGraphRoot::GINST); obj++)
        graph_instances.append(static_cast<const PIGraphInstance*>(*obj));
    return graph_instances; // see note above for graphTypes, same situation.
}

int I_Graph::load(const QString& input_filename)
{
    QString substituted(input_filename);
    QFile* i_file = new QFile(substituted.replace(QChar('\\'), QChar('/')), this->parent());
    if (i_file->open(QIODevice::ReadOnly))
    {
        app_xml_def.setDevice(i_file);
        return SUCCESS;
    }
    else return ERR_INPUT_FILE_NOTFOUND;
}

int I_Graph::parse()
{

    if (dynamic_cast<QFile*>(app_xml_def.device()) != 0)
    {
       app_graph_def = new PIGraphRoot(static_cast<QFile*>(app_xml_def.device())->fileName(), this);
       app_graph_def->defineObject(&app_xml_def);
       return SUCCESS;
    }
    else return ERR_INVALID_XML;
}

int I_Graph::translate(const QString &input_filename, OrchBase *orchestrator)
{
    int r_val = SUCCESS;
    if ((r_val = load(input_filename)) != SUCCESS) return r_val;
    if ((r_val = parse()) != SUCCESS) return r_val;
    return r_val = elaborate(orchestrator);
}

int I_Graph::elaborate(OrchBase *o_instance)
{
    if (o_instance == NULL)
    {
        if (orchestrator == NULL)
        {
           // hope this works...
           char* cmd_line_emul[1];
           QString dummy_name = "OrchestratorFETest";
           cmd_line_emul[0] = dummy_name.toUtf8().data();
           orchestrator = new OrchBaseDummy(1, cmd_line_emul, QString("OrchestratorFETest:OrchBase:CommonBase").toStdString(), QString("i_graph.cpp").toStdString());
           internal_orch = true;
        }
    }
    else orchestrator = o_instance;
    if (app_graph_def)
    {
        app_graph_def->elaborateRoot(orchestrator);
        return SUCCESS;
    }
    else return ERR_UNREC_GRAPH;
}

void I_Graph::display()
{
    if (app_graph_def == NULL) return; // later this will pop up an info message
    app_text_def = new QString(app_graph_def->print_def());
}

int I_Graph::save(const QString& filename)
{
    if (app_text_def == NULL) return ERR_UNREC_GRAPH; // later this will pop up an info message
    QFile outputFile(QString(filename).replace("\\","/"));
    if (outputFile.open(QIODevice::WriteOnly))
    {
       if (outputFile.write(app_text_def->toUtf8()) < 0) return ERR_OUTPUT_FILE_WRITE;
       return SUCCESS;
    }
    return ERR_INVALID_OUTPUT_FILE;
}

