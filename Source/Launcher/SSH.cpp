#include "SSH.h"

/* Runs an SSH command, returning the exit code.
 *
 * This function opens a single SSH session, and runs commands natively in the
 * shell configured in the SSH config file, falling back to the SHELL
 * environment variable on the host (usually /bin/bash).
 *
 * Beware, this method forks.
 *
 * Arguments:
 *
 * - host: host to connect to (see SSH.h).
 * - command: command to run, passed naively to the shell. Each command runs
 *   under the same SSH connection.
 * - stdout: string populated with the contents of the standard output.
 * - error: string populated with the contents of the standard error, or other
 *   connection errors.
 *
 * Returns the exit code of the called process. */
int SSH::call(std::string host, std::string command, std::string* stdout,
              std::string* stderr)
{
    stderr->clear();
    stdout->clear();

    /* Construct some pipes for reading stdout and stderr. */
    int stdoutPipe[2];
    int stderrPipe[2];
    pipe(stdoutPipe);
    pipe(stderrPipe);

    /* They should not block. */
    fcntl(stdoutPipe[0], F_SETFL, O_NONBLOCK);
    fcntl(stderrPipe[0], F_SETFL, O_NONBLOCK);

    /* Grab the executor PID for the examiner, so we can get the return
     * code. */
    pid_t pid = fork();

    /* Process executor */
    if (!pid)
    {
        /* Map pipes to stdout and stderr. */
        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stderrPipe[1], STDERR_FILENO);

        /* Close all descriptors - we won't need them where we're going. */
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        close(stderrPipe[0]);
        close(stderrPipe[1]);

        /* Run the command, supplanting our existing execution. */
        char* const args[] = {const_cast<char*>(SSH_COMMAND),
                              const_cast<char*>(host.c_str()),
                              const_cast<char*>("--"),
                              const_cast<char*>(command.c_str()), PNULL};
        execvp(SSH_COMMAND, args);  /* We die after this. */
        exit(errno);  /* But just in case we don't... */
    }

    /* The other process reads the result, and lives on to tell the tale. */

    /* Wait for it to finish, then get the exit code. */
    int status;
    do {
        waitpid(pid, &status, 0);
    } while (!WIFEXITED(status));
    int exitCode = WEXITSTATUS(status);

    /* Standard output */
    char buffer[STDOUTERR_BUFFER_SIZE];
    do {
        const ssize_t amountRead = read(stdoutPipe[0], buffer,
                                        STDOUTERR_BUFFER_SIZE);
        if (amountRead > 0)
        {
            stdout->append(buffer, amountRead);
        }
        else break; /* read returns -1 if nothing was there to read, and if the
                     * pipe is still open. */

    } while (errno == EAGAIN || errno == EINTR);

    /* Standard error */
    do {
        const ssize_t amountRead = read(stderrPipe[0], buffer,
                                        STDOUTERR_BUFFER_SIZE);
        if (amountRead > 0)
        {
            stderr->append(buffer, amountRead);
        }
        else break;

    } while (errno == EAGAIN || errno == EINTR);

    /* Close up shop. */
    close(stdoutPipe[0]);
    close(stdoutPipe[1]);
    close(stderrPipe[0]);
    close(stderrPipe[1]);
    return exitCode;
}

/* Deploys a directory to a host over SSH. Arguments:
 *
 * - host: host to connect to (see SSH.h).
 * - source: Path to the directory whose contents are to be copied.
 * - target: Path to the directory to populate on the host.
 *
 * Returns 0 on success, and 1 on failure. */

int SSH::deploy(std::string host, std::string source, std::string target)
{
    return 0;
}
