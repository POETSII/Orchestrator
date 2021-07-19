#!/bin/sh

# Sets up and runs the placement tests.
export PLACEMENT_TEST_DIR="Tests/Placement"
PLACEMENT_TEST_SCRIPT="run-all-tests.sh"
cd ../
"${PLACEMENT_TEST_DIR}"/"${PLACEMENT_TEST_SCRIPT}"
exit $?
