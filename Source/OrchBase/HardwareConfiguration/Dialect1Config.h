#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_DIALECT1CONFIG_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_DIALECT1CONFIG_H

/* Defines a intermediate configuration format for dialect 1-style files. This
   data structure holds configuration so that it can be deployed to a given
   engine and address format. */

#include <vector>
#include "dfprintf.h"
#include "HardwareModel.h"

class Dialect1Config
{
public:

    void deploy(PoetsEngine* engine, HardwareAddressFormat* addressFormat);

    /* Items in the header */
    std::string author;
    long datetime;
    std::string version;
    std::string fileOrigin;

    /* Packet address format sizes */
    unsigned boxWordLength;
    std::vector<unsigned> boardWordLengths;
    std::vector<unsigned> mailboxWordLengths;
    unsigned coreWordLength;
    unsigned threadWordLength;

    /* Engine properties */
    int boxesInEngine;
    std::vector boardsInEngine;
    float costExternalBox;

    /* Box properties */
    float costBoxBoard;
    float costBoardBoard;
    int supervisorMemory;

    /* Board properties */
    std::vector mailboxesInBoard;
    float costBoardMailbox;
    float costMailboxMailbox;
    int supervisorMemory;
    int dram;

    /* Mailbox properties */
    int coresInMailbox;
    float costMailboxCore;
    float costCoreCore;

    /* Core properties */
    int threadsInCore;
    float coreThreadCost;
    float threadThreadCost;
    int dataMemory;
    int instructionMemory;
}

#endif
