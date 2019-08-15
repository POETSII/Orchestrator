#ifndef __ORCHESTRATOR_SOURCE_LAUNCHER_SSH_H
#define __ORCHESTRATOR_SOURCE_LAUNCHER_SSH_H

/* A simple library to perform SSH commands. Probably won't work on Windows.
 *
 * The "host" argument in each of the SSH methods corresponds to a target that
 * can be reached over SSH without user intervention. This usually means that:
 *
 * - A "User" is defined in the SSH configuration for this host, if it is
 *   different from the user on this machine.
 *
 * - There is a secure keypair that permits the connection, or that an
 *   SSH-agent of some kind is running.
 *
 * - The host is known (i.e. is registered in known_hosts).
 *
 * The "right way" to do this is via something like libssh, but:
 *
 * - It's going to take me some time for me to learn how to use it.
 *
 * - The C++ API is not exactly well documented.
 *
 * - The functionality provided by this library is going to be replaced by a
 *   NFS eventually anyway. */

#include "OSFixes.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <set>
#include <string>

#define STDOUTERR_BUFFER_SIZE 4096
#define SSH_COMMAND "/usr/bin/ssh"

namespace SSH
{
    int call(std::string host, std::string command, std::string* stdout,
             std::string* stderr);
    int deploy(std::string host, std::string source, std::string target);
}

#endif
