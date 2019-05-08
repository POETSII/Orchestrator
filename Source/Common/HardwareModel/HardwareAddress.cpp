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
 * - target: Software address object to populate.
 * - resetFirst: Whether or not to reset the target software address before
 *               populating it. May have a default definined in the
 *               accompanying header. */
void HardwareAddress::populate_a_software_address(P_addr* target,
                                                  bool resetFirst)
{
    if (resetFirst){target->Reset();}
    if (is_box_defined()){target->SetBox(get_box());}
    if (is_board_defined()){target->SetBoard(get_board());}
    if (is_mailbox_defined()){target->SetMailbox(get_mailbox());}
    if (is_core_defined()){target->SetCore(get_core());}
    if (is_thread_defined()){target->SetThread(get_thread());}
}

/* Populates this hardware address object with information from a software
 * address object. Software-specific information is lost. Arguments:
 *
 * - source: Software address object to read from. */
void HardwareAddress::populate_from_software_address(P_addr* source)
{
    AddressComponent staging;
    if (source->GetBox(staging) == 0){set_box(staging);}
    if (source->GetBoard(staging) == 0){set_board(staging);}
    if (source->GetMailbox(staging) == 0){set_mailbox(staging);}
    if (source->GetCore(staging) == 0){set_core(staging);}
    if (source->GetThread(staging) == 0){set_thread(staging);}
}

/* Setters, which set the address component with a value.
 *
 * Also updates the "definitions" word. Raises if the input does not fit within
 * the loaded format. Arguments:
 *
 * - value: Value to set. */
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
void HardwareAddress::Dump(FILE* file)
{
    std::string prefix = dformat("Hardware address at %#018lx",
                                 (uint64_t) this);
    HardwareDumpUtils::open_breaker(file, prefix);

    /* Just contains the components, and whether or not they have been
     * defined. */
    fprintf(file, "boxComponent:     %u ", boxComponent);
    fprintf(file, "%s\n", is_box_defined() ? "" : "(not defined)");

    fprintf(file, "boardComponent:   %u ", boardComponent);
    fprintf(file, "%s\n", is_board_defined() ? "" : "(not defined)");

    fprintf(file, "mailboxComponent: %u ", mailboxComponent);
    fprintf(file, "%s\n", is_mailbox_defined() ? "" : "(not defined)");

    fprintf(file, "coreComponent:    %u ", coreComponent);
    fprintf(file, "%s\n", is_core_defined() ? "" : "(not defined)");

    fprintf(file, "threadComponent:  %u ", threadComponent);
    fprintf(file, "%s\n", is_thread_defined() ? "" : "(not defined)");

    /* Close breaker and flush the dump. */
    HardwareDumpUtils::close_breaker(file, prefix);
    fflush(file);
}
