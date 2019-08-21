#ifndef __ORCHESTRATOR_SOURCE_LAUNCHER_LAUNCHERMAIN_H
#define __ORCHESTRATOR_SOURCE_LAUNCHER_LAUNCHERMAIN_H

#include "Cli.h"
#include "Debug.h"
#include "jnj.h"
#include "macros.h"
#include "OSFixes.hpp"
#include "RootArgs.h"
#include "SSH.h"

#include <set>
#include <sstream>
#include <string>
#include <map>

#include <stdlib.h>

namespace Launcher
{
    const char* debugHeader = "[LAUNCHER] ";
    const char* debugIndent = "    ";
    const char* deployDir = ".orchestrator/launcher";  /* No trailing slash,
                                                        * relative to home. */
    const char* errorHeader = "[LAUNCHER] [ERROR] ";
    const char* fileOnlyOnPoetsBox = "/local/ecad/setup-quartus17v0.bash";

    const char* execClock = "rtcl";
    const char* execLogserver = "logserver";
    const char* execMothership = "mothership";
    const char* execRoot = "root";

    bool AreWeRunningOnAPoetsBox();
    void BuildCommand(bool useMotherships, std::string internalPath,
                      std::string overrideHost, std::string batchPath,
                      std::string hdfPath,
                      std::set<std::string>* mothershipHosts,
                      std::map<std::string, std::string>* executablePaths,
                      std::string* command);
    int DeployBinaries(std::set<std::string>* hosts,
                       std::map<std::string, std::string>* paths);
    int GetHosts(std::string hdfPath, std::set<std::string>* hosts);
    int Launch(int argc, char** argv);
}

#endif
