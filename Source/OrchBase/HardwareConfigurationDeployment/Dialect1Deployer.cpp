#include "Dialect1Deployer.h"

Dialect1Deployer::Dialect1Deployer()
{
    author = "";
    version = "";
    fileOrigin = "";
}

/* Defines logic for deploying a dialect 1-style configuration to an
 * engine. Arguments:
 *
 *  - engine: POETS Engine to modify in-place.
 *
 * Configurations are deployed statically, but the deployer dynamically creates
 * objects within the static objects. Your engine and addressFormat should be
 * constructed by your containing object, and passed in by reference.
 *
 * Validation must be performed by the logic that populates deployer
 * objects. If you don't validate your input, your deployment will fall over.
 *
 * There is a lot of code duplication here, notably between
 * populate_<something>_map methods and the various connection methods, but the
 * function-template solution became quite obtuse to read, which is why I have
 * kept them separate here.
 *
 * Deployers must free their internal data structures before they are
 * destructed to avoid memory leaks - "free_" methods are providied to
 * facilitate this.
 *
 * See accompanying header for the set of variables that are used during
 * deployment. */
void Dialect1Deployer::deploy(P_engine* engine)
{
    /* Name the engine, if it isn't already named. */
    name_engine(engine);

    /* Assign metadata to the engine. */
    assign_metadata_to_engine(engine);

    /* Assign format sizes to the address format. */
    assign_sizes_to_address_format(&(engine->addressFormat));

    /* Engine costings */
    engine->costExternalBox = costExternalBox;

    /* Populate the engine with boxes. */
    populate_engine_with_boxes_and_their_costs(engine);

    /* Create a series of boards, storing them in a map, addressed by their
     * hierarchical address components. */
    populate_board_map();

    /* Divide up the boards between the boxes naively, and assign them. */
    populate_boxes_evenly_with_boardmap(&(engine->P_boxm));

    /* Connect the boards in a graph for the engine. */
    connect_boards_from_boardmap_in_engine(engine);

    /* For each board, create a bunch of mailboxes and connect them up. */
    for (BoardMap::iterator boardIterator=boardMap.begin();
         boardIterator!=boardMap.end(); boardIterator++)
    {
        /* Create a series of mailboxes, storing them in a map, addressed by
         * their hierarchical address components. */
        populate_mailbox_map();

        /* Connect the mailboxes in the board. */
        connect_mailboxes_from_mailboxmap_in_board(
            boardIterator->second->poetsItem);

        /* Clear the mailbox map for the next iteration. NB: Does not free the
         * P_mailbox objects themselves, only the itemAndAddress objects. */
        free_items_in_mailbox_map();
        mailboxMap.clear();
    }

    /* For each mailbox, create a bunch of cores and store them within. */
    for (unsigned mailboxIndex=0; mailboxIndex<allMailboxes.size();
         create_cores_in_mailbox(allMailboxes[mailboxIndex++]));

    /* For each core, create a bunch of threads and store them within. */
    for (unsigned coreIndex=0; coreIndex<allCores.size();
         create_threads_in_core(allCores[coreIndex++]));

    /* Free itemAndAddress objects in the boardMap. */
    free_items_in_board_map();
}

/* Assigns a name to the engine, if one isn't already assigned. Arguments:
 *
 * - engine: the P_engine to assign a name to. */
void Dialect1Deployer::name_engine(P_engine* engine)
{
    if ((engine->Name().size() == 0 ||
         engine->Name() == "**undefined**") &&
        fileOrigin.size() >= 0)
    {
        engine->Name(fileOrigin);
    }
}

/* Assigns all metadata defined in this deployer to the engine. Arguments:
 *
 * - engine: the P_engine to assign metadata to. */
void Dialect1Deployer::assign_metadata_to_engine(P_engine* engine)
{
    engine->author = author;
    engine->datetime = datetime;
    engine->version = version;
    engine->fileOrigin = fileOrigin;
}

/* Assigns all sizes to an address format object. Arguments:
 *
 * - format: the HardwareAddressObject to assign sizes to. */
