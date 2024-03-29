# This Makefile builds the various components of the Orchestrator using gcc. It
# assumes you are running GNU/Linux, and have certain commonly-available GNU
# utilities installed (you'll know if you don't).

# Bash is used over sh for hashtables when building orchestrate (and probably
# other reasons).
SHELL = /bin/bash

# Smart directory creation.
MKDIR = mkdir --parents

# Software dependencies (e.g. MPI) are largely hardcoded, because I'm too short
# on time to write an autoconf wrapper or some other dependency management
# system.
#
# If you are a user, you should modify the Makefile.dependencies file in this
# directory to correctly path to the dependencies required to build the
# components of the Orchestrator. You should not need to modify this particular
# file.
#
# If you are a developer, you may also need to modify the following file to
# incorporate your additional sources:
include Makefile.dependencies

# Where sources live.
GENERICS_DIR := $(ROOT_DIR)/Generics
SOURCE_DIR := $(ROOT_DIR)/Source
TESTS_DIR := $(ROOT_DIR)/Tests

# Objects constructed from Orchestrator sources are placed in here. Objects are
# placed in a directory structure that mimics the source directory structure.
OBJECT_DIR := ./Objects

# Executable binaries are built to here (directory is created as executable
# binaries are built):
EXECUTABLE_DIR := $(ROOT_DIR)/bin

# Test binaries are built to here:
TEST_EXECUTABLE_DIR := $(ROOT_DIR)/test_bin

# Composer links and binaries are built to here:
COMPOSER_ENV_DIR := $(ROOT_DIR)/Output/Composer

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
$(shell $(MKDIR) $(DEPENDENCY_DIR))

# Include generated dependency files, because now is as good a time as any. We
# use find to do this, which is not the most portable way. If you can think of
# something better, let me know!
include $(shell find $(DEPENDENCY_DIR) -name "*.d" ! -name "*.temp.d")

# Where the compiler can look for header files when building Orchestrator
# objects:
INCLUDE_FLAGS = -I$(GENERICS_DIR) \
                -I$(SOURCE_DIR)/Common \
                -I$(SOURCE_DIR)/OrchBase \
                -I$(SOURCE_DIR)/OrchBase/AppStructures \
                -I$(SOURCE_DIR)/OrchBase/Handlers \
                -I$(SOURCE_DIR)/OrchBase/HardwareFileReader \
                -I$(SOURCE_DIR)/OrchBase/HardwareConfigurationDeployment \
                -I$(SOURCE_DIR)/OrchBase/Placement \
                -I$(SOURCE_DIR)/OrchBase/Placement/Algorithms \
                -I$(SOURCE_DIR)/OrchBase/Placement/Constraints \
                -I$(SOURCE_DIR)/OrchBase/Placement/Exceptions \
                -I$(SOURCE_DIR)/OrchBase/HardwareModel \
                -I$(SOURCE_DIR)/Launcher \
                -I$(SOURCE_DIR)/OrchBase/XMLProcessing \
                -I$(SOURCE_DIR)/Injector \
                -I$(SOURCE_DIR)/Parser \
                -I$(SOURCE_DIR)/Root \
                -I$(SOURCE_DIR)/NameServer \
                -I$(SOURCE_DIR)/NameServer/AddressBook \
                -I$(SOURCE_DIR)/Softswitch/inc \
                -I$(SOURCE_DIR)/Supervisor \
                -I$(MPICH_INC_DIR) \
                -I$(TINSEL_INC_DIR) \
                -I$(TINSEL_HOSTLINK_DIR)

# All flags to be used in compilation.
CXXFLAGS = $(DEPENDENCY_FLAGS) $(INCLUDE_FLAGS) \
           -std=c++98 -Wall -fPIC -pthread -pedantic -O3 -DTRIVIAL_LOG_HANDLER

# Sanitize flags (used only during debugging)
SANITIZEFLAGS = -fsanitize=address -fsanitize=undefined \
                -fno-sanitize=alignment  # Msg_p uses these deliberately

# Debugging!
DEBUGFLAGS = -g3 -O0 $(SANITIZEFLAGS)

# Flags on top of default linking flags used in rules.
POSTLINKFLAGS =

# Debug target logic.
debug: CXXFLAGS += $(DEBUGFLAGS) -DORCHESTRATOR_DEBUG=1
debug: POSTLINKFLAGS += $(SANITIZEFLAGS)
debug: all

# Handy aliases for building executables, scripts, and libraries. Order doesn't
# matter.
ORCHESTRATE_BASENAME = orchestrate.sh
ORCHESTRATE_SCRIPT = $(ROOT_DIR)/$(ORCHESTRATE_BASENAME)

