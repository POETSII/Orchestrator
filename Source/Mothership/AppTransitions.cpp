/* This source file defined methods that the Mothership uses to transition
 * applications through states. */

#include "Mothership.h"

void initialise_application(AppInfo* app)
{
    printf("Initialising application '%s'!\n", app->name.c_str());
}

void run_application(AppInfo* app)
{
    printf("Running application '%s'!\n", app->name.c_str());
}

void stop_application(AppInfo* app)
{
    printf("Stopping application '%s'!\n", app->name.c_str());
}

void recall_application(AppInfo* app)
{
    printf("Recalling application '%s'!\n", app->name.c_str());
}
