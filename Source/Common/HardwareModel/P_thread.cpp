/* Defines POETS Thread behaviour (see the accompanying header for further
 * information). */

#include "P_thread.h"

/* Constructs a POETS Thread. Arguments:
 *
 * - name: Name of this thread object (see namebase) */
P_thread::P_thread(std::string name)
{
    Name(name);
}

/* Write debug and diagnostic information about the POETS thread, recursively,
 * using dumpchan. Arguments:
 *
 * - file: File to dump to. */
void P_thread::dump(FILE* file)
{
    std::string fullName = FullName();  /* Name of this from namebase. */

    /* About this object and its parent, if any. */
    char breaker[MAXIMUM_BREAKER_LENGTH + 1];
    int breakerLength = sprintf(breaker, "P_thread %s ", fullName.c_str());
    for(int index=breakerLength; index<MAXIMUM_BREAKER_LENGTH - 1;
        breaker[index++]='+');
    breaker[MAXIMUM_BREAKER_LENGTH - 1] = '\n';
    breaker[MAXIMUM_BREAKER_LENGTH] = '\0';
    fprintf(file, "%s", breaker);
    NameBase::Dump(file);

    /* About devices, if any. */
    fprintf(file, "Devices in this thread %s\n", std::string(56, '+').c_str());
    if (P_devicel.empty())
        fprintf(file, "The device map is empty.\n");
    else
    {
        WALKLIST(P_device*,P_devicel,iterator)
            fprintf(file, "%s\n", (*iterator)->FullName().c_str());
    }
    fprintf(file, "Devices in this thread %s\n", std::string(56, '-').c_str());

    /* Close breaker and flush the dump. */
    for(int index=breakerLength; index<MAXIMUM_BREAKER_LENGTH - 1;
        breaker[index++]='-');
    fprintf(file, "%s", breaker);
    fflush(file);
}

/* Hook that a container calls to contain this object. Arguments:
 *
 * - container: Address of the core that contains this thread */
void P_thread::on_being_contained_hook(P_core* container)
{
    parent = container;
    Npar(container);
}
