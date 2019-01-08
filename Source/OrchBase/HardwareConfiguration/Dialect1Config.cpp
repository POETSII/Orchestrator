/* Defines logic for deploying a dialect 1-style configuration to an engine and
   address format. Arguments:

    - engine: POETS Engine to modify in-place.

    - addressFormat: Address format to populate.

   Configurations are deployed statically, but dynamically create objects
   within the static objects. Your engine and addressFormat should be
   constructed by your containing object, and passed in by reference.

   Validation must be performed by the logic that populates these configuration
   objects. If you don't validate your input, your deployment will fall over.

   See accompanying header for the set of variables that are used during
   deployment. */
void Dialect1Config::deploy(PoetsEngine* engine,
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
    std::map<AddressComponent, PoetsBoard*> boards;
    populate_map_with_boards(&boards);

    /* Divide up the boards between the boxes naively, and assign them. */
    populate_boxes_evenly_with_boards(&(engine->PoetsBoxes), &boards);
}


/* Distributes all boards in a map to all boxes evenly, arbitrarily. Assumes
   that the number of boards divides into the number of boxes without
   remainder. Arguments:

    - boxMap: Boxes, mapped by their address components.
    - boardMap: All boards (previously uncontained), mapped by their address
      components.

   Modifies the boxes inplace.
*/
void Dialect1Config::populate_boxes_evenly_with_boards(
    std::map<AddressComponent, PoetsBox*>* boxMap,
    std::map<AddressComponent, PoetsBoard*>* boardMap);
{
    /* An even distribution (we hope). */
    unsigned boardsPerBox = boardMap.size() / boxMap.size();

    /* We iterate through the map of boxes and boards, distributing all boards
       up to a given amount, such that an even distribution is maintained. */
    std::map<AddressComponent, PoetsBox*>::iterator \
        boxIterator = boxMap->begin();
    std::map<AddressComponent, PoetsBoard*>::iterator \
        boardIterator = boardMap->begin();
    for (boxIterator=boxMap->begin(); boxIterator!=boxMap->end();
         boxIterator++)
    {
        /* Contain 'boardsPerBox' boards in this box. */
        for (unsigned boardIndex=0; boardIndex<boardsPerBox; boardIndex++)
        {
            boxIterator->second->contain(boardIterator->first,
                                         boardIterator->second);
            boardIterator++;
        }
    }
}

/* Assigns all metadata defined in this config to the engine. Arguments:

    - engine: the PoetsEngine to assign metadata to.
*/
void Dialect1Config::assign_metadata_to_engine(PoetsEngine* engine)
{
    engine->author = author;
    engine->datetime = datetime;
    engine->version = version;
    engine->fileOrigin = fileOrigin;
}

/* Assigns all sizes to an address format object. Arguments

    - format: the HardwareAddressObject to assign sizes to.
*/
void Dialect1Config::assign_sizes_to_address_format(
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
AddressComponent Dialect1Config::flatten_address(
    std::vector<AddressComponent> address,
    std::vector<unsigned> wordLengths)
{
    AddressComponent returnValue = 0;
    for (unsigned dimension=0; dimension<address.size; dimension++)
    {
        returnValue |= address[dimension];
        returnValue << wordLengths[dimension];
    }
    return returnValue;
}

/* Populates a PoetsEngine with boxes, and defines the box-board costs in those
   boxes. Arguments:

    - engine: PoetsEngine to populate. */
void Dialect1Config::populate_engine_with_boxes_and_their_costs(
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

    - boards: Map containing poetsboards, mapped by (flat) address components.

 */
void Dialect1Config::populate_map_with_boards(
    std::map<AddressComponent, PoetsBoard*>* boardMap)
{
    AddressComponent flatAddress;
    unsigned boardDimensions = boardsInEngine.size();

    /* A temporary address, one for each created board in the map. A vector is
       used over a std::array here to make the reduction operation in
       flatten_address easier to write. */
    std::vector<AddressComponent> boardAddress(boardDimensions, 0);

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
            /* Create a flattened address. */
            flatAddress = flatten_address(boardAddress, boardWordLengths);

            /* Store */
            *boardMap[flatAddress] = new Board(dformat("Board%06d",
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
