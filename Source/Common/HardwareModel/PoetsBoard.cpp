/* Defines POETS Board behaviour (see the accompanying header for further
   information). */

#include "PoetsBoard.h"

/* Constructs a POETS Board. Arguments:
   - name: Name of this board object (see namebase)
*/
PoetsBoard::PoetsBoard(std::string name)
{
    arcKey = 0;
    Name(name);

    /* Set up callbacks for the graph container (command pattern). */
    struct GraphCallbacks {
        CALLBACK node_key(AddressComponent const& key){printf("%u", key);}
        CALLBACK node(PoetsMailbox* const& mailbox)
        {
            printf("%s", mailbox->FullName().c_str());
        }
        CALLBACK arc_key(unsigned int const& key){printf("%u", key);}
        CALLBACK arc(float const& weight){printf("%f", weight);}
    };

    PoetsMailboxes.SetNK_CB(GraphCallbacks::node_key);
    PoetsMailboxes.SetND_CB(GraphCallbacks::node);
    PoetsMailboxes.SetAK_CB(GraphCallbacks::arc_key);
    PoetsMailboxes.SetAD_CB(GraphCallbacks::arc);
}

PoetsBoard::~PoetsBoard(){clear();}

/* Clears the dynamically-allocated elements of the data structure of this
   board, deleting all contained components recursively.
*/
void PoetsBoard::clear()
{
    /* Clear all mailboxes that this board knows about. This in turn will clear
       structures lower in the hierarchy recursively. */
    WALKPDIGRAPHNODES(AddressComponent, PoetsMailbox*,
                      unsigned int, float,
                      unsigned int, unsigned int,
                      PoetsMailboxes, iterator)
    {
        if (iterator != PoetsMailboxes.NodeEnd())
        {
            delete PoetsMailboxes.NodeData(iterator);
        }
    }

    /* Clear the graph object itself. */
    PoetsMailboxes.Clear();
}

/* Donates an uncontained mailbox to this board. Arguments:

   - addressComponent: Used to index the mailbox in this board.
   - mailbox: Pointer to the mailbox object to contain. Must not already have a
     parent.
*/
void PoetsBoard::contain(AddressComponent addressComponent,
                         PoetsMailbox* mailbox)
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
       because we've checked the item is not owned before adding it. */
    PoetsMailboxes.InsertNode(addressComponent, mailbox);
}

/* Connects two mailboxes together that are owned by this board. The connection
   is bidirectional. Arguments:

   - start, end: Pointers to two mailbox objects to connect. Must be owned by
     this board.
   - weight: Edge weight for the connection.
*/
void PoetsBoard::connect(AddressComponent start, AddressComponent end,
                      float weight)
{
    PoetsMailboxes.InsertArc(arcKey++, start, end, weight);
    PoetsMailboxes.InsertArc(arcKey++, end, start, weight);
}

/* Write debug and diagnostic information about the POETS board, recursively,
   using dumpchan. Arguments:

   - file: File to dump to.
*/
void PoetsBoard::dump(FILE* file)
{
    std::string fullName = FullName();  /* Name of this from namebase. */

    /* About this object and its parent, if any. */
    char breaker[MAXIMUM_BREAKER_LENGTH + 1];
    int breakerLength = sprintf(breaker, "PoetsBoard %s ", fullName.c_str());
    for(int index=breakerLength; index<MAXIMUM_BREAKER_LENGTH - 1;
        breaker[index++]='+');
    breaker[MAXIMUM_BREAKER_LENGTH - 1] = '\n';
    breaker[MAXIMUM_BREAKER_LENGTH] = '\0';
    fprintf(file, "%s", breaker);
    NameBase::Dump(file);

    /* About the mailbox graph. */
    fprintf(file, "Mailbox connectivity in this board %s\n",
            std::string(44, '+').c_str());
    if (PoetsMailboxes.SizeNodes() == 0)
        fprintf(file, "The mailbox graph is empty.\n");
    else
    {
        /* Dump graph (which does not dump items). */
        PoetsMailboxes.Dump();
    }
    fprintf(file, "Mailbox connectivity in this board %s\n",
            std::string(44, '-').c_str());

    /* About contained items, if any. */
    if (PoetsMailboxes.SizeNodes() > 0)
    {
        fprintf(file, "Mailboxes in this board %s\n",
                std::string(55, '+').c_str());

        /* Set up callbacks for walking through the mailbox nodes. */
        struct WalkCallbacks {
            CALLBACK node(void*, AddressComponent const&,
                          PoetsMailbox* &mailbox)
            {
                mailbox->dump();
            }
        };

        /* Dump mailboxes in the graph recursively. */
        PoetsMailboxes.WALKNODES(NULL, WalkCallbacks::node);

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
   - container: Address of the box that contains this board.
*/
void PoetsBoard::on_being_contained_hook(PoetsBox* container)
{
    parent = container;
    Npar(container);
}