void Dialect1Deployer::assign_sizes_to_address_format(
    HardwareAddressFormat* format)
{
    format->boxWordLength = boxWordLength;
    format->coreWordLength = coreWordLength;
    format->threadWordLength = threadWordLength;

    /* Sum-reduce the formats that are defined as vectors. */
    format->boardWordLength = std::accumulate(boardWordLengths.begin(),
                                              boardWordLengths.end(), 0);
    format->mailboxWordLength = std::accumulate(mailboxWordLengths.begin(),
                                                mailboxWordLengths.end(), 0);
}

/* Donates all boards in the boardMap to the engine, and connects them
 * up. Arguments:
 *
 *  - engine: Engine to populate. */
void Dialect1Deployer::connect_boards_from_boardmap_in_engine(P_engine* engine)
{
    /* The connection behaviour depends on whether or not the hypercube
     * argument was specified. If a hypercube, connect them in a hypercube
     * (obviously). If not a hypercube, connect them all-to-all
     * (not-so-obviously).
     *
     * Whatever we do, we'll need these declarations however. */
    BoardMap::iterator outerBoardIterator;
    AddressComponent outerFlatAddress;
    AddressComponent innerFlatAddress;

    if (boardsAsHypercube)
    {
        /* For each board, donate that board to the engine. */
        for (outerBoardIterator=boardMap.begin();
             outerBoardIterator!=boardMap.end(); outerBoardIterator++)
        {
            engine->contain(outerBoardIterator->second->address,
                            outerBoardIterator->second->poetsItem);
        }

        /* For each board, connect that board to its neighbours. Note that this
         * loop is not rolled with the other loop because a board must first be
         * donated before it is connected, and try/catching for duplicate
         * containment operations is (probably) more expensive than keeping the
         * loops separate. */
        unsigned boardDimensions = boardsInEngine.size();

        /* Variables in the loop to determine whether or not to connect ahead
         * or behind a given board in a given dimension. */
        bool isAnyoneAhead;
        bool isAnyoneBehind;

        /* Variable in the loop to hold our hierarchical address, and the
         * computed hierarchical address of a neighbour. */
        MultiAddressComponent outerHierarchicalAddress;
        MultiAddressComponent innerHierarchicalAddress;

        for (outerBoardIterator=boardMap.begin();
             outerBoardIterator!=boardMap.end(); outerBoardIterator++)
        {
            /* For each dimension, connect to the neighbour ahead and
             * behind (if there is one). */
            outerHierarchicalAddress = outerBoardIterator->first;
            outerFlatAddress = outerBoardIterator->second->address;
            for (unsigned dimension=0; dimension<boardDimensions; dimension++)
            {
                /* Handle edge-case. */
                isAnyoneAhead = true;
                isAnyoneBehind = true;
                if (outerHierarchicalAddress[dimension] == 0)
                {
                    isAnyoneBehind = false;
                }
                if (outerHierarchicalAddress[dimension] ==
                    boardsInEngine[dimension] - 1)
                {
                    isAnyoneAhead = false;
                }

                /* Connect to the neighbour ahead, if any. */
                if (isAnyoneAhead || boardHypercubePeriodicity[dimension])
                {
                    innerHierarchicalAddress = outerHierarchicalAddress;
                    if (isAnyoneAhead)
                    {
                        /* Compute the address of the neighbour. */
                        innerHierarchicalAddress[dimension] += 1;
                    }
                    else
                    {
                        innerHierarchicalAddress[dimension] = 0;
                    }

                    innerFlatAddress = flatten_address(
                        innerHierarchicalAddress, boardWordLengths);

                    /* Do the actual connecting. This connection happens
                     * one-way, because we are iterating through each board. */
                    engine->connect(outerFlatAddress, innerFlatAddress,
                                    costBoardBoard, true);
                }

                /* As previous, but for boards behind us. */
                if (isAnyoneBehind || boardHypercubePeriodicity[dimension])
                {
                    innerHierarchicalAddress = outerHierarchicalAddress;
                    if (isAnyoneBehind)
                    {
                        /* Compute the address of the neighbour. */
                        innerHierarchicalAddress[dimension] -= 1;
                    }
                    else
                    {
                        innerHierarchicalAddress[dimension] =
                            boardsInEngine[dimension] - 1;
                    }

                    innerFlatAddress = flatten_address(
                        innerHierarchicalAddress, boardWordLengths);

                    /* Do the actual connecting. This connection happens
                     * one-way, because we are iterating through each board. */
                    engine->connect(outerFlatAddress, innerFlatAddress,
                                    costBoardBoard, true);
                }
            }  /* End for each dimension. */
        }  /* End for each board. */
    }
    else  /* All-to-all */
    {
        /* For each board, donate that board to the engine, then connect that
         * board to every board in the engine so far (handshaking problem). */
        BoardMap::iterator innerBoardIterator;
        for (outerBoardIterator=boardMap.begin();
             outerBoardIterator!=boardMap.end(); outerBoardIterator++)
        {
            /* Donation. */
            outerFlatAddress = outerBoardIterator->second->address;
            engine->contain(outerFlatAddress,
                            outerBoardIterator->second->poetsItem);

            /* Connection between the previously-donated boards, again
             * flattening the inner address. */
            for (innerBoardIterator=boardMap.begin();
                 innerBoardIterator!=outerBoardIterator; innerBoardIterator++)
            {
                innerFlatAddress = innerBoardIterator->second->address;
                engine->connect(outerFlatAddress, innerFlatAddress,
                                costBoardBoard);
            }
        }
    }
}

