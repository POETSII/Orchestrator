# This Makefile builds the various components of the Orchestrator using gcc. It
# assumes you are running GNU/Linux, and have certain commonly-available GNU
# utilities installed (you'll know if you don't).

# Bash is used over sh for hashtables when building orchestrate (and probably
# other reasons).
SHELL = /bin/bash

# Software dependencies (MPI, Qt, etc.) are largely hardcoded, because I'm too
# short on time to write an autoconf wrapper or some other dependency
# management system.
#
# If you are a user, you should modify the Makefile.dependencies file in this
# directory to correctly path to the dependencies required to build the
# components of the Orchestrator. You should not need to modify this particular
# file.
#
# If you are a developer, you may also need to modify the following file to
# incorporate your additional sources:
include Makefile.dependencies

# Path to the root directory of the Orchestrator (not to be confused with the
# root process of the Orchestrator).
ROOT_DIR = ../..

# Where sources live.
GENERICS_DIR := $(ROOT_DIR)/Generics
SOURCE_DIR := $(ROOT_DIR)/Source
TESTS_DIR := $(ROOT_DIR)/Tests

# Objects constructed from Orchestrator sources are placed in here. Objects are
# placed in a directory structure that mimics the source directory structure.
OBJECT_DIR := ./Objects

# We also compile some hostlink sources for the Mothership component of the
# Orchestrator.
HOSTLINK_OBJECT_DIR := ./Objects/Hostlink

# Executable binaries are built to here (directory is created as executable
# binaries are built):
EXECUTABLE_DIR := $(ROOT_DIR)/bin

# Test binaries are built to here:
TEST_EXECUTABLE_DIR := $(ROOT_DIR)/test_bin

# Supervisors are placed in here. Supervisors (.so) are shared object library
# files that are generated from supervisor object files (.o), which are in turn
# generated from source files (.cpp).
SUPERVISOR_LIB_DIR := $(EXECUTABLE_DIR)

# A note on dependency handling: we couple gcc's various -M flags with Make's
# ability to import dependency files to manage build dependencies. For more
# information,
# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/#combine
#
# For context, we only want to rebuild objects if the source files that
# "affect" the object have changed. Maintaining a static list of dependencies
# (formally prerequisites), but this project is too big to make that a viable
# option, hence why I'm generating them at compilation time.
#
# In essence, when we build an object from a source file, we also create a
# Makefile dependency list for that source file if one already doesn't exist,
# at $(DEPENDENCY_DIR)/$*.temp.d. If the compilation is successful, we then
# move it to $(DEPENDENCY_DIR)/$*.d (see *Note 1*).
DEPENDENCY_DIR = ./Dependency_lists
DEPENDENCY_FLAGS = -MT "$@" -MMD -MP -MF $(DEPENDENCY_DIR)/$*.temp.d
$(shell mkdir --parents $(DEPENDENCY_DIR))

# Include generated dependency files, because now is as good a time as any. We
# use find to do this, which is not the most portable way. If you can think of
# something better, let me know!
include $(shell find $(DEPENDENCY_DIR) -name "*.d" ! -name "*.temp.d")

# Where the compiler can look for header files when building Orchestrator
# objects:
INCLUDE_FLAGS = -I$(GENERICS_DIR) \
                -I$(SOURCE_DIR)/Common \
                -I$(SOURCE_DIR)/Common/HardwareModel \
                -I$(SOURCE_DIR)/OrchBase \
                -I$(SOURCE_DIR)/OrchBase/HardwareFileManager \
                -I$(SOURCE_DIR)/OrchBase/HardwareConfigurationDeployment \
                -I$(SOURCE_DIR)/Injector \
                -I$(SOURCE_DIR)/Parser \
                -I$(SOURCE_DIR)/NameServer \
                -I$(SOURCE_DIR)/Softswitch/inc \
                -I$(MPICH_INC_DIR) \
                -I$(QT_INC_DIR) \
                -I$(QT_INC_DIR)/QtCore \
                -I$(TINSEL_INC_DIR) \
                -I$(TINSEL_HOSTLINK_DIR)

