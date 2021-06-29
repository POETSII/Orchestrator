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
HardwareAddressInt HardwareAddress::get_hardware_address()
{
    HardwareAddressInt returnValue = 0;
    unsigned offset = 0;
    returnValue += threadComponent;
    offset += format->get_thread_word_length();
    returnValue += coreComponent << offset;
    offset += format->get_core_word_length();
    returnValue += mailboxComponent << offset;
    offset += format->get_mailbox_word_length();
    returnValue += boardComponent << offset;
    if (!IGNORE_BOX_ADDRESS_COMPONENT)
    {
        offset += format->get_board_word_length();
        returnValue += boxComponent << offset;
    }
    return returnValue;
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
    if (!IGNORE_BOX_ADDRESS_COMPONENT)
    {
        if (value >= format->get_box_max_value())
        {
            throw InvalidAddressException(
                dformat("[ERROR] Box component value \"%d\" does not fit "
                        "format with box length \"%d\".",
                        value, format->get_box_word_length()));
        }
    }

    /* Set */
    boxComponent = value;
    set_defined(0);
}

void HardwareAddress::set_board(AddressComponent value)
{
    /* Validate */
    if (value >= format->get_board_max_value())
    {
        throw InvalidAddressException(
            dformat("[ERROR] Board component value \"%d\" does not fit format "
                    "with board length \"%d\".",
                    value, format->get_board_word_length()));
    }

    /* Set */
    boardComponent = value;
    set_defined(1);
}

void HardwareAddress::set_mailbox(AddressComponent value)
{
    /* Validate */
    if (value >= format->get_mailbox_max_value())
    {
        throw InvalidAddressException(
            dformat("[ERROR] Mailbox component value \"%d\" does not fit "
                    "format with mailbox length \"%d\".",
                    value, format->get_mailbox_word_length()));
    }

    /* Set */
    mailboxComponent = value;
    set_defined(2);
}

void HardwareAddress::set_core(AddressComponent value)
{
    /* Validate */
    if (value >= format->get_core_max_value())
    {
        throw InvalidAddressException(
            dformat("[ERROR] Core component value \"%d\" does not fit format "
                    "with core length \"%d\".",
                    value, format->get_core_word_length()));
    }

    /* Set */
    coreComponent = value;
    set_defined(3);
}

void HardwareAddress::set_thread(AddressComponent value)
{
    /* Validate */
    if (value >= format->get_thread_max_value())
    {
        throw InvalidAddressException(
            dformat("[ERROR] Thread component value \"%d\" does not fit "
                    "format with thread length \"%d\".",
                    value, format->get_thread_word_length()));
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
    DumpUtils::open_breaker(file, prefix);

    /* Just contains the components, and whether or not they have been
     * defined. */
    fprintf(file, "boxComponent:     %u ", boxComponent);
    fprintf(file, "%s", is_box_defined() ? "" : "(not defined)");
    fprintf(file, "%s", IGNORE_BOX_ADDRESS_COMPONENT ? " (ignored)\n" : "\n");

    fprintf(file, "boardComponent:   %u ", boardComponent);
    fprintf(file, "%s\n", is_board_defined() ? "" : "(not defined)");

    fprintf(file, "mailboxComponent: %u ", mailboxComponent);
    fprintf(file, "%s\n", is_mailbox_defined() ? "" : "(not defined)");

    fprintf(file, "coreComponent:    %u ", coreComponent);
    fprintf(file, "%s\n", is_core_defined() ? "" : "(not defined)");

    fprintf(file, "threadComponent:  %u ", threadComponent);
    fprintf(file, "%s\n", is_thread_defined() ? "" : "(not defined)");

    /* Close breaker and flush the dump. */
    DumpUtils::close_breaker(file, prefix);
    fflush(file);
}
