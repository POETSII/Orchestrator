#include "AppInfo.h"

AppInfo::AppInfo(std::string name, uint32_t distCountExpected):
    name(name),
    distCountExpected(distCountExpected)
{
    distCountCurrent = 0;
    state = UNDERDEFINED;
}

/* Returns true if a state transition is possible for this application, and
 * false otherwise. A state transition is possible if the application has been
 * commanded to move to its next state, and it is ready to do so. */
bool AppInfo::should_we_continue()
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
 * stream to dump to. */
void AppInfo::dump(std::ofstream* stream)
{
    bool anyStagedCommands = false;

    *stream << "AppInfo dump for application \"" << name << "\":\n";

    *stream << "Current state: ";
    /* Bloody C enums. */
    if (state == UNDERDEFINED) *stream << "UNDERDEFINED";
    if (state == DEFINED) *stream << "DEFINED";
    if (state == LOADING) *stream << "LOADING";
    if (state == READY) *stream << "READY";
    if (state == RUNNING) *stream << "RUNNING";
    if (state == STOPPING) *stream << "STOPPING";
    if (state == STOPPED) *stream << "STOPPED";
    if (state == BROKEN) *stream << "BROKEN";
    *stream << "\n";

    *stream << "Staged commands: ";
    if (is_init_staged())
    {
        *stream << "INIT ";
        anyStagedCommands = true;
    }
    if (is_run_staged())
    {
        *stream << "RUN ";
        anyStagedCommands = true;
    }
    if (is_stop_staged())
    {
        *stream << "STOP ";
        anyStagedCommands = true;
    }
    if (is_recl_staged())
    {
        *stream << "RECL ";
        anyStagedCommands = true;
    }
    if (!anyStagedCommands) *stream << "(none)";
    *stream << "\n";

    *stream << "Cores loaded: " << distCountCurrent << "/" <<
        distCountExpected << "\n";

    if (!coreInfos.empty())
    {
        *stream << "Known core information:\n";
        std::map<uint32_t, CoreInfo>::iterator coreInfoIt;
        for (coreInfoIt = coreInfos.begin(); coreInfoIt != coreInfos.end();
             coreInfoIt++)
        {
            CoreInfo info = coreInfoIt->second;
            *stream << "Core " << std::hex << coreInfoIt->first << ":\n";
            *stream << "  Threads reported in: " << info.numThreadsCurrent
                       << "/" << info.numThreadsExpected << "\n";
            *stream << "  Data Binary: " << info.dataPath << "\n";
            *stream << "  Instruction Binary: " << info.codePath << "\n";
        }
    }
    else
    {
        *stream << "No core information.\n";
    }
}