/* Donates all mailboxes in the mailboxMap to the board, and connects them
 * up. Arguments:
 *
 * - board: Board to populate. */
void Dialect1Deployer::connect_mailboxes_from_mailboxmap_in_board(
    P_board* board)
{
    /* The connection behaviour depends on whether or not the hypercube
     * argument was specified. If a hypercube, connect them in a hypercube
     * (obviously). If not a hypercube, connect them all-to-all
     * (not-so-obviously).
     *
     * Whatever we do, we'll need these declarations however. */
    MailboxMap::iterator outerMailboxIterator;
    AddressComponent outerFlatAddress;
    AddressComponent innerFlatAddress;

    if (mailboxesAsHypercube)
    {
        /* For each mailbox, donate that mailbox to the board. */
        for (outerMailboxIterator=mailboxMap.begin();
             outerMailboxIterator!=mailboxMap.end(); outerMailboxIterator++)
        {
            board->contain(outerMailboxIterator->second->address,
                           outerMailboxIterator->second->poetsItem);
        }

        /* For each mailbox, connect that mailbox to its neighbours. Note that
         * this loop is not rolled with the other loop because a mailbox must
         * first be donated before it is connected, and try/catching for
         * duplicate containment operations is (probably) more expensive than
         * keeping the loops separate. */
        unsigned mailboxDimensions = mailboxesInBoard.size();

        /* Variables in the loop to determine whether or not to connect ahead
         * or behind a given mailbox in a given dimension. */
        bool isAnyoneAhead;
        bool isAnyoneBehind;

        /* Variable in the loop to hold our hierarchical address, and the
         * computed hierarchical address of a neighbour. */
        MultiAddressComponent outerHierarchicalAddress;
        MultiAddressComponent innerHierarchicalAddress;

        for (outerMailboxIterator=mailboxMap.begin();
             outerMailboxIterator!=mailboxMap.end(); outerMailboxIterator++)
        {
            /* For each dimension, connect to the neighbour ahead and
             * behind (if there is one). */
            outerHierarchicalAddress = outerMailboxIterator->first;
            outerFlatAddress = outerMailboxIterator->second->address;
            for (unsigned dimension=0; dimension<mailboxDimensions;
                 dimension++)
            {
                /* Handle edge-case. */
                isAnyoneAhead = true;
                isAnyoneBehind = true;
                if (outerHierarchicalAddress[dimension] == 0)
                {
                    isAnyoneBehind = false;
                }
                if (outerHierarchicalAddress[dimension] ==
                    mailboxesInBoard[dimension] - 1)
                {
                    isAnyoneAhead = false;
                }

                /* Connect to the neighbour ahead, if any. */
                if (isAnyoneAhead || mailboxHypercubePeriodicity[dimension])
                {
                    innerHierarchicalAddress = outerHierarchicalAddress;
                    if (isAnyoneAhead)
                    {
                        /* Compute the address of the neighbour. */
                        innerHierarchicalAddress[dimension] += 1;
                    }
                    else
                    {
                        innerHierarchicalAddress[dimension] = 0;
                    }

                    innerFlatAddress = flatten_address(
                        innerHierarchicalAddress, mailboxWordLengths);

                    /* Do the actual connecting. This connection happens
                     * one-way, because we are iterating through each
                     * mailbox. */
                    board->connect(outerFlatAddress, innerFlatAddress,
                                   costMailboxMailbox, true);
                }

                /* As previous, but for mailboxes behind us. */
                if (isAnyoneBehind || mailboxHypercubePeriodicity[dimension])
                {
                    innerHierarchicalAddress = outerHierarchicalAddress;
                    if (isAnyoneBehind)
                    {
                        /* Compute the address of the neighbour. */
                        innerHierarchicalAddress[dimension] -= 1;
                    }
                    else
                    {
                        innerHierarchicalAddress[dimension] =
                            mailboxesInBoard[dimension] - 1;
                    }

                    innerFlatAddress = flatten_address(
                        innerHierarchicalAddress, mailboxWordLengths);

                    /* Do the actual connecting. This connection happens
                     * one-way, because we are iterating through each
                     * mailbox. */
                    board->connect(outerFlatAddress, innerFlatAddress,
                                   costMailboxMailbox, true);
                }
            }  /* End for each dimension. */
        }  /* End for each mailbox. */
    }
    else  /* All-to-all */
    {
        /* For each mailbox, donate that mailbox to the board, then connect
         * that mailbox to every mailbox in the board so far (handshaking
         * problem). */
        MailboxMap::iterator innerMailboxIterator;
        for (outerMailboxIterator=mailboxMap.begin();
             outerMailboxIterator!=mailboxMap.end(); outerMailboxIterator++)
        {
            /* Donation. */
            outerFlatAddress = outerMailboxIterator->second->address;
            board->contain(outerFlatAddress,
                           outerMailboxIterator->second->poetsItem);

            /* Connection between the previously-donated mailboxes, again
             * flattening the inner address. */
            for (innerMailboxIterator=mailboxMap.begin();
                 innerMailboxIterator!=outerMailboxIterator;
                 innerMailboxIterator++)
            {
                innerFlatAddress = innerMailboxIterator->second->address;
                board->connect(outerFlatAddress, innerFlatAddress,
                               costMailboxMailbox);
            }
        }
    }
}

