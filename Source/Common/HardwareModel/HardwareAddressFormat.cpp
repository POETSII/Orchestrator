/* Defines hardware address format behaviour (see the accompanying header for
 * further information). */

#include "HardwareAddressFormat.h"
#include "stdint.h"

/* Constructs a hardware address format. Each argument corresponds to the
 * length of the address section. */
HardwareAddressFormat::HardwareAddressFormat(unsigned boxWordLength,
                          unsigned boardWordLength,
                          unsigned mailboxWordLength,
                          unsigned coreWordLength,
                          unsigned threadWordLength)
    :boxWordLength(boxWordLength),
     boardWordLength(boardWordLength),
     mailboxWordLength(mailboxWordLength),
     coreWordLength(coreWordLength),
     threadWordLength(threadWordLength){}

/* Constructs without populating. You really don't want to use this without
 * populating it first (either by passing it to a deployer, or setting the
 * values yourself later). */
HardwareAddressFormat::HardwareAddressFormat(){}

/* Write debug and diagnostic information using dumpchan. Arguments:
 *
 * - file: File to dump to. */
void HardwareAddressFormat::Dump(FILE* file)
{
    std::string prefix = dformat("Hardware address format at %#018lx",
                                 (uint64_t) this);
    DumpUtils::open_breaker(file, prefix);

    /* The dump basically just contains the lengths. */
    fprintf(file, "boxWordLength:     %u\n\
boardWordLength:   %u\n\
mailboxWordLength: %u\n\
coreWordLength:    %u\n\
threadWordLength:  %u\n",
            boxWordLength,
            boardWordLength,
            mailboxWordLength,
            coreWordLength,
            threadWordLength);

    /* Close breaker and flush the dump. */
    DumpUtils::close_breaker(file, prefix);
    fflush(file);
}
