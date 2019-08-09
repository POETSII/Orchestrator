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
#
# Call with the -h argument for usage.

# Setup for building applications
export RISCV_PATH="{{ RISCV_DIR }}"
export MPICH_PATH="{{ MPICH_DIR }}"
export PATH="{{ MPICH_DIR }}/bin:{{ RISCV_BIN_DIR }}:$PATH"
export TRIVIAL_LOG_HANDLER=1

# Quartus
QUARTUS_SETUP_SCRIPT="/local/ecad/setup-quartus17v0.bash"
if [ -f "$QUARTUS_SETUP_SCRIPT" ]; then
    export LM_LICENSE_FILE=:27012@localhost:27001@localhost
    source "$QUARTUS_SETUP_SCRIPT"
fi

# Paths for dynamically-linked libraries.
MPI_LIB_DIR="{{ MPICH_LIB_DIR }}"
QT_LIB_DIR="{{ QT_LIB_DIR }}"
GCC_LIB_DIR="{{ GCC_LIB_DIR }}"
CR_LIB_DIR="{{ CR_LIB_DIR }}"
INTERNAL_LIB_PATH="$QT_LIB_DIR":"$MPI_LIB_DIR":"$GCC_LIB_DIR":

# Paths for dynamically-linked libraries required by MPI.
export LD_LIBRARY_PATH="$CR_LIB_DIR":"$LD_LIBRARY_PATH":./

# Grab the file (/f) command line argument, put it in suppressed quotes for the
# Cli class in the launcher, and make it relative to this directory if it is
# itself a relative path (because the executable is run from the executable
# directory).
#
# Also help if the user is naive.
while [ $# -gt 0 ]; do
    case "$1" in
        /f|-f)
            # Absolute/relative control flow split.
            case "$3" in
                /*) #Absolute
                    ARGS="$ARGS /f = \"$3\""
                    ;;
                *) # Relative
                    ARGS="$ARGS /f = \"$PWD/$3\""
                    ;;
            esac
            shift; shift; shift;;

        --help)
            ARGS="$ARGS /h"
            shift;;

        -*)
            # Convert to Windows-style, to a degree (mercy)
            ARGS="$ARGS /${1:1}"
            shift;;

        *)  # Benign argument
            ARGS="$ARGS $1"
            shift;;
    esac
done

# Run the launcher from the build directory.
pushd "{{ EXECUTABLE_DIR }}" > /dev/null
./orchestrate /p = "\"$INTERNAL_LIB_PATH\"" $ARGS
popd > /dev/null
