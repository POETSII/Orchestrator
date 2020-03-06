#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_APPDB_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_APPDB_H

/* Describes the applications container used by the Mothership to manage
 * applications. */

#include <map>
#include <ofstream>
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

    AppInfo* check_create_app(std::string);
    bool check_defined_app(std::string);
    void dump(ofstream*);
};

#endif