/* Creates a series of cores, and donates them to the mailbox passed in as an
 * argument. */
void Dialect1Deployer::create_cores_in_mailbox(P_mailbox* mailbox)
{
    P_core* temporaryCore;

    for(AddressComponent coreIndex = 0; coreIndex<coresInMailbox; coreIndex++)
    {
        temporaryCore = create_core(dformat("Core%06d", coreIndex));
        mailbox->contain(coreIndex, temporaryCore);
    }
}

/* Creates a series of threads, and donates them to the core passed in as an
 * argument. */
void Dialect1Deployer::create_threads_in_core(P_core* core)
{
    P_thread* temporaryThread;

    for(AddressComponent threadIndex = 0; threadIndex<threadsInCore;
        threadIndex++)
    {
        temporaryThread = create_thread(dformat("Thread%06d", threadIndex));
        core->contain(threadIndex, temporaryThread);
    }
}

/* Flattens a multidimensional address (or a vector-address with one
 * dimension), and returns it. Arguments:
 *
 * - address: address to flatten.
 * - wordLengths: word lengths of each component of the address
 *
 * address and wordLengths must have the same number of dimensions.
 *
 * Examples:
 *
 *  1. address is (0,0,0), wordLengths is (3,2,1), returns 0b000000 = 0.
 *  2. address is (3,1,1), wordLengths is (3,2,1), returns 0b011011 = 27.
 *  3. address is (7,3,1), wordLengths is (3,2,1), returns 0b111111 = 63. */
AddressComponent Dialect1Deployer::flatten_address(
    MultiAddressComponent address, std::vector<unsigned> wordLengths)
{
    AddressComponent returnValue = 0;
    for (int dimension=address.size()-1; dimension>=0; dimension--)
    {
        returnValue <<= wordLengths[dimension];
        returnValue |= address[dimension];
    }
    return returnValue;
}

/* Distributes all boards in the boardMap to all boxes evenly,
 * arbitrarily. Assumes that the number of boards divides into the number of
 * boxes without remainder. Modifies the boxes inplace. Arguments:
 *
 * - boxMap: Boxes, mapped by their address components. */
