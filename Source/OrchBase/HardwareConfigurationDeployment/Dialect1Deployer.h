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
   have not been flattened, are vectors of address components. A vector is used
   over a std::array here to make the reduction operation in flatten_address
   easier to write. */
typedef std::vector<AddressComponent> MultiAddressComponent;

/* Simple container for storing items with their flattened addresses, so they
   can be indexed in maps by the hierarchical addresses. */
template <typename PoetsItem>
struct itemAndAddress {
    PoetsItem poetsItem;  /* E.g. PoetsBoard*, or PoetsMailbox*. */
    AddressComponent address;
};

/* Types for private maps, for readability. */
typedef std::map<MultiAddressComponent, itemAndAddress<PoetsBoard*>*> BoardMap;
typedef std::map<MultiAddressComponent, itemAndAddress<PoetsMailbox*>*>
    MailboxMap;

class Dialect1Deployer
{
public:

    Dialect1Deployer();

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
    unsigned boxSupervisorMemory;

    /* Board properties */
    std::vector<unsigned> mailboxesInBoard;
    bool mailboxesAsHypercube;
    std::vector<bool> mailboxHypercubePeriodicity;
    float costBoardMailbox;
    float costMailboxMailbox;
    unsigned boardSupervisorMemory;
    unsigned dram;

    /* Mailbox properties */
    unsigned coresInMailbox;
    float costMailboxCore;
    float costCoreCore;

    /* Core properties */
    unsigned threadsInCore;
    float costCoreThread;
    float costThreadThread;
    unsigned dataMemory;
    unsigned instructionMemory;

private:
    /* Item factories and their indeces. */
    unsigned createdBoardIndex;
    PoetsBoard* create_board();

    unsigned createdMailboxIndex;
    PoetsMailbox* create_mailbox();

    unsigned createdCoreIndex;
    PoetsCore* create_core();

    unsigned createdThreadIndex;
    PoetsThread* create_thread();

    /* Maps for staging POETS items during deployment, required to persist
       connectivity information. These need to be maps so that we can determine
       the neighbours of an item given its hierarchical address. */
    BoardMap boardMap;
    MailboxMap mailboxMap;

    void free_items_in_board_map();
    void free_items_in_mailbox_map();

    /* Vectors for holding pointers to created items, for code simplification
       (means we don't need to iterate through every board-mailbox to find
       every core, for example). We don't need one of these for boards, because
       boardMap stores the content of all boards in the engine for the duration
       of the deployment. */
    std::vector<PoetsMailbox*> allMailboxes;
    std::vector<PoetsCore*> allCores;

    /* Population methods for internal maps. */
    void populate_board_map();
    void populate_mailbox_map();  /* Only populates enough for one board. */

    /* Assignment and population methods used during deployment. */
    void assign_metadata_to_engine(PoetsEngine* engine);
    void assign_sizes_to_address_format(HardwareAddressFormat* format);
    void connect_boards_from_boardmap_in_engine(PoetsEngine* engine);
    void connect_mailboxes_from_mailboxmap_in_board(PoetsBoard* board);
    void create_cores_in_mailbox(PoetsMailbox* mailbox);
    void create_threads_in_core(PoetsCore* core);
    AddressComponent flatten_address(MultiAddressComponent address,
                                     std::vector<unsigned> wordLengths);
    void populate_boxes_evenly_with_boardmap(
        std::map<AddressComponent, PoetsBox*>* boxMap);
    void populate_engine_with_boxes_and_their_costs(PoetsEngine* engine);
};

#endif
