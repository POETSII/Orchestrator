# Orchestrator

The Orchestrator is the configuration and run-time management system for POETS
platforms.

Be sure to take a look at the user documentation at
https://github.com/POETSII/orchestrator-documentation for building and running
instructions, including dependency information, environment setup, and
command-line invocation.

The Orchestrator is a work in progress; please submit issues using the GitHub
issue tracker, and the development team will respond.

Note: This repository contains Tinsel as a submodule. This is not needed for
operation on Tinsel-POETS boxes (because they already have a Tinsel
installation), but if you wish to build or run the Orchestrator elsewhere, you
will need this submodule.

# Dependencies

The Orchestrator depends on the list of software below to function. Note that,
if you are running the Orchestrator on a POETS system (as per the usage
documentation), the "Orchestrator Dependencies tarball"
(https://github.com/POETSII/orchestrator-dependencies, private) may already be
installed on that system. If so, you do not need to install these dependencies
in order to use the Orchestrator.

 - MPICH 3.2.1. Used for multiprocess communication.

 - GCC 7.3.0: Used to compile C extracted by the XML parser into supervisor
   binaries.

 - RISCV GCC: Must support the rv32imf target.

 - Tinsel 0.7+: For running applications on POETS hardware
   (https://github.com/poetsii/tinsel).

## For Testing

To test the reference XML, you'll need `expect`
(https://core.tcl-lang.org/expect/index). Also see the `orchestrator-ci` repo.
