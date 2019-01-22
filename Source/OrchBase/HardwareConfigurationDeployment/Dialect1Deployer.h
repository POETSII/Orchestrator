#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_DIALECT1DEPLOYER_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_DIALECT1DEPLOYER_H

/* Defines how dialect-1 style files are deployed to define P_engine
 * configurations. This data structure holds configuration information, so that
 * it can be deployed to a given engine and address format. */

#include <map>
#include <numeric>
#include <vector>

#include "dfprintf.h"
#include "HardwareModel.h"

/* Address components that are multidimensional (to support hypercubes), that
 * have not been flattened, are vectors of address components. A vector is used
 * over a std::array here to make the reduction operation in flatten_address
 * easier to write. */
typedef std::vector<AddressComponent> MultiAddressComponent;

/* Simple container for storing items with their flattened addresses, so they
 * can be indexed in maps by the hierarchical addresses. */
template <typename PoetsItem>
struct itemAndAddress {
    PoetsItem poetsItem;  /* E.g. P_board*, or P_mailbox*. */
    AddressComponent address;
};

/* Types for private maps, for readability. */
typedef std::map<MultiAddressComponent, itemAndAddress<P_board*>*> BoardMap;
typedef std::map<MultiAddressComponent, itemAndAddress<P_mailbox*>*>
    MailboxMap;

class Dialect1Deployer
{
public:

    Dialect1Deployer();

    void deploy(P_engine* engine);

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
                                            * equally between the number of
                                            * boxes. */
    bool boardsAsHypercube;
    std::vector<bool> boardHypercubePeriodicity;
    float costExternalBox;
    float costBoardBoard;

    /* Box properties */
    float costBoxBoard;
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
    P_board* create_board();

    unsigned createdMailboxIndex;
    P_mailbox* create_mailbox();

    unsigned createdCoreIndex;
    P_core* create_core();

    unsigned createdThreadIndex;
    P_thread* create_thread();

    /* Maps for staging POETS items during deployment, required to persist
     * connectivity information. These need to be maps so that we can determine
     * the neighbours of an item given its hierarchical address. */
    BoardMap boardMap;
    MailboxMap mailboxMap;

    void free_items_in_board_map();
    void free_items_in_mailbox_map();

    /* Vectors for holding pointers to created items, for code simplification
     * (means we don't need to iterate through every board-mailbox to find
     * every core, for example). We don't need one of these for boards, because
     * boardMap stores the content of all boards in the engine for the duration
     * of the deployment. */
    std::vector<P_mailbox*> allMailboxes;
    std::vector<P_core*> allCores;

    /* Population methods for internal maps. */
    void populate_board_map();
    void populate_mailbox_map();  /* Only populates enough for one board. */

    /* Assignment and population methods used during deployment. */
    void assign_metadata_to_engine(P_engine* engine);
    void assign_sizes_to_address_format(HardwareAddressFormat* format);
    void connect_boards_from_boardmap_in_engine(P_engine* engine);
    void connect_mailboxes_from_mailboxmap_in_board(P_board* board);
    void create_cores_in_mailbox(P_mailbox* mailbox);
    void create_threads_in_core(P_core* core);
    AddressComponent flatten_address(MultiAddressComponent address,
                                     std::vector<unsigned> wordLengths);
    void populate_boxes_evenly_with_boardmap(
        std::map<AddressComponent, P_box*>* boxMap);
    void populate_engine_with_boxes_and_their_costs(P_engine* engine);
};

#endif
