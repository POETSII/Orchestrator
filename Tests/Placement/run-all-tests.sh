#!/bin/bash
#
# Bash is needed (over shell), as we're using Bash arrays. Read the docstring.

# This hacky script of convenience, given some paths and other help, runs each
# of the placement tests using the Orchestrator, and complains when tests
# fail. The intention is that you do a bit of digging when tests fail, because
# I wrote this script under time pressure.
#
# This script clears all Orchestrator placement dumps in:
PLACEMENT_DUMP_DIR="./Output/Placement"
# so save those before running these tests if you care.
#
# The caller needs to:
#
#  - Run this script from the root directory of the Orchestrator repository,
#    and where the Orchestrator has been built without debug mode.
#
#  - Define PLACEMENT_TEST_DIR in the environment of this script, as the
#    absolute path to the placement tests directory. Don't add a trailing slash
#    if you know what's good for you *shakes fist*.
#
# Exit codes:
#
#  0: Everything was set up correctly, and all tests passed.
#  1: Everything was set up correctly, but one or more tests failed.
#  2: There was an error in setting up.

TEST_FILE_PREFIX="placement_test_"
TEST_MAX_TIME=10  # seconds per test
INTERESTING_DUMP_FILE_PREFIX="placement_gi_to_hardware_"
RESULT_FILE_NAME="result.csv"

# Are we running in the same directory as a built Orchestrator?
if [ ! -f "orchestrate.sh" -o ! -d "bin" ]; then
    echo "[ERROR] Placement tests must be run from an Orchestrator" \
         "directory, and that orchestrator must be built." > /dev/stderr
    exit 2
fi

# Is the placement test directory set correctly?
if [ ! -d "${PLACEMENT_TEST_DIR}/001" ]; then
    echo "[ERROR] Could not find any placement tests at PLACEMENT_TEST_DIR" \
         "(currently set to '${PLACEMENT_TEST_DIR}'.)" > /dev/stderr
    exit 2
fi

# Get all available tests without trailing slashes. No tests have spaces in
# their name.
pushd "${PLACEMENT_TEST_DIR}" > /dev/null
TESTS=($(ls --directory */ | cut --delimiter=/ --fields=1))
popd > /dev/null

# Remove old result files.
for TEST in ${TESTS[@]}; do
    rm --force "${PLACEMENT_TEST_DIR}/${TEST}/${RESULT_FILE_NAME}"
done

# Run each test in turn, and copy the file of interest to the placement tests
# directory.
for TEST in ${TESTS[@]}; do
    printf "Running test '${TEST}'... "
    THIS_TEST_FILE_BASENAME="${TEST_FILE_PREFIX}${TEST}"

    # Deploy application file, hardware description file, and batch file into
    # the root directory of the Orchestrator.
    cp "${PLACEMENT_TEST_DIR}/${TEST}/${THIS_TEST_FILE_BASENAME}."* "./"

    # Clear placement dumps from previous runs.
    rm --force "${PLACEMENT_DUMP_DIR}/placement_"*

    # Run the Orchestrator, without motherships, in the background. Write
    # stdout and stderr to the same file, to aid diagnosis of failing tests.
    ./orchestrate.sh -n -b "${THIS_TEST_FILE_BASENAME}.poets" 2>&1 > /dev/null &
    ORCH_PID=$!

    # When TEST_MAX_TIME has passed, kill the Orchestrator, and wait for a bit
    # afterwards for the OS to catch up. Escape early if the Orchestrator is closed.
    START_TIME=$SECONDS
    while $(kill -0 "${ORCH_PID}" 2> /dev/null); do
        if [ $((SECONDS - START_TIME)) -gt $TEST_MAX_TIME ]; then
            pkill --full "${PWD}/bin"  # Dangerous
            sleep 0.1
            break
        fi
        sleep 0.1
    done

    # Copy file of interest, if it's there. If it's not there, the comparison
    # logic (later) will complain.
    TMP=("${PLACEMENT_DUMP_DIR}/${INTERESTING_DUMP_FILE_PREFIX}"*)
    if [ -e "${TMP[0]}" ]; then
        cp "${PLACEMENT_DUMP_DIR}/${INTERESTING_DUMP_FILE_PREFIX}"* \
           "${PLACEMENT_TEST_DIR}/${TEST}/${RESULT_FILE_NAME}"
    fi
    TMP=""

    # Remove deployed test files.
    rm --force "${THIS_TEST_FILE_BASENAME}"*

    echo "done!"
done

# Print test outputs and leave.
OUTVAL=0
for TEST in ${TESTS[@]}; do
    diff "${PLACEMENT_TEST_DIR}/${TEST}/${RESULT_FILE_NAME}" \
         "${PLACEMENT_TEST_DIR}/${TEST}/placement_gi_to_hardware_placement_test_${TEST}_instance.csv" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        OUTVAL=1
        echo "Test ${TEST} FAILED."
    else
        rm --force "${PLACEMENT_TEST_DIR}/${TEST}/${RESULT_FILE_NAME}"
    fi
done

if [ $OUTVAL -eq 0 ]; then
   echo "All tests passed."
fi

exit $OUTVAL
