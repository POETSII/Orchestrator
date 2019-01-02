/* Defines POETS Core behaviour (see the accompanying header for further
   information). */

#include "PoetsCore.h"

/* Constructs a POETS Core. Arguments:
   - name: Name of this core object (see namebase)
*/
PoetsCore::PoetsCore(std::string name)
{
    Name(name);
    /* <!> About binaries */
}

/* Donates an uncontained thread to this core. Arguments:

   - addressComponent: Used to index the thread in this core.
   - thread: Pointer to the thread object to contain. Must not already have a
     parent.
*/
void PoetsCore::contain(AddressComponent addressComponent, PoetsThread* thread)
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

    PoetsThreads[addressComponent] = thread;
}

/* Write debug and diagnostic information about the POETS core, recursively,
   using dumpchan. Arguments:

   - file: File to dump to.
*/
void PoetsCore::dump(FILE* file)
{
    std::string fullName = FullName();  /* Name of this from namebase. */

    /* About this object and its parent, if any. */
    char breaker[MAXIMUM_BREAKER_LENGTH + 1];
    int breakerLength = sprintf(breaker, "PoetsCore %s ", fullName.c_str());
    for(int index=breakerLength; index<MAXIMUM_BREAKER_LENGTH - 1;
        breaker[index++]='+');
    breaker[MAXIMUM_BREAKER_LENGTH - 1] = '\n';
    breaker[MAXIMUM_BREAKER_LENGTH] = '\0';
    fprintf(file, "%s", breaker);
    NameBase::Dump(file);

    /* About contained items, if any. */
    fprintf(file, "Threads in this core %s\n", std::string(58, '+').c_str());
    if (PoetsThreads.empty())
        fprintf(file, "The thread map is empty.\n");
    else
    {
        WALKMAP(AddressComponent,PoetsThread*,PoetsThreads,iterator)
        {
            PoetsThread* iterThread = iterator->second;
            /* Print information from the map. */
            fprintf(file, "%u: %s (%p)\n",
                    iterator->first,
                    iterThread->FullName().c_str(), iterThread);
            /* Recursive-dump. */
            iterThread->dump();
        }
    }
    fprintf(file, "Threads in this core %s\n", std::string(58, '-').c_str());

    /* Close breaker and flush the dump. */
    for(int index=breakerLength; index<MAXIMUM_BREAKER_LENGTH - 1;
        breaker[index++]='-');
    fprintf(file, "%s", breaker);
    fflush(file);
}

/* Hook that a container calls to contain this object. Arguments:
   - container: Address of the mailbox that contains this core.
*/
void PoetsCore::on_being_contained_hook(PoetsMailbox* container)
{
    parent = container;
    Npar(container);
}
