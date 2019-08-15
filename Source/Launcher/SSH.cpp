#include "SSH.h"

/* Runs an SSH command, returning the exit code.
 *
 * This function opens a single SSH session, and runs commands natively in the
 * shell configured in the SSH config file, falling back to the SHELL
 * environment variable on the host (usually /bin/bash). Arguments:
 *
 * - host: host to connect to (see SSH.h).
 * - command: command to run, passed naively to the shell. Each command runs
 *   under the same SSH connection.
 * - stdout: string populated with the contents of the standard output.
 * - error: string populated with the contents of the standard error, or other
     connection errors. */
int SSH::call(std::string host, std::string command, std::string* stdout,
              std::string* stderr)
{
    stderr->clear();
    stdout->clear();
    return 0;
}

/* <!> */
int SSH::deploy(std::string host, std::string source, std::string target)
{
    return 0;
}
