#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_APPINFO_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_APPINFO_H

/* Describes the states of applications that this Mothership knows about, in
 * addition to their loading and "distribution" progress. See the Mothership
 * documentation for further information on application loading, deployment,
 * and removal. */

#include <map>
#include <fstream>
#include <set>
#include <string>

class CoreInfo;

#include "CoreInfo.h"
#include "OSFixes.hpp"

/* An application can be in one of these states at a time (these are defined
 * formally in the Mothership documentation). Note that, as far as the
 * Mothership is concerned, applications that have been RECLd
 * (recalled/removed) do not have an AppInfo object in the AppDB. */
enum AppState{UNDERDEFINED,  /* We're still receiving DIST messages. */
              DEFINED,       /* All DIST messages have been received. */
              LOADING,       /* Waiting for cores to acknowledge loading. */
              READY,         /* All cores have acknowledged loading. */
              RUNNING,       /* ...really? */
              STOPPING,      /* Waiting for cores to acknowledge stopping. */
              STOPPED,       /* ...you try my patience. */
              BROKEN};       /* ...I mean, seriously. */

/* pendingCommands is used as a series of booleans, where each bit represents a
 * command. If in doubt, RTFM, and test. */
#define STAGE_INIT_BIT 0
#define STAGE_RUN_BIT 1
#define STAGE_STOP_BIT 2
#define STAGE_RECL_BIT 3

class AppInfo
{
public:
    AppInfo(std::string nameArg, uint32_t distCountExpected);

    std::string name;
    uint32_t distCountExpected;
    uint32_t distCountCurrent;
    AppState state;
    std::map<uint32_t, CoreInfo> coreInfos;
    std::set<uint32_t> coresLoaded;

    bool should_we_continue();

    inline void stage_init(){stage_command(STAGE_INIT_BIT);};
    inline void stage_run(){stage_command(STAGE_RUN_BIT);};
    inline void stage_stop(){stage_command(STAGE_STOP_BIT);};
    inline void stage_recl(){stage_command(STAGE_RECL_BIT);};

    inline bool is_init_staged(){return is_command_staged(STAGE_INIT_BIT);};
    inline bool is_run_staged(){return is_command_staged(STAGE_RUN_BIT);};
    inline bool is_stop_staged(){return is_command_staged(STAGE_STOP_BIT);};
    inline bool is_recl_staged(){return is_command_staged(STAGE_RECL_BIT);};

    void dump(std::ofstream*);

private:
    uint8_t pendingCommands;
    inline void stage_command(uint8_t bit){pendingCommands |= 1 << bit;};
    inline bool is_command_staged(uint8_t bit)
        {return pendingCommands & 1 << bit;};
};

#endif