# All flags to be used in compilation. We're not complying with C++98 yet due
# to the hostlink and some other minor changes, though nobody has studied this
# comprehensively.
CXXFLAGS = $(DEPENDENCY_FLAGS) $(INCLUDE_FLAGS) \
           -std=c++11 -Wall -fPIC -pthread

# Handy aliases for building executables, scripts, and libraries. Order doesn't
# matter.
ORCHESTRATE_BASENAME = orchestrate.sh
ORCHESTRATE_SCRIPT = $(ROOT_DIR)/$(ORCHESTRATE_BASENAME)

ROOT_EXECUTABLE = $(EXECUTABLE_DIR)/root
DUMMY_EXECUTABLE = $(EXECUTABLE_DIR)/dummy
LOGSERVER_EXECUTABLE = $(EXECUTABLE_DIR)/logserver
LOGSERVER_MESSAGE_FILE = $(EXECUTABLE_DIR)/OrchestratorMessages.txt
RTCL_EXECUTABLE = $(EXECUTABLE_DIR)/rtcl
INJECTOR_EXECUTABLE = $(EXECUTABLE_DIR)/injector
MOTHERSHIP_EXECUTABLE = $(EXECUTABLE_DIR)/mothership
STUB_SUPERVISOR = $(SUPERVISOR_LIB_DIR)/libSupervisor.so
JTAG_ATLANTIC_LOCAL = $(EXECUTABLE_DIR)/libjtag_atlantic.so
JTAG_CLIENT_LOCAL = $(EXECUTABLE_DIR)/libjtag_client.so

all: orchestrate root dummy logserver rtcl injector mothership
orchestrate: $(ORCHESTRATE_SCRIPT)
root: $(ROOT_EXECUTABLE)
dummy: $(DUMMY_EXECUTABLE)
logserver: $(LOGSERVER_EXECUTABLE) $(LOGSERVER_MESSAGE_FILE)
rtcl: $(RTCL_EXECUTABLE)
injector: $(INJECTOR_EXECUTABLE)
mothership: $(MOTHERSHIP_EXECUTABLE) \
            $(JTAG_ATLANTIC_LOCAL) $(JTAG_CLIENT_LOCAL)
stub_supervisor: $(STUB_SUPERVISOR)

# Prerequisites (objects) for building executables are defined here:
include Makefile.executable_prerequisites

# Prerequisites for building tests are defined here:
include Makefile.test_prerequisites
tests: $(ALL_TESTS)

# Linking targets (to generate executables). See
# Makefile.executable_prerequisites for the prerequisites of these executables
# (obviously...)
$(ROOT_EXECUTABLE) $(DUMMY_EXECUTABLE) $(LOGSERVER_EXECUTABLE) \
$(RTCL_EXECUTABLE) $(INJECTOR_EXECUTABLE) $(ALL_TESTS):
	@mkdir --parents $$(dirname $@)
	$(CXX) -pthread -Wl,-rpath-link=$(QT_LIB_DIR) \
        -L$(MPICH_LIB_DIR) -L/usr/lib -L$(QT_LIB_DIR) \
        -o $@ $^ \
        -lQt5Core -lmpi -lpthread

$(MOTHERSHIP_EXECUTABLE):
	@$(shell mkdir --parents $(EXECUTABLE_DIR))
	$(CXX) -pthread -Wl,-rpath-link=$(QT_LIB_DIR) \
        -L$(MPICH_LIB_DIR) -L/usr/lib -L$(QT_LIB_DIR) -L$(JTAG_LIB_DIR) \
        -L$(SUPERVISOR_LIB_DIR) \
        -o $@ $^ \
        -lQt5Core -lmpi -lpthread \
        -ljtag_atlantic -ljtag_client -lSupervisor

# Object generation. Note: The move command installs the assembled dependency
# file (see *Note 1*). The touch updates the timestamp on the target (to avoid
# pointless rebuilds), and is predicated on the move command succeeding.
define build-object
    @mkdir --parents $$(dirname $@)
    $(CXX) $(CXXFLAGS) -c -o $@ $<
    @mv --force $(DEPENDENCY_DIR)/$*.temp.d $(DEPENDENCY_DIR)/$*.d
    @touch $@
endef

$(OBJECT_DIR)/%.o: $(ROOT_DIR)/%.cpp $(DEPENDENCY_DIR)/%.d
	$(build-object)

