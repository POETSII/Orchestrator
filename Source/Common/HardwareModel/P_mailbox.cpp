/* Defines POETS Mailbox behaviour (see the accompanying header for further
 * information). */

#include "P_mailbox.h"

/* Constructs a POETS Mailbox. Arguments:
 *
 * - name: Name of this mailbox object (see namebase) */
P_mailbox::P_mailbox(std::string name)
{
    parent = PNULL;
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

    /* Define a hardware address for the core. */
    if (isAddressBound)
    {
        HardwareAddress* coreHardwareAddress = copy_hardware_address();
        coreHardwareAddress->set_core(addressComponent);
        core->set_hardware_address(coreHardwareAddress);
    }
}

/* Write debug and diagnostic information about the POETS mailbox, recursively,
 * using dumpchan. Arguments:
 *
 * - file: File to dump to. */
void P_mailbox::Dump(FILE* file)
{
    std::string prefix = dformat("P_mailbox %s", FullName().c_str());
    DumpUtils::open_breaker(file, prefix);

    /* About this object and its parent, if any. */
    NameBase::Dump(0,file);

    /* About contained items, if any. */
    DumpUtils::open_breaker(file, "Cores in this mailbox");
    if (P_corem.empty())
        fprintf(file, "The core map is empty.\n");
    else
    {
        WALKMAP(AddressComponent, P_core*, P_corem, iterator)
        {
            /* Recursive-dump. */
            iterator->second->Dump(file);
        }
    }
    DumpUtils::close_breaker(file, "Cores in this mailbox");

    /* Close breaker and flush the dump. */
    DumpUtils::close_breaker(file, prefix);
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
