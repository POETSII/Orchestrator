/* This source file defines Mothership methods responsible for provisioning the
 * API objects in supervisors.
 *
 * The "master" method is 'provision_supervisor_api', which binds the other
 * methods to function pointers in the API object. */

#include "Mothership.h"

/* Define the supervisor's API functions and variables. Returns true on
 * successful load, and false otherwise. */
bool Mothership::provision_supervisor_api(std::string appName)
{
    SupervisorApi* api = superdb.get_supervisor_api(appName);
    if (api == PNULL) return false;
    api->appName = appName;
    api->stop_application = &Mothership::supervisor_api_stop_application;
    return true;
}

/* Sends a message to root, requesting that this application be stopped (across
 * all Motherships) */
void Mothership::supervisor_api_stop_application(std::string appName)
{
    PMsg_p message;
    message.Src(Urank);
    message.Key(Q::MSHP, Q::REQ, Q::RUN);
    message.Put<std::string>(0, &(appName));
    message.Tgt(pPmap->U.Root);
    queue_mpi_message(&message);
}
