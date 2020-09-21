#include "SSH.h"

/* Runs an SSH command, returning the exit code.
 *
 * This function opens a single SSH session, and runs commands natively in the
 * shell configured in the SSH config file, falling back to the SHELL
 * environment variable on the host (usually /bin/bash).
 *
 * Beware, all of the caveats with Call::call apply here.
 *
 * Arguments:
 *
 * - host: host to connect to (see SSH.h).
 * - command: command to run, passed naively to the shell.
 * - stdout: string populated with the contents of the standard output.
 * - error: string populated with the contents of the standard error, or other
 *   connection errors.
 *
 * Returns the exit code of the called process, or -1 (and populates errno). */
int SSH::call(std::string host, std::string command, std::string* stdout,
              std::string* stderr)
{
    /* Build the full command. */
    std::vector<std::string> sshCommand;
    sshCommand.push_back(SSH_COMMAND);
    sshCommand.push_back(host);
    sshCommand.push_back("--");
    sshCommand.push_back(command);

    /* Run it. */
    return Call::call(sshCommand, stdout, stderr);
}

/* Deploys a directory to a host over SSH with many caveats. Arguments:
 *
 * - host: host to connect to (see SSH.h).
 * - source: Path to the directory whose contents are to be copied, assumed to
 *   exist. Must not have a trailing slash.
 * - target: Path to the directory to populate on the host, assumed to
 *   exist. Must not have a trailing slash.
 *
 * Returns 0 on success, 1 on failure, and -1 on system failure (and populates
 * errno). */
int SSH::deploy_directory(std::string host, std::string source,
                          std::string target, std::string* stdout,
                          std::string* stderr)
{
    /* Build the command */
    std::vector<std::string> scpCommand;
    scpCommand.push_back(SCP_COMMAND);
    scpCommand.push_back(RECURSIVE);
    scpCommand.push_back(source);
    scpCommand.push_back(host + ":~/" + target);

    /* Run it. */
    return Call::call(scpCommand, stdout, stderr);
}
