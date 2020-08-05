/* Defines POETS Engine behaviour (see the accompanying header for further
 * information). */

#include "P_engine.h"

/* Constructs a POETS Engine. Arguments:
 *
 * - name: Name of this engine object (see namebase) */
P_engine::P_engine(std::string name)
{
    arcKey = 0;
    portKey = 0;
    Name(name);

    /* Set up callbacks for the graph container (command pattern). Note that we
     * don't print the boards recursively; boards are printed when the boxes
     * are printed, though we still dump the connectivity information from the
     * graph of boards. */
    struct GraphCallbacks {
        GRAPH_CALLBACK all_keys(unsigned int const& key)
        {
            fprintf(dfp, "%u", key);
        }
        GRAPH_CALLBACK node(P_board* const& board)
        {
            fprintf(board->dfp, "%s", board->FullName().c_str());
        }
        GRAPH_CALLBACK arc(P_link* const& link)
        {
            fprintf(link->dfp, "%f", link->weight);
        }
        GRAPH_CALLBACK port(P_port* const& port)
        {
            fprintf(dfp, "%#018lx", (uint64_t) port);
        }
    };

    G.SetNK_CB(GraphCallbacks::all_keys);
    G.SetND_CB(GraphCallbacks::node);
    G.SetAK_CB(GraphCallbacks::all_keys);
    G.SetAD_CB(GraphCallbacks::arc);
    G.SetPK_CB(GraphCallbacks::all_keys);
    G.SetPD_CB(GraphCallbacks::port);

    /* Set up default metadata information. If these are unchanged, the engine
     * will not print them when dump is called (strings are initialised
     * empty). */
    author = "(undefined)";
    datetime = 0;
    version = "(undefined)";
    fileOrigin = "(undefined)";
}

P_engine::~P_engine(){clear();}

/* Clears the dynamically-allocated elements of the data structure of this
 * engine, deleting all contained components recursively. */
void P_engine::clear()
{
    /* Clear all boxes that this engine knows about. This should clear
       recursively. Since the engine cannot contain boards that are not
       contained by its boxes, the graph of boards does not need to be
       cleared in this way. */
    WALKMAP(AddressComponent, P_box*, P_boxm, iterator)
    {
        delete iterator->second;
    }
    P_boxm.clear();

    /* Clear the links of the graph object. */
    WALKPDIGRAPHARCS(AddressComponent,P_board*,
                     unsigned,P_link*,
                     unsigned,P_port*,G,arcIterator)
    {
        delete G.ArcData(arcIterator);
    }

    /* Clear the ports of the graph object (by iterating through the nodes). */
    AddressComponent nodeKey;
    WALKPDIGRAPHNODES(AddressComponent, P_board*,
                      unsigned, P_link*,
                      unsigned, P_port*, G, nodeIterator)
    {
        nodeKey = G.NodeKey(nodeIterator);
        WALKPDIGRAPHINPINS(AddressComponent, P_board*,
                           unsigned, P_link*,
                           unsigned, P_port*, G, nodeKey, pinIterator)
        {
            delete G.PinData(pinIterator);
        }

        WALKPDIGRAPHOUTPINS(AddressComponent, P_board*,
                            unsigned, P_link*,
                            unsigned, P_port*, G, nodeKey, pinIterator)
        {
            delete G.PinData(pinIterator);
        }
    }

    /* But we do want to clear the graph object itself, even though the boards
     * inside it have been freed by this point. */
    G.Clear();
}

/* Donates an uncontained box to this engine. Arguments:
 *
 * - addressComponent: Used to index the box in this engine.
 * - box: Pointer to the box object to contain. Must not already have a
 *        parent. */
void P_engine::contain(AddressComponent addressComponent, P_box* box)
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

    /* Verify that a box with that address doesn't already exist, to avoid
     * clobbering. */
    std::map<AddressComponent, P_box*>::iterator boxFinder;
    boxFinder = P_boxm.find(addressComponent);
    if (boxFinder != P_boxm.end())
    {
        std::stringstream errorMessage;
        errorMessage << "Cannot add box \"" << box->Name()
                     << "\" with address component \"" << addressComponent
                     << "\" because box \"" << boxFinder->second->Name()
                     << "\" already exists at that address component.";
        throw OwnershipException(errorMessage.str());
    }

    P_boxm[addressComponent] = box;

    /* Define a hardware address for the box. */
    HardwareAddress* boxHardwareAddress = new HardwareAddress(&addressFormat);
    boxHardwareAddress->set_box(addressComponent);
    box->set_hardware_address(boxHardwareAddress);
}

