/* Defines POETS Board behaviour (see the accompanying header for further
 * information). */

#include "P_board.h"

/* Constructs a POETS Board. Arguments:
 *
 * - name: Name of this board object (see namebase) */
P_board::P_board(std::string name)
{
    arcKey = 0;
    Name(name);

    /* Set up callbacks for the graph container (command pattern). */
    struct GraphCallbacks {
        BOARD_GRAPH_CALLBACK key(unsigned int const& key)
        {
            fprintf(dfp, "%u", key);
        }
        BOARD_GRAPH_CALLBACK node(P_mailbox* const& mailbox)
        {
            fprintf(mailbox->dfp, mailbox->FullName().c_str());
        }
        BOARD_GRAPH_CALLBACK arc(P_link* const& link)
        {
            fprintf(link->dfp, "%f", link->weight);
        }
    };

    G.SetNK_CB(GraphCallbacks::key);
    G.SetND_CB(GraphCallbacks::node);
    G.SetAK_CB(GraphCallbacks::key);
    G.SetAD_CB(GraphCallbacks::arc);
}

P_board::~P_board(){clear();}

/* Clears the dynamically-allocated elements of the data structure of this
 * board, deleting all contained components recursively. */
void P_board::clear()
{
    /* Clear the links of the graph object. */
    WALKPDIGRAPHARCS(AddressComponent, P_mailbox*,
                     unsigned, P_link*,
                     unsigned, P_port*,
                     G, iterator)
    {
        delete G.ArcData(iterator);  /* Deletes the edge. */
    }

    /* Clear all mailboxes that this board knows about. This in turn will clear
     * structures lower in the hierarchy recursively. */
    AddressComponent nodeKey;
    WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                      unsigned, P_link*,
                      unsigned, P_port*,
                      G, iterator)
    {
        if (iterator != G.NodeEnd())
        {
            delete G.NodeData(iterator);  /* Deletes the mailbox. */

            /* Also clear the ports of the graph while we're here. */
            nodeKey = G.NodeKey(iterator);
            WALKPDIGRAPHINPINS(AddressComponent, P_mailbox*,
                               unsigned, P_link*,
                               unsigned, P_port*,
                               G, nodeKey, pinIterator)
            {
                delete G.PinData(pinIterator);  /* Deletes the input port. */
            }

            WALKPDIGRAPHOUTPINS(AddressComponent, P_mailbox*,
                                unsigned, P_link*,
                                unsigned, P_port*,
                                G, nodeKey, pinIterator)
            {
                delete G.PinData(pinIterator);  /* Deletes the output port. */
            }
        }
    }

    /* Clear the graph object itself. */
    G.Clear();

    /* Supervisors begone! */
    sup_offv.clear();
}

/* Donates an uncontained mailbox to this board. Arguments:
 *
 * - addressComponent: Used to index the mailbox in this board.
 * - mailbox: Pointer to the mailbox object to contain. Must not already have a
 *   parent. */
void P_board::contain(AddressComponent addressComponent, P_mailbox* mailbox)
{
    /* Verify that the mailbox is unowned. */
    if (mailbox->parent != NULL)
    {
        std::stringstream errorMessage;
        errorMessage << "Mailbox \"" << mailbox->Name()
                     << "\" is already owned by board \""
                     << mailbox->parent->Name()
                     << "\". The mailbox's full name is \""
                     << mailbox->FullName() << "\".";
        throw OwnershipException(errorMessage.str());
    }

    /* Claim it. */
    mailbox->on_being_contained_hook(this);

    /* Verify that the mailbox is now owned by us. */
    if (mailbox->parent != this)
    {
        std::stringstream errorMessage;
        errorMessage << "Mailbox \"" << mailbox->Name()
                     << "\" not owned by this board (" << Name()
                     << ") after being claimed. The mailbox's full name is \""
                     << mailbox->FullName() << "\".";
        throw OwnershipException(errorMessage.str());
    }

    /* We don't care about the result of inserting the mailbox into this graph,
     * because we've checked the item is not owned before adding it. */
    G.InsertNode(addressComponent, mailbox);

    /* Define a hardware address for the mailbox, if we have a hardware
     * address. */
    if (isAddressBound)
    {
        HardwareAddress* mailboxHardwareAddress = copy_hardware_address();
        mailboxHardwareAddress->set_mailbox(addressComponent);
        mailbox->set_hardware_address(mailboxHardwareAddress);
    }
}

/* Connects two mailboxes together that are owned by this board. Arguments:
 *
 * - start, end: Address components of two mailbox objects owned by this board
 *   to connect together.
 * - weight: Edge weight for the connection.
 * - oneWay: If false, the connection is bidirectional, otherwise is
 *   unidirectional, from start to end. */
void P_board::connect(AddressComponent start, AddressComponent end,
                      float weight, bool oneWay)
{
    P_link* startToEndLink = new P_link(weight, this);
    if (!(G.InsertArc(arcKey++, start, end, startToEndLink)))
    {
        delete startToEndLink;
        throw OwnershipException(dformat("Connection from mailbox with index "
            "'%u' to mailbox with index '%u' failed.", start, end));
    }

    /* Make the reverse connection, if so desired. */
    if (!oneWay)
    {
        connect(end, start, weight, true);
    }
}

/* Write debug and diagnostic information about the POETS board, recursively,
 * using dumpchan. Arguments:
 *
 * - file: File to dump to. */
void P_board::Dump(FILE* file)
{
    std::string fullName = FullName();  /* Name of this from namebase. */
    std::string nameWithPrefix = dformat("P_board %s ", fullName.c_str());
    std::string breakerTail;
    if (nameWithPrefix.size() >= MAXIMUM_BREAKER_LENGTH)
    {
        breakerTail.assign("+");
    }
    else
    {
        breakerTail.assign(MAXIMUM_BREAKER_LENGTH - nameWithPrefix.size() - 1,
                           '+');
    }
    fprintf(file, "%s%s\n", nameWithPrefix.c_str(), breakerTail.c_str());

    /* About this object and its parent, if any. */
    NameBase::Dump(file);

    /* About the mailbox graph. */
    fprintf(file, "Mailbox connectivity in this board %s\n",
            std::string(44, '+').c_str());
    if (G.SizeNodes() == 0)
        fprintf(file, "The mailbox graph is empty.\n");
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
    fprintf(file, "Mailbox connectivity in this board %s\n",
            std::string(44, '-').c_str());

    /* About contained items, if any. */
    if (G.SizeNodes() > 0)
    {
        fprintf(file, "Mailboxes in this board %s\n",
                std::string(55, '+').c_str());

        /* Set up callbacks for walking through the mailbox nodes, passing in
         * the file pointer. */
        struct WalkCallbacks {
            BOARD_GRAPH_CALLBACK node(void* file, AddressComponent const&,
                                      P_mailbox* &mailbox)
            {
                mailbox->Dump(static_cast<FILE*>(file));
            }
        };

        /* Dump mailboxes in the graph recursively. */
        G.WALKNODES(file, WalkCallbacks::node);

        fprintf(file, "Mailboxes in this board %s\n",
                std::string(55, '-').c_str());
    }

    /* Close breaker and flush the dump. */
    std::replace(breakerTail.begin(), breakerTail.end(), '+', '-');
    fprintf(file, "%s%s\n", nameWithPrefix.c_str(), breakerTail.c_str());
    fflush(file);
}

/* Hook that a container calls to contain this object. Arguments:
 *
 * - container: Address of the box that contains this board. */
void P_board::on_being_contained_hook(P_box* container)
{
    parent = container;
    Npar(container);
}
