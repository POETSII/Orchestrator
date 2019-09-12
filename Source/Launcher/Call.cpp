#include "Call.h"

/* Runs a command, returning the exit code. Blocks until the command is
 * complete.
 *
 * This method uses the fork-pipe pattern for executing arbitrary commands. In
 * brief, this process is forked into a caller and a reader process; the caller
 * process hooks up stdout and stderr pipes, and runs a command using execvp
 * (then closes), whereas the reader listens on the other side of the pipe to
 * gather stdout and stderr.
 *
 * Arguments:
 *
 * - command: command to run, passed naively into the shell.
 * - stdout: string populated with the contents of the standard output.
 * - stderr: string populated with the contents of the standard error, or other
 *   connection errors.
 *
 * Returns the exit code of the called process. */
int Call::call(std::vector<std::string> command, std::string* stdout,
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

        /* Run the command, supplanting our existing execution. execvp has
         * the somewhat pathological signature of
         *
         *   execvp(const char* file, char *const argv),
         *
         * so we need to convert our command into a char *const, with a NULL at
         * the end. */

        /* argv must be dynamic, because its size is a function of command. We
         * also need to include space for a NULL terminator.
         *
         * Note that we don't delete argv - this is okay, because execvp
         * replaces the entire process anyway. */
        const char** argv = new const char*[command.size() + 1];

        /* Populate argv from command. */
        for (unsigned it=0; it<command.size(); it++)
        {
            argv[it] = command[it].c_str();
        }

        /* Add NULL terminator. */
        argv[command.size()] = PNULL;
        execvp(command[0].c_str(), (char**) argv);  /* We end after this. */
    }

    /* The other process reads the result, and lives on to tell the tale. */

    /* Wait for it to finish, then get the exit code. */
    int status;
    do {
        waitpid(pid, &status, 0);
    } while (!WIFEXITED(status));
    int exitCode = WEXITSTATUS(status);

    /* Read all of standard output, then all of standard error. */
    char buffer[STDOUTERR_BUFFER_SIZE];
    std::vector<std::pair<int, std::string*> > outChannels;  /* pipe and str */
    std::vector<std::pair<int, std::string*> >::iterator outChannel;
    outChannels.push_back(std::make_pair(stdoutPipe[0], stdout));
    outChannels.push_back(std::make_pair(stderrPipe[0], stderr));

    for (outChannel=outChannels.begin();
         outChannel!=outChannels.end(); outChannel++)
    {
        do {
            /* Read in chunks. */
            const ssize_t amountRead = read(outChannel->first, buffer,
                                            STDOUTERR_BUFFER_SIZE);
            if (amountRead > 0)
            {
                outChannel->second->append(buffer, amountRead);
            }
            else break; /* read returns -1 if nothing was there to read, and if
                         * the pipe is still open. */

        } while (errno == EAGAIN || errno == EINTR);
    }

    /* Close up shop. */
    close(stdoutPipe[0]);
    close(stdoutPipe[1]);
    close(stderrPipe[0]);
    close(stderrPipe[1]);
    return exitCode;
}
