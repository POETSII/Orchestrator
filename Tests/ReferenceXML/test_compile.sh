#!/bin/bash

echo "TAP version 13"

HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ORCHROOT="$(realpath $HERE/../..)"

VERBOSE=1
ABORT_ON_ERROR=1

TN=0

############################################
## Compilation

function test_compile_success {
    F="$1"
    $ORCHROOT/compile_app.exp $F > /dev/null
    RES=$?

    RR=$(realpath --relative-to="$ORCHROOT" "$F")

    if [[ $RES -eq 0 ]] ; then
        echo "ok $TN - Compile $RR"
    else
        echo "not ok $TN - Compile $RR"
    fi

    TN=$((TN+1))
}

function test_compile_failure {
    F="$1"
    $ORCHROOT/compile_app.exp $F > /dev/null
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
    test_compile_success $i
done 
for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/tests/valid/*/*.xml ; do
    test_compile_success $i
done 

for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/tests/invalid/L0-syntax/*.xml ; do
    test_parse_failure $i
done 
for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/tests/invalid/L1-graph-topology/*.xml ; do
    test_parse_failure $i
done 
for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/tests/invalid/L2-graph-attributes/*.xml ; do
    test_parse_failure $i
done 
for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/tests/invalid/L3-compilation/*.xml ; do
    test_parse_failure $i
done 
