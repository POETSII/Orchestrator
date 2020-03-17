#include "AppInfo.h"

/* This constructor is used by SPEC messages. */
AppInfo::AppInfo(std::string name, uint32_t distCountExpected):
    name(name),
    distCountExpected(distCountExpected)
{
    distCountCurrent = 0;
    state = UNDERDEFINED;
}

/* This constructor is used by non-SPEC messages. The zero in distCountExpected
 * is a special value (see the documentation). */
AppInfo::AppInfo(std::string name):
    name(name)
{
    distCountExpected = 0;
    distCountCurrent = 0;
    state = UNDERDEFINED;
}

/* Checks whether this application, is now fully defined, given that it's state
 * is UNDERDEFINED. If so, update the state to DEFINED. If the state is not
 * UNDERDEFINED, this does nothing. Returns true if the state is not
 * UNDERDEFINED or BROKEN, and false otherwise. */
bool AppInfo::check_update_defined_state()
{
    /* If the task is underdefined, but we've received all dist messages and a
     * spec message, update the state to DEFINED. */
    if (state == UNDERDEFINED)
    {
        if (distCountExpected > 0 and distCountExpected == distCountCurrent)
        {
            state = DEFINED;
        }
    }

    /* Return according to state. */
    return not(state == UNDERDEFINED or state == BROKEN);
}

/* Returns the current state of the application as a string, inelegantly
 * (because we're working with C++98 enumerated types...). */
std::string AppInfo::get_state_colloquial()
{
    if (state == UNDERDEFINED) return "UNDERDEFINED";
    if (state == DEFINED) return "DEFINED";
    if (state == LOADING) return "LOADING";
    if (state == READY) return "READY";
    if (state == RUNNING) return "RUNNING";
    if (state == STOPPING) return "STOPPING";
    if (state == STOPPED) return "STOPPED";
    if (state == BROKEN) return "BROKEN";
    return "UNKNOWN";
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

/* Returns true if a recall state transition is possible for this application,
 * and false otherwise. This transition is only possible if the application has
 * been recalled from a message. */
bool AppInfo::should_we_recall()
{
    if (state == DEFINED or
        state == READY or
        state == RUNNING or
        state == STOPPED)
    {
        return is_recl_staged();
    }
    return false;
}

/* Prints a bunch of diagnostic information. Obviously. The argument is the
 * stream to dump to. */
void AppInfo::dump(std::ofstream* stream)
{
    bool anyStagedCommands = false;

    *stream << "AppInfo dump for application \"" << name << "\":\n";

    *stream << "Current state: " << get_state_colloquial() << "\n";

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
