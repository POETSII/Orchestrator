/* This file contains the `main` free function for the Orchestrator-MPI
 * launcher.
 *
 * If you're interested in input arguments, call with "h", or read helpDoc. */

#include "LauncherMain.h"

#include <string>

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

namespace Launcher
{

bool AreWeRunningOnAPoetsBox()
{
    return file_exists(fileOnlyOnPoetsBox);
}

/* Constructs the MPI command to run and writes it to 'command', given a set
 * of 'hosts' and other stuff. */
void BuildCommand(bool useMotherships, std::string internalPath,
                  std::string overrideHost, std::string batchPath,
                  std::string hdfPath,
                  std::set<std::string>* mothershipHosts,
                  std::map<std::string, std::string>* executablePaths,
                  std::string* command)
{
    std::stringstream commandStream;

    /* Will be populated with processes to spawn. Members are on the heap
     * (bloody stringstreams). */
    std::vector<std::stringstream*> hydraProcesses;

    /* Will be populated with hosts, to define which processes run on which
     * hosts in the command. Members are *not* on the heap. */
    std::vector<std::string> orderedHosts;

    /* We'll need our hostname. */
    std::string ourHostname = POETS::get_hostname();

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
    std::string localBinDir = POETS::dirname(POETS::get_executable_path());

    hydraProcesses.push_back(new std::stringstream);
    orderedHosts.push_back(ourHostname);
    *(hydraProcesses.back()) << "-n 1 "
                             << localBinDir << "/" << execRoot;
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

    hydraProcesses.push_back(new std::stringstream);
    orderedHosts.push_back(ourHostname);
    *(hydraProcesses.back()) << "-n 1 " << localBinDir << "/" << execLogserver;

    hydraProcesses.push_back(new std::stringstream);
    orderedHosts.push_back(ourHostname);
    *(hydraProcesses.back()) << "-n 1 " << localBinDir << "/" << execClock;

    /* Adding motherships... */
    if (useMotherships)
    {
        /* If we've set the override, just spawn a mothership on that host. */
        if (!overrideHost.empty())
        {
            hydraProcesses.push_back(new std::stringstream);
            orderedHosts.push_back(overrideHost);
            *(hydraProcesses.back())
                << "-n 1  " << deployDir << "/" << execMothership;
        }

        /* Otherwise, if there are no hosts, spawn a mothership on this box
         * (we've already checked that it's a POETS box). */
        else if (mothershipHosts->empty())
        {
            hydraProcesses.push_back(new std::stringstream);
            orderedHosts.push_back(ourHostname);

            *(hydraProcesses.back()) << "-n 1 " << localBinDir << "/"
                                     << execMothership;
        }

        /* Otherwise, spawn one mothership for each host. */
        else
        {
            WALKSET(std::string, (*mothershipHosts), host)
            {
                hydraProcesses.push_back(new std::stringstream);
                orderedHosts.push_back(*host);
                *(hydraProcesses.back()) <<
                    "-n 1 " << (*executablePaths)[(*host)] <<
                    "/" << execMothership;
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
    sourceDir = POETS::dirname(POETS::get_executable_path());

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
    std::string stdout;
    std::string stderr;
    WALKSET(string, (*hosts), host)
    {
        DebugPrint("%sDeploying to host '%s'...\n",
                   debugHeader, host->c_str());

        /* Ensure .orchestrator exists. */
        if (SSH::call((*host),
                      dformat("mkdir --parents \"%s\"\n",
                              POETS::dirname(deployDir).c_str()),
                      &stdout, &stderr) > 0)
        {
            printf("%sSSH command to host '%s' failed (can you connect to the "
                   "host by SSH?): %s",
                   errorHeader, (*host).c_str(), stderr.c_str());
            return 1;
        }

        /* Remove the target directory, dangerously. */
        if (SSH::call((*host),
                      dformat("rm --force --recursive \"%s\"\n", deployDir),
                      &stdout, &stderr) > 0)
        {
            /* NB: rm -rf can't fail outside mad edge cases... */
            printf("%sSSH command to host '%s' failed (can you connect to the "
                   "host by SSH?): %s",
                   errorHeader, (*host).c_str(), stderr.c_str());
            return 1;
        }

        /* Deploy binaries. */
        if (SSH::deploy_directory((*host), sourceDir, deployDir,
                                  &stdout, &stderr) > 0)
        {
            printf("%sFailed to deploy to host '%s': %s",
                   errorHeader, (*host).c_str(), stderr.c_str());
            return 1;
        }

        /* Grab the full path of the directory created (we can't compute that
         * here, because user names may vary, etc.) */
        if (SSH::call((*host),
                      dformat("realpath \"%s\" | tr --delete '\n'\n",
                              deployDir),
                      &stdout, &stderr) > 0)
        {
            printf("%sSSH command to host '%s' failed (can you connect to the "
                   "host by SSH?): %s",
                   errorHeader, (*host).c_str(), stderr.c_str());
            return 1;
        }

        (*paths)[*host] = stdout;
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
    /* Print input arguments, if we're in debug mode. */
    DebugPrint("%sWelcome to the POETS Launcher. Raw arguments:\n",
               debugHeader);
    #if ORCHESTRATOR_DEBUG
    for (int argIndex=0; argIndex<argc; argIndex++)
    {
        DebugPrint("%s%sArgument %d: %s\n", debugHeader, debugIndent, argIndex,
                   argv[argIndex]);
    }
    DebugPrint("%s\n", debugHeader);
    #endif

    /* Argument map. */
    std::map<std::string, std::string> argKeys;
    argKeys["batchPath"] = "b";
    argKeys["help"] = "h";
    argKeys["file"] = "f";
    argKeys["noMotherships"] = "n";
    argKeys["override"] = "o";
    argKeys["internalPath"] = "p";

    /* Defines help string, printed when user calls with `-h`. */
    std::string helpDoc = dformat(
"Usage: %s [/h] [/f = FILE] [/o = HOST] [/p = PATH]\n"
"\n"
"This is the Orchestrator launcher. It starts the following Orchestrator "
"processes:\n"
"\n"
" - root\n"
" - logserver\n"
" - rtcl\n"
" - mothership (some number of them)\n"
"\n"
"If you are bamboozled, try compiling with debugging enabled (i.e. `make "
"debug`). This launcher accepts the following optional arguments:\n"
"\n"
" /%s: Path to a batch script to run on root on startup.\n"
" /%s: Prints this help text.\n"
" /%s = FILE: Path to a hardware file to read hostnames from, in order to "
"start motherships.\n"
" /%s: Does not spawn any mothership processes.\n"
" /%s = HOST: Override all Mothership hosts, specified from a hardware "
"description file, with HOST.\n"
" /%s = PATH: Define an LD_LIBRARY_PATH environment variable for called "
"processes.\n",
argv[0],
argKeys["batchPath"].c_str(),
argKeys["help"].c_str(),
argKeys["file"].c_str(),
argKeys["noMotherships"].c_str(),
argKeys["override"].c_str(),
argKeys["internalPath"].c_str());

    /* Parse the input arguments. */
    std::string concatenatedArgs;
    for(int i=1; i<argc; concatenatedArgs += argv[i++]);

    /* Decode, and check for errors. */
    Cli cli(concatenatedArgs);
    if (cli.problem.prob)
    {
        printf("%sCommand line error at about input character %d\n",
               errorHeader, cli.problem.col);
        return 1;
    }

    /* Staging for input arguments. */
    std::string batchPath;
    std::string hdfPath;
    bool useMotherships = true;
    std::string overrideHost;
    std::string internalPath;

    /* Read each argument in turn. */
    std::string currentArg;
    WALKVECTOR(Cli::Cl_t, cli.Cl_v, argIt)
    {
        currentArg = (*argIt).Cl;
        if (currentArg == argKeys["batchPath"])
        {
            if (batchPath.empty())
            {
                batchPath = (*argIt).GetP(0);
            }
            else
            {
                printf("[WARN] Launcher: Ignoring duplicate input batchPath "
                       "argument.\n");
            }
        }
        if (currentArg == argKeys["help"])  /* Help: Print and leave. */
        {
            printf("%s", helpDoc.c_str());
            return 0;
        }

        if (currentArg == argKeys["file"])  /* Hardware file: Store it. */
        {
            if (hdfPath.empty())
            {
                hdfPath = (*argIt).GetP(0);
            }
            else
            {
                printf("[WARN] Launcher: Ignoring duplicate input file "
                       "argument.\n");
            }
        }

        if (currentArg == argKeys["noMotherships"])
        {
            if (useMotherships)
            {
                useMotherships = false;
            }
            else
            {
                printf("[WARN] Launcher: Ignoring duplicate noMotherships "
                       "argument.\n");
            }
        }

        if (currentArg == argKeys["override"])  /* Override: Store it. */
        {
            if (overrideHost.empty())
            {
                overrideHost = (*argIt).GetP(0);
                if (overrideHost.empty())
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

        if (currentArg == argKeys["internalPath"])
        {
            if (internalPath.empty())
            {
                internalPath = (*argIt).GetP(0);
                if (internalPath.empty())
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
    }

    /* Print what happened, if anyone is listening. */
    #if ORCHESTRATOR_DEBUG
    DebugPrint("%sParsed arguments:\n", debugHeader);
    if (hdfPath.empty())
    {
        DebugPrint("%s%sHardware description file path: Not specified.\n",
                   debugHeader, debugIndent);
    }
    else
    {
        DebugPrint("%s%sHardware description file path: %s\n",
                   debugHeader, debugIndent, hdfPath.c_str());
    }
    if (overrideHost.empty())
    {
        DebugPrint("%s%sOverride host: Not specified.\n",
                   debugHeader, debugIndent);
    }
    else
    {
        DebugPrint("%s%sOverride host: %s\n", debugHeader, debugIndent,
                   overrideHost.c_str());
    }
    if (useMotherships)
    {
        DebugPrint("%s%sMotherships: enabled\n", debugHeader, debugIndent);
    }
    else
    {
        DebugPrint("%s%sMotherships: disabled\n", debugHeader, debugIndent);
    }

    DebugPrint("%s\n", debugHeader);
    #endif

    /* If the default hardware description file path has a file there, and we
     * weren't passed a file explicitly, let's roll with the one we've
     * found.
     *
     * If the operator doesn't like this default, they can always load a new
     * one in the usual way, which will clobber the target. */
    if (hdfPath.empty() && file_exists(defaultHdfPath))
    {
        hdfPath = defaultHdfPath;
        DebugPrint("%sFound a hardware description file in the default "
                   "search location (%s). Using that one.\n",
                   debugHeader, hdfPath.c_str());
    }

    /* Read the input file, if supplied, and get a set of hosts we must launch
     * Mothership processes on. Don't bother if we're overriding, or if
     * motherships are disabled.
     *
     * On future work, we may want to make hosts a std::map<std::string, ENUM>,
     * so that you can launch specific processes with it from another input
     * file. Just a passing thought. */
    std::set<std::string> hosts;  /* May well remain empty. */
    if (useMotherships)
    {
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
            DebugPrint("%sIgnoring input file, and instead using the override "
                       "passed in as an argument.\n", debugHeader);
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
     * using motherships. */
    std::string binaryDir;
    std::map<std::string, std::string> deployedPaths;
    if (useMotherships)
    {
        if (DeployBinaries(&hosts, &deployedPaths) != 0) return 1;
    }

    /* Build the MPI command. */
    std::string command;
    DebugPrint("%sBuilding command...\n", debugHeader);
    BuildCommand(useMotherships, internalPath, overrideHost, batchPath,
                 hdfPath, &hosts, &deployedPaths, &command);

    /* Run the MPI command. Note we don't use call here, because we don't care
     * about the stdout, stderr, or returncode. */
    DebugPrint("%sRunning this command: %s\n", debugHeader, command.c_str());
    system(command.c_str());
    return 0;
}
}  /* End namespace */