void Dialect1Deployer::populate_boxes_evenly_with_boardmap(
    std::map<AddressComponent, P_box*>* boxMap)
{
    /* An even distribution (we hope). */
    unsigned boardsPerBox = boardMap.size() / boxMap->size();

    /* We iterate through the map of boxes and boards, distributing all boards
     * up to a given amount, so that an even distribution is maintained. */
    std::map<AddressComponent, P_box*>::iterator boxIterator = boxMap->begin();
    BoardMap::iterator boardIterator = boardMap.begin();
    for (boxIterator=boxMap->begin(); boxIterator!=boxMap->end();
         boxIterator++)
    {
        /* Contain 'boardsPerBox' boards in this box, using the flattened
         * address, and set the name of the board appropriately. */
        for (unsigned boardIndex=0; boardIndex<boardsPerBox; boardIndex++)
        {
            boxIterator->second->contain(boardIterator->second->address,
                                         boardIterator->second->poetsItem);
            boardIterator->second->poetsItem->Name(dformat("Board%06d",
                                                           boardIndex));
            boardIterator++;
        }
    }
}

/* Populates a P_engine with boxes, and defines the box-board costs in those
 * boxes. Arguments:
 *
 * - engine: P_engine to populate. */
void Dialect1Deployer::populate_engine_with_boxes_and_their_costs(
    P_engine* engine)
{
    P_box* temporaryBox;

    for (AddressComponent addressComponent=0; addressComponent < boxesInEngine;
         addressComponent++)
    {
        temporaryBox = create_box(dformat("Box%06d", addressComponent));
        engine->contain(addressComponent, temporaryBox);
    }
}

/* Populates boardMap with dynamically-allocated itemAndAddress objects,
 * containing dynamically-allocated P_boards. Also defines their addresses, and
 * includes that information in the map. */
void Dialect1Deployer::populate_board_map()
{
    unsigned boardDimensions = boardsInEngine.size();

    /* A temporary address, one for each created board in the map. */
    MultiAddressComponent boardAddress(boardDimensions, 0);

    /* A temporary board-address pair for populating the map. */
    itemAndAddress<P_board*>* boardAndAddress;

    /* A placeholder variable for defining board names. */
    std::string boardName = "BoardWillBeNamedWhenMappedToABox";

    /* We loop until we have created all of the boards that we need to. */
    bool looping = true;
    while(looping)
    {
        looping = false;

        /* Create and store a board in the map. Assign simple properties to the
         * board while we're here. */
        boardAndAddress = new itemAndAddress<P_board*>;
        boardAndAddress->address = flatten_address(boardAddress,
                                                   boardWordLengths);
        boardAndAddress->poetsItem = create_board(boardName);
        boardMap.insert(std::make_pair(boardAddress, boardAndAddress));

        /* Increment hierarchical address.
         *
         * If there are higher dimensions, increment to the next one
         * recursively until there are no more addresses to use.
         *
         * The left-most dimension is more significant than the right-most
         * dimension (big-endian).
         *
         * Examples:
         *
         *  1. If boardAddress is (0,0,3) and boardsInEngine is (2,2,4), then
         *     boardAddress would become (0,1,0).
         *
         *  2. If boardAddress is (0,1,3) and boardsInEngine is (2,2,4), then
         *     boardAddress would become (1,0,0), because the second dimension
         *     addition is carried over into the first dimension.
         *
         *  3. If boardAddress is (1,1,3) and boardsInEngine is (2,2,4), then
         *     iteration stops, and no more boards are created. */
        for (int dimension=boardDimensions-1; dimension>=0; dimension--)
        {
            if (boardAddress[dimension] == boardsInEngine[dimension] - 1)
            {
                boardAddress[dimension] = 0;
                continue;
            }
            else
            {
                boardAddress[dimension]++;
                looping = true;
                break;
            }
        }
    }
}

/* Populates mailboxMap with dynamically-allocated itemAndAddress objects,
 * containing dynamically-allocated P_mailboxes. Also defines their addresses,
 * and includes that information in the map. Only creates enough mailboxes to
 * fit in one board. */
