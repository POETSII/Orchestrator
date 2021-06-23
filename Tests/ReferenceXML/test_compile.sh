#!/bin/bash

echo "TAP version 13"

HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ORCHROOT="$(realpath $HERE/../..)"

VERBOSE=0
ABORT_ON_ERROR=0

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

function TODO_compile {
    F="$1"
    RR=$(realpath --relative-to="$ORCHROOT" "$F")
    echo "not ok $TN - # TODO Compile $RR, $2"
    TN=$((TN+1))
}

############################################
## Compilation

PLOG_FILTER='s/^.*will be written to '\''([^'\'']+[.]plog)'\''.+$/\1/p'

function test_compile_success {
    F="$1"
    if [[ $VERBOSE -eq 1 ]] ; then
        OUTPUT=$($HERE/compile_app.exp $F )
        RES=$?

        if [[ $RES -ne 0 ]] ; then
            PLOG_LINE=$(echo $OUTPUT |  tr -dc '[[:print:]]' | sed -r -n "${PLOG_FILTER}" )
            echo ""
            echo "#Validation of $F Failed."
            echo "$OUTPUT" | while read l ; do 
                echo "# > $l"
            done 
            echo "# plog is in ${PLOG_LINE}"
            echo ""
            while read l ; do
                echo "# > $l"
            done < $ORCHROOT/bin/${PLOG_LINE}
        fi
    else
        $HERE/compile_app.exp $F > /dev/null
        RES=$?
    fi
    RR=$(realpath --relative-to="$ORCHROOT" "$F")

    if [[ $RES -eq 0 ]] ; then
        echo "ok $TN - Compile $RR"
    else
        echo "not ok $TN - Compile $RR"
        if [[ $ABORT_ON_ERROR -eq 1 ]] ; then
            exit 1
        fi
    fi

    TN=$((TN+1))
}

function test_compile_failure {
    F="$1"
    $HERE/compile_app.exp $F > /dev/null
    RES=$?

    RR=$(realpath --relative-to="$ORCHROOT" "$F")

    if [[ $RES -ne 0 ]] ; then
        echo "ok $TN - Should fail to compile $RR"
    else
        echo "not ok $TN - Should fail to compile $RR"
    fi

    TN=$((TN+1))
}

for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/apps/*.xml ; do
    if [[ "$i" == */apsp_vec_barrier_150_10.xml ]] ; then
        TODO_compile $i "Bug #232 needs to be fixed."
    elif [[ "$i" == */betweeness_centrality_16_16_20_20_v4.xml ]] ; then
        TODO_compile $i "orchestrator needs indexed sends."
    elif [[ "$i" == */example_device_idle.xml ]] ; then
        TODO_compile $i "orchestrator needs requestIdle."
    else 
        test_compile_success $i
    fi
done 
for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/tests/valid/*/*.xml ; do
    if [[ "$i" == */L4-run-time/external-*.xml ]] ; then
        TODO_compile $i "No support for externals."
    else
        test_compile_success $i
    fi
done 

for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/tests/invalid/L3-compilation/*.xml ; do
    test_compile_failure $i
done 
