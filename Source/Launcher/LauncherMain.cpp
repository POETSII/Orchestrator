/* This file contains the `main` free function for the Orchestrator-MPI
 * launcher.
 *
 * If you're interested in input arguments, call with "/f", or read the
 * helpDoc variable in Launcher::ParseArgs. */

#include "LauncherMain.h"

/* Calls 'Launch' in a try/catch block. */
int main(int argc, char** argv)
{
    /* Run the launcher in a try/catch block. */
    try
    {
        return Launcher::Launch(argc, argv);
    }

    catch(std::bad_alloc& e)
    {
        printf("%sWe caught a bad_alloc: %s. Closing.\n",
               Launcher::errorHeader, e.what());
        fflush(stdout);
        return 1;
    }

    catch(...)
    {
        printf("%sUnhandled exception caught in main. Closing.\n",
               Launcher::errorHeader);
        fflush(stdout);
        return 1;
    }
}

namespace Launcher  /* Who indents function definitions in namespaces, */
{                   /* honestly? */

/* I mean, you could break this if you really wanted to, but that would make
 * you an arse. */
bool AreWeRunningOnAPoetsBox()
{
    return file_exists(fileOnlyOnPoetsBox);
}

/* Constructs the MPI command to run and writes it to 'command', given:
 *
 * - useMotherships: Whether or not to create any motherships at all.
 * - internalPath: Path to define LD_LIBRARY_PATH on child processes.
 * - overrideHost: Host to override, if applicable.
 * - batchPath: Path to a batch script to run on root when everything kicks
 *   off.
 * - hdfPath: Path to a topology file to load on root when everything kicks
 *   off.
 * - mothershipHosts: If not overriding, spawn motherships on these hosts. If
 *   this is empty and useMotherships is true, we start a mothership on this
 *   machine.
 * - executablePaths: For remote hosts, this should contain the directory
 *   containing the executable to run (-wdir).
 * - gdbProcs: Process names to run GDB on.
 * - valgrindProcs: Process names to run Valgrind on.
 * - quietValgrind: Whether or not to pass '--quiet' to Valgrind.
 * - command: Output, where the command is written to. It's up to the caller to
     examine and run it as they desire. */
void BuildCommand(bool useMotherships, std::string internalPath,
                  std::string overrideHost, std::string batchPath,
                  std::string hdfPath,
                  std::set<std::string> mothershipHosts,
                  std::map<std::string, std::string> executablePaths,
                  std::map<std::string, bool> gdbProcs,
                  std::map<std::string, bool> valgrindProcs,
                  bool quietValgrind, std::string* command)
{
    std::stringstream commandStream;

    /* Will be populated with processes to spawn. Members are on the heap
     * (bloody stringstreams). */
    std::vector<std::stringstream*> hydraProcesses;

    /* Will be populated with hosts, to define which processes run on which
     * hosts in the command. Members are *not* on the heap. */
    std::vector<std::string> orderedHosts;

    /* We'll need our hostname. */
    std::string ourHostname = OSFixes::get_hostname();

    /* Boilerplate */
    commandStream << "mpiexec.hydra";
    if (!internalPath.empty())
    {
        commandStream << " -genv LD_LIBRARY_PATH \""
                      << internalPath
                      << "\"";
    }

    /* Prepend rank to stdout in debug mode. */
    if (ORCHESTRATOR_DEBUG) commandStream << " -l";

    /* Standard-issue processes (sorry). */
    std::string localBinDir = OSFixes::dirname(OSFixes::get_executable_path());
    hydraProcesses.push_back(new std::stringstream);

    /* Setup valgrind flags */
    std::string custValgrindFlags;
    if (quietValgrind)
        custValgrindFlags = std::string(flagsValgrind) +
                            " " + quietFlagValgrind;
    else custValgrindFlags = flagsValgrind;

    /* Root */
    orderedHosts.push_back(ourHostname);
    *(hydraProcesses.back()) << "-n 1 ";
    if (gdbProcs[execRoot]) *(hydraProcesses.back()) << execGdb << " "
                                                     << flagsGdb << " ";
    if (valgrindProcs[execRoot]) *(hydraProcesses.back()) << execValgrind
                                                          << " "
                                                          << custValgrindFlags
                                                          << " ";
    *(hydraProcesses.back()) << localBinDir << "/" << execRoot;

    /* Root args */
    if (!hdfPath.empty())  /* Pass HDF, quoted, to root. */
    {
        *(hydraProcesses.back()) << " /" << ROOT_ARG_HDF
                                 << "=\\\"" << hdfPath << "\\\"";
    }

    if (!batchPath.empty())  /* Pass batch script, quoted, to root. */
    {
        *(hydraProcesses.back()) << " /" << ROOT_ARG_BATCH
                                 << "=\\\"" << batchPath << "\\\"";
    }

    /* Logserver */
    hydraProcesses.push_back(new std::stringstream);
    orderedHosts.push_back(ourHostname);
    *(hydraProcesses.back()) << "-n 1 ";
    if (gdbProcs[execLogserver]) *(hydraProcesses.back()) << execGdb << " ";
    if (valgrindProcs[execLogserver])
        *(hydraProcesses.back()) << execValgrind
                                 << " "
                                 << custValgrindFlags
                                 << " ";
    *(hydraProcesses.back()) << localBinDir << "/" << execLogserver;

    /* Clock - optionally disabled. */
    if (USE_CLOCK)
    {
        hydraProcesses.push_back(new std::stringstream);
        orderedHosts.push_back(ourHostname);
        *(hydraProcesses.back()) << "-n 1 ";
        if (gdbProcs[execClock]) *(hydraProcesses.back()) << execGdb << " ";
        if (valgrindProcs[execClock])
            *(hydraProcesses.back()) << execValgrind
                                     << " ";
        *(hydraProcesses.back()) << localBinDir << "/" << execClock;
    }
    else DebugPrint("%sThe real-time clock is currently DISABLED.\n",
                    debugHeader);

    /* Adding motherships... */
    if (useMotherships)
    {
        /* If we've set the override, just spawn a mothership on that host. */
        if (!overrideHost.empty())
        {
            hydraProcesses.push_back(new std::stringstream);
            orderedHosts.push_back(overrideHost);
            *(hydraProcesses.back())
                << "-n 1 -wdir ~/" << deployDir << " ./" << execMothership;
        }

        /* Otherwise, if there are no hosts, spawn a mothership on this box
         * (we've already checked that it's a POETS box). In this case, if
         * valgrind and/or gdb are requested for the Mothership, invoke them
         * here.  */
        else if (mothershipHosts.empty())
        {
            hydraProcesses.push_back(new std::stringstream);
            orderedHosts.push_back(ourHostname);
            *(hydraProcesses.back()) << "-n 1 ";
            if (gdbProcs[execMothership]) *(hydraProcesses.back()) <<
                                              execGdb << " ";
            if (valgrindProcs[execMothership])
                *(hydraProcesses.back()) << execValgrind
                                         << " "
                                         << custValgrindFlags
                                         << " ";
            *(hydraProcesses.back()) << localBinDir << "/" << execMothership;
        }

        /* Otherwise, spawn one mothership for each host. */
        else
        {
            WALKSET(std::string, mothershipHosts, host)
            {
                hydraProcesses.push_back(new std::stringstream);
                orderedHosts.push_back(*host);
                *(hydraProcesses.back())
                    << "-n 1 -wdir ~/" << executablePaths[(*host)]
                    << " ./" << execMothership;
            }
        }
    }

    /* Construct the host list, and append it to the command. Example:
     *
     * If orderedHosts is [hostA, hostA, hostB, hostC], then this adds
     * " -hostlist hostA:2,hostB:1,hostC:1".
     *
     * If orderedHosts is [hostD, hostE, hostE, hostD], then this adds
     * " -hostlist hostD:1,hostE:2,hostD:1" (so we can't simply count the
     * occurences...) */
    std::stringstream hostlistStream;
    std::string currentHost;
    unsigned hostStreak = 0;
    unsigned index;
    hostlistStream << " -hostlist ";  /* There's always going to be at least
                                       * one. */
    for (index=0; index<orderedHosts.size(); index++)
    {
        /* Does the streak continue? If so, increment the streak counter. Also,
         * if this is our first iteration, define the current streaking
         * host. */
        if (index == 0 || currentHost == orderedHosts[index])
        {
            hostStreak += 1;
            if (index == 0)
            {
                currentHost = orderedHosts[index];
            }
        }

        /* Otherwise, we've broken the streak, and we need to writeout and
         * reset the streak. */
        else
        {
            hostlistStream << currentHost << ":" << hostStreak << ",";
            hostStreak = 1;
            currentHost = orderedHosts[index];
        }

        /* Regardless of whether or not we've broken the streak, writeout if
         * this is the final element. */
        if (index == orderedHosts.size() - 1)
        {
            hostlistStream << currentHost << ":" << hostStreak;
            break;  /* We're about to leave anyway... */
        }
    }

    /* Put the command together, and write the command. Assumes hydraProcesses
     * is not empty (why would it be?!). */
    commandStream << hostlistStream.str();
    commandStream << " " << hydraProcesses[0]->str();
    for (index=1; index<hydraProcesses.size();
         commandStream << " : " << hydraProcesses[index++]->str());
    *command = commandStream.str();

    /* Cleanup. */
    for (index=0; index<hydraProcesses.size(); delete hydraProcesses[index++]);
}

/* Deploys all of the compiled binaries (runtime stuff) to a set of 'hosts'.
 *
 * So MPI, in most of its implementations, has the somewhat inconvenient
 * characteristic that you need to deploy the binary you want to execute to the
 * host - the hydra proxy does not do that for you. So we do it here (using
 * SSH). Arguments:
 *
 * - hosts: Hosts to deploy to (not modified, honest).
 *
 * - paths: Output of paths to where the binaries are deployed to, for a given
 *   host. Cleared if there are no hosts, or if deployment fails.
 *
 * Returns 0 on success, and another number on failure. */
int DeployBinaries(std::set<std::string>* hosts,
                   std::map<std::string, std::string>* paths)
{
    paths->clear();

    /* Save us from ourselves. */
    if (hosts->empty()) return 0;

    /* Figure out where the executables all are on this box. We assume that
     * they are in the same directory as the launcher. */
    DebugPrint("%sIdentifying where the binaries are on this box, from where "
               "the launcher is...\n", debugHeader);
    std::string sourceDir;
    sourceDir = OSFixes::dirname(OSFixes::get_executable_path());

    if (sourceDir.empty())
    {
        printf("%sCould not identify where the binaries are on this "
               "box. Closing.\n", errorHeader);
        return 1;
    }
    else
    {
        DebugPrint("%sFound the binaries to copy at '%s'.\n", debugHeader,
                   sourceDir.c_str());
    }

    /* Deploy! */
    std::string cmdstdout;
    std::string cmdstderr;
    int sshRc;
    WALKSET(string, (*hosts), host)
    {
        DebugPrint("%sDeploying to host '%s'...\n",
                   debugHeader, host->c_str());

        /* Ensure .orchestrator exists. */
        sshRc = SSH::call((*host),
                          dformat("mkdir --parents \"%s\"\n",
                                  OSFixes::dirname(deployDir).c_str()),
                          &cmdstdout, &cmdstderr);
        if (sshRc > 0)
        {
            fprintf(stderr,
                    "%sSSH command to host '%s' failed (can you connect to "
                    "the host by SSH?): %s",
                    errorHeader, (*host).c_str(), cmdstderr.c_str());
            return 1;
        }
        else if (sshRc == -1)
        {
            fprintf(stderr, "%sError calling SSH: %s.\n", errorHeader,
                    strerror(errno));
            return 1;
        }

        /* Remove the target directory, dangerously. */
        sshRc = SSH::call((*host),
                          dformat("rm --force --recursive \"%s\"\n",
                                  deployDir),
                          &cmdstdout, &cmdstderr);
        if (sshRc > 0)
        {
            /* NB: rm -rf can't fail outside mad edge cases... */
            fprintf(stderr,
                    "%sSSH command to host '%s' failed (can you connect to "
                    "the host by SSH?): %s",
                    errorHeader, (*host).c_str(), cmdstderr.c_str());
            return 1;
        }
        else if (sshRc == -1)
        {
            fprintf(stderr, "%sError calling SSH: %s.\n", errorHeader,
                    strerror(errno));
            return 1;
        }

        /* Deploy binaries. */
        sshRc = SSH::deploy_directory((*host), sourceDir, deployDir,
                                      &cmdstdout, &cmdstderr);
        if (sshRc > 0)
        {
            fprintf(stderr,
                    "%sFailed to deploy to host '%s': %s",
                    errorHeader, (*host).c_str(), cmdstderr.c_str());
            return 1;
        }
        else if (sshRc == -1)
        {
            fprintf(stderr, "%sError calling SSH: %s.\n", errorHeader,
                    strerror(errno));
            return 1;
        }

        /* Grab the full path of the directory created (we can't compute that
         * here, because user names may vary, etc.) */
        sshRc = SSH::call((*host),
                          dformat("realpath \"%s\" | tr --delete '\n'\n",
                                  deployDir),
                          &cmdstdout, &cmdstderr);
        if (sshRc > 0)
        {
            fprintf(stderr,
                    "%sSSH command to host '%s' failed (can you connect to "
                    "the host by SSH?): %s",
                    errorHeader, (*host).c_str(), cmdstderr.c_str());
            return 1;
        }
        else if (sshRc == -1)
        {
            fprintf(stderr, "%sError calling SSH: %s.\n", errorHeader,
                    strerror(errno));
            return 1;
        }

        DebugPrint("%sDeployment to host '%s' complete.\n",
                   debugHeader, host->c_str());

        (*paths)[*host] = cmdstdout;
    }

    return 0;
}

/* Reads the hardware description file at hdfPath, and populates the vector at
 * hosts with the names of hosts obtained from that file. Returns 0 if all is
 * well, and 1 if we need to leave. */
int GetHosts(std::string hdfPath, std::set<std::string>* hosts)
{
    if (!file_readable(hdfPath.c_str()))
    {
        printf("%sCannot find hardware description file at %s. Closing.\n",
               errorHeader, hdfPath.c_str());
        return 1;
    }

// Take it away, ADB.
JNJ Jh(hdfPath);                        // OK, let's do it
vH sects;
Jh.FndSect("engine_box",sects);         // Got all the sections called ....
if (sects.empty()) return 0;            // None there?
WALKVECTOR(hJNJ,sects,i) {              // Walk all the sections...
  vH recds;
  Jh.GetRecd(*i,recds);                 // Get the records
  WALKVECTOR(hJNJ,recds,j) {            // Walk the records
    vH varis;
    Jh.GetVari(*j,varis);               // Get the variables (box names)
    WALKVECTOR(hJNJ,varis,k) {
      if ((*k)->str.empty()) continue;        // If it's not blank.....
      if ((*k)->qop==Lex::Sy_plus) continue;  // If it's not a '+'.....
      vH subs;
      Jh.GetSub(*k,subs);               // Get the box variable subname(s)
      WALKVECTOR(hJNJ,subs,l) {         // Walk them
        if ((*l)->str=="hostname") {    // Look for.....
          vH subs2;
          Jh.GetSub(*l,subs2);          // And get them
          if (subs2.empty()) continue;  // Non-empty set?
          hosts->insert(subs2[0]->str); // At last! Save the damn thing
        }
      }
    }
  }
}
return 0;
}

/* Launches the Orchestrator, unsurprisingly. */
int Launch(int argc, char** argv)
{
    /* Parse input arguments. */
    std::string batchPath;
    std::string hdfPath;
    bool useMotherships;
    bool dryRun;
    bool quietValgrind = false;
    std::string overrideHost;
    std::string internalPath;
    std::map<std::string, bool> gdbProcs;
    std::map<std::string, bool> valgrindProcs;
    gdbProcs["root"] = false;
    gdbProcs["rtcl"] = false;
    gdbProcs["logserver"] = false;
    gdbProcs["mothership"] = false;
    valgrindProcs["root"] = false;
    valgrindProcs["rtcl"] = false;
    valgrindProcs["logserver"] = false;
    valgrindProcs["mothership"] = false;

    if (ParseArgs(argc, argv, &batchPath, &hdfPath, &useMotherships, &dryRun,
                  &quietValgrind, &overrideHost, &internalPath, &gdbProcs,
                  &valgrindProcs) > 0) return 1;

    /* If the default hardware description file path has a file there, and we
     * weren't passed a file explicitly, let's roll with the one we've
     * found.
     *
     * If the operator doesn't like this default, they can always load a new
     * one in the usual way, which will clobber the target. file_exists is from
     * flat.h. */
    if (hdfPath.empty() && file_exists(defaultHdfPath))
    {
        hdfPath = defaultHdfPath;
        DebugPrint("%sFound a hardware description file in the default search "
                   "location (%s). Using that one.\n",
                   debugHeader, hdfPath.c_str());
    }

    /* Read the input file, if supplied, and get a set of hosts we must launch
     * Mothership processes on. Don't bother if we're overriding, or if
     * motherships are disabled.
     *
     * Future work, we may want to make hosts a std::map<std::string, ENUM>, so
     * that you can launch specific processes with it from a more advanced
     * input file. Just a passing thought. */
    std::set<std::string> hosts;  /* May remain empty. */
    if (useMotherships)  /* User may have disabled them, in which case there's
                          * no point in us looking. */
    {
        /* Have we been given a file to read, and it hasn't been overridden? */
        if (!hdfPath.empty() and overrideHost.empty())
        {
            int rc = GetHosts(hdfPath, &hosts);
            if (rc != 0) return rc;

            /* Print the hosts we found, if anyone is listening. */
            #if ORCHESTRATOR_DEBUG
            if(!hosts.empty())
            {
                DebugPrint("%sAfter reading the input file, I found the "
                           "following hosts:\n", debugHeader);
                WALKSET(std::string, hosts, hostIterator)
                {
                    DebugPrint("%s%s- %s\n", debugHeader, debugIndent,
                               (*hostIterator).c_str());
                }
                DebugPrint("%s\n", debugHeader);
            }

            else
            {
                DebugPrint("%sAfter parsing the input file, I found no "
                           "hosts.\n", debugHeader);
            }
            #endif
        }

        /* If we're overriding, that makes the host list quite simple... */
        else if (!overrideHost.empty())
        {
            hosts.insert(overrideHost);
            DebugPrint("%sIgnoring input file, and instead using the "
                       "override passed in as an argument.\n", debugHeader);
        }
    }

    /* Warn the user that we can't spawn any motherships iff:
     *  - the user hasn't disabled motherships,
     *  - no hosts were found from any input file passed in,
     *  - no override host was proposed, and
     *  - we're not running on a POETS box. */
    DebugPrint("%sPerforming POETS box check...\n", debugHeader);
    if (useMotherships && hosts.empty() && !AreWeRunningOnAPoetsBox())
    {
        printf("[WARN] Launcher: Not running on a POETS box, and found no "
               "motherships running on alternative machines, so we're not "
               "spawning any mothership processes.\n");
        useMotherships = false;
    }

    /* Deploy the binaries to the hosts that are running motherships, if we're
     * using motherships. If we do deploy anything, store the paths that we've
     * deployed stuff to, so that we can build the command later. Fail fast. */
    std::map<std::string, std::string> deployedPaths;
    if (DeployBinaries(&hosts, &deployedPaths) != 0) return 1;

    /* Build the MPI command. */
    std::string command;
    DebugPrint("%sBuilding command...\n", debugHeader);
    BuildCommand(useMotherships, internalPath, overrideHost, batchPath,
                 hdfPath, hosts, deployedPaths, gdbProcs, valgrindProcs,
                 quietValgrind, &command);

    /* Run the MPI command, or not. */
    DebugPrint("%sRunning this command: %s\n", debugHeader, command.c_str());
    if (dryRun)
    {
        #if ORCHESTRATOR_DEBUG
        #else
        printf("%s\n", command.c_str());
        #endif
        return 0;
    }

    /* Note we don't use call here, because we don't care about capturing the
     * stdout, stderr, or returncode (though we might as well propagate the
     * latter). */
    return system(command.c_str());
}

/* Parses arguments given to the launcher on the command line, and populates
 * various reference-passed variables as it goes. May also do some
 * printing. Arguments:
 *
 * - argc: Input argument count
 * - argv: Input argument vector (har har)
 * - batchPath: Output, populated with the path to a batch script from
 *   arguments (or empty if not defined).
 * - hdfPath: Output, populated with the path to a hardware description file
 *   to read from arguments (or empty if not defined).
 * - useMotherships: Output, whether or not motherships have been explicitly
 *   disabled.
 * - dryRun: Output, true if the caller wants to just see what would be
 *   run. Still deploys binaries.
 * - quietValgrind: Output, true if caller wants valgrind to run quietly.
 * - overrideHost: Output, populated with the name of the host to run a
 *   mothership on, regardless of whether or not an input file has been passed
 *   in (or empty if not defined).
 * - internalPath: Output, holds the internal path argument if defined, see the
 *   help string.
 * - gdbProcs: Processes to point GDB at.
 * - valgrindProcs: Processes to point Valgrind at.
 *
 * Returns non-zero if a fast exit is needed. */
int ParseArgs(int argc, char** argv, std::string* batchPath,
              std::string* hdfPath, bool* useMotherships, bool* dryRun,
              bool* quietValgrind, std::string* overrideHost,
              std::string* internalPath,
              std::map<std::string, bool>* gdbProcs,
              std::map<std::string, bool>* valgrindProcs)
{
    /* Print input arguments, if we're in debug mode. */
    DebugPrint("%sWelcome to the POETS Launcher. Raw arguments:\n",
               debugHeader);
    #if ORCHESTRATOR_DEBUG
    for (int argIndex=0; argIndex<argc; argIndex++)
    {
        DebugPrint("%s%sArgument %d: %s\n", debugHeader, debugIndent,
                   argIndex, argv[argIndex]);
    }
    DebugPrint("%s\n", debugHeader);
    #endif

    /* Argument map. */
    std::map<std::string, std::string> argKeys;
    argKeys["batchPath"] = "b";
    argKeys["dontStartTheOrchestrator"] = "d";
    argKeys["hdfPath"] = "f";
    argKeys["gdb"] = "g";
    argKeys["help"] = "h";
    argKeys["noMotherships"] = "n";
    argKeys["override"] = "o";
    argKeys["internalPath"] = "p";
    argKeys["quietValgrind"] = "q";
    argKeys["valgrind"] = "v";

    /* Defines help string, printed when user calls with `-h`. */
    std::string helpDoc = dformat(
"Usage: %s [/b=FILE] [/d] [/f=FILE] [/h] [/g=PROCESS] [/n] [/o=HOST] [/p=PATH] [/v=PROCESS]\n"
"\n"
"This is the Orchestrator launcher. It starts the following Orchestrator processes:\n"
"\n"
" - root\n"
" - logserver\n"
" - rtcl\n"
" - mothership (some number of them)\n"
"\n"
"For verbosity, compile with debugging enabled by setting ORCHESTRATOR_DEBUG to 1 (i.e. `make debug`, or `$(CXX) -DORCHESTRATOR_DEBUG`. See `Debug.{cpp,h}`). This launcher accepts the following optional arguments:\n"
"\n"
"\t/%s=FILE: Path to a batch script to run on the root process on startup. Each command is added to root's MPI message queue, and each runs independently of the other. See the Orchestrator documentation for more information on the batch system.\n"
"\n"
"\t/%s: Don't start the Orchestrator! Still deploys binaries, but does not execute the command. Instead, the command is printed in your shell.\n"
"\n"
"\t/%s=FILE: Path to a hardware file to read hostnames from, in order to get hostnames for starting remote mothership processes. If none is provided, '%s' is searched (this default is defined in the launcher source, and is not yet read from any configuration file). If the operator does not wish to use this default, the operator can reset the topology in the root shell using a 'topology' command.\n"
"\n"
"\t/%s: Prints this help text.\n"
"\n"
"\t/%s: Points gdb (%s) at one of the non-mothership processes listed above. Will not work as you indend on remote processes (unless you're smarter than I am).\n"
"\n"
"\t/%s: Does not spawn any mothership processes, regardless of other arguments you pass in.\n"
"\n"
"\t/%s=HOST: Override all Mothership hosts, specified from a hardware description file, with HOST. Using this option will only spawn one mothership process (unless /%s is used, in which case no mothership processes are spawned).\n"
"\n"
"\t/%s=PATH: Define an LD_LIBRARY_PATH environment variable for all spawned processes. This is useful for defining where shared object files can be found by children.\n"
"\n"
"\t/%s: If valgrind is requested, make it run in quiet mode.\n"
"\n"
"\t/%s: Points valgrind (%s) at one of the processes listed above, except motherships spawned via a host list. Combine with /%s at your own risk.\n"
"\n"
"If you are still bamboozled, or you're a developer, check out the Orchestrator documentation.\n",
argv[0],
argKeys["batchPath"].c_str(),
argKeys["dontStartTheOrchestrator"].c_str(),
argKeys["hdfPath"].c_str(), defaultHdfPath,
argKeys["help"].c_str(),
argKeys["gdb"].c_str(), execGdb,
argKeys["noMotherships"].c_str(),
argKeys["override"].c_str(), argKeys["noMotherships"].c_str(),
argKeys["internalPath"].c_str(),
argKeys["quietValgrind"].c_str(),
argKeys["valgrind"].c_str(), execValgrind, argKeys["gdb"].c_str());

    /* Parse the input arguments. */
    std::string concatenatedArgs;
    for(int i=1; i<argc; concatenatedArgs += argv[i++]);

    /* Decode, and check for errors. */
    int line, column;
    Cli cli(concatenatedArgs);
    cli.Err(line, column);
    if (line >= 0)
    {
        printf("%sCommand line error at about input character %d\n",
               errorHeader, column);
        return 1;
    }

    /* And here we go! */
    *dryRun = false;  /* Pessimism. */
    *useMotherships = true;  /* Optimism. */

    std::string currentArg;
    std::string currentProcess;
    WALKVECTOR(Cli::Cl_t, cli.Cl_v, argIt)
    {
        currentArg = (*argIt).Cl;
        if (currentArg == argKeys["batchPath"])
        {
            if (batchPath->empty())
            {
                *batchPath = (*argIt).GetP(0);
            }
            else
            {
                printf("[WARN] Launcher: Ignoring duplicate input batchPath "
                       "argument.\n");
            }
        }

        if (currentArg == argKeys["dontStartTheOrchestrator"])
        {
            if (!*dryRun)
            {
                *dryRun = true;
            }
            else
            {
                printf("[WARN] Launcher: Ignoring duplicate 'don't start the "
                       "Orchestrator!' argument.\n");
            }

        }

        if (currentArg == argKeys["hdfPath"])  /* Hardware file: Store it. */
        {
            if (hdfPath->empty())
            {
                *hdfPath = (*argIt).GetP(0);
            }
            else
            {
                printf("[WARN] Launcher: Ignoring duplicate input file "
                       "argument.\n");
            }
        }

        if (currentArg == argKeys["gdb"])
        {
            currentProcess = (*argIt).GetP(0);
            if (currentProcess.empty())
            {
                printf("[WARN] Launcher: GDB process argument empty. "
                       "Ignoring.\n");
            }

            /* If it's not a valid process, we panic. */
            if (gdbProcs->find(currentProcess) == gdbProcs->end())
            {
                printf("[WARN] Launcher: Gdb process argument '%s' does not "
                       "correspond to an Orchestrator executable.\n",
                       currentProcess.c_str());
            }

            /* If it is, it's true! */
            else (*gdbProcs)[currentProcess] = true;
        }

        if (currentArg == argKeys["help"])  /* Help: Print and leave. */
        {
            printf("%s", helpDoc.c_str());
            return 1;
        }

        if (currentArg == argKeys["noMotherships"])
        {
            if (*useMotherships)
            {
                *useMotherships = false;
            }
            else
            {
                printf("[WARN] Launcher: Ignoring duplicate noMotherships "
                       "argument.\n");
            }
        }

        if (currentArg == argKeys["override"])  /* Override: Store it. */
        {
            if (overrideHost->empty())
            {
                *overrideHost = (*argIt).GetP(0);
                if (overrideHost->empty())
                {
                    printf("[WARN] Launcher: Override host argument empty. "
                           "Ignoring.\n");
                }
            }
            else
            {
                printf("[WARN] Launcher: Ignoring duplicate override"
                       "argument.\n");
            }
        }

        if (currentArg == argKeys["quietValgrind"])
        {
            if (!(*quietValgrind))
            {
                *quietValgrind = true;
            }
            else
            {
                printf("[WARN] Launcher: Ignoring duplicate 'quiet valgrind' "
                       "argument.\n");
            }
        }

        if (currentArg == argKeys["internalPath"])
        {
            if (internalPath->empty())
            {
                *internalPath = (*argIt).GetP(0);
                if (internalPath->empty())
                {
                    printf("[WARN] Launcher: Internal path argument empty. "
                           "Ignoring.\n");
                }
            }
            else
            {
                printf("[WARN] Launcher: Ignoring duplicate internal path "
                       "argument.\n");
            }
        }

        if (currentArg == argKeys["valgrind"])
        {
            currentProcess = (*argIt).GetP(0);
            if (currentProcess.empty())
            {
                printf("[WARN] Launcher: Valgrind process argument empty. "
                       "Ignoring.\n");
            }

            /* If it's not a valid process, we panic. */
            if (valgrindProcs->find(currentProcess) == valgrindProcs->end())
            {
                printf("[WARN] Launcher: Valgrind process argument '%s' does "
                       "not correspond to an Orchestrator executable.\n",
                       currentProcess.c_str());
            }

            /* If it is, it's true! */
            else (*valgrindProcs)[currentProcess] = true;
        }
    }

    /* Print what happened, if anyone is listening. */
    #if ORCHESTRATOR_DEBUG
    DebugPrint("%sParsed arguments:\n", debugHeader);
    if (hdfPath->empty())
    {
        DebugPrint("%s%sHardware description file path: Not specified.\n",
                   debugHeader, debugIndent);
    }
    else
    {
        DebugPrint("%s%sHardware description file path: %s\n",
                   debugHeader, debugIndent, hdfPath->c_str());
    }
    if (overrideHost->empty())
    {
        DebugPrint("%s%sOverride host: Not specified.\n",
                   debugHeader, debugIndent);
    }
    else
    {
        DebugPrint("%s%sOverride host: %s\n", debugHeader, debugIndent,
                    overrideHost->c_str());
    }
    if (*useMotherships)
    {
        DebugPrint("%s%sMotherships: enabled\n", debugHeader, debugIndent);
    }
    else
    {
        DebugPrint("%s%sMotherships: disabled\n", debugHeader, debugIndent);
    }
    if (*quietValgrind)
    {
        DebugPrint("%s%sIf requested, valgrind will be run quietly.\n",
                   debugHeader, debugIndent);
    }
    else
    {
        DebugPrint("%s%sIf requested, valgrind will not be run quietly.\n",
                   debugHeader, debugIndent);
    }
    if (*dryRun)
    {
        DebugPrint("%s%sWe're not actually going to run the command.\n",
                   debugHeader, debugIndent);
    }
    else
    {
        DebugPrint("%s%sWe are going to run the generated command.\n",
                   debugHeader, debugIndent);
    }

    DebugPrint("%s\n", debugHeader);
    #endif

    return 0;
}
}  /* End namespace */
