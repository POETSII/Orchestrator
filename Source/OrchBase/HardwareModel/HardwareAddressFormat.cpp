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
{
    set_box_word_length(boxWordLength);
    set_board_word_length(boardWordLength);
    set_mailbox_word_length(mailboxWordLength);
    set_core_word_length(coreWordLength);
    set_thread_word_length(threadWordLength);
}

/* Constructs without populating. You really don't want to use this without
 * populating it first (either by passing it to a deployer, or setting the
 * values yourself later). */
HardwareAddressFormat::HardwareAddressFormat(){}

/* Getters/setters. Oh the games we play. Part of me wanted to use the
 * preprocessor to do this, but then I was concerned that nobody could read it
 * (including me)... */
void HardwareAddressFormat::set_box_word_length(const unsigned value)
{
    boxWordLength = value;
    boxMaxValue = max_from_length(value);
}
unsigned HardwareAddressFormat::get_box_word_length(){return boxWordLength;}
unsigned HardwareAddressFormat::get_box_max_value(){return boxMaxValue;}
/* --- */
void HardwareAddressFormat::set_board_word_length(const unsigned value)
{
    boardWordLength = value;
    boardMaxValue = max_from_length(value);
}
unsigned HardwareAddressFormat::get_board_word_length()
{return boardWordLength;}
unsigned HardwareAddressFormat::get_board_max_value(){return boardMaxValue;}
/* --- */
void HardwareAddressFormat::set_mailbox_word_length(const unsigned value)
{
    mailboxWordLength = value;
    mailboxMaxValue = max_from_length(value);
}
unsigned HardwareAddressFormat::get_mailbox_word_length()
{return mailboxWordLength;}
unsigned HardwareAddressFormat::get_mailbox_max_value()
{return mailboxMaxValue;}
/* --- */
void HardwareAddressFormat::set_core_word_length(const unsigned value)
{
    coreWordLength = value;
    coreMaxValue = max_from_length(value);
}
unsigned HardwareAddressFormat::get_core_word_length(){return coreWordLength;}
unsigned HardwareAddressFormat::get_core_max_value(){return coreMaxValue;}
/* --- */
void HardwareAddressFormat::set_thread_word_length(const unsigned value)
{
    threadWordLength = value;
    threadMaxValue = max_from_length(value);
}
unsigned HardwareAddressFormat::get_thread_word_length()
{return threadWordLength;}
unsigned HardwareAddressFormat::get_thread_max_value(){return threadMaxValue;}

/* Given a length, returns the maximum value that can fit in that length. It's
 * just 2 ** value... */
unsigned HardwareAddressFormat::max_from_length(const unsigned length)
{
    unsigned out = 1;
    for (unsigned repeat = 0; repeat < length; repeat++) out *= 2;
    return out;
}

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
