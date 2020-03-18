/* This source file defined methods that the Mothership uses to transition
 * applications through states. */

#include "Mothership.h"

void Mothership::initialise_application(AppInfo* app)
{
    printf("Initialising application '%s'!\n", app->name.c_str());
}

void Mothership::run_application(AppInfo* app)
{
    printf("Running application '%s'!\n", app->name.c_str());
}

void Mothership::stop_application(AppInfo* app)
{
    printf("Stopping application '%s'!\n", app->name.c_str());
}

void Mothership::recall_application(AppInfo* app)
{
    printf("Recalling application '%s'!\n", app->name.c_str());
}
