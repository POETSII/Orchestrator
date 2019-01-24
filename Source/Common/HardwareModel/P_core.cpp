/* Defines POETS Core behaviour (see the accompanying header for further
 * information). */

#include "P_core.h"

/* Constructs a POETS Core. Arguments:
 *
 * - name: Name of this core object (see namebase) */
P_core::P_core(std::string name)
{
    Name(name);
    dataBinary = new Bin();
    instructionBinary = new Bin();
}

P_core::~P_core(){clear();}

/* Clears the dynamically-allocated elements of the data structure of this
 * core, deleting all contained components recursively. */
void P_core::clear()
{
    /* Clear all threads that this core knows about. This should clear
     * recursively. */
    WALKMAP(AddressComponent,P_thread*,P_threadm,iterator)
    {
        delete iterator->second;
    }
    P_threadm.clear();

    clear_binaries();
}

/* Clears the dynamically-allocated binaries. */
void P_core::clear_binaries()
{
    if (dataBinary != 0) delete dataBinary;
    if (instructionBinary != 0) delete instructionBinary;
}

/* Donates an uncontained thread to this core. Arguments:
 *
 * - addressComponent: Used to index the thread in this core.
 * - thread: Pointer to the thread object to contain. Must not already have a
 *   parent. */
void P_core::contain(AddressComponent addressComponent, P_thread* thread)
{
    /* Verify that the thread is unowned. */
    if (thread->parent != NULL)
    {
        std::stringstream errorMessage;
        errorMessage << "Thread \"" << thread->Name()
                     << "\" is already owned by core \""
                     << thread->parent->Name()
                     << "\". The thread's full name is \"" << thread->FullName()
                     << "\".";
        throw OwnershipException(errorMessage.str());
    }

    /* Claim it. */
    thread->on_being_contained_hook(this);

    /* Verify that the thread is now owned by us. */
    if (thread->parent != this)
    {
        std::stringstream errorMessage;
        errorMessage << "Thread \"" << thread->Name()
                     << "\" not owned by this core (" << Name()
                     << ") after being claimed. The thread's full name is \""
                     << thread->FullName() << "\".";
        throw OwnershipException(errorMessage.str());
    }

    P_threadm[addressComponent] = thread;

    /* Define a hardware address for the thread. */
    if (isAddressBound)
    {
        HardwareAddress* threadHardwareAddress = copy_hardware_address();
        threadHardwareAddress->set_thread(addressComponent);
        thread->set_hardware_address(threadHardwareAddress);
    }
}

/* Write debug and diagnostic information about the POETS core, recursively,
 * using dumpchan. Arguments:
 *
 * - file: File to dump to. */
void P_core::dump(FILE* file)
{
    std::string fullName = FullName();  /* Name of this from namebase. */
    std::string nameWithPrefix = dformat("P_thread %s ", fullName.c_str());
    std::string breakerTail;
    if (nameWithPrefix.size() >= MAXIMUM_BREAKER_LENGTH)
    {
        breakerTail.assign("+");
    }
    else
    {
        breakerTail.assign(MAXIMUM_BREAKER_LENGTH - nameWithPrefix.size(),
                           '+');
    }
    fprintf(file, "%s%s\n", nameWithPrefix.c_str(), breakerTail.c_str());

    /* About this object and its parent, if any. */
    NameBase::Dump(file);

    /* Dump information about binaries. */
    if (dataBinary != 0)
    {
        fprintf(file, "No data binary assigned to this core.\n");
    }
    else
    {
        fprintf(file, "Data binary:\n");
        dataBinary->Dump(file);
    }

    if (instructionBinary != 0)
    {
        fprintf(file, "No instruction binary assigned to this core.\n");
    }
    else
    {
        fprintf(file, "Instruction binary:\n");
        instructionBinary->Dump(file);
    }

    /* About contained items, if any. */
    fprintf(file, "Threads in this core %s\n", std::string(58, '+').c_str());
    if (P_threadm.empty())
        fprintf(file, "The thread map is empty.\n");
    else
    {
        WALKMAP(AddressComponent, P_thread*, P_threadm, iterator)
        {
            P_thread* iterThread = iterator->second;
            /* Print information from the map. */
            fprintf(file, "%u: %s (%p)\n",
                    iterator->first,
                    iterThread->FullName().c_str(), iterThread);
            /* Recursive-dump. */
            iterThread->dump(file);
        }
    }
    fprintf(file, "Threads in this core %s\n", std::string(58, '-').c_str());

    /* Close breaker and flush the dump. */
    std::replace(breakerTail.begin(), breakerTail.end(), '+', '-');
    fprintf(file, "%s%s\n", nameWithPrefix.c_str(), breakerTail.c_str());
    fflush(file);
}

/* Hook that a container calls to contain this object. Arguments:
 *
 * - container: Address of the mailbox that contains this core. */
void P_core::on_being_contained_hook(P_mailbox* container)
{
    parent = container;
    Npar(container);
}
