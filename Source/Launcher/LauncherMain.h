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

#define INTERNAL_LIB_PATH "INTERNAL_LIB_PATH_PLACEHOLDER"

namespace Launcher
{
    const char* debugHeader = "[LAUNCHER] ";
    const char* debugIndent = "    ";
    const char* errorHeader = "[LAUNCHER] [ERROR] ";

    void BuildCommand(std::string overrideHost, std::set<std::string>* hosts,
                      std::string* command);
    int GetHosts(std::string hdfPath, std::set<std::string>* hosts);
    int Launch(int argc, char** argv);
}

#endif
