#!/bin/bash

echo "TAP version 13"

HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ORCHROOT="$(realpath $HERE/../..)"

VERBOSE=0
ABORT_ON_ERROR=0
RUN_TIMEOUT=60

while [[ $# -gt 0 ]] ; do
    case $1 in
        --verbose)
        VERBOSE=1
        shift
        ;;
        --abort-on-error)
        ABORT_ON_ERROR=1
        shift
        ;;
        *)    # unknown option
        >&2 "Didnt understand option $1"
        exit 1
        ;;
    esac
done

TN=0

function TODO_run {
    F="$1"
    RR=$(realpath --relative-to="$ORCHROOT" "$F")
    echo "not ok $TN - # TODO Run $RR, $2"
    TN=$((TN+1))
}

############################################
## Compilation

PLOG_FILTER='s/^.*will be written to '\''([^'\'']+[.]plog)'\''.+$/\1/p'

function test_run_success {
    local TRIES
    local RES
    local OUTPUT
    local TIMEOUT
    TRIES=0
    TIMEOUT=1

    while [[ $TRIES -lt 5 ]] ; do

        F="$1"
        OUTPUT=$($HERE/run_app_standard_outputs.exp $F ${RUN_TIMEOUT} )
        RES=$?

        if [[ $RES -ne 0 ]] ; then
            if [[ "$OUTPUT" == *"Can't connect to "* ]] ; then
                echo "#  HostLink locked. Sleeping $TIMEOUT seconds"
                sleep ${TIMEOUT}
                TIMEOUT=$((TIMEOUT+2))
                TRIES=$((TRIES+1))
                continue
            else
                break
            fi
        else
            break
        fi
    done
        
    STATS=$(echo "$OUTPUT" | sed -n '/STATS_fba956f3/p' )
    echo "# $STATS"

    RR=$(realpath --relative-to="$ORCHROOT" "$F")

    if [[ $RES -eq 0 ]] ; then
        echo "ok $TN - Run $RR"
    else
        PLOG_LINE=$(echo $OUTPUT |  tr -dc '[[:print:]]' | sed -r -n "${PLOG_FILTER}" )
        
        if [[ $VERBOSE -ne 0 ]] ; then
            echo ""
            echo "#Execution of $F Failed."
            echo "$OUTPUT" | while read l ; do 
                echo "# > $l"
            done 
            #echo "# plog is in ${PLOG_LINE}"
            #echo ""
        # while read l ; do
            #    echo "# > $l"
            #done < $ORCHROOT/bin/${PLOG_LINE}
        fi

        echo "not ok $TN - Run $RR"
        if [[ $ABORT_ON_ERROR -eq 1 ]] ; then
            exit 1
        fi
    fi

    TN=$((TN+1))
}

test_run_success "${HERE}/v4/PEP20/tests/valid/L4-run-time/single-device-print-hello-in-init.xml"

test_run_success "${HERE}/v4/PEP20/tests/valid/L4-run-time/single-device-loopback-once--succeed-recv.xml"
test_run_success "${HERE}/v4/PEP20/tests/valid/L4-run-time/single-device-loopback-once--succeed-send.xml"
test_run_success "${HERE}/v4/PEP20/tests/valid/L4-run-time/single-device-loopback-once-no-edge--succeed-send.xml"

test_run_success "${HERE}/v4/PEP20/apps/amg_poisson_8_8_v4.xml"
test_run_success "${HERE}/v4/PEP20/apps/gals_heat_8x8_v4.xml"
test_run_success "${HERE}/v4/PEP20/apps/relaxation_heat_16x16_v4.xml"
test_run_success "${HERE}/v4/PEP20/apps/storm_16_4_8_v4.xml"