#ifndef __SupervisorApiEntrypointsH__H
#define __SupervisorApiEntrypointsH__H

/* This header contains namespaced functions that a supervisor device in an
 * application can call, which piggyback off functions defined in the
 * SupervisorApi class. */

namespace Super {
    std::string get_output_directory(std::string suffix="")
    {
        return (Supervisor::__api.get_output_directory)
            (Supervisor::__api.mship,
             Supervisor::__api.appName,
             suffix);
    }

    void post(std::string message)
    {
        (Supervisor::__api.post)(Supervisor::__api.mship,
                                 Supervisor::__api.appName,
                                 message);
    }

    void stop_application()
    {
        (Supervisor::__api.stop_application)(Supervisor::__api.mship,
                                             Supervisor::__api.appName);
    }
}

#endif
