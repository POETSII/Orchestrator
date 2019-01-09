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
    /* Temporary staging variables for items, which are allocated
       dynamically. */
    PoetsBox* temporaryBox;

    /* Assign metadata to the engine. */
    assign_metadata_to_engine(engine);

    /* Assign format sizes to the address format. */
    assign_sizes_to_address_format(addressFormat);

    /* Engine costings */
    engine->costExternalBox = costExternalBox;

    /* Populate the engine with boxes. */
    populate_engine_with_boxes_and_their_costs(engine);

    /* Create a series of boards, storing them in a map address by their
       address component. */
    std::map<MultiAddressComponent, PoetsBoard*> boards;
    populate_map_with_boards(&boards);

    /* Divide up the boards between the boxes naively, and assign them. */
    populate_boxes_evenly_with_boards(&(engine->PoetsBoxes), &boards);

    /* Connect the boards in a graph for the engine. */
    connect_boards_in_engine(&boards, engine);
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

    - boards: Map of boards, given their address components.
    - engine: Engine to populate.
*/
void Dialect1Deployer::connect_boards_in_engine(
    std::map<AddressComponent, PoetsBoard*>* boards,
    PoetsEngine* engine)
{
    /* The connection behaviour depends on whether or not the hypercube
       argument was specified. If a hypercube, connect them in a hypercube
       (obviously). If not a hypercube, connect them all-to-all
       (not-so-obviously).

       Whatever we do, we'll need these declarations however. */
    std::map<AddressComponent, PoetsBoard*>::iterator innerBoardIterator;
    std::map<AddressComponent, PoetsBoard*>::iterator outerBoardIterator;
    AddressComponent innerFlatAddress;
    AddressComponent outerFlatAddress;

    if (boardsAsHypercube)
    {
        ; // <!>
    }
    else  /* All-to-all */
    {
        /* Donate the board to the engine, then connect that board to every
           board in the engine so far (handshaking problem). */
        for (outerBoardIterator=boards->begin();
             outerBoardIterator!=boards->end(); outerBoardIterator++)
        {
            /* Donation, making sure to flatten the address first. */
            outerFlatAddress = flatten_address(outerBoardIterator->first,
                                               boardWordLengths);
            engine->contain(outerFlatAddress, outerBoardIterator->second);

            /* Connection to the previously-donated boards, again flattening
               the inner address. */
            for (innerBoardIterator=boards->begin();
                 innerBoardIterator!=outerBoardIterator; innerBoardIterator++)
            {
                innerFlatAddress = flatten_address(innerBoardIterator->first,
                                                   boardWordLengths);
                engine.connect(outerFlatAddress, innerFlatAddress,
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
    for (unsigned dimension=0; dimension<address.size; dimension++)
    {
        returnValue |= address[dimension];
        returnValue << wordLengths[dimension];
    }
    return returnValue;
}

/* Distributes all boards in a map to all boxes evenly, arbitrarily. Assumes
   that the number of boards divides into the number of boxes without
   remainder. Arguments:

    - boxMap: Boxes, mapped by their address components.
    - boardMap: All boards (previously uncontained), mapped by their address
      components. The address components should not have been flattened.

   Modifies the boxes inplace.
*/
void Dialect1Deployer::populate_boxes_evenly_with_boards(
    std::map<AddressComponent, PoetsBox*>* boxMap,
    std::map<MultiAddressComponent, PoetsBoard*>* boardMap);
{
    /* An even distribution (we hope). */
    unsigned boardsPerBox = boardMap.size() / boxMap.size();
    AddressComponent flatAddress;

    /* We iterate through the map of boxes and boards, distributing all boards
       up to a given amount, such that an even distribution is maintained. */
    std::map<AddressComponent, PoetsBox*>::iterator \
        boxIterator = boxMap->begin();
    std::map<MultiAddressComponent, PoetsBoard*>::iterator  \
        boardIterator = boardMap->begin();
    for (boxIterator=boxMap->begin(); boxIterator!=boxMap->end();
         boxIterator++)
    {
        /* Contain 'boardsPerBox' boards in this box, making sure to flatten
           the address first. */
        for (unsigned boardIndex=0; boardIndex<boardsPerBox; boardIndex++)
        {
            flatAddress = flatten_address(boardIterator->first,
                                          boardWordLengths);
            boxIterator->second->contain(flatAddress, boardIterator->second);
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
    for (AddressComponent addressComponent=0; addressComponent < boxesInEngine;
         addressComponent++)
    {
        temporaryBox = new Box(dformat("Box%06d", addressComponent));
        temporaryBox->costBoxBoard = costBoxBoard;
        engine->contain(addressComponent, &temporaryBox);
    }
}

/* Populates a map passed as an argument with dynamically-allocated
   PoetsBoards. Arguments:

    - boards: Map containing poetsboards, mapped by non-flat address
      components.
*/
void Dialect1Deployer::populate_map_with_boards(
    std::map<MultiAddressComponent, PoetsBoard*>* boardMap)
{
    unsigned boardDimensions = boardsInEngine.size();

    /* A temporary address, one for each created board in the map. A vector is
       used over a std::array here to make the reduction operation in
       flatten_address easier to write. */
    MultiAddressComponent boardAddress(boardDimensions, 0);

    /* We loop until we have created all of the boards that we need to. */
    unsigned boardIndex = 0;  /* Increases monotonically. */
    bool looping = true;
    while(looping)
    {
        looping = false;

        /* Since there must be a first dimension, create a board for each
           element in this first dimension. */
        for (boardAddress[0]=0; boardAddress[0]<boardsInEngine[0];
             boardAddress[0]++)
        {
            /* Store */
            *boardMap[boardAddress] = new Board(dformat("Board%06d",
                                                        boardIndex));
            index++;
        }

        /* If there are higher dimensions, increment to the next one
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
        for (unsigned dimension=1; dimension<boardDimensions; dimension++)
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
            }
        }
    }
}
