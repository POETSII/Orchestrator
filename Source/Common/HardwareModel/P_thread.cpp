/* Defines POETS Thread behaviour (see the accompanying header for further
 * information). */

#include "P_thread.h"

/* Constructs a POETS Thread. Arguments:
 *
 * - name: Name of this thread object (see namebase) */
P_thread::P_thread(std::string name)
{
    parent = NULL;
    Name(name);
}

/* Write debug and diagnostic information about the POETS thread, recursively,
 * using dumpchan. Arguments:
 *
 * - file: File to dump to. */
void P_thread::Dump(FILE* file)
{
    std::string prefix = dformat("P_thread %s", FullName().c_str());
    DumpUtils::open_breaker(file, prefix);

    /* About this object and its parent, if any. */
    NameBase::Dump(file);

    /* About devices, if any. */
    DumpUtils::open_breaker(file, "Devices in this thread");
    if (P_devicel.empty())
        fprintf(file, "The device map is empty.\n");
    else
    {
        WALKLIST(DevI_t *,P_devicel,iterator)
            fprintf(file, "%s\n", (*iterator)->FullName().c_str());
    }
    DumpUtils::close_breaker(file, "Devices in this thread");

    /* Close breaker and flush the dump. */
    DumpUtils::close_breaker(file, prefix);
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