/* Donates a board to this engine. The board must be contained by a box, which
 * is in turn contained by this engine. Arguments:
 *
 * - addressComponent: Used to index the box in this engine.
 * - board: Pointer to the board object to contain. */
void P_engine::contain(AddressComponent addressComponent, P_board* board)
{
    /* Verify that the board is owned by a box, which is owned by this
     * engine. The predicates evaluated in order to prevent segfaulting. NULL
     * has no parent, but all boxes define a parent. */
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
    if (!G.InsertNode(addressComponent, board))
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
 * owns. Arguments:
 *
 * - start, end: Pointers to two board objects to connect. Must be owned by
 *   boxes that this engine owns.
 * - weight: Edge weight for the connection.
 * - oneWay: If false, the connection is bidirectional, otherwise is
 *   unidirectional, from start to end. */
void P_engine::connect(AddressComponent start, AddressComponent end,
                       float weight, bool oneWay)
{
    P_link* startToEndLink = new P_link(weight, this);

    /* We'll be needing ports at each end. */
    P_port* startPort = new P_port(this);
    P_port* endPort = new P_port(this);
    unsigned int startPortKey = portKey++;
    unsigned int endPortKey = portKey++;

    if (!(G.InsertArc(arcKey++, start, end, startToEndLink,
                      startPortKey, startPort,
                      endPortKey, endPort)))
    {
        /* We didn't add the edge successfully, so we should free the memory
         * held by the objects that we've just created. */
        delete startToEndLink;
        delete startPort;
        delete endPort;

        throw OwnershipException(dformat("Connection from board with index "
            "'%u' to board with index '%u' failed.", start, end));
    }

    /* Make the reverse connection, if so desired. */
    if (!oneWay)
    {
        connect(end, start, weight, true);
    }
}

/* Write debug and diagnostic information about the POETS engine, recursively,
 * using dumpchan. Arguments:
 *
 * - file: File to dump to. */
void P_engine::Dump(FILE* file)
{
    std::string prefix = dformat("P_engine %s", FullName().c_str());
    DumpUtils::open_breaker(file, prefix);

    /* About this object. */
    NameBase::Dump(file);

    /* Metadata from a configuration file, if set. */
    if (!author.empty())
    {
        fprintf(file, "Author: %s\n", author.c_str());
    }
    if (datetime > 0)
    {
        fprintf(file, "Configuration datetime (YYYYMMDDHHmmss): %lld\n",
                datetime);
    }
    if (!version.empty())
    {
        fprintf(file, "Written for file reader version: %s\n",
                version.c_str());
    }
    if (!fileOrigin.empty())
    {
        fprintf(file, "Read from file: %s\n", fileOrigin.c_str());
    }

    /* About the board graph. */
    DumpUtils::open_breaker(file, "Board connectivity");
    if (G.SizeNodes() == 0)
        fprintf(file, "The board graph is empty.\n");
    else
    {
        /* Change channels to the output file, storing the old values */
        FILE* previousBoardChannel = P_board::dfp;
        FILE* previousLinkChannel = P_link::dfp;
        FILE* previousPortChannel = P_port::dfp;
        P_board::dfp = file;
        P_link::dfp = file;
        P_port::dfp = file;

        /* Dump graph (which does not dump items). */
        G.DumpChan(file);
        G.Dump();

        /* Set the old channels back. */
        P_board::dfp = previousBoardChannel;
        P_link::dfp = previousLinkChannel;
        P_port::dfp = previousPortChannel;
    }
    DumpUtils::close_breaker(file, "Board connectivity");

    /* About contained boxes, if any. */
    DumpUtils::open_breaker(file, "Boxes in this engine");
    if (P_boxm.empty())
        fprintf(file, "The box map is empty.\n");
    else
    {
        WALKMAP(AddressComponent, P_box*, P_boxm, iterator)
        {
            /* Recursive-dump. */
            iterator->second->Dump(file);
        }
    }
    DumpUtils::close_breaker(file, "Boxes in this engine");

    /* Close breaker and flush the dump. */
    DumpUtils::close_breaker(file, prefix);
    fflush(file);
}

/* Defines whether or not the engine is empty.
 *
 * An engine is empty if it contains no boxes. Engines cannot contain boards
 * without first containing boxes, and since the only way to remove boxes and
 * boards from an engine is by clearing it completely, checking for boxes alone
 * is enough. */
bool P_engine::is_empty(){return P_boxm.empty();}
