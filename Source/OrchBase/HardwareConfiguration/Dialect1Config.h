#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_DIALECT1CONFIG_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_DIALECT1CONFIG_H

/* Defines a intermediate configuration format for dialect 1-style files. This
   data structure holds configuration so that it can be deployed to a given
   engine and address format. */

#include <array>
#include <map>
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
    std::vector<unsigned> boardsInEngine;  /* NB: Number of boards must divide
                                              equally between the number of
                                              boxes. */
    bool boardsAsHypercube;
    float costExternalBox;

    /* Box properties */
    float costBoxBoard;
    float costBoardBoard;
    int supervisorMemory;

    /* Board properties */
    std::vector<unsigned> mailboxesInBoard;
    bool mailboxesAsHypercube;
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

private:
    /* Assignment and population methods used during deployment. */
    void assign_metadata_to_engine(PoetsEngine* engine);
    void assign_sizes_to_address_format(HardwareAddressFormat* format);
    AddressComponent flatten_address(std::vector<AddressComponent> address);
    void populate_boxes_evenly_with_boards(\
        std::map<AddressComponent, PoetsBox*>* boxMap,
        std::map<AddressComponent, PoetsBoard*>* boardMap);
    void populate_engine_with_boxes_and_their_costs(PoetsEngine* engine);
    void populate_map_with_boards(std::map<std::array<AddressComponent>,
                                  PoetsBoard*>* boardMap);

}

#endif
