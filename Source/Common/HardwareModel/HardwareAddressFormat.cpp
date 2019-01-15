/* Defines hardware address format behaviour (see the accompanying header for
 * further information). */

#include "HardwareAddressFormat.h"

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
     threadWordLength(threadWordLength)
{
    /* <!> We should perform this check and throw if appropriate... but
       <!> according to Cambridge, hardware addresses that are less than the
       <!> fixed length are acceptable. I'll leave this here in case we decide
       <!> to zero-pad, or change our mind.

    if (boxWordLength +\
        boardWordLength +\
        mailboxWordLength +\
        coreWordLength +\
        threadWordLength != HARDWARE_ADDRESS_LENGTH)
    {
        // Throw
    }
    */
}

/* Constructs without populating. You really don't want to use this without
 * populating it first (either by passing it to a deployer, or setting the
 * values yourself later). */
HardwareAddressFormat::HardwareAddressFormat(){}

/* Write debug and diagnostic information using dumpchan. Arguments:
 *
 * - file: File to dump to. */
void HardwareAddressFormat::dump(FILE* file)
{
    /* The dump basically just contains the lengths. */
    fprintf(file, "Hardware Address Format +++++++++++++++++++++++++\n\
boxWordLength     %u\n\
boardWordLength   %u\n\
mailboxWordLength %u\n\
coreWordLength    %u\n\
threadWordLength  %u\n\
Hardware Address Format -------------------------\n",
            boxWordLength,
            boardWordLength,
            mailboxWordLength,
            coreWordLength,
            threadWordLength);

    fflush(file);
}
