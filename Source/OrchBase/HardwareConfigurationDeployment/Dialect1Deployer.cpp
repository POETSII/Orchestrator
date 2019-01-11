
#include "Dialect1Deployer.h"

/* Defines logic for deploying a dialect 1-style configuration to an engine and
   address format. Arguments:

    - engine: POETS Engine to modify in-place.

    - addressFormat: Address format to populate.

   Configurations are deployed statically, but the deployer dynamically creates
   objects within the static objects. Your engine and addressFormat should be
   constructed by your containing object, and passed in by reference.

   Validation must be performed by the logic that populates deployer
   objects. If you don't validate your input, your deployment will fall over.

   See accompanying header for the set of variables that are used during
   deployment. */
void Dialect1Deployer::deploy(PoetsEngine* engine,
                              HardwareAddressFormat* addressFormat)
{
    /* Assign metadata to the engine. */
    assign_metadata_to_engine(engine);

    /* Assign format sizes to the address format. */
    assign_sizes_to_address_format(addressFormat);

    /* Engine costings */
    engine->costExternalBox = costExternalBox;

    /* Populate the engine with boxes. */
    populate_engine_with_boxes_and_their_costs(engine);

    /* Create a series of boards, storing them in a map, addressed by their
       hierarchical address components. Also store the flattened addresses
       in a second map. */
    std::map<MultiAddressComponent, itemAndAddress<PoetsBoard*>*> boards;
    populate_map_with_boards(&boards);

    /* Divide up the boards between the boxes naively, and assign them. */
    populate_boxes_evenly_with_boards(&(engine->PoetsBoxes), &boards);

    /* Connect the boards in a graph for the engine. */
    connect_boards_in_engine(engine, &boards);

    /* Free dynamically-allocated itemAndAddress<PoetsBoard*>* objects in
     * boards. */
    std::map<MultiAddressComponent, itemAndAddress<PoetsBoard*>*>::iterator \
        boardIterator;
    for (boardIterator=boards.begin(); boardIterator!=boards.end();
         boardIterator++)
    {
        delete boardIterator->second;
    }
}

/* Assigns all metadata defined in this deployer to the engine. Arguments:

    - engine: the PoetsEngine to assign metadata to.
*/
void Dialect1Deployer::assign_metadata_to_engine(PoetsEngine* engine)
{
    engine->author = author;
    engine->datetime = datetime;
    engine->version = version;
    engine->fileOrigin = fileOrigin;
}

/* Assigns all sizes to an address format object. Arguments

    - format: the HardwareAddressObject to assign sizes to.
*/
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

/* Donates all boards in the map to the engine, and connects them
   up. Arguments:

    - engine: Engine to populate.

    - boardMap: Map of boards, given their hierarchical addresses.
*/
void Dialect1Deployer::connect_boards_in_engine(
    PoetsEngine* engine,
    std::map<MultiAddressComponent,
             itemAndAddress<PoetsBoard*>*>* boardMap)
{
    /* The connection behaviour depends on whether or not the hypercube
       argument was specified. If a hypercube, connect them in a hypercube
       (obviously). If not a hypercube, connect them all-to-all
       (not-so-obviously).

       Whatever we do, we'll need these declarations however. */
    std::map<MultiAddressComponent, itemAndAddress<PoetsBoard*>*>::iterator \
        outerBoardIterator;
    AddressComponent outerFlatAddress;
    AddressComponent innerFlatAddress;

    if (boardsAsHypercube)
    {
        /* For each board, donate that board to the engine. */
        for (outerBoardIterator=boardMap->begin();
             outerBoardIterator!=boardMap->end(); outerBoardIterator++)
        {
            engine->contain(outerBoardIterator->second->address,
                            outerBoardIterator->second->poetsItem);
        }

        /* For each board, connect that board to its neighbours. Note that this
           loop is not rolled with the other loop because a board must first be
           donated before it is connected, and try/catching for duplicate
           containment operations is (probably) more expensive than keeping the
           loops separate. */
        unsigned boardDimensions = boardsInEngine.size();

        /* Variables in the loop to determine whether or not to connect ahead
           or behind a given board in a given dimension. */
        bool isAnyoneAhead;
        bool isAnyoneBehind;

        /* Variable in the loop to hold our hierarchical address, and the
           computed hierarchical address of a neighbour. */
        MultiAddressComponent outerHierarchicalAddress;
        MultiAddressComponent innerHierarchicalAddress;

        for (outerBoardIterator=boardMap->begin();
             outerBoardIterator!=boardMap->end(); outerBoardIterator++)
        {
            /* For each dimension, connect to the neighbour ahead and
               behind (if there is one). */
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
                else if (outerHierarchicalAddress[dimension] ==
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
                       one-way, because we are iterating through each board. */
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
                       one-way, because we are iterating through each board. */
                    engine->connect(outerFlatAddress, innerFlatAddress,
                                    costBoardBoard, true);
                }
            }  /* End for each dimension. */
        }  /* End for each board. */
    }
    else  /* All-to-all */
    {
        /* For each board, donate that board to the engine, then connect that
           board to every board in the engine so far (handshaking problem). */
        std::map<MultiAddressComponent,
                 itemAndAddress<PoetsBoard*>*>::iterator innerBoardIterator;

        for (outerBoardIterator=boardMap->begin();
             outerBoardIterator!=boardMap->end(); outerBoardIterator++)
        {
            /* Donation. */
            outerFlatAddress = outerBoardIterator->second->address;
            engine->contain(outerFlatAddress,
                            outerBoardIterator->second->poetsItem);

            /* Connection between the previously-donated boards, again
               flattening the inner address. */
            for (innerBoardIterator=boardMap->begin();
                 innerBoardIterator!=outerBoardIterator; innerBoardIterator++)
            {
                innerFlatAddress = innerBoardIterator->second->address;
                engine->connect(outerFlatAddress, innerFlatAddress,
                                costBoardBoard);
            }
        }
    }
}

