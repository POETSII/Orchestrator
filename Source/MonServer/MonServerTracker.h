#ifndef __MonServerTrackerH__H
#define __MonServerTrackerH__H

/* MonServerTracker, if enabled, writes outgoing data messages to a series of
 * files in a directory defined by configuration. The interaction with this
 * tracker is very similar to that of the MonServerSpy - it is activated by a
 * Q::MONI,Q::TRAC message, which can be triggered by the `moni /track` command
 * at the POETS prompt.
 *
 * Once enabled, each incoming data packet is recorded as per the Volume VI
 * specification. Packet "sequences" are identified by a shared UUID
 * (<int>(2)). Each sequence has a file pointer associated with it, which is
 * stored in the `files` map, using its UUID as a key.
 *
 * As with Volume VI, we assume the signature field (<std::string>(0)) remains
 * constant for a given sequence - if not, your data will become nonsensical.
 *
 * Files are only closed when the MonServerTracker is destroyed. This has
 * obvious problems. Tempus fugit, my fair engineer.
 */

#include <stdio.h>
#include <string>

#include "PMsg_p.hpp"

class MonServerTracker
{
public:
    MonServerTracker();
    ~MonServerTracker();

    inline int operator()(PMsg_p* message){return Track(message);}
    int Track(PMsg_p*);

    inline std::string GetError(){return error;}
    inline bool IsEnabled(){return isTrackerEnabled;}
    bool SetTrackerDir(std::string);
    inline void Toggle(){isTrackerEnabled = !isTrackerEnabled;}

private:
    std::string error;
    std::map<int, FILE*> files;
    bool isTrackerDirNew;
    bool isTrackerEnabled;
    std::string trackerDir;
};

#endif
