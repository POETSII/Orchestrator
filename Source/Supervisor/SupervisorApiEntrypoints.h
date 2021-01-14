#ifndef __SupervisorApiEntrypointsH__H
#define __SupervisorApiEntrypointsH__H

/* This header contains namespaced functions that a supervisor device in an
 * application can call, which piggyback off functions defined in the
 * SupervisorApi class. */

namespace Super {
    void post(std::string message)
    {
        (Supervisor::__api.post)(Supervisor::__api.mship, message);
    }

    void stop_application()
    {
        (Supervisor::__api.stop_application)
            (Supervisor::__api.mship, Supervisor::__api.appName);
    }
}

#endif
