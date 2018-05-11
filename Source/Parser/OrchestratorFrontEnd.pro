QT += core
QT -= gui

CONFIG += c++11

QMAKE_CC = mpicc
QMAKE_CXX = mpicxx

TARGET = root
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    i_graph.cpp \
    orchbasedummy.cpp \
    pannotateddef.cpp \
    pcodefragment.cpp \
    pconcretedef.cpp \
    pconcreteinstance.cpp \
    pdeviceinstance.cpp \
    pdevicetype.cpp \
    pedgeinstance.cpp \
    pidatatype.cpp \
    pidatavalue.cpp \
    pigraphbranch.cpp \
    pigraphinstance.cpp \
    pigraphleaf.cpp \
    pigraphobject.cpp \
    pigraphroot.cpp \
    pigraphtype.cpp \
    piinputpin.cpp \
    pioutputpin.cpp \
    pipin.cpp \
    pmessagetype.cpp \
    pmetadatapatch.cpp \
    poetsdatatype.cpp \
    psubobjects.cpp \
    psupervisordevicetype.cpp \
    Root.cpp \
    RootMain.cpp \
    ../../Orchestrator/Source_B/OrchBase/Bin.cpp \
    ../../Orchestrator/Source_B/OrchBase/CFrag.cpp \
    ../../Orchestrator/Source_B/OrchBase/Config_t.cpp \
    ../../Orchestrator/Source_B/OrchBase/Constraints.cpp \
    ../../Orchestrator/Source_B/OrchBase/D_graph.cpp \
    ../../Orchestrator/Source_B/OrchBase/OrchBase.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_board.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_box.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_builder.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_channel.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_core.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_devdcl.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_device.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_devtyp.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_graph.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_link.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_message.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_pin.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_pintyp.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_port.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_task.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_thread.cpp \
    ../../Orchestrator/Source_B/OrchBase/P_typdcl.cpp \
    ../../Orchestrator/Source_B/OrchBase/Placement.cpp \
    ../../Orchestrator/Source_B/OrchBase/T_gen.cpp \
    ../../Orchestrator/Source/Common/CommonBase.cpp \
    ../../Orchestrator/Source/Common/Environment.cpp \
    ../../Orchestrator/Source/Common/NameBase.cpp \
    ../../Orchestrator/Source/Common/PMsg_p.cpp \
    ../../Orchestrator/Source/Common/ProcMap.cpp \
    ../../Orchestrator/Source/Common/Unrec_t.cpp \
    ../../Orchestrator/Generics/Cli.cpp \
    ../../Orchestrator/Generics/dfprintf.cpp \
    ../../Orchestrator/Generics/dumpchan.cpp \
    ../../Orchestrator/Generics/filename.cpp \
    ../../Orchestrator/Generics/flat.cpp \
    ../../Orchestrator/Generics/header.cpp \
    ../../Orchestrator/Generics/jnj.cpp \
    ../../Orchestrator/Generics/lex.cpp \
    ../../Orchestrator/Generics/map2.tpp \
    ../../Orchestrator/Generics/Msg_p.cpp \
    ../../Orchestrator/Generics/Msg_p.tpp \
    ../../Orchestrator/Generics/pdigraph.tpp \
    ../../Orchestrator/Generics/rand.cpp \
    ../../Orchestrator/Generics/uif.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    i_graph.h \
    orchbasedummy.h \
    pannotateddef.h \
    pcodefragment.h \
    pconcretedef.h \
    pconcreteinstance.h \
    pdeviceinstance.h \
    pdevicetype.h \
    pedgeinstance.h \
    pidatatype.h \
    pidatavalue.h \
    pigraphbranch.h \
    pigraphinstance.h \
    pigraphleaf.h \
    pigraphobject.h \
    pigraphroot.h \
    pigraphtype.h \
    piinputpin.h \
    pioutputpin.h \
    pipin.h \
    pmessagetype.h \
    pmetadatapatch.h \
    poetsdatatype.h \
    psubobjects.h \
    psupervisordevicetype.h \
    Root.h \
    ../../Orchestrator/Source_B/OrchBase/Bin.h \
    ../../Orchestrator/Source_B/OrchBase/CFrag.h \
    ../../Orchestrator/Source_B/OrchBase/Config_t.h \
    ../../Orchestrator/Source_B/OrchBase/Constraints.h \
    ../../Orchestrator/Source_B/OrchBase/D_graph.h \
    ../../Orchestrator/Source_B/OrchBase/OrchBase.h \
    ../../Orchestrator/Source_B/OrchBase/P_board.h \
    ../../Orchestrator/Source_B/OrchBase/P_box.h \
    ../../Orchestrator/Source_B/OrchBase/P_builder.h \
    ../../Orchestrator/Source_B/OrchBase/P_channel.h \
    ../../Orchestrator/Source_B/OrchBase/P_core.h \
    ../../Orchestrator/Source_B/OrchBase/P_devdcl.h \
    ../../Orchestrator/Source_B/OrchBase/P_device.h \
    ../../Orchestrator/Source_B/OrchBase/P_devtyp.h \
    ../../Orchestrator/Source_B/OrchBase/P_graph.h \
    ../../Orchestrator/Source_B/OrchBase/P_link.h \
    ../../Orchestrator/Source_B/OrchBase/P_message.h \
    ../../Orchestrator/Source_B/OrchBase/P_pin.h \
    ../../Orchestrator/Source_B/OrchBase/P_pintyp.h \
    ../../Orchestrator/Source_B/OrchBase/P_port.h \
    ../../Orchestrator/Source_B/OrchBase/P_task.h \
    ../../Orchestrator/Source_B/OrchBase/P_thread.h \
    ../../Orchestrator/Source_B/OrchBase/P_typdcl.h \
    ../../Orchestrator/Source_B/OrchBase/Placement.h \
    ../../Orchestrator/Source_B/OrchBase/T_gen.h \
    ../../Orchestrator/Source/Common/CommonBase.h \
    ../../Orchestrator/Source/Common/Environment.h \
    ../../Orchestrator/Source/Common/NameBase.h \
    ../../Orchestrator/Source/Common/Pglobals.h \
    ../../Orchestrator/Source/Common/PMsg_p.hpp \
    ../../Orchestrator/Source/Common/ProcMap.h \
    ../../Orchestrator/Source/Common/Unrec_t.h \
    ../../Orchestrator/Generics/Cli.h \
    ../../Orchestrator/Generics/dfprintf.h \
    ../../Orchestrator/Generics/e.h \
    ../../Orchestrator/Generics/filename.h \
    ../../Orchestrator/Generics/flat.h \
    ../../Orchestrator/Generics/header.h \
    ../../Orchestrator/Generics/jnj.h \
    ../../Orchestrator/Generics/lex.h \
    ../../Orchestrator/Generics/macros.h \
    ../../Orchestrator/Generics/map2.h \
    ../../Orchestrator/Generics/Msg_p.hpp \
    ../../Orchestrator/Generics/msg_p.hpp \
    ../../Orchestrator/Generics/pdigraph.hpp \
    ../../Orchestrator/Generics/rand.h \
    ../../Orchestrator/Generics/uif.h \
    ../../Orchestrator/Source_B/OrchBase/build_defs.h

LIBS += "-L/usr/lib" -lmpi

INCLUDEPATH += ../../Orchestrator/Source/Common \
    ../../Orchestrator/Source_B/OrchBase \
    ../../Orchestrator/Generics \
    /usr/include \
    /usr/include/mpi
