#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_DIALECT1DEPLOYER_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_DIALECT1DEPLOYER_H

/* Defines how dialect-1 style files are deployed to define PoetsEngine
   configurations. This data structure holds configuration information, so that
   it can be deployed to a given engine and address format. */

#include <map>
#include <numeric>
#include <vector>

#include "dfprintf.h"
#include "HardwareModel.h"

/* Address components that are multidimensional (to support hypercubes), that
   have not been flattened, are vectors of address components. */
typedef std::vector<AddressComponent> MultiAddressComponent;

class Dialect1Deployer
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
    std::vector<bool> boardHypercubePeriodicity;
    float costExternalBox;

    /* Box properties */
    float costBoxBoard;
    float costBoardBoard;
    int boxSupervisorMemory;

    /* Board properties */
    std::vector<unsigned> mailboxesInBoard;
    bool mailboxesAsHypercube;
    std::vector<bool> mailboxHypercubePeriodicity;
    float costBoardMailbox;
    float costMailboxMailbox;
    int boardSupervisorMemory;
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
    void connect_boards_in_engine(
        PoetsEngine* engine,
        std::map<MultiAddressComponent, PoetsBoard*>* boardMap,
        std::map<MultiAddressComponent, AddressComponent>* addressMap);
    AddressComponent flatten_address(MultiAddressComponent address);
    void populate_boxes_evenly_with_boards(
        std::map<AddressComponent, PoetsBox*>* boxMap,
        std::map<MultiAddressComponent, PoetsBoard*>* boardMap,
        std::map<MultiAddressComponent, AddressComponent>* addressMap);
    void populate_engine_with_boxes_and_their_costs(PoetsEngine* engine);
    void populate_map_with_boards(
        std::map<MultiAddressComponent, PoetsBoard*>* boardMap,
        std::map<MultiAddressComponent, AddressComponent>* addressMap);
}

#endif