LAUNCHER_EXECUTABLE = $(EXECUTABLE_DIR)/orchestrate
ROOT_EXECUTABLE = $(EXECUTABLE_DIR)/root
DUMMY_EXECUTABLE = $(EXECUTABLE_DIR)/dummy
LOGSERVER_EXECUTABLE = $(EXECUTABLE_DIR)/logserver
RTCL_EXECUTABLE = $(EXECUTABLE_DIR)/rtcl
INJECTOR_EXECUTABLE = $(EXECUTABLE_DIR)/injector
MOTHERSHIP_EXECUTABLE = $(EXECUTABLE_DIR)/mothership

all: orchestrate launcher root dummy logserver rtcl injector mothership \
     composer_environment
launcher: $(LAUNCHER_EXECUTABLE)
orchestrate: $(ORCHESTRATE_SCRIPT)
root: $(ROOT_EXECUTABLE)
dummy: $(DUMMY_EXECUTABLE)
logserver: $(LOGSERVER_EXECUTABLE)
rtcl: $(RTCL_EXECUTABLE)
injector: $(INJECTOR_EXECUTABLE)
mothership: $(MOTHERSHIP_EXECUTABLE)

# Prerequisites (objects) for building executables are defined here:
include Makefile.executable_prerequisites

# Prerequisites for building tests are defined here:
include Makefile.test_prerequisites
TEST_SCRIPT_BASENAME = run-tests.sh
TEST_SCRIPT = $(TEST_EXECUTABLE_DIR)/$(TEST_SCRIPT_BASENAME)
tests: CXXFLAGS += $(DEBUGFLAGS)
tests: POSTLINKFLAGS += $(SANITIZEFLAGS)
tests: $(ALL_TESTS) $(TEST_SCRIPT)

# Linking targets (to generate executables). See
# Makefile.executable_prerequisites for the prerequisites of these executables
# (obviously...)
$(LAUNCHER_EXECUTABLE) $(ROOT_EXECUTABLE) $(DUMMY_EXECUTABLE) \
$(LOGSERVER_EXECUTABLE) $(RTCL_EXECUTABLE) $(INJECTOR_EXECUTABLE):
	@$(shell $(MKDIR) $(EXECUTABLE_DIR))
	$(CXX) -pthread -L$(MPICH_LIB_DIR) -L/usr/lib \
        -o $@ $^ \
        -lmpi -lpthread \
        $(POSTLINKFLAGS)

$(ALL_TESTS):
	@$(shell $(MKDIR) $(TEST_EXECUTABLE_DIR))
	$(CXX) -pthread -L$(MPICH_LIB_DIR) -L/usr/lib \
        -o $@ $^ \
        -lmpi -lpthread \
        $(POSTLINKFLAGS)

