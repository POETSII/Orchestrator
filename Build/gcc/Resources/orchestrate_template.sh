#!/bin/bash

# This script sets up the (runtime) environment for the Orchestrator to operate
# in, then starts the Orchestrator.
#
# Note that this script will not work correctly if you move any of the
# dependencies, or the build artefacts. In that case, you're on your own (or
# you can just rebuild).
#
# This script is generated by the build process using a templating
# mechanism. The double-curly braces indicate where the build process will
# substitute in variable names to create the executable. There should be no
# double-curly braces in the output version of this file.

# Determine whether or not we are running on a POETS box, trivially.
QUARTUS_SETUP_SCRIPT="/local/ecad/setup-quartus17v0.bash"
if [ -f "$QUARTUS_SETUP_SCRIPT" ]; then
    ON_POETS_BOX=1  # True
else
    ON_POETS_BOX=0
fi

# Setup
export RISCV_DIR=
export PATH="{{ MPICH_DIR }}/bin:$RISCV_DIR/bin:$PATH"
export TRIVIAL_LOG_HANDLER=1

if [ $ON_POETS_BOX -eq 1 ]; then
    export LM_LICENSE_FILE=:27012@localhost:27001@localhost
    source "$QUARTUS_SETUP_SCRIPT"
fi

# Paths for dynamically-linked libraries.
MPI_LIB_DIR="{{ MPICH_LIB_DIR }}"
QT_LIB_DIR="{{ QT_LIB_DIR }}"
JTAG_LIB_DIR="{{ JTAG_LIB_DIR }}"
SUPERVISOR_LIB_DIR="{{ SUPERVISOR_LIB_DIR }}"
GCC_LIB_DIR="{{ GCC_LIB_DIR }}"
INTERNAL_LIB_PATH=./:"$QT_LIB_DIR":"$MPI_LIB_DIR":\
"$GCC_LIB_DIR":"$SUPERVISOR_LIB_DIR":"$JTAG_LIB_DIR"

# Define general MPI execution command.
COMMAND='mpiexec.hydra -genv LD_LIBRARY_PATH "$INTERNAL_LIB_PATH" \
    -n 1 ./root : \
    -n 1 ./logserver : \
    -n 1 ./rtcl :'

# Running the Orchestrator from the build directory. Only start a mothership if
# we are running on a POETS box.
pushd "{{ EXECUTABLE_DIR }}" > /dev/null
if [ $ON_POETS_BOX -eq 1 ]; then
    echo "$COMMAND -n 1 ./mothership"
else
    echo "$COMMAND"
fi
popd > /dev/null