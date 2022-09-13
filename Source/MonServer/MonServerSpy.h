#ifndef __MonServerSpyH__H
#define __MonServerSpyH__H

/* MonServerSpy (or just "Spy") is a simple spy that writes out all messages
 * from the MonServer to a Monitor client (over a socket) to a series of files
 * in a directory defined by configuration. By default, Spy does nothing.
 *
 * MonServer owns one Spy. Root can request a MonServer to activate its Spy by
 * sending a:
 *
 *    Q::MONI,Q::SPY
 *
 * message to that MonServer. The MonServer will then call the Spy's `void
 * Toggle()` method, and set its output directory by calling bool
 * `SetSpyDir(std::string)` by introspecting the std::string stored at index
 * zero.
 *
 * The operator can toggle the activity of the spy by commanding:
 *
 *    moni /spy
 *
 * at the POETS prompt.
 *
 * Once the Spy has been enabled, each time the MonServer sends a PMsg_p to a
 * Monitor, that PMsg_p is introspected and written out. The directory to which
 * these writeouts occur is defined in configuration. By default, this
 * directory is defined in the `monserver_spy` field under the `default_paths`
 * heading in the Orchestrator configuration file, though this may be
 * overwritten on a per-session basis. */

#include <string>

#include "flat.h"
#include "OSFixes.hpp"
#include "Pglobals.h"
#include "PMsg_p.hpp"

class MonServerSpy
{
public:
    MonServerSpy();
    ~MonServerSpy();

    inline int operator()(PMsg_p* message){return Spy(message);}
    int Spy(PMsg_p*);

    inline std::string GetError(){return error;}
    inline bool IsEnabled(){return isSpyEnabled;}
    bool SetSpyDir(std::string);
    inline void Toggle(){isSpyEnabled = !isSpyEnabled;}

private:
    FILE* dumpMap;
    std::string dumpMapDate;
    unsigned dumpMapIndex;
    std::string error;
    bool isSpyDirNew;
    bool isSpyEnabled;
    std::string spyDir;

    void CheckCloseDumpMap();

    /* Miniature dumping methods, bespoke for each key permutation we
     * expect. */
    void DumpCommonBits(FILE*, PMsg_p*);
    void DumpDataBits(FILE*, PMsg_p*);
    void DumpMoniDeviAck(FILE*, PMsg_p*);
    void DumpMoniInjeAck(FILE*, PMsg_p*);
    void DumpMoniMothData(FILE*, PMsg_p*);
    void DumpMoniSoftData(FILE*, PMsg_p*);
};

#endif
