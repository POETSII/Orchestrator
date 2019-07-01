MPICHDIR  = /home/adr1r17/Prg/mpich-3.2.1
MPICHLIBDIR = $(MPICHDIR)/lib
MPICHINCDIR = $(MPICHDIR)/include/mpi
GCCDIR    = /home/adr1r17/Prg/gcc-7.3.0
LIBDIR    = $(GCCDIR)/lib
LIBXMLDIR = /home/adr1r17/Local_prg/libxml2.2.9.8
INCDIR    = $(GCCDIR)/include
ORCHDIR   = ../..
SOURCEDIR = $(ORCHDIR)/Source
GENERICSDIR = $(ORCHDIR)/Generics
COMMONDIR = $(SOURCEDIR)/Common
ORCHBASEDIR = $(SOURCEDIR)/OrchBase
NAMESRVDIR = $(SOURCEDIR)/NameServer
PARSERDIR = ./
XMLINCDIR = $(LIBXMLDIR)/include/libxml2/libxml
XMLIBDIR = $(LIBXMLDIR)/include/libxml2
XMLLIBDIR = $(LIBXMLDIR)/lib
JSONINCDIR = /home/alex_rast/Local_src/rapidJSON1.1/rapidjson-master/include/rapidjson
#HALFINCDIR = /home/alex_rast/adr1r17/Src/half-1.12.0/include
#BUILDDIR  = build
BINDIR    = bin

CC        = $(GCCDIR)/bin/gcc
CXX       = $(GCCDIR)/bin/g++
MPICC     = $(MPICHDIR)/bin/mpicc
MPICXX    = $(MPICHDIR)/bin/mpicxx

# must use -std=c++11 to get uintx_t, intx_t fixed-width types.
CPPFLAGS += -std=c++98 -g -I $(INCDIR) -I $(PARSERDIR) -I $(GENERICSDIR) -I $(COMMONDIR) -I $(ORCHBASEDIR) -I $(MPICHINCDIR) -I $(XMLINCDIR) -I $(JSONINCDIR) -I $(XMLIBDIR) -I $(NAMESRVDIR) -fPIC #-I $(HALFINCDIR) 

LDFLAGS += -lxml2
ORCHLDFLAGS := -lpthread -lmpi 
LDLIBS := -L$(LIBDIR) -L$(XMLLIBDIR)
ORCHLDLIBS := -L$(MPICHLIBDIR)

COMMON_SRC := $(wildcard $(SOURCEDIR)/Common/*.cpp)
COMMON_SRC := $(filter-out $(SOURCEDIR)/Common/Decode.cpp, $(COMMON_SRC))
COMMON_SRC += $(GENERICSDIR)/Msg_p.cpp $(GENERICSDIR)/flat.cpp $(GENERICSDIR)/rand.cpp $(GENERICSDIR)/Cli.cpp  $(GENERICSDIR)/lex.cpp $(GENERICSDIR)/dfprintf.cpp $(GENERICSDIR)/filename.cpp
COMMON_SRC += $(GENERICSDIR)/jnj.cpp $(GENERICSDIR)/uif.cpp
COMMON_IGNORE := $(addprefix $(GENERICSDIR)/, Msg_p.cpp jnj.cpp uif.cpp)
COMMON_IGNORE += $(addprefix $(SOURCEDIR)/Common/, CommonBase.cpp P_frame.cpp PMsg_p.cpp ProcMap.cpp Unrec_t.cpp)
COMMON_SRC := $(filter-out $(COMMON_IGNORE), $(COMMON_SRC))
COMMON_OBJ = $(subst .cpp,.o,$(COMMON_SRC))

ORCH_SRC := $(wildcard $(ORCHBASEDIR)/*.cpp)
ORCH_SHARE_SRC := $(addprefix $(ORCHBASEDIR)/, D_graph.cpp Config_t.cpp build_defs.cpp P_addr.cpp P_task.cpp P_box.cpp P_board.cpp P_core.cpp P_device.cpp P_thread.cpp Bin.cpp CFrag.cpp P_pin.cpp P_message.cpp CMsg_p.cpp)
ORCH_SHARE_SRC += $(GENERICSDIR)/dumpchan.cpp
ORCH_SHARE_IGNORE := $(ORCHBASEDIR)/CMsg_p.cpp
#ORCH_SRC += $(COMMON_SRC)
#ORCH_SRC += $(NAMESRVDIR)/Ns_el.cpp
ORCH_NOT_TRANS_U := $(addprefix $(ORCHBASEDIR)/, OrchBaseTask.cpp OrchBaseLink.cpp OrchBaseTopo.cpp OrchBaseOwner.cpp)
ORCH_IGNORE := $(addprefix $(ORCHBASEDIR)/, P_datatype.cpp P_datavalue.cpp P_builder.cpp OrchBase.cpp)
ORCH_SRC := $(filter-out $(ORCH_NOT_TRANS_U) $(ORCH_SHARE_SRC) $(ORCH_IGNORE), $(ORCH_SRC))
ORCH_SHARE_SRC := $(filter-out $(ORCH_SHARE_IGNORE), $(ORCH_SHARE_SRC))
ORCH_OBJ := $(subst .cpp,.o,$(ORCH_SRC))
ORCH_SHARE_OBJ := $(subst .cpp,.o,$(ORCH_SHARE_SRC))

BASEPARSER_SRC := P_Pparser.cpp
TESTPARSER_SRC := P_PparserTest.cpp PXMLParserMain.cpp
PARSER_SRC := $(wildcard *.cpp)
PARSER_NOTIMPL_SRC := P_Scalar.cpp P_Tuple.cpp P_Array.cpp P_Union.cpp P_JSONparser.cpp
PARSER_SRC := $(filter-out $(TESTPARSER_SRC) $(PARSER_NOTIMPL_SRC) orchbasedummy.cpp, $(PARSER_SRC))
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
#	$(MPICXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(ORCHLDFLAGS) $(LDLIBS) $(ORCHLDLIBS)
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(ORCHLDFLAGS) $(LDLIBS) $(ORCHLDLIBS)

$(XMLPARSER_TGT) : $(XMLPARSER_OBJ)
	mkdir -p $(BINDIR)
#	mkdir -p $(BUILDDIR)
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean :
	rm -rf $(PARSER_TGT) $(COMMON_OBJ) $(ORCH_OBJ) $(ORCH_SHARE_OBJ) $(PARSER_OBJ) $(XMLPARSER_TGT) $(XMLPARSER_OBJ)
