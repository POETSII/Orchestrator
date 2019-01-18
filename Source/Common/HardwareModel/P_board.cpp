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
        CALLBACK node_key(AddressComponent const& key){printf("%u", key);}
        CALLBACK node(P_mailbox* const& mailbox)
        {
            printf("%s", mailbox->FullName().c_str());
        }
        CALLBACK arc_key(unsigned int const& key){printf("%u", key);}
        CALLBACK arc(P_link* const& link){printf("%f", link->weight);}
    };

    G.SetNK_CB(GraphCallbacks::node_key);
    G.SetND_CB(GraphCallbacks::node);
    G.SetAK_CB(GraphCallbacks::arc_key);
    G.SetAD_CB(GraphCallbacks::arc);
}

P_board::~P_board()
{
    clear();
    sup_offv.clear();
}

/* Clears the dynamically-allocated elements of the data structure of this
 * board, deleting all contained components recursively. */
void P_board::clear()
{
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
            delete G.NodeData(iterator);

            /* Also clear the ports of the graph while we're here. */
            nodeKey = G.NodeKey(iterator);
            WALKPDIGRAPHINPINS(AddressComponent, P_mailbox*,
                               unsigned, P_link*,
                               unsigned, P_port*,
                               G, nodeKey, pinIterator)
            {
                delete G.PinData(pinIterator);
            }

            WALKPDIGRAPHOUTPINS(AddressComponent, P_mailbox*,
                                unsigned, P_link*,
                                unsigned, P_port*,
                                G, nodeKey, pinIterator)
            {
                delete G.PinData(pinIterator);
            }
        }


    }

    /* Clear the graph object itself. */
    G.Clear();
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
 * - start, end: Pointers to two mailbox objects to connect. Must be owned by
 *   this board.
 * - weight: Edge weight for the connection.
 * - oneWay: If false, the connection is bidirectional, otherwise is
 *   unidirectional, from start to end. */
void P_board::connect(AddressComponent start, AddressComponent end,
                      float weight, bool oneWay)
{
    P_link* startToEndLink = new P_link();
    startToEndLink->AutoName();
    startToEndLink->weight = weight;
    G.InsertArc(arcKey++, start, end, startToEndLink);
    if (!oneWay)
    {
        P_link* endToStartLink = new P_link();
        endToStartLink->AutoName();
        endToStartLink->weight = weight;
        G.InsertArc(arcKey++, end, start, endToStartLink);
    }
}

/* Write debug and diagnostic information about the POETS board, recursively,
 * using dumpchan. Arguments:
 *
 * - file: File to dump to. */
void P_board::dump(FILE* file)
{
    std::string fullName = FullName();  /* Name of this from namebase. */

    /* About this object and its parent, if any. */
    char breaker[MAXIMUM_BREAKER_LENGTH + 1];
    int breakerLength = sprintf(breaker, "P_board %s ", fullName.c_str());
    for(int index=breakerLength; index<MAXIMUM_BREAKER_LENGTH - 1;
        breaker[index++]='+');
    breaker[MAXIMUM_BREAKER_LENGTH - 1] = '\n';
    breaker[MAXIMUM_BREAKER_LENGTH] = '\0';
    fprintf(file, "%s", breaker);
    NameBase::Dump(file);

    /* About the mailbox graph. */
    fprintf(file, "Mailbox connectivity in this board %s\n",
            std::string(44, '+').c_str());
    if (G.SizeNodes() == 0)
        fprintf(file, "The mailbox graph is empty.\n");
    else
    {
        /* Dump graph (which does not dump items). */
        G.DumpChan(file);
        G.Dump();
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
            CALLBACK node(void* file, AddressComponent const&,
                          P_mailbox* &mailbox)
            {
                mailbox->dump(static_cast<FILE*>(file));
            }
        };

        /* Dump mailboxes in the graph recursively. */
        G.WALKNODES(file, WalkCallbacks::node);

        fprintf(file, "Mailboxes in this board %s\n",
                std::string(55, '-').c_str());
    }

    /* Close breaker and flush the dump. */
    for(int index=breakerLength; index<MAXIMUM_BREAKER_LENGTH - 1;
        breaker[index++]='-');
    fprintf(file, "%s", breaker);
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
