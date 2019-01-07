/* Defines POETS Engine behaviour (see the accompanying header for further
   information). */

#include "PoetsEngine.h"

/* Constructs a POETS Engine. Arguments:
   - name: Name of this engine object (see namebase)
*/
PoetsEngine::PoetsEngine(std::string name)
{
    Name(name);

    /* Set up callbacks for the graph container (command pattern). Note that we
       don't print the boards recursively; boards are printed when the boxes
       are printed, though we still dump the connectivity information from the
       graph of boards. */
    struct GraphCallbacks {
        CALLBACK node_key(AddressComponent const& key){printf("%u", key);}
        CALLBACK node(PoetsBoard* const& board)
        {
            printf("%s", board->FullName().c_str());
        }
        CALLBACK arc_key(unsigned int const& key){printf("%u", key);}
        CALLBACK arc(float const& weight){printf("%f", weight);}
    };

    PoetsBoards.SetNK_CB(GraphCallbacks::node_key);
    PoetsBoards.SetND_CB(GraphCallbacks::node);
    PoetsBoards.SetAK_CB(GraphCallbacks::arc_key);
    PoetsBoards.SetAD_CB(GraphCallbacks::arc);

    /* Set up default metadata information. If these are unchanged, the engine
       will not print them when dump is called (strings are initialised
       empty). */
    datetime = 0;
}

PoetsEngine::~PoetsEngine(){clear();}

/* Clears the dynamically-allocated elements of the data structure of this
   engine, deleting all contained components recursively.
*/
void PoetsEngine::clear()
{
    /* Clear all boxes that this engine knows about. This should clear
       recursively. Since the engine cannot contain boards that are not
       contained by its boxes, the graph of boards does not need to be
       cleared in this way. */
    WALKMAP(AddressComponent,PoetsBox*,PoetsBoxes,iterator)
    {
        delete iterator->second;
    }
    PoetsBoxes.clear();

    /* But we do want to clear the graph object itself, even though the boards
       inside it have been freed by this point. */
    PoetsBoards.Clear();
}

/* Donates an uncontained box to this engine. Arguments:

   - addressComponent: Used to index the box in this engine.
   - box: Pointer to the box object to contain. Must not already have a parent.
*/
void PoetsEngine::contain(AddressComponent addressComponent, PoetsBox* box)
{
    /* Verify that the box is unowned. */
    if (box->parent != NULL)
    {
        std::stringstream errorMessage;
        errorMessage << "Box \"" << box->Name()
                     << "\" is already owned by engine \""
                     << box->parent->Name()
                     << "\". The box's full name is \"" << box->FullName()
                     << "\".";
        throw OwnershipException(errorMessage.str());
    }

    /* Claim it. */
    box->on_being_contained_hook(this);

    /* Verify that the box is now owned by us. */
    if (box->parent != this)
    {
        std::stringstream errorMessage;
        errorMessage << "Box \"" << box->Name()
                     << "\" not owned by this engine (" << Name()
                     << ") after being claimed. The box's full name is \""
                     << box->FullName() << "\".";
        throw OwnershipException(errorMessage.str());
    }

    PoetsBoxes[addressComponent] = box;
}

/* Donates a board to this engine. The board must be contained by a box, which
   is in turn contained by this engine. Arguments:

   - addressComponent: Used to index the box in this engine.
   - board: Pointer to the board object to contain.
*/
void PoetsEngine::contain(AddressComponent addressComponent, PoetsBoard* board)
{
    /* Verify that the board is owned by a box, which is owned by this
       engine. The predicates evaluated in order to prevent segfaulting. NULL
       has no parent, but all boxes define a parent. */
    bool raisingOnUnownedBoard = false;
    if (board->parent == NULL) raisingOnUnownedBoard = true;
    else if (board->parent->parent != this) raisingOnUnownedBoard = true;
    if (raisingOnUnownedBoard)
    {
        std::stringstream errorMessage;
        errorMessage << "Board \"" << board->Name()
                     << "\" is not owned by a box owned by this engine. "
                     << "The board's full name is \"" << board->FullName()
                     << "\".";
        throw OwnershipException(errorMessage.str());
    }

    /* The operation in the predicate performs the containment. */
    if (!PoetsBoards.InsertNode(addressComponent, board))
    {
        std::stringstream errorMessage;
        errorMessage << "Board \"" << board->Name()
            << "\" is already present in the graph held by this engine ("
            << Name()
            << "). The board's full name is \"" << board->FullName() << "\".";
        throw OwnershipException(errorMessage.str());
    }
}

