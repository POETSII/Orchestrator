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
Dialect1config::deploy(PoetsEngine* engine,
                       HardwareAddressFormat* addressFormat)
{
    /* Temporary staging variables for items, which are allocated
       dynamically. */
    PoetsBox* temporaryBox;

    /* Assign metadata to the engine. */
    engine->author = author;
    engine->datetime = datetime;
    engine->version = version;
    engine->fileOrigin = fileOrigin;

    /* Assign format sizes to the address format, sum-reducing the vectors. */
    addressFormat->boxWordLength = boxWordLength;
    addressFormat->coreWordLength = coreWordLength;
    addressFormat->threadWordLength = threadWordLength;
    addressFormat->boardWordLength = \
        std::accumulate(boardWordLengths.begin(), boardWordLengths.end(), 0);
    addressFormat->mailboxWordLength = \
        std::accumulate(mailboxWordLengths.begin(),
                        mailboxWordLengths.end(), 0);

    /* Engine costings */
    engine->costExternalBox = costExternalBox;

    /* Create and contain a box for each box we need, and define the box-board
       cost while we're at it. */
    for (AddressComponent addressComponent=0; addressComponent < boxesInEngine;
         addressComponent++)
    {
        temporaryBox = new Box(dformat("Box%06d", addressComponent));
        temporaryBox->costBoxBoard = costBoxBoard;
        engine->contain(addressComponent, &temporaryBox);
    }

    /* Divide boards intelligently into boxes.



}