$(MOTHERSHIP_EXECUTABLE):
	@$(shell $(MKDIR) $(EXECUTABLE_DIR))
	$(CXX) -pthread -Wl,-export-dynamic -L$(MPICH_LIB_DIR) -L/usr/lib \
        -o $@ $^ $(TINSEL_HOSTLINK_DIR)/*.o \
        -lmpi -lpthread -ldl \
        $(POSTLINKFLAGS)

# Object generation. Note: The move command installs the assembled dependency
# file (see *Note 1*). The touch updates the timestamp on the target (to avoid
# pointless rebuilds), and is predicated on the move command succeeding.
define build-object
    @$(MKDIR) $$(dirname $@)
    $(CXX) $(CXXFLAGS) -c -o $@ $<
    @mv --force $(DEPENDENCY_DIR)/$*.temp.d $(DEPENDENCY_DIR)/$*.d
    @touch $@
endef

$(OBJECT_DIR)/%.o: $(ROOT_DIR)/%.cpp $(DEPENDENCY_DIR)/%.d
	$(build-object)

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
		["GCC_LIB_DIR"]="$(realpath $(GCC_LIB_DIR))" \
		["CR_LIB_DIR"]="$(realpath $(CR_LIB_DIR))" \
		["RISCV_DIR"]="$(realpath $(RISCV_DIR))" \
		["RISCV_BIN_DIR"]="$(realpath $(RISCV_BIN_DIR))" \
		["EXECUTABLE_DIR"]="$$(realpath -m $(EXECUTABLE_DIR))");\
	for KEY in "$${!SUBSTITUTIONS[@]}"; do \
		sed -i "s|{{ $$KEY }}|$${SUBSTITUTIONS[$$KEY]}|" "$(ORCHESTRATE_TMP)";\
	done

	@# Deploy
	cp "$(ORCHESTRATE_TMP)" "$@"

	@# Remove staging file and directory, carefully.
	rm "$(ORCHESTRATE_TMP)"
	rmdir "$(dir $(ORCHESTRATE_TMP))"

# Build test script in a similar manner to the above.
$(TEST_SCRIPT): TEST_SCRIPT_TMP:=$(shell mktemp --directory)/$(TEST_SCRIPT_BASENAME)
$(TEST_SCRIPT): $(TEST_SCRIPT_RESOURCE)
	@$(shell $(MKDIR) $(TEST_EXECUTABLE_DIR))

	@# Create staging file space for subtitutions.
	cp "$<" "$(TEST_SCRIPT_TMP)"

	@# Create hashtable and perform substitutions, all in the same shell.
	declare -A SUBSTITUTIONS=(\
		["MPICH_LIB_DIR"]="$(realpath $(MPICH_LIB_DIR))");\
	for KEY in "$${!SUBSTITUTIONS[@]}"; do \
		sed -i "s|{{ $$KEY }}|$${SUBSTITUTIONS[$$KEY]}|" "$(TEST_SCRIPT_TMP)";\
	done

	@# Deploy
	cp "$(TEST_SCRIPT_TMP)" "$@"

	@# Remove staging file and directory, carefully.
	rm "$(TEST_SCRIPT_TMP)"
	rmdir "$(dir $(TEST_SCRIPT_TMP))"

# Building the environment for XML applications to be composed in the
# Orchestrator. Made of a bunch of symlinks.
COMPOSER_ENV_DEPENDENCIES := \
	$(COMPOSER_ENV_DIR)/Orchestrator/Generics \
	$(COMPOSER_ENV_DIR)/Orchestrator/Source/Common \
	$(COMPOSER_ENV_DIR)/Softswitch \
	$(COMPOSER_ENV_DIR)/Tinsel \
	$(COMPOSER_ENV_DIR)/Makefile \
	$(COMPOSER_ENV_DIR)/Supervisor.cpp \
	$(COMPOSER_ENV_DIR)/Supervisor.h \
	$(COMPOSER_ENV_DIR)/SupervisorApi.h \
	$(COMPOSER_ENV_DIR)/SupervisorApiEntrypoints.h

composer_environment: $(COMPOSER_ENV_DEPENDENCIES)

$(COMPOSER_ENV_DIR)/Orchestrator/Generics:
	$(MKDIR) "$(dir $@)"
	ln --force --symbolic "$(abspath $(GENERICS_DIR))" "$@"

$(COMPOSER_ENV_DIR)/Orchestrator/Source/Common:
	$(MKDIR) "$(dir $@)"
	ln --force --symbolic "$(abspath $(SOURCE_DIR)/Common)" "$@"

$(COMPOSER_ENV_DIR)/Softswitch:
	$(MKDIR) "$(dir $@)"
	ln --force --symbolic "$(abspath $(SOURCE_DIR)/Softswitch)" "$@"

$(COMPOSER_ENV_DIR)/Tinsel:
	$(MKDIR) "$(dir $@)"
	ln --force --symbolic "$(abspath $(TINSEL_DIR))" "$@"

$(COMPOSER_ENV_DIR)/Makefile:
	$(MKDIR) "$(dir $@)"
	ln --force --symbolic "$(abspath $(SOURCE_DIR)/Softswitch/Makefile)" "$@"

$(COMPOSER_ENV_DIR)/SupervisorApi.h:
	ln --force --symbolic \
	"$(abspath $(SOURCE_DIR)/Supervisor/SupervisorApi$(suffix $@))" "$@"

$(COMPOSER_ENV_DIR)/SupervisorApiEntrypoints.h:
	ln --force --symbolic \
	"$(abspath $(SOURCE_DIR)/Supervisor/SupervisorApiEntrypoints$(suffix $@))" "$@"

$(COMPOSER_ENV_DIR)/Supervisor.cpp $(COMPOSER_ENV_DIR)/Supervisor.h:
	$(MKDIR) "$(dir $@)"
	ln --force --symbolic \
	"$(abspath $(SOURCE_DIR)/Supervisor/Supervisor$(suffix $@))" "$@"

# Dependency files have no build instructions (Make must not fall over if the
# dependency file is not there), because they are created from the compilation
# process. However, the directory structure for dependency files must be
# created.
$(DEPENDENCY_DIR)/%.d:
	@$(MKDIR) $$(dirname $@)

clean:
	rm --force --recursive $(OBJECT_DIR) $(DEPENDENCY_DIR) $(EXECUTABLE_DIR) \
	$(ORCHESTRATE_SCRIPT) $(TEST_EXECUTABLE_DIR) $(COMPOSER_ENV_DEPENDENCIES)

# Dependency files must not be removed by Make (unless explicitly cleaned).
.PRECIOUS: $(DEPENDENCY_DIR)/%.d

# Non-builtin targets that do not explicitly represent files that are created.
.PHONY: all clean debug tests orchestrate root dummy logserver rtcl injector \
    mothership

# The "all" target should be the default.
.DEFAULT_GOAL := all
