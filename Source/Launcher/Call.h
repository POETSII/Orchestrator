#ifndef __ORCHESTRATOR_SOURCE_LAUNCHER_COMMAND_H
#define __ORCHESTRATOR_SOURCE_LAUNCHER_COMMAND_H

/* Defines a method that allows you to perform system commands and retrieve
 * stdout/stderr/rc. Will certainly not work on Windows. */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <string>
#include <vector>

#include "OSFixes.hpp"

#define STDOUTERR_BUFFER_SIZE 4096

namespace Call
{
    int call(std::vector<std::string> command, std::string* stdout,
             std::string* stderr);
}

#endif
