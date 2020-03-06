#include "AppInfo.h"

/* Returns true if a state transition is possible for this application, and
 * false otherwise. A state transition is possible if the application has been
 * commanded to move to its next state, and it is ready to do so. */
bool AppInfo::continue()
{
    /* Only continue under certain application states. */
    if (state == DEFINED or
        state == READY or
        state == RUNNING or
        state == STOPPED)
    {
        /* RECL takes precedence. */
        if (is_recl_staged()) return true;

        /* If defined and we're waiting to INIT... */
        if (state == DEFINED and is_init_staged()) return true;

        /* If ready and we're waiting to RUN... */
        if (state == READY and is_run_staged()) return true;

        /* If running and we're waiting to STOP... */
        if (state == RUNNING and is_stop_staged()) return true;
    }

    /* Otherwise, we out yo. */
    return false;
}

/* Prints a bunch of diagnostic information. Obviously. The argument is the
 * path to dump to (passed directly to ofstream::open), in clobber mode. */
void AppInfo::dump(std::string path)
{
    ofstream dumpStream;
    bool anyStagedCommands = false;

    dumpStream.open(path);
    dumpStream << "AppInfo dump for application \"" << name << "\":\n";

    dumpStream << "Current state: ";
    /* Bloody C enums. */
    if (state == UNDERDEFINED) dumpStream << "UNDERDEFINED";
    if (state == DEFINED) dumpStream << "DEFINED";
    if (state == LOADING) dumpStream << "LOADING";
    if (state == READY) dumpStream << "READY";
    if (state == RUNNING) dumpStream << "RUNNING";
    if (state == STOPPING) dumpStream << "STOPPING";
    if (state == STOPPED) dumpStream << "STOPPED";
    if (state == BROKEN) dumpStream << "BROKEN";
    dumpStream << "\n";

    dumpStream << "Staged commands: ";
    if (is_init_staged())
    {
        dumpStream << "INIT ";
        anyStagedCommands = true;
    }
    if (is_run_staged())
    {
        dumpStream << "RUN ";
        anyStagedCommands = true;
    }
    if (is_stop_staged())
    {
        dumpStream << "STOP ";
        anyStagedCommands = true;
    }
    if (is_recl_staged())
    {
        dumpStream << "RECL ";
        anyStagedCommands = true;
    }
    if (!anyStagedCommands) dumpStream << "(none)";
    dumpStream << "\n";

    dumpStream << "Cores loaded: " << distCountCurrent << "/" <<
        distCountExpected << "\n";

    if (!coreInfos.empty())
    {
        dumpStream << "Known core information:\n";
        std::map<uint32_t, coreInfo>::iterator coreInfoIt;
        for (coreInfoIt = coreInfos.begin(); coreInfoIt != coreInfos.end();
             coreInfoIt++)
        {
            coreInfo info = coreInfoIt->second;
            dumpStream << "Core " << std::hex << coreInfoIt->first << ":\n";
            dumpStream << "  Threads reported in: " << info.numThreadsCurrent
                       << "/" << info.numThreadsExpected << "\n";
            dumpStream << "  Data Binary: " << info.dataPath << "\n";
            dumpStream << "  Instruction Binary: " << info.codePath << "\n";
        }
    }
    else
    {
        dumpStream << "No core information.\n");
    }

    dumpStream.close();
}
