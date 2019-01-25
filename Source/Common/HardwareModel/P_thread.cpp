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
    std::string nameWithPrefix = dformat("P_thread %s ", fullName.c_str());
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
    std::replace(breakerTail.begin(), breakerTail.end(), '+', '-');
    fprintf(file, "%s%s\n", nameWithPrefix.c_str(), breakerTail.c_str());
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