/* Connects two boards together that are owned by boxes that this engine
   owns. The connection is bidirectional. Arguments:

   - start, end: Pointers to two board objects to connect. Must be owned by
     boxes that this engine owns.
   - weight: Edge weight for the connection.
*/
void PoetsEngine::connect(AddressComponent start, AddressComponent end,
                      float weight)
{
    PoetsBoards.InsertArc(arcKey++, start, end, weight);
    PoetsBoards.InsertArc(arcKey++, end, start, weight);
}

/* Write debug and diagnostic information about the POETS engine, recursively,
   using dumpchan. Arguments:

   - file: File to dump to.
*/
void PoetsEngine::dump(FILE* file)
{
    std::string fullName = FullName();  /* Name of this from namebase. */

    /* About this object. */
    char breaker[MAXIMUM_BREAKER_LENGTH + 1];
    int breakerLength = sprintf(breaker, "PoetsEngine %s ", fullName.c_str());
    for(int index=breakerLength; index<MAXIMUM_BREAKER_LENGTH - 1;
        breaker[index++]='+');
    breaker[MAXIMUM_BREAKER_LENGTH - 1] = '\n';
    breaker[MAXIMUM_BREAKER_LENGTH] = '\0';
    fprintf(file, "%s", breaker);
    NameBase::Dump(file);

    /* Metadata from a configuration file, if set. */
    if (!author.empty())
    {
        fprintf(file, "Author: %s\n", author.c_str());
    }
    if (datetime > 0)
    {
        fprintf(file, "Configuration datetime (YYYYMMDDHHmmss): %ld\n",
                datetime);
    }
    if (!version.empty())
    {
        fprintf(file, "Written for parser version: %s\n", version.c_str());
    }
    if (!fileOrigin.empty())
    {
        fprintf(file, "Read from file: %s\n", fileOrigin.c_str());
    }

    /* About the board graph. */
    fprintf(file, "Board connectivity in this engine %s\n",
            std::string(45, '+').c_str());
    if (PoetsBoards.SizeNodes() == 0)
        fprintf(file, "The board graph is empty.\n");
    else
    {
        /* Dump graph (which does not dump items). */
        PoetsBoards.Dump();
    }
    fprintf(file, "Board connectivity in this engine %s\n",
            std::string(45, '-').c_str());

    /* About contained boxes, if any. */
    fprintf(file, "Boxes in this engine %s\n", std::string(58, '+').c_str());
    if (PoetsBoxes.empty())
        fprintf(file, "The box map is empty.\n");
    else
    {
        WALKMAP(AddressComponent,PoetsBox*,PoetsBoxes,iterator)
        {
            PoetsBox* iterThread = iterator->second;
            /* Print information from the map. */
            fprintf(file, "%u: %s (%p)\n",
                    iterator->first,
                    iterThread->FullName().c_str(), iterThread);
            /* Recursive-dump. */
            iterThread->dump();
        }
    }
    fprintf(file, "Boxes in this engine %s\n", std::string(58, '-').c_str());

    /* Close breaker and flush the dump. */
    for(int index=breakerLength; index<MAXIMUM_BREAKER_LENGTH - 1;
        breaker[index++]='-');
    fprintf(file, "%s", breaker);
    fflush(file);
}

/* Defines whether or not the engine is empty.

   An engine is empty if it contains no boxes. Engines cannot contain boards
   without first containing boxes, and since the only way to remove boxes and
   boards from an engine is by clearing it completely, checking for boxes alone
   is enough. */
bool PoetsEngine::is_empty(){return PoetsBoxes.empty();}
