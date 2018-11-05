MPICHDIR  = /home/adr1r17/Prg/mpich-3.2.1
MPICHLIBDIR = $(MPICHDIR)/lib
MPICHINCDIR = $(MPICHDIR)/include
LIBDIR    = /usr/lib
INCDIR    = /usr/include
ROOTDIR = ../..
BUILDDIR  = $(ROOTDIR)/Build
BINDIR    = $(ROOTDIR)/bin
SOURCEDIR = $(ROOTDIR)/Source
GENERICSDIR = $(ROOTDIR)/Generics
QTLIBDIR  = /home/adr1r17/Local_prg/Qt.5/Qt5.6/5.6.3/gcc_64/lib
QTINCDIR  = /home/adr1r17/Local_prg/Qt.5/Qt5.6/5.6.3/gcc_64/include
#TINSELDIR = /home/adr1r17/adr1r17_Soton/data/code/tinsel/tinsel-tinsel-0.3.1
TINSELDIR = /home/adr1r17/adr1r17_Soton/data/code/tinsel/tinsel-0.4/tinsel
TINSELINCDIR = $(realpath $(TINSELDIR)/include)
TINSELLIBDIR = $(realpath $(TINSELDIR)/lib)
TINSELHLDIR  = $(realpath $(TINSELDIR)/hostlink)
QUARTUSLIBDIR = /home/Local_prg/QuartusPro.18/quartus/linux64
SOFTSWITCHDIR = /home/adr1r17/adr1r17_Soton/data/code/Orchestrator/Source/Softswitch
PLIBDIR = /home/adr1r17/adr1r17_Soton/data/code/Orchestrator/Build
ORCHBASE_DIR = $(SOURCEDIR)/OrchBase

CC        = $(MPICHDIR)/bin/mpicc
CXX       = $(MPICHDIR)/bin/mpicxx

# must use -std=c++11 to get uintx_t, intx_t fixed-width types.
#CPPFLAGS += -I $(GENERICSDIR) -I Source/Common -std=c++98 -I $(INCDIR) -I $(QTINCDIR) -include string.h -fpermissive
CPPFLAGS += -I $(GENERICSDIR) -I $(SOURCEDIR)/Common -std=c++11 -I $(MPICHINCDIR) -I $(INCDIR) -I $(QTINCDIR) -I $(QTINCDIR)/QtCore
ORCHFLAGS := -I $(SOURCEDIR)/OrchBase -I $(SOURCEDIR)/Injector -I $(SOURCEDIR)/Parser -I $(SOURCEDIR)/NameServer -fPIC
MOTHFLAGS :=  -I $(TINSELINCDIR) -I $(TINSELHLDIR) -I $(SOFTSWITCHDIR)/inc -I $(ORCHBASE_DIR)
#-I /usr/include/mpi

LDFLAGS += -lpthread -L$(MPICHLIBDIR) -L$(LIBDIR) -L$(QTLIBDIR)
ORCHLDFLAGS = -lQt5Core -lmpi
MOTHLDFLAGS = -L$(QUARTUSLIBDIR) -L$(PLIBDIR) -ljtag_atlantic -ljtag_client -lSupervisor

