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
    void post(Mothership* mship, std::string appName, std::string message)
    {
        mship->Post(527, appName, message);
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

    /* Creates a specific, guaranteed-empty output directory, with an optional
     * suffix. Directories are of the form:
     *
     *   /home/$USER/<REMOTE_OUTDIR>/<APPNAME>/<DATETIME_ISO8601><SUFFIX>
     *
     * where <REMOTE_OUTDIR> is defined in the Orchestrator configuration file,
     * and <APPNAME> is the combined application-graphinstance name with the
     * colons substituted out for underscores. Directory (and parents) are
     * created if they don't exist. Uses GNU-isms and Linux-isms.
     *
     * The directory may change between successive calls from the same
     * supervisor device - considering storing it in Supervisor state (in your
     * application) if you want it to remain constant.
     *
     * Returns the path to the directory on success, and an empty string on
     * failure. Also posts on failure. */
    std::string get_output_directory(Mothership* mship, std::string appName,
                                     std::string suffix)
    {
        /* Remove :s from the application name. */
        std::string sanitisedAppName = appName;
        std::replace(sanitisedAppName.begin(), sanitisedAppName.end(),
                     ':', '_');

        /* Get datetime */
        time_t native;
        time(&native);
        char time[sizeof "YYYY_MM_DDTHH_MM_SS"];
        strftime(time, sizeof time, "%Y_%m_%dT%H_%M_%S", localtime(&native));

        /* Construct path (no filesystem library). */
        std::string userDir = \
            std::string(getenv("HOME")) + "/" +
            mship->userOutDir + "/" +
            sanitisedAppName + "/" +
            time + suffix;

        /* Attempt to make it (fairly safe to assume it doesn't already
         * exist, given the timestamp... NFS eat your heart out). */
        if(system((MAKEDIR + " --parents " + userDir).c_str()))
        {
            mship->Post(528, userDir, appName,
                        OSFixes::getSysErrorString(errno));
            return "";
        }

        /* Now that we have it, attempt to write to a file within it. */
        std::ofstream testFile;
        std::string testFileName = "test_file.txt";
        testFile.open((userDir + "/" + testFileName).c_str());
        if (testFile.fail())  /* Best efforts */
        {
            mship->Post(528, userDir, appName,
                        "Could not open a file for writing in the directory "
                        "(check directory permissions and ownership).");
            if(system((REMOVEDIR + " " + userDir + "/" +
                       testFileName).c_str()))
            {
                mship->Post(528, userDir, appName,
                            "While cleaning up from the previous error, could "
                            "not remove the directory.");
            }
            return "";
        }

        testFile << "This file is transient. If it persists, speak to an "
            "Orchestrator developer." << std::endl;
        testFile.close();

        /* Remove the transient file. */
        if(system((REMOVEDIR + " " + userDir + "/" + testFileName).c_str()))
        {
            mship->Post(528, userDir, appName,
                        "Could not remove a file opened as a test "
                        "(check directory permissions and ownership).");

            return "";
        }

        return userDir;
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
    api->get_output_directory = &SuperAPIBindings::get_output_directory;
    api->post = &SuperAPIBindings::post;
    api->stop_application = &SuperAPIBindings::stop_application;
    return true;
}