void Dialect1Deployer::populate_mailbox_map()
{
    unsigned mailboxDimensions = mailboxesInBoard.size();

    /* A temporary address, one for each created mailbox in the map. */
    MultiAddressComponent mailboxAddress(mailboxDimensions, 0);

    /* A temporary mailbox-address pair for populating the map. */
    itemAndAddress<P_mailbox*>* mailboxAndAddress;

    /* A placeholder variable for defining mailbox names. Unlike
     * populate_board_map, where all of the boards might go in different boxes,
     * we know that all of the mailboxes in this map will be stored in the same
     * board. This means we can meaningfully index the mailbox names as they
     * are created. */
    unsigned mailboxIndex;

    /* We loop until we have created all of the mailboxes that we need to. */
    bool looping = true;
    while(looping)
    {
        looping = false;

        /* Create and store a mailbox in the map. Assign simple properties to
         * the mailbox while we're here.*/
        mailboxAndAddress = new itemAndAddress<P_mailbox*>;
        mailboxAndAddress->address = flatten_address(mailboxAddress,
                                                     mailboxWordLengths);
        mailboxAndAddress->poetsItem = create_mailbox(dformat("Mailbox%06d",
                                                              mailboxIndex));
        mailboxMap.insert(std::make_pair(mailboxAddress, mailboxAndAddress));

        /* Increment hierarchical address.
         *
         * If there are higher dimensions, increment to the next one
         * recursively until there are no more addresses to use.
         *
         * The left-most dimension is more significant than the right-most
         * dimension (big-endian).
         *
         * See populate_board_map for examples. */
        for (int dimension=mailboxDimensions-1; dimension>=0; dimension--)
        {
            if (mailboxAddress[dimension] == mailboxesInBoard[dimension] - 1)
            {
                mailboxAddress[dimension] = 0;
                continue;
            }
            else
            {
                mailboxAddress[dimension]++;
                looping = true;
                break;
            }
        }
    }
}

/* Dynamically creates a new POETS box, and populates it with it's common
 * parameters. Does not define contained items. */
P_box* Dialect1Deployer::create_box(std::string name)
{
    P_box* returnAddress;
    returnAddress = new P_box(name);
    returnAddress->costBoxBoard = costBoxBoard;
    returnAddress->supervisorMemory = boxSupervisorMemory;
    return returnAddress;
}

/* Dynamically creates a new POETS board, and populates it with it's common
 * parameters. Does not define contained items. */
P_board* Dialect1Deployer::create_board(std::string name)
{
    P_board* returnAddress;
    returnAddress = new P_board(name);
    returnAddress->dram = dram;
    returnAddress->supervisorMemory = boardSupervisorMemory;
    returnAddress->costBoardMailbox = costBoardMailbox;
    return returnAddress;
}

/* Dynamically creates a new POETS mailbox, and populates it with it's common
 * parameters. Does not define contained items. */
P_mailbox* Dialect1Deployer::create_mailbox(std::string name)
{
    P_mailbox* returnAddress;
    returnAddress = new P_mailbox(name);
    returnAddress->costCoreCore = costCoreCore;
    returnAddress->costMailboxCore = costMailboxCore;

    /* Add it to allMailboxes. */
    allMailboxes.push_back(returnAddress);
    return returnAddress;
}

/* Dynamically creates a new POETS core, and populates it with it's common
 * parameters. Does not define contained items. */
P_core* Dialect1Deployer::create_core(std::string name)
{
    P_core* returnAddress;
    returnAddress = new P_core(name);
    returnAddress->dataMemory = dataMemory;
    returnAddress->instructionMemory = instructionMemory;
    returnAddress->costCoreThread = costCoreThread;
    returnAddress->costThreadThread = costThreadThread;

    /* Add it to allCores. */
    allCores.push_back(returnAddress);
    return returnAddress;
}

/* Dynamically creates a new POETS thread. */
P_thread* Dialect1Deployer::create_thread(std::string name)
{
    return new P_thread(name);
}

/* Frees dyamically-allocated value objects (itemAndAddress<P_board*>*) objects
 * in the boardMap. */
void Dialect1Deployer::free_items_in_board_map()
{
    for (BoardMap::iterator boardIterator=boardMap.begin();
         boardIterator!=boardMap.end(); boardIterator++)
    {
        delete boardIterator->second;
    }
}

/* Frees dyamically-allocated value objects (itemAndAddress<P_mailbox*>*)
 * objects in the mailboxMap. */
void Dialect1Deployer::free_items_in_mailbox_map()
{
    for (MailboxMap::iterator mailboxIterator=mailboxMap.begin();
         mailboxIterator!=mailboxMap.end(); mailboxIterator++)
    {
        delete mailboxIterator->second;
    }
}
