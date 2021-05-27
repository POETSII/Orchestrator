#!/bin/bash

echo "TAP version 13"

HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ORCHROOT="$(realpath $HERE/../..)"

VERBOSE=1
ABORT_ON_ERROR=1

TN=0

############################################
## Pure syntax

LOAD_PLOG_FILTER='s/^.*for the command '\''load \/app = [^'\'']+'\'' will be written to '\''([^'\'']+)'\''.+$/\1/p'

function test_parse_success {
    F="$1"
    if [[ $VERBOSE -eq 1 ]] ; then
        OUTPUT=$($ORCHROOT/parse_app.exp $F )
        RES=$?

        if [[ $RES -ne 0 ]] ; then
            PLOG_LINE=$(echo $OUTPUT |  tr -dc '[[:print:]]' | sed -r -n "${LOAD_PLOG_FILTER}" )
            echo ""
            echo "#Validation of $F Failed."
            echo "# plog is in ${PLOG_LINE}"
            echo ""
            while read l ; do
                echo "# > $l"
            done < $ORCHROOT/bin/${PLOG_LINE}
        fi
    else
        $ORCHROOT/parse_app.exp $F > /dev/null
    fi
    

    RR=$(realpath --relative-to="$ORCHROOT" "$F")

    if [[ $RES -eq 0 ]] ; then
        echo "ok $TN - Parse $RR"
    else
        echo "not ok $TN - Parse $RR"
        if [[ ${ABORT_ON_ERROR} -eq 1 ]] ; then
            exit 1;
        fi
    fi

    TN=$((TN+1))
}

function test_parse_failure {
    F="$1"
    $ORCHROOT/parse_app.exp $F > /dev/null
    RES=$?

    RR=$(realpath --relative-to="$ORCHROOT" "$F")

    if [[ $RES -ne 0 ]] ; then
        echo "ok $TN - Should fail to parse $RR"
    else
        echo "not ok $TN - Should fail to parse $RR"
    fi

    TN=$((TN+1))
}

for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/apps/*.xml ; do
    test_parse_success $i
done 
for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/tests/valid/*/*.xml ; do
    test_parse_success $i
done 

for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/tests/invalid/L0-syntax/*.xml ; do
    test_parse_failure $i
done 

for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/tests/invalid/L0-syntax/*.xml ; do
    test_parse_failure $i
done 
