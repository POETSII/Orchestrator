#ifndef I_GRAPH_H
#define I_GRAPH_H

#include <QtCore>
#include <QObject>
#include <QXmlStreamReader>
#include <QString>
#include <typeinfo>
#include "pigraphroot.h"
#include "pigraphtype.h"
#include "pigraphinstance.h"
#include "OrchBase.h"

class I_Graph : public QObject
{

public:
    I_Graph(QObject *parent = 0);
    I_Graph(OrchBase* input_orchestrator, QObject *parent = 0);
    I_Graph(const QString& input_filename, OrchBase* input_orchestrator = NULL, QObject *parent = 0);
    ~I_Graph();

    int translate(const QString& input_filename, OrchBase* orchestrator = NULL);
    const QVector<const PIGraphType*> graphTypes() const;
    const QVector<const PIGraphInstance*> graphInstances() const;
    int load(const QString& input_filename);
    int parse();
    int elaborate(OrchBase* o_instance = NULL);
    int save(const QString& filename);
    void display();

    enum errs {SUCCESS, ERR_INPUT_FILE_NOTFOUND, ERR_INVALID_XML, ERR_UNREC_GRAPH, ERR_INVALID_OUTPUT_FILE, ERR_OUTPUT_FILE_WRITE};

private:
    QXmlStreamReader app_xml_def;
    PIGraphRoot* app_graph_def;
    OrchBase* orchestrator;
    QString* app_text_def;
    bool internal_orch; // for testing - indicates if the orchestrator should be deleted on I_Graph destruction.

};

#endif // I_GRAPH_H