$(HOSTLINK_OBJECT_DIR)/%.o: $(TINSEL_HOSTLINK_DIR)/%.cpp $(DEPENDENCY_DIR)/%.d
	$(build-object)

# Supervisor generation.
$(STUB_SUPERVISOR): $(STUB_SUPERVISOR_OBJECTS)
	@mkdir --parents $$(dirname $@)
	$(CXX) -shared -L$(MPICH_LIB_DIR) -Wl,-soname,$$(basename $@) -o $@ $<

# Miscellaneous files to be copied to the execution directory in order for
# various components of the Orchestrator to function.
$(LOGSERVER_MESSAGE_FILE): $(LOGSERVER_MESSAGE_FILE_ORIGIN)
$(JTAG_ATLANTIC_LOCAL): $(JTAG_LIB_DIR)/libjtag_atlantic.so
$(JTAG_CLIENT_LOCAL): $(JTAG_LIB_DIR)/libjtag_client.so
$(LOGSERVER_MESSAGE_FILE) $(JTAG_ATLANTIC_LOCAL) $(JTAG_CLIENT_LOCAL):
	@mkdir --parents $$(dirname $@)
	cp "$<" "$@"

# Building the orchestrate script, used to setup the runtime environment for
# the Orchestrator, and to start it. The build process is largely
# templating. This build target uses a target-specific variable to determine
# the temporary path for building the orchestrate script.
#
# This logic assumes Bash 4 for the SUBSTITUTIONS hashtable, but that's not too
# unrealistic in this day and age.
$(ORCHESTRATE_SCRIPT): ORCHESTRATE_TMP:=$(shell mktemp --directory)/$(ORCHESTRATE_BASENAME)
$(ORCHESTRATE_SCRIPT): $(ORCHESTRATE_TEMPLATE)
	@# Create staging file space for subtitutions.
	cp "$<" "$(ORCHESTRATE_TMP)"

	@# Create hashtable and perform substitutions, all in the same shell.
	declare -A SUBSTITUTIONS=(\
        ["MPICH_DIR"]="$(realpath $(MPICH_DIR))" \
		["MPICH_LIB_DIR"]="$(realpath $(MPICH_LIB_DIR))" \
		["QT_LIB_DIR"]="$(realpath $(QT_LIB_DIR))" \
		["JTAG_LIB_DIR"]="$(realpath $(JTAG_LIB_DIR))" \
		["GCC_LIB_DIR"]="$(realpath $(GCC_LIB_DIR))" \
		["CR_LIB_DIR"]="$(realpath $(CR_LIB_DIR))" \
		["SUPERVISOR_LIB_DIR"]="$$(realpath -m $(SUPERVISOR_LIB_DIR))" \
		["EXECUTABLE_DIR"]="$$(realpath -m $(EXECUTABLE_DIR))");\
	for KEY in "$${!SUBSTITUTIONS[@]}"; do \
		sed -i "s|{{ $$KEY }}|$${SUBSTITUTIONS[$$KEY]}|" "$(ORCHESTRATE_TMP)";\
	done

	@# Deploy
	cp "$(ORCHESTRATE_TMP)" "$@"

	@# Remove staging file and directory, carefully.
	rm "$(ORCHESTRATE_TMP)"
	rmdir "$(dir $(ORCHESTRATE_TMP))"

# Dependency files have no build instructions (Make must not fall over if the
# dependency file is not there), because they are created from the compilation
# process. However, the directory structure for dependency files must be
# created.
$(DEPENDENCY_DIR)/%.d:
	@mkdir --parents $$(dirname $@)

clean:
	rm --force --recursive $(OBJECT_DIR) $(DEPENDENCY_DIR) $(EXECUTABLE_DIR) \
	$(SUPERVISOR_LIB_DIR) $(HOSTLINK_OBJECT_DIR) $(ORCHESTRATE_SCRIPT) \
    $(TEST_EXECUTABLE_DIR)

# Dependency files must not be removed by Make (unless explicitly cleaned).
.PRECIOUS: $(DEPENDENCY_DIR)/%.d

# Non-builtin targets that do not explicitly represent files that are created.
.PHONY: all tests clean orchestrate root dummy logserver rtcl injector \
mothership stub_supervisor
