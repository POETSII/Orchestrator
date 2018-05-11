MPICHDIR  = /usr
LIBDIR    = /usr/lib
INCDIR    = /usr/include
BUILDDIR  = build
BINDIR    = bin
QTLIBDIR     = /home/adr1r17/Local_prg/Qt.5/Qt5.6/5.6.3/gcc_64/lib
QTINCDIR     = /home/adr1r17/Local_prg/Qt.5/Qt5.6/5.6.3/gcc_64/include/QtCore

CC        = $(MPICHDIR)/bin/mpicc
CXX       = $(MPICHDIR)/bin/mpicxx

CPPFLAGS += -I Generics -I Source/Common -std=c++98 -I $(INCDIR) -I $(QTINCDIR) -include string.h -fpermissive
ORCHFLAGS := -I Source/OrchBase -I Source/Injector -I Source/Parser
#-I /usr/include/mpi

LDFLAGS += -lpthread -L$(LIBDIR) -L$(QTLIBDIR)


COMMON_SRC := $(wildcard Source/Common/*.cpp)
COMMON_SRC := $(filter-out Source/Common/Decode.cpp, $(COMMON_SRC))
COMMON_SRC += Generics/Msg_p.cpp Generics/flat.cpp Generics/rand.cpp Generics/Cli.cpp  Generics/lex.cpp Generics/dfprintf.cpp
COMMON_SRC += Generics/jnj.cpp Generics/uif.cpp 

ORCH_SRC := $(wildcard Source/Root/*.cpp)
ORCH_SRC += $(wildcard Source/OrchBase/*.cpp)
ORCH_SRC += $(wildcard Source/Parser/*.cpp)
ORCH_SRC += $(COMMON_SRC)
ORCH_NOT_TRANS_U := Source/OrchBase/OrchBaseTask.cpp Source/OrchBase/OrchBaseLink.cpp Source/OrchBase/OrchBaseTopo.cpp Source/OrchBase/OrchBaseOwner.cpp
ORCH_SRC := $(filter-out $(ORCH_NOT_TRANS_U), $(ORCH_SRC))
ORCH_DIROBJ := $(subst .cpp,.o,$(ORCH_SRC))
ORCH_OBJ = $(addprefix $(BUILDDIR)/,$(notdir $(ORCH_DIROBJ)))
ORCH_TGT = $(BINDIR)/orchestrator

DUMMY1_SRC := $(wildcard Source/Dummy/*.cpp)
DUMMY1_SRC += $(COMMON_SRC)
DUMMY1_DIROBJ := $(subst .cpp,.o,$(DUMMY1_SRC))
DUMMY1_OBJ = $(addprefix $(BUILDDIR)/,$(notdir $(DUMMY1_DIROBJ)))
DUMMY1_TGT = $(BINDIR)/dummy

LOGSERVER_SRC := $(wildcard Source/LogServer/*.cpp)
LOGSERVER_SRC += $(COMMON_SRC)
LOGSERVER_DIROBJ := $(subst .cpp,.o,$(LOGSERVER_SRC))
LOGSERVER_OBJ = $(addprefix $(BUILDDIR)/,$(notdir $(LOGSERVER_DIROBJ)))
LOGSERVER_TGT = $(BINDIR)/logserver

RTCL_SRC := $(wildcard Source/RTCL/*.cpp)
RTCL_SRC += $(COMMON_SRC)
RTCL_DIROBJ := $(subst .cpp,.o,$(RTCL_SRC))
RTCL_OBJ = $(addprefix $(BUILDDIR)/,$(notdir $(RTCL_DIROBJ)))
RTCL_TGT = $(BINDIR)/rtcl

INJECTOR_SRC := $(wildcard Source/Injector/*.cpp)
INJECTOR_SRC += $(COMMON_SRC)
INJECTOR_DIROBJ := $(subst .cpp,.o,$(INJECTOR_SRC))
INJECTOR_OBJ = $(addprefix $(BUILDDIR)/,$(notdir $(INJECTOR_DIROBJ)))
INJECTOR_TGT = $(BINDIR)/injector

#NAMESERVER_SRC := $(wildcard Source/NameServer/*.cpp)
#NAMESERVER_SRC += Source/Common/NameBase.cpp Source/Common/PMsg_p.cpp
#NAMESERVER_SRC += Source/OrchBase/P_addr.cpp
#NAMESERVER_SRC += Generics/Msg_p.cpp Generics/flat.cpp
#NAMESERVER_DIROBJ := $(subst .cpp,.o,$(NAMESERVER_SRC))
#NAMESERVER_OBJ = $(addprefix $(BUILDDIR)/,$(notdir $(NAMESERVER_DIROBJ)))
#NAMESERVER_TGT = $(BINDIR)/nameserver

all : $(ORCH_TGT) $(DUMMY1_TGT) $(LOGSERVER_TGT) $(RTCL_TGT) $(INJECTOR_TGT) # $(NAMESERVER_TGT)

Orchestrator : $(ORCH_TGT)

Dummy : $(DUMMY1_TGT)

Logserver : $(LOGSERVER_TGT)

RTCL: $(RTCL_TGT)

Injector: $(INJECTOR_TGT)

#Nameserver: $(NAMESERVER_TGT)

$(ORCH_DIROBJ) : $(ORCH_SRC)
	$(CXX) $(ORCHFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(ORCH_TGT) : $(ORCH_DIROBJ)
	mkdir -p $(BINDIR)
#	mkdir -p $(BUILDDIR)
	$(CXX) $(CPPFLAGS) $(ORCHFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(DUMMY1_TGT) : $(DUMMY1_DIROBJ)
	mkdir -p $(BINDIR)
#	mkdir -p $(BUILDDIR)
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(LOGSERVER_TGT) : $(LOGSERVER_DIROBJ)
	mkdir -p $(BINDIR)
#	mkdir -p $(BUILDDIR)
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(RTCL_TGT) : $(RTCL_DIROBJ)
	mkdir -p $(BINDIR)
#	mkdir -p $(BUILDDIR)
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(INJECTOR_TGT) : $(INJECTOR_DIROBJ)
	mkdir -p $(BINDIR)
#	mkdir -p $(BUILDDIR)
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

#$(NAMESERVER_DIROBJ) : $(NAMESERVER_SRC)
#	$(CXX) $(ORCHFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

#$(NAMESERVER_TGT) : $(NAMESERVER_DIROBJ)
#	mkdir -p $(BINDIR)
##	mkdir -p $(BUILDDIR)
#	$(CXX) $(ORCHFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean :
	rm -rf $(ORCH_TGT) $(DUMMY1_TGT) $(LOGSERVER_TGT) $(RTCL_TGT) $(INJECTOR_TGT) $(ORCH_OBJ) $(DUMMY1_OBJ) $(LOGSERVER_OBJ) $(RTCL_OBJ) $(INJECTOR_OBJ) $(ORCH_DIROBJ) $(DUMMY1_DIROBJ) $(LOGSERVER_DIROBJ) $(RTCL_DIROBJ) $(INJECTOR_DIROBJ)
#$(NAMESERVER_TGT) $(NAMESERVER_OBJ) $(NAMESERVER_DIROBJ)