/* Flattens a multidimensional address (or a vector-address with one
   dimension), and returns it. Arguments:

    - address: address to flatten.
    - wordLengths: word lengths of each component of the address

   address and wordLengths must have the same number of dimensions.

   Examples:

    1. address is (0,0,0), wordLengths is (3,2,1), returns 0b000000 = 0.
    2. address is (3,1,1), wordLengths is (3,2,1), returns 0b011011 = 27.
    3. address is (7,3,1), wordLengths is (3,2,1), returns 0b111111 = 63.
*/
AddressComponent Dialect1Deployer::flatten_address(
    MultiAddressComponent address, std::vector<unsigned> wordLengths)
{
    AddressComponent returnValue = 0;
    for (int dimension=address.size()-1; dimension>=0; dimension--)
    {
        returnValue <<= wordLengths[dimension] - 1;
        returnValue |= address[dimension];
    }
    return returnValue;
}

/* Distributes all boards in a map to all boxes evenly, arbitrarily. Assumes
   that the number of boards divides into the number of boxes without
   remainder. Arguments:

    - boxMap: Boxes, mapped by their address components.

    - boardMap: All boards (previously uncontained), mapped by their
      hierarchical addresses.

   Modifies the boxes inplace.
*/
void Dialect1Deployer::populate_boxes_evenly_with_boards(
    std::map<AddressComponent, PoetsBox*>* boxMap,
    std::map<MultiAddressComponent, itemAndAddress<PoetsBoard*>*>* boardMap)
{
    /* An even distribution (we hope). */
    unsigned boardsPerBox = boardMap->size() / boxMap->size();

    /* We iterate through the map of boxes and boards, distributing all boards
       up to a given amount, such that an even distribution is maintained. */
    std::map<AddressComponent, PoetsBox*>::iterator \
        boxIterator = boxMap->begin();
    std::map<MultiAddressComponent, itemAndAddress<PoetsBoard*>*>::iterator \
        boardIterator = boardMap->begin();
    for (boxIterator=boxMap->begin(); boxIterator!=boxMap->end();
         boxIterator++)
    {
        /* Contain 'boardsPerBox' boards in this box, using the flattened
           address. */
        for (unsigned boardIndex=0; boardIndex<boardsPerBox; boardIndex++)
        {
            boxIterator->second->contain(boardIterator->second->address,
                                         boardIterator->second->poetsItem);
            boardIterator++;
        }
    }
}

/* Populates a PoetsEngine with boxes, and defines the box-board costs in those
   boxes. Arguments:

    - engine: PoetsEngine to populate. */
void Dialect1Deployer::populate_engine_with_boxes_and_their_costs(
    PoetsEngine* engine)
{
    PoetsBox* temporaryBox;  /* Staging variable for boxes. */

    for (AddressComponent addressComponent=0; addressComponent < boxesInEngine;
         addressComponent++)
    {
        temporaryBox = new PoetsBox(dformat("Box%06d", addressComponent));
        temporaryBox->costBoxBoard = costBoxBoard;
        temporaryBox->supervisorMemory = boxSupervisorMemory;
        engine->contain(addressComponent, temporaryBox);
    }
}

/* Populates a map passed as an argument with dynamically-allocated
   PoetsBoards. Also defines their addresses, and includes that information in
   the map. Arguments:

    - boardMap: Maps hierarchical addresses onto POETS boards.
*/
void Dialect1Deployer::populate_map_with_boards(
    std::map<MultiAddressComponent, itemAndAddress<PoetsBoard*>*>* boardMap)
{
    unsigned boardDimensions = boardsInEngine.size();

    /* A temporary address, one for each created board in the map. A vector is
       used over a std::array here to make the reduction operation in
       flatten_address easier to write. */
    MultiAddressComponent boardAddress(boardDimensions, 0);

    /* A temporary board-address pair for populating the map. */
    itemAndAddress<PoetsBoard*>* boardAndAddress;

    /* We loop until we have created all of the boards that we need to. */
    bool looping = true;
    while(looping)
    {
        looping = false;

        /* Create and store a board in the map. Assign simple properties to the
         * board while we're here.*/
        boardAndAddress = new itemAndAddress<PoetsBoard*>;
        boardAndAddress->address = flatten_address(boardAddress,
                                                   boardWordLengths);
        boardAndAddress->poetsItem = create_board();
        boardMap->insert(std::make_pair(boardAddress, boardAndAddress));

        /* Increment hierarchical address.

           If there are higher dimensions, increment to the next one
           recursively until there are no more addresses to use.

           Examples:

            1. If boardAddress is (3,0,0) and boardsInEngine is (4,2,2), then
               boardAddress would become (3,1,0), which would be set to (0,1,0)
               when the next board is created.

            2. If boardAddress is (3,1,0) and boardsInEngine is (4,2,2), then
               boardAddress would become (3,0,1), because the second dimension
               addition is carried over into the third dimension. This address
               would then be set to (0,0,1) when the next board is created.

            3. If boardAddress is (3,1,1) and boardsInEngine is (4,2,2), then
               iteration stops, and no more boards are created. */
        for (unsigned dimension=0; dimension<boardDimensions; dimension++)
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

/* Dynamically creates a new POETS board, and populates it with it's common
   parameters. Does not define contained items. */
PoetsBoard* Dialect1Deployer::create_board()
{
    PoetsBoard* returnAddress;
    returnAddress = new PoetsBoard(dformat("Board%06d", boardIndex++));
    returnAddress->dram = dram;
    returnAddress->supervisorMemory = boardSupervisorMemory;
    return returnAddress;
}
