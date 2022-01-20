#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_APPDB_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_APPDB_H

/* Describes the applications container used by the Mothership to manage
 * applications. */

#include <map>
#include <fstream>
#include <string>

#include "AppInfo.h"
#include "OSFixes.hpp"

/* I'm lazy. */
typedef std::map<std::string, AppInfo>::iterator AppInfoIt;

class AppDB
{
public:
    std::map<std::string, AppInfo> appInfos;
    std::map<uint32_t, std::string> coreToApp;
    std::map<uint32_t, uint32_t> threadToCoreAddr;
    std::map<uint8_t, std::string> numberToApp;

    AppInfo* check_create_app(std::string, uint32_t, bool);
    AppInfo* check_create_app(std::string);
    bool check_defined_app(std::string);
    void recall_app(AppInfo*);
    void dump(std::ofstream*);
};

#endif
