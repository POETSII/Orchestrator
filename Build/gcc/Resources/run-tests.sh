#!/bin/bash

# Runs all tests in this directory. If they all pass, runs Valgrind against
# them, and writes some logging output. Exit codes:
#
#  0: The tests passed, both with and without Valgrind watching over them.
#  1: The tests failed.
#  2: The tests passed, but failed when Valgrind was watching.
#  3: The tests passed, but Valgrind could not be found.

# Verify all tests pass.
LOGFILE="tests.log"
FAILURE=0
echo "Running all tests..."
printf "Tests running at $(date)...\n\n" > $LOGFILE
for TEST in *.test; do
    echo "${TEST}:" >> $LOGFILE
    ./${TEST} >> $LOGFILE 2>&1
    FAILURE=$(($FAILURE + $?))
done

# Exit if any failed.
if [ $FAILURE -ne 0 ]; then
    echo "One or more tests failed. See tests.log for more details. Exiting."
    exit 1
else
    echo "All tests passed."
fi

# Check for Valgrind's existence.
command -v valgrind > /dev/null
if [ $? -ne 0 ]; then
    echo "Could not find Valgrind, so can't run memory checks against the "
    "test. Exiting."
    exit 3
else
    echo "Running memory checks on all tests..."
fi

# NB: FAILURE is still zero before we enter this loop.
FAILURE_LOGS=" "
for TEST in *.test; do
    MEMCHECK_LOGFILE="${TEST%%.*}_memcheck.log"
    printf "Memchecking at $(date):\n\n" > "$MEMCHECK_LOGFILE"
    valgrind --error-exitcode=1 --track-origins=yes --leak-check=full \
             ./${TEST} >> "$MEMCHECK_LOGFILE" 2>&1
    EXIT_CODE=$?
    if [ $EXIT_CODE -ne 0 ]; then
        FAILED_TESTS="$FAILED_TESTS$TEST "
        FAILURE=$(($FAILURE + $EXIT_CODE))
    fi
done

# Exit conditions
if [ $FAILURE -ne 0 ]; then
    echo "One or more tests failed their memory check (specifically,"\
         "${FAILED_TESTS}all failed)."
    exit 2
else
    echo "All memory checks passed."
    exit 0
fi
