#ifndef __ORCHESTRATOR_SOURCE_LAUNCHER_SSH_H
#define __ORCHESTRATOR_SOURCE_LAUNCHER_SSH_H

/* A simple library to perform SSH commands. */

#include <set>
#include <string>

namespace SSH
{
    int call(std::string host, std::string command, std::string* stdout,
             std::string* stderr); // <!> Fork and pipes!
    int deploy(std::string host, std::string source, std::string target);
    int mktemp(std::set<std::string>* hosts, std::string prefix,
               std::string* whatItsCalled);
}

#endif
