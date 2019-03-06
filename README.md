Note: This repository contains a submodule. Ensure you clone with
`--recursive-submodules` (or `--recursive` if on Git 2.12 or earlier)

# Orchestrator

The Orchestrator is the configuration and run-time management system for POETS
platforms.

Be sure to take a look at the user documentation at
https://github.com/POETSII/orchestrator-documentation for building and running
instructions.

## PREREQUISITES

The following essential utilities and libraries are prequisite:
MPICH version 3.2.1
gcc version 7.3.0,
Qt version 5.6
riscv gcc compiler version 1.0 with multilib support (rv32imf target at minimum)
tinsel version 0.3 (or later, not yet tested)

## NOTES ON INSTALL DIRECTORY STRUCTURE

When running applications, the /Source/Softswitch directory and the Tinsel directory should be placed in the same directory as the one that contains your directory of application definition scripts. So if you have a file '/home/myname/applications/xml/application1.xml' that you are loading, /Softswitch and /Tinsel directories should be in /home/myname/applications. The Makefile (in /Softswitch) should also reside in the same place, e.g 'home/myname/applications/Makefile'. This is not required if you are only trying to build the Orchestrator from source.

The compiled binaries from the Orchestrator can go almost anywhere as long as they're in a common directory, but keep these out of the directory hierarchy containing your application definitions, and out of the hierarchy containing the support tools and libraries. Also note that if your version of the Tinsel software changes or your hardware environment changes (as reflected in the config.py script in /Tinsel), you must recompile the Orchestrator, at least for the moment (Later we will support dynamic hardware discovery and much of this problem should go away)

File OrchestratorMessages.txt has to be in the same directory as the Orchestrator binaries. You can freely edit this file if you don't like the messages it outputs, as long as the message number (at the beginning of each line) and the number of %s parameters remain fixed (you can even delete messages altogether and nothing will break, although you might get some strange output messages). By default, the Orchestrator logserver messages will be dumped to a file located in the same directory as the binaries.

## ENVIRONMENT SETUP

You need to set (export) the following environment variables:

export RISCV_PATH={path_to_riscv-gcc-xcompile}
export PATH={path_to_mpich-3.2.1}/bin:$RISCV_PATH/bin:$PATH
export TRIVIAL_LOG_HANDLER=1
export LM_LICENSE_FILE={box's LM_LICENSE_FILE setting}

and you probably need to run the quartus setup script for the machine.

TRIVIAL_LOG_HANDLER=1 supplies a very basic form of handler_log which simply outputs the log string and doesn't substitute any parameters. In future it should be possible to set the following alternative options:
TYPICAL_LOG_HANDLER=1 to allow substitution of simple ints and floats only
GENERAL_LOG_HANDLER=1 which allows arbitrary printf-like substitution
but as of now these cause the linker to run out of instruction memory when it tries to create the riscv binaries, so they are temporarily non-functional.

## COMMAND LINE

Orchestrator is an MPI application with several distinct binaries:

root, containing the 'main' process which handles operator input and contains the main copy of the database
mothership, the process managing connection and communication with the tinsel boards
logserver, the system-logging facility
rtcl, a system-wide real-time clock
injector, a process to allow conditional/programmatic script execution in the Orchestrator
dummy, a process simply to stress the MPI network with traffic

You have to use the MPI startup command: mpiexec.hydra, to execute these binaries. An mpiexec.hydra command creates a distinct MPI 'universe': a collection of communicating processes that in some sense can be viewed as a single application. *In principle* you can run any subset of these processes within any mpiexec command, it would be, for example, perfectly legal to start a root and a dummy only, but *in practice* certain setups will cause strange behaviour:

* Starting root without also starting logserver will disable system messages and could lead to strange output.
* Starting any process in isolation without also starting root will mean there is no user input (or control) within that universe, (though subject to possible later Connect commands from other universes which will allow control to be acquired externally)
* Starting dummy without a root will result in a series of MPI messages that never complete (thus deadlocking the dummy)
* Starting injector without a root will result in an a process that cannot do anything.

Only a small subset of the possible combinations of startup has actually been tried and tested!

The 'usual' command looks like this:

mpiexec.hydra -genv LD_LIBRARY_PATH ./:{Path_to_Qt_libs}:{Path_to_mpich-3.2.1}/lib:{Path_to_gcc-7.3.0}/lib64 -n 1 ./root : -n 1 ./logserver : -n 1 ./rtcl : -n 1 ./mothership

For some reason, attempting to set LD_LIBRARY_PATH as an environment variable outside the mpiexec line has proven not to work, so if you're tempted to try that approach, be aware that it is likely to be fruitless.

## CONNECTING TO OTHER UNIVERSES

Provided an Orchestrator universe has been started up containing a root, you can subsequently link it to other Orchestrator universes either started beforehand or subsequently, using the system /conn command. One potential use of this would be to allow Motherships (which take control of Tinsel hardware) to be dynamically started and stopped independently of the rest of the Orchestrator, and possibly be connected to by several other Orchestrator instances. This requires the following additional setup:

You have to start up a hydra nameserver process. Assuming the mpich bin directory is in your path, you can do this by typing

hydra_nameserver &

If any mpiexec command you run is operating on a different host machine than the one you started up hydra_nameserver, you need the following additional switch in your mpiexec.hydra command

-nameserver {host_name}

This switch should probably go right after the specification of the library directories (i.e. before any -n {x} {executable} clauses).

This should be done BEFORE attempting to connect to any universes you might wish to.

Once this has been done, you can link the 2 universes, making them appear as one large Orchestrator system, by typing

system /conn = {service name}

where service name is the text string indicating the universe to which you want to connect. Currently the default service name is "POETS_MPI_Master". This can be changed on the server universe (the one you are connecting to) provided root has been started in that universe, by typing

system /svcn = {service name}

Once connected you should be able to execute all commands seamlessly and the Orchestrator will work out where to direct the result.

None of this functionality has been tested in more than a rudimentary way yet!
=======
