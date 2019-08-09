#ifndef __ORCHESTRATOR_SOURCE_LAUNCHER_LAUNCHERMAIN_H
#define __ORCHESTRATOR_SOURCE_LAUNCHER_LAUNCHERMAIN_H

#include "Cli.h"
#include "Debug.h"
#include "jnj.h"
#include "macros.h"

#include <set>
#include <string>
#include <map>

#include <stdlib.h>

namespace Launcher
{
    const char* debugHeader = "[LAUNCHER] ";
    const char* debugIndent = "    ";
    const char* errorHeader = "[LAUNCHER] [ERROR] ";
    const char* fileOnlyOnPoetsBox = "/local/ecad/setup-quartus17v0.bash";

    bool AreWeRunningOnAPoetsBox();
    void BuildCommand(bool useMotherships, std::string internalPath,
                      std::string overrideHost, std::set<std::string>* hosts,
                      std::string* command);
    int GetHosts(std::string hdfPath, std::set<std::string>* hosts);
    int Launch(int argc, char** argv);
}

#endif
