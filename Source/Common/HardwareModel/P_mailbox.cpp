/* Defines POETS Mailbox behaviour (see the accompanying header for further
 * information). */

#include "P_mailbox.h"

/* Constructs a POETS Mailbox. Arguments:
 *
 * - name: Name of this mailbox object (see namebase) */
P_mailbox::P_mailbox(std::string name)
{
    Name(name);
}

P_mailbox::~P_mailbox(){clear();}

/* Clears the dynamically-allocated elements of the data structure of this
 * mailbox, deleting all contained components recursively. */
void P_mailbox::clear()
{
    /* Clear all cores that this mailbox knows about. This should clear
     * recursively. */
    WALKMAP(AddressComponent,P_core*,P_corem,iterator)
    {
        delete iterator->second;
    }
    P_corem.clear();
}

/* Donates an uncontained core to this mailbox. Arguments:
 *
 * - addressComponent: Used to index the core in this mailbox.
 * - core: Pointer to the core object to contain. Must not already have a
 *   parent. */
void P_mailbox::contain(AddressComponent addressComponent, P_core* core)
{
    /* Verify that the core is unowned. */
    if (core->parent != NULL)
    {
        std::stringstream errorMessage;
        errorMessage << "Core \"" << core->Name()
                     << "\" is already owned by mailbox \""
                     << core->parent->Name()
                     << "\". The core's full name is \"" << core->FullName()
                     << "\".";
        throw OwnershipException(errorMessage.str());
    }

    /* Claim it. */
    core->on_being_contained_hook(this);

    /* Verify that the core is now owned by us. */
    if (core->parent != this)
    {
        std::stringstream errorMessage;
        errorMessage << "Core \"" << core->Name()
                     << "\" not owned by this mailbox (" << Name()
                     << ") after being claimed. The core's full name is \""
                     << core->FullName() << "\".";
        throw OwnershipException(errorMessage.str());
    }

    P_corem[addressComponent] = core;
}

/* Write debug and diagnostic information about the POETS mailbox, recursively,
 * using dumpchan. Arguments:
 *
 * - file: File to dump to. */
void P_mailbox::dump(FILE* file)
{
    std::string fullName = FullName();  /* Name of this from namebase. */

    /* About this object and its parent, if any. */
    char breaker[MAXIMUM_BREAKER_LENGTH + 1];
    int breakerLength = sprintf(breaker, "P_mailbox %s ", fullName.c_str());
    for(int index=breakerLength; index<MAXIMUM_BREAKER_LENGTH - 1;
        breaker[index++]='+');
    breaker[MAXIMUM_BREAKER_LENGTH - 1] = '\n';
    breaker[MAXIMUM_BREAKER_LENGTH] = '\0';
    fprintf(file, "%s", breaker);
    NameBase::Dump(file);

    /* About contained items, if any. */
    fprintf(file, "Cores in this mailbox %s\n", std::string(57, '+').c_str());
    if (P_corem.empty())
        fprintf(file, "The core map is empty.\n");
    else
    {
        WALKMAP(AddressComponent,P_core*,P_corem,iterator)
        {
            P_core* iterCore = iterator->second;
            /* Print information from the map. */
            fprintf(file, "%u: %s (%p)\n",
                    iterator->first,
                    iterCore->FullName().c_str(), iterCore);
            /* Recursive-dump. */
            iterCore->dump();
        }
    }
    fprintf(file, "Cores in this mailbox %s\n", std::string(57, '-').c_str());

    /* Close breaker and flush the dump. */
    for(int index=breakerLength; index<MAXIMUM_BREAKER_LENGTH - 1;
        breaker[index++]='-');
    fprintf(file, "%s", breaker);
    fflush(file);
}

/* Hook that a container calls to contain this object. Arguments:
 *
 * - container: Address of the board that contains this mailbox. */
void P_mailbox::on_being_contained_hook(P_board* container)
{
    parent = container;
    Npar(container);
}
