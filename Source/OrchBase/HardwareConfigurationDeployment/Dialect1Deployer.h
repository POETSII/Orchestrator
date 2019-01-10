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

/* Simple container for storing items with their flattened addresses, so they
   can be indexed in maps by the hierarchical addresses. */
template <typename PoetsItem>
struct itemAndAddress {
    PoetsItem poetsItem;  /* E.g. PoetsBoard*, or PoetsMailbox*. */
    AddressComponent address;
};

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
    unsigned boxesInEngine;
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
    unsigned coresInMailbox;
    float costMailboxCore;
    float costCoreCore;

    /* Core properties */
    unsigned threadsInCore;
    float coreThreadCost;
    float threadThreadCost;
    unsigned dataMemory;
    unsigned instructionMemory;

private:
    /* Assignment and population methods used during deployment. */
    void assign_metadata_to_engine(PoetsEngine* engine);
    void assign_sizes_to_address_format(HardwareAddressFormat* format);
    void connect_boards_in_engine(
        PoetsEngine* engine,
        std::map<MultiAddressComponent,
                 itemAndAddress<PoetsBoard*>*>* boardMap);
    AddressComponent flatten_address(MultiAddressComponent address,
                                     std::vector<unsigned> wordLengths);
    void populate_boxes_evenly_with_boards(
        std::map<AddressComponent, PoetsBox*>* boxMap,
        std::map<MultiAddressComponent,
                 itemAndAddress<PoetsBoard*>*>* boardMap);
    void populate_engine_with_boxes_and_their_costs(PoetsEngine* engine);
    void populate_map_with_boards(
        std::map<MultiAddressComponent,
                 itemAndAddress<PoetsBoard*>*>* boardMap);
};

#endif
