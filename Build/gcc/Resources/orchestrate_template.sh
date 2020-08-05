#!/bin/bash

# This script sets up the (runtime) environment for the Orchestrator to operate
# in, tweaks input arguments, then starts the Orchestrator launcher.
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
export LD_LIBRARY_PATH="$CR_LIB_DIR":"$MPI_LIB_DIR":"$LD_LIBRARY_PATH":./

# Transform input arguments:
#
# Grab the file and batch (/f, /b) command line arguments, put them in
# suppressed quotes for the Cli class in the launcher, and make them relative
# to this directory if it is itself a relative path (because the executable is
# run from the executable directory).
#
# Also perform the "=" style split for Windows/GNU. Windows options are of the
# form:
#   /d
#   /f = FILE
# Whereas GNU options are of the form:
#   -d
#   -f FILE
GNU_HELP=0
THIS_ARG_MODE=""
while [ $# -gt 0 ]; do
    VALUE=""

    # Get switch and value, if appropriate.
    case "$1" in
        /*) # Windows
            THIS_ARG_MODE="Windows"  # i.e. not GNU.
            if [[ "$1" =~ /[a-z]=* ]]; then  # e.g. "/f=FILE"
                SWITCH="${1:0:2}"
                VALUE="${1:3}"
                shift;
            elif [ "$2" == "=" ]; then  # e.g. "/f = FILE"
                SWITCH="$1"
                VALUE="$3"
                shift; shift; shift
            else  # e.g. "/d"
                SWITCH="$1"
                shift;
            fi
            ;;

        -*) # GNU
            THIS_ARG_MODE="GNU"
            SWITCH="/${1:1}"
            case "$2" in
                -*) # e.g. "-d"
                    shift
                    ;;
                *) # e.g. "-f FILE", or "-d" with no further arguments (VALUE
                   # will remain empty).
                    VALUE="$2"
                    shift; shift;
                    ;;
            esac
    esac

    # Absolute/relative control flow split for arguments that accept a path.
    if [ "$SWITCH" == "/f" ] || [ "$SWITCH" == "/b" ]; then
        case "$VALUE" in
            /*) # Absolute
                ARGS="$ARGS $SWITCH = \"$VALUE\""
                ;;
            *)  # Relative, or user has not specified one
                ARGS="$ARGS $SWITCH = \"$PWD/$VALUE\""
                ;;
        esac

    # Offer more extensive help for GNU people and the naive.
    elif [ "$SWITCH" == "/h" ] || [ "$SWITCH" == "/-help" ]; then
        ARGS="$ARGS /h"
        [ "$THIS_ARG_MODE" == "GNU" ] && GNU_HELP=1

    # Otherwise, concatenate the command normally.
    else
        if [ -z "$VALUE" ]; then
            ARGS="$ARGS $SWITCH"
        else
            ARGS="$ARGS $SWITCH = $VALUE"
        fi
    fi
done

# Run the launcher from the build directory.
pushd "{{ EXECUTABLE_DIR }}" > /dev/null
./orchestrate /p = "\"$INTERNAL_LIB_PATH\"" $ARGS
if [ $GNU_HELP -eq 1 ]; then
    printf "\nNote that there is limited support for short-form GNU-style switches (e.g. '-h -f FILE').\n"
fi
popd > /dev/null
