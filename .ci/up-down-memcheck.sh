#!/bin/sh

# Starts up the Orchestrator, and closes it down again. Returns 0 if no
# memchecks report errors, and 1 otherwise.
#
# I know it sucks, but it's difficult to propagate all of the valgrind exit
# codes back out. Well okay, it's not difficult, but this is faster.
MEMCHECK_TMP="memcheck_tmp.txt"
> "${MEMCHECK_TMP}"
../orchestrate.sh -v root -v logserver -v mothership -b quick_exit.poets -q \
                  2> ${MEMCHECK_TMP}
if [ -s "${MEMCHECK_TMP}" ]; then
    cat "${MEMCHECK_TMP}"
    exit 1
else
    exit 0
fi
