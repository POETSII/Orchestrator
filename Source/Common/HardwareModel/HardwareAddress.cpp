/* Defines hardware address behaviour (see the accompanying header for further
   information). */

#include "HardwareAddress.h"

/* Constructs a hardware address. Arguments:

   - format: A format for the hardware address.

   - Each argument after this: values for those components of the hardware
     address.
*/
HardwareAddress::HardwareAddress(HardwareAddressFormat* format,
                                 AddressComponent boxComponent,
                                 AddressComponent boardComponent,
                                 AddressComponent mailboxComponent,
                                 AddressComponent coreComponent,
                                 AddressComponent threadComponent)
    :format(format),
     boxComponent(boxComponent),
     boardComponent(boardComponent),
     mailboxComponent(mailboxComponent),
     coreComponent(coreComponent),
     threadComponent(threadComponent){}

/* Returns the hardware address as an unsigned integer. */
unsigned HardwareAddress::getHardwareAddress()
{
    unsigned returnValue = 0;
    unsigned offset = 0;
    returnValue += threadComponent;
    offset += format->threadWordLength;
    returnValue += coreComponent << offset;
    offset += format->coreWordLength;
    returnValue += mailboxComponent << offset;
    offset += format->mailboxWordLength;
    returnValue += boardComponent << offset;
    offset += format->boardWordLength;
    returnValue += boxComponent << offset;
    return returnValue;
}

/* Write debug and diagnostic information using dumpchan. Arguments:

   - file: File to dump to.
*/
void HardwareAddress::dump(FILE* file)
{
    /* The dump basically just contains the lengths. */
    fprintf(file, "Hardware Address +++++++++++++++++++++++++\n\
boxComponent     %u\n\
boardComponent   %u\n\
mailboxComponent %u\n\
coreComponent    %u\n\
threadComponent  %u\n\
Hardware Address -------------------------\n",
            boxComponent,
            boardComponent,
            mailboxComponent,
            coreComponent,
            threadComponent);

    fflush(file);
}