COMMON_SRC := $(wildcard $(SOURCEDIR)/Common/*.cpp)
COMMON_SRC := $(filter-out $(SOURCEDIR)/Common/Decode.cpp, $(COMMON_SRC))
COMMON_SRC += $(GENERICSDIR)/Msg_p.cpp $(GENERICSDIR)/flat.cpp $(GENERICSDIR)/rand.cpp $(GENERICSDIR)/Cli.cpp  $(GENERICSDIR)/lex.cpp $(GENERICSDIR)/dfprintf.cpp
COMMON_SRC += $(GENERICSDIR)/jnj.cpp $(GENERICSDIR)/uif.cpp
COMMON_DIROBJ = $(subst .cpp,.o,$(COMMON_SRC))
COMMON_OBJ = $(addprefix $(BUILDDIR)/,$(notdir $(COMMON_DIROBJ)))

ORCH_SRC := $(wildcard $(SOURCEDIR)/Root/*.cpp)
ORCH_SRC += $(wildcard $(ORCHBASE_DIR)/*.cpp)
ORCH_SRC += $(wildcard $(SOURCEDIR)/Parser/*.cpp)
ORCH_SHARE_SRC := $(addprefix $(ORCHBASE_DIR)/, D_graph.cpp Config_t.cpp build_defs.cpp P_addr.cpp P_task.cpp P_box.cpp P_board.cpp P_core.cpp P_device.cpp P_thread.cpp Bin.cpp CFrag.cpp P_pin.cpp P_message.cpp CMsg_p.cpp)
ORCH_SHARE_SRC += $(GENERICSDIR)/dumpchan.cpp
#ORCH_SRC += $(COMMON_SRC)
ORCH_NOT_TRANS_U := $(addprefix $(ORCHBASE_DIR)/, OrchBaseTask.cpp OrchBaseLink.cpp OrchBaseTopo.cpp OrchBaseOwner.cpp)
ORCH_SRC := $(filter-out $(ORCH_NOT_TRANS_U) $(ORCH_SHARE_SRC), $(ORCH_SRC))
ORCH_DIROBJ := $(subst .cpp,.o,$(ORCH_SRC))
ORCH_SHARE_DIROBJ := $(subst .cpp,.o,$(ORCH_SHARE_SRC))
ORCH_OBJ = $(addprefix $(BUILDDIR)/,$(notdir $(ORCH_DIROBJ)))
ORCH_SHARE_OBJ = $(addprefix $(BUILDDIR)/, $(notdir $(ORCH_SHARE_DIROBJ)))
ORCH_TGT = $(BINDIR)/orchestrator

DUMMY1_SRC := $(wildcard $(SOURCEDIR)/Dummy/*.cpp)
#DUMMY1_SRC += $(COMMON_SRC)
DUMMY1_DIROBJ := $(subst .cpp,.o,$(DUMMY1_SRC))
DUMMY1_OBJ = $(addprefix $(BUILDDIR)/,$(notdir $(DUMMY1_DIROBJ)))
DUMMY1_TGT = $(BINDIR)/dummy

LOGSERVER_SRC := $(wildcard $(SOURCEDIR)/LogServer/*.cpp)
#LOGSERVER_SRC += $(COMMON_SRC)
LOGSERVER_DIROBJ := $(subst .cpp,.o,$(LOGSERVER_SRC))
LOGSERVER_OBJ = $(addprefix $(BUILDDIR)/,$(notdir $(LOGSERVER_DIROBJ)))
LOGSERVER_TGT = $(BINDIR)/logserver

RTCL_SRC := $(wildcard $(SOURCEDIR)/RTCL/*.cpp)
#RTCL_SRC += $(COMMON_SRC)
RTCL_DIROBJ := $(subst .cpp,.o,$(RTCL_SRC))
RTCL_OBJ = $(addprefix $(BUILDDIR)/,$(notdir $(RTCL_DIROBJ)))
RTCL_TGT = $(BINDIR)/rtcl

INJECTOR_SRC := $(wildcard $(SOURCEDIR)/Injector/*.cpp)
#INJECTOR_SRC += $(COMMON_SRC)
INJECTOR_DIROBJ := $(subst .cpp,.o,$(INJECTOR_SRC))
INJECTOR_OBJ = $(addprefix $(BUILDDIR)/,$(notdir $(INJECTOR_DIROBJ)))
INJECTOR_TGT = $(BINDIR)/injector

#NAMESERVER_SRC := $(wildcard $(SOURCEDIR)/NameServer/*.cpp)
#NAMESERVER_SRC += $(SOURCEDIR)/Common/NameBase.cpp $(SOURCEDIR)/Common/PMsg_p.cpp
#NAMESERVER_SRC += $(SOURCEDIR)/OrchBase/P_addr.cpp
#NAMESERVER_SRC += $(GENERICSDIR)/Msg_p.cpp $(GENERICSDIR)/flat.cpp
#NAMESERVER_DIROBJ := $(subst .cpp,.o,$(NAMESERVER_SRC))
#NAMESERVER_OBJ = $(addprefix $(BUILDDIR)/,$(notdir $(NAMESERVER_DIROBJ)))
#NAMESERVER_TGT = $(BINDIR)/nameserver

MOTHERSHIP_DIR := $(SOURCEDIR)/Mothership
MOTHERSHIP_SRC := $(addprefix $(MOTHERSHIP_DIR)/, MothershipMain.cpp TMoth.cpp TaskInfo.cpp)
MOTHERSHIP_SRC += $(addprefix $(TINSELHLDIR)/, HostLink.cpp DebugLink.cpp MemFileReader.cpp UART.cpp PowerLink.cpp)
MOTHERSHIP_SRC += $(SOFTSWITCHDIR)/src/poets_msg.cpp
MOTHERSHIP_INC := $(addprefix $(MOTHERSHIP_DIR)/, TMoth.h TaskInfo.h)
MOTHERSHIP_INC += $(addprefix $(TINSELINCDIR)/, config.h boot.h)
MOTHERSHIP_INC += $(addprefix $(TINSELHLDIR)/, HostLink.h DebugLink.h MemFileReader.h UART.h PowerLink.h)
MOTHERSHIP_INC += $(SOFTSWITCHDIR)/inc/poets_msg.h
MOTHERSHIP_DIROBJ := $(subst .cpp,.o, $(MOTHERSHIP_SRC))
MOTHERSHIP_OBJ = $(addprefix $(BUILDDIR)/,$(notdir $(MOTHERSHIP_DIROBJ)))
MOTHERSHIP_TGT = $(BINDIR)/mothership

all : $(ORCH_TGT) $(DUMMY1_TGT) $(LOGSERVER_TGT) $(RTCL_TGT) $(INJECTOR_TGT) $(MOTHERSHIP_TGT)# $(NAMESERVER_TGT)

Orchestrator : $(ORCH_TGT)

Dummy : $(DUMMY1_TGT)

Logserver : $(LOGSERVER_TGT)

RTCL: $(RTCL_TGT)

Injector: $(INJECTOR_TGT)

Mothership: $(MOTHERSHIP_TGT)

SupervisorLib: $(PLIBDIR)/libSupervisor.so

#Nameserver: $(NAMESERVER_TGT)

$(TINSELINCDIR)/config.h: $(TINSELDIR)/config.py
	echo '#ifndef _CONFIG_H_' > $(TINSELINCDIR)/config.h
	echo '#define _CONFIG_H_\n' >> $(TINSELINCDIR)/config.h
	python $(TINSELDIR)/config.py cpp >> $(TINSELINCDIR)/config.h
	echo '\n#endif' >> $(TINSELINCDIR)/config.h

$(PLIBDIR)/libSupervisor.so: $(MOTHERSHIP_DIR)/Supervisor.cpp $(MOTHERSHIP_DIR)/TMoth.h $(TINSELINCDIR)/config.h
	$(CXX) $(CPPFLAGS) $(MOTHFLAGS) -fPIC -c $< -o $(BUILDDIR)/Supervisor.o
	$(CXX) -shared -Wl,-soname,libSupervisor.so -o $@ $(BUILDDIR)/Supervisor.o

$(ORCH_DIROBJ) : $(ORCH_SRC)
	$(CXX) $(ORCHFLAGS) $(CPPFLAGS) -o $@ $^

$(ORCH_TGT) : $(ORCH_DIROBJ) $(ORCH_SHARE_DIROBJ) $(COMMON_DIROBJ)
	mkdir -p $(BINDIR)
#	mkdir -p $(BUILDDIR)
	$(CXX) $(CPPFLAGS) $(ORCHFLAGS) -o $@ $^ $(LDFLAGS) $(ORCHLDFLAGS) $(LDLIBS)

$(DUMMY1_TGT) : $(DUMMY1_DIROBJ) $(COMMON_DIROBJ)
	mkdir -p $(BINDIR)
#	mkdir -p $(BUILDDIR)
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(LOGSERVER_TGT) : $(LOGSERVER_DIROBJ) $(COMMON_DIROBJ)
	mkdir -p $(BINDIR)
#	mkdir -p $(BUILDDIR)
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(RTCL_TGT) : $(RTCL_DIROBJ) $(COMMON_DIROBJ)
	mkdir -p $(BINDIR)
#	mkdir -p $(BUILDDIR)
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(INJECTOR_TGT) : $(INJECTOR_DIROBJ) $(COMMON_DIROBJ)
	mkdir -p $(BINDIR)
#	mkdir -p $(BUILDDIR)
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

#$(NAMESERVER_DIROBJ) : $(NAMESERVER_SRC)
#	$(CXX) $(ORCHFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

#$(NAMESERVER_TGT) : $(NAMESERVER_DIROBJ)
#	mkdir -p $(BINDIR)
##	mkdir -p $(BUILDDIR)
#	$(CXX) $(ORCHFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(MOTHERSHIP_DIROBJ) : %.o: %.cpp $(MOTHERSHIP_INC)
	$(CXX) $(CPPFLAGS) $(MOTHFLAGS) -o $@ -c $*.cpp

$(MOTHERSHIP_TGT) : $(MOTHERSHIP_DIROBJ) $(COMMON_DIROBJ) $(ORCH_SHARE_DIROBJ) $(PLIBDIR)/libSupervisor.so
	mkdir -p $(BINDIR)
#	mkdir -p $(BUILDDIR)
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS) $(MOTHLDFLAGS)

clean :
	rm -rf $(ORCH_TGT) $(DUMMY1_TGT) $(LOGSERVER_TGT) $(RTCL_TGT) $(INJECTOR_TGT) $(MOTHERSHIP_TGT) $(ORCH_OBJ) $(ORCH_SHARE_OBJ) $(DUMMY1_OBJ) $(LOGSERVER_OBJ) $(RTCL_OBJ) $(INJECTOR_OBJ) $(MOTHERSHIP_OBJ) $(ORCH_DIROBJ) $(ORCH_SHARE_DIROBJ) $(DUMMY1_DIROBJ) $(LOGSERVER_DIROBJ) $(RTCL_DIROBJ) $(INJECTOR_DIROBJ) $(MOTHERSHIP_DIROBJ) $(TINSELINCDIR)/config.h
#$(NAMESERVER_TGT) $(NAMESERVER_OBJ) $(NAMESERVER_DIROBJ)
