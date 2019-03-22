MPICHDIR  = /usr
MPICHLIBDIR = $(MPICHDIR)/lib
MPICHINCDIR = $(MPICHDIR)/include/mpi
GCCDIR    = /usr/bin
LIBDIR    = /usr/lib
LIBXMLDIR = /home/alex_rast/Local_prg/libxml2.2.9.8
INCDIR    = /usr/include
ORCHDIR   = ../..
SOURCEDIR = $(ORCHDIR)/Source
GENERICSDIR = $(ORCHDIR)/Generics
COMMONDIR = $(SOURCEDIR)/Common
ORCHBASEDIR = $(SOURCEDIR)/OrchBase
NAMESRVDIR = $(SOURCEDIR)/NameServer
PARSERDIR = ./
XMLINCDIR = $(LIBXMLDIR)/include/libxml2/libxml
XMLINBDIR = $(LIBXMLDIR)/include/libxml2
XMLLIBDIR = $(LIBXMLDIR)/lib
#BUILDDIR  = build
BINDIR    = bin

CC        = $(GCCDIR)/gcc
CXX       = $(GCCDIR)/g++
MPICC     = $(MPICHDIR)/bin/mpicc
MPICXX    = $(MPICHDIR)/bin/mpicxx

# must use -std=c++11 to get uintx_t, intx_t fixed-width types.
#CPPFLAGS += -I Generics -I Source/Common -std=c++98 -I $(INCDIR) -I $(QTINCDIR) -include string.h -fpermissive
CPPFLAGS += -std=c++98 -g -I $(INCDIR) -I $(GENERICSDIR) -I $(COMMONDIR) -I $(ORCHBASEDIR) -I $(MPICHINCDIR) -I $(XMLINCDIR) -I $(XMLINBDIR) -I $(PARSERDIR) -I $(NAMESRVDIR) -fPIC

LDFLAGS += -lxml2
ORCHLDFLAGS := -lpthread -lmpi 
LDLIBS := -L$(LIBDIR) -L$(XMLLIBDIR)
ORCHLDLIBS := -L$(MPICHLIBDIR)

COMMON_SRC := $(wildcard $(SOURCEDIR)/Common/*.cpp)
COMMON_SRC := $(filter-out $(SOURCEDIR)/Common/Decode.cpp, $(COMMON_SRC))
COMMON_SRC += $(GENERICSDIR)/Msg_p.cpp $(GENERICSDIR)/flat.cpp $(GENERICSDIR)/rand.cpp $(GENERICSDIR)/Cli.cpp  $(GENERICSDIR)/lex.cpp $(GENERICSDIR)/dfprintf.cpp $(GENERICSDIR)/filename.cpp
COMMON_SRC += $(GENERICSDIR)/jnj.cpp $(GENERICSDIR)/uif.cpp
COMMON_OBJ = $(subst .cpp,.o,$(COMMON_SRC))

ORCH_SRC := $(wildcard $(ORCHBASEDIR)/*.cpp)
ORCH_SHARE_SRC := $(addprefix $(ORCHBASEDIR)/, D_graph.cpp Config_t.cpp build_defs.cpp P_addr.cpp P_task.cpp P_box.cpp P_board.cpp P_core.cpp P_device.cpp P_thread.cpp Bin.cpp CFrag.cpp P_pin.cpp P_message.cpp CMsg_p.cpp)
ORCH_SHARE_SRC += $(GENERICSDIR)/dumpchan.cpp
#ORCH_SRC += $(COMMON_SRC)
ORCH_SRC += $(NAMESRVDIR)/Ns_el.cpp
ORCH_NOT_TRANS_U := $(addprefix $(ORCHBASEDIR)/, OrchBaseTask.cpp OrchBaseLink.cpp OrchBaseTopo.cpp OrchBaseOwner.cpp)
ORCH_SRC := $(filter-out $(ORCH_NOT_TRANS_U) $(ORCH_SHARE_SRC), $(ORCH_SRC))
ORCH_OBJ := $(subst .cpp,.o,$(ORCH_SRC))
ORCH_SHARE_OBJ := $(subst .cpp,.o,$(ORCH_SHARE_SRC))

BASEPARSER_SRC := P_Pparser.cpp
TESTPARSER_SRC := P_PparserTest.cpp PXMLParserMain.cpp
PARSER_SRC := $(wildcard *.cpp)
PARSER_SRC := $(filter-out $(TESTPARSER_SRC), $(PARSER_SRC))
XMLPARSER_SRC := $(BASEPARSER_SRC) $(TESTPARSER_SRC)
PARSER_OBJ := $(subst .cpp,.o,$(PARSER_SRC))
XMLPARSER_OBJ := $(subst .cpp,.o,$(XMLPARSER_SRC))
PARSER_TGT = $(BINDIR)/OrchParser
XMLPARSER_TGT = $(BINDIR)/PXMLParser

.PHONY : all OrchParser PXMLParser clean

all : $(PARSER_TGT) $(XMLPARSER_TGT)

OrchParser: $(PARSER_TGT)

PXMLParser: $(XMLPARSER_TGT)

#$(ORCH_OBJ) : $(ORCH_SRC)
#	$(CXX) $(CPPFLAGS) -o $@ $^

#$(ORCH_SHARE_OBJ) : $(ORCH_SHARE_SRC)
#	$(CXX) $(CPPFLAGS) -o $@ $^

$(PARSER_TGT) : $(PARSER_OBJ) $(COMMON_OBJ) $(ORCH_OBJ) $(ORCH_SHARE_OBJ)
	mkdir -p $(BINDIR)
#	mkdir -p $(BUILDDIR)
	$(MPICXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(ORCHLDFLAGS) $(LDLIBS) $(ORCHLDLIBS)

$(XMLPARSER_TGT) : $(XMLPARSER_OBJ)
	mkdir -p $(BINDIR)
#	mkdir -p $(BUILDDIR)
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean :
	rm -rf $(PARSER_TGT) $(PARSER_OBJ) $(XMLPARSER_TGT) $(XMLPARSER_OBJ)
