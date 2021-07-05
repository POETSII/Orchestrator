#!/bin/bash

echo "TAP version 13"

HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ORCHROOT="$(realpath $HERE/../..)"

echo $HERE
echo $ORCHROOT
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

############################################
## Pure syntax

LOAD_PLOG_FILTER='s/^.*for the command '\''load \/app = [^'\'']+'\'' will be written to '\''([^'\'']+)'\''.+$/\1/p'

function TODO_parse {
    F="$1"
    RR=$(realpath --relative-to="$ORCHROOT" "$F")
    echo "not ok $TN - # TODO Parse $RR, $2"
    TN=$((TN+1))
    return 0
}

function test_parse_success {
    F="$1"
    if [[ $VERBOSE -eq 1 ]] ; then
        OUTPUT=$($HERE/parse_app.exp $F )
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
        $HERE/parse_app.exp $F > /dev/null
        RES=$?
    fi


    RR=$(realpath --relative-to="$ORCHROOT" "$F")

    if [[ $RES -eq 0 ]] ; then
        echo "ok $TN - Parse $RR"
        TN=$((TN+1))
        return 0
    else
        echo "not ok $TN - Parse $RR"
        TN=$((TN+1))
        if [[ ${ABORT_ON_ERROR} -eq 1 ]] ; then
            exit 1
        fi
        return 1
    fi
}

function test_parse_failure {
    F="$1"
    $HERE/parse_app.exp $F > /dev/null
    RES=$?

    RR=$(realpath --relative-to="$ORCHROOT" "$F")

    if [[ $RES -ne 0 ]] ; then
        echo "ok $TN - Should fail to parse $RR"
        TN=$((TN+1))
        return 0
    else
        echo "not ok $TN - Should fail to parse $RR"
        TN=$((TN+1))
        return 1
    fi
}

RC=0
for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/apps/*.xml ; do
    if [[ "$i" == *betweeness_centrality_16_16_20_20_v4.xml ]] ; then
        TODO_parse $i "orchestrator needs indexed sends."
    else
        test_parse_success $i
    fi
    if [ $? -ne 0 ]; then RC=1; fi
done
for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/tests/valid/*/*.xml ; do
    test_parse_success $i
    if [ $? -ne 0 ]; then RC=1; fi
done

for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/tests/invalid/L0-syntax/*.xml ; do
    test_parse_failure $i
    if [ $? -ne 0 ]; then RC=1; fi
done

for i in $ORCHROOT/Tests/ReferenceXML/v4/PEP20/tests/invalid/L0-syntax/*.xml ; do
    test_parse_failure $i
    if [ $? -ne 0 ]; then RC=1; fi
done
exit $RC
