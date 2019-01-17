/* Defines hardware address behaviour (see the accompanying header for further
 * information). */

#include "HardwareAddress.h"

/* Constructs a hardware address. Arguments:
 *
 * - format: A format for the hardware address.
 *
 * - Each argument after this: values for those components of the hardware
     address. */
HardwareAddress::HardwareAddress(
    HardwareAddressFormat* format,
    AddressComponent boxComponent,
    AddressComponent boardComponent,
    AddressComponent mailboxComponent,
    AddressComponent coreComponent,
    AddressComponent threadComponent):format(format)
{
    definitions = 0;

    /* Use the setters to check for validity. */
    set_box(boxComponent);
    set_board(boardComponent);
    set_mailbox(mailboxComponent);
    set_core(coreComponent);
    set_thread(threadComponent);

    /* Update definition word. */
    for(unsigned index=0; index<5; set_defined(index++));
}

/* Alternatively, construct a hardware address without priming it with values
 * explicitly. */
HardwareAddress::HardwareAddress(HardwareAddressFormat* format)
    :format(format)
{
    definitions = 0;
    boxComponent = 0;
    boardComponent = 0;
    mailboxComponent = 0;
    coreComponent = 0;
    threadComponent = 0;
}

/* Returns the hardware address as an unsigned integer. */
unsigned HardwareAddress::get_hardware_address()
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

/* Populates a software address object with information from this hardware
 * address object. Arguments:
 *
 * - target: Software address object to populate. */
void HardwareAddress::populate_software_address(P_addr* target)
{
    if (is_box_defined()){target->SetBox(get_box());}
    if (is_board_defined()){target->SetBoard(get_board());}
    if (is_mailbox_defined()){target->SetMailbox(get_mailbox());}
    if (is_core_defined()){target->SetCore(get_core());}
    if (is_thread_defined()){target->SetThread(get_thread());}
}

/* Setters, which set the address component with a value.
 *
 * Also updates the "definitions" word. Raises if the input does not fit within
 * the loaded format. Arguments:
 *
 * - value: Value to set.
*/
void HardwareAddress::set_box(AddressComponent value)
{
    /* Validate */
    if (value >= std::pow(2, format->boxWordLength))
    {
        throw InvalidAddressException(
            dformat("[ERROR] Box component value \"%d\" does not fit format "
                    "with box length \"%d\".", value, format->boxWordLength));
    }

    /* Set */
    boxComponent = value;
    set_defined(0);
}

void HardwareAddress::set_board(AddressComponent value)
{
    /* Validate */
    if (value >= std::pow(2, format->boardWordLength))
    {
        throw InvalidAddressException(
            dformat("[ERROR] Board component value \"%d\" does not fit format "
                    "with board length \"%d\".",
                    value, format->boardWordLength));
    }

    /* Set */
    boardComponent = value;
    set_defined(1);
}

void HardwareAddress::set_mailbox(AddressComponent value)
{
    /* Validate */
    if (value >= std::pow(2, format->mailboxWordLength))
    {
        throw InvalidAddressException(
            dformat("[ERROR] Mailbox component value \"%d\" does not fit "
                    "format with mailbox length \"%d\".",
                    value, format->mailboxWordLength));
    }

    /* Set */
    mailboxComponent = value;
    set_defined(2);
}

void HardwareAddress::set_core(AddressComponent value)
{
    /* Validate */
    if (value >= std::pow(2, format->coreWordLength))
    {
        throw InvalidAddressException(
            dformat("[ERROR] Core component value \"%d\" does not fit format "
                    "with core length \"%d\".",
                    value, format->coreWordLength));
    }

    /* Set */
    coreComponent = value;
    set_defined(3);
}

void HardwareAddress::set_thread(AddressComponent value)
{
    /* Validate */
    if (value >= std::pow(2, format->threadWordLength))
    {
        throw InvalidAddressException(
            dformat("[ERROR] Thread component value \"%d\" does not fit "
                    "format with thread length \"%d\".",
                    value, format->threadWordLength));
    }

    /* Set */
    threadComponent = value;
    set_defined(4);
}

/* Updates the internal record of which components of this address have been
 * defined. Arguments:
 *
 * - index: which element of the binary word to set to 1. */
void HardwareAddress::set_defined(unsigned index)
{
    definitions |= (1 << index);
}

/* Write debug and diagnostic information using dumpchan. Arguments:
 *
 * - file: File to dump to. */
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
