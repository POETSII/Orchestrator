/* This source file defines Mothership methods responsible for provisioning the
 * API objects in supervisors.
 *
 * The "master" method is 'provision_supervisor_api', which binds the other
 * methods to function pointers in the API object.
 *
 * The other functions are all (namespaced) free fuctions. */

#include "Mothership.h"

namespace SuperAPIBindings {
    /* Posts a logserver message. */
    void post(Mothership* mship, std::string message)
    {
        mship->Post(527, message);
    }

    /* Sends a message to root, requesting that this application be stopped
     * (across all Motherships) */
    void stop_application(Mothership* mship, std::string appName)
    {
        PMsg_p message;
        message.Src(mship->Urank);
        message.Key(Q::MSHP, Q::REQ, Q::STOP);
        message.Put(0, &(appName));
        message.Tgt(mship->pPmap->U.Root);
        mship->Post(526, appName);
        mship->queue_mpi_message(&message);
    }
}

/* Define the supervisor's API functions and variables. Returns true on
 * successful load, and false otherwise. */
bool Mothership::provision_supervisor_api(std::string appName)
{
    SupervisorApi* api = superdb.get_supervisor_api(appName);
    if (api == PNULL) return false;
    api->mship = this;
    api->appName = appName;
    api->post = &SuperAPIBindings::post;
    api->stop_application = &SuperAPIBindings::stop_application;
    return true;
}
