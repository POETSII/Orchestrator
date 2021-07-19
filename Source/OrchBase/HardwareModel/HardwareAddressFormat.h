#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_HARDWAREADDRESSFORMAT_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_HARDWAREADDRESSFORMAT_H

/* Describes the format of hardware addresses for a given POETS Engine.
 *
 * This format defines the length of the constituent fields of hardware
 * addresses in the POETS Engine.
 *
 * See the hardware model documentation for further information about the
 * format of hardware addresses. */

#include "DumpUtils.h"

class HardwareAddressFormat: public DumpChan
{
public:
    HardwareAddressFormat(unsigned boxWordLength,
                          unsigned boardWordLength,
                          unsigned mailboxWordLength,
                          unsigned coreWordLength,
                          unsigned threadWordLength);
    HardwareAddressFormat();
    void Dump(FILE* = stdout);

    void set_box_word_length(const unsigned value);
    void set_board_word_length(const unsigned value);
    void set_mailbox_word_length(const unsigned value);
    void set_core_word_length(const unsigned value);
    void set_thread_word_length(const unsigned value);
    unsigned get_box_word_length();
    unsigned get_board_word_length();
    unsigned get_mailbox_word_length();
    unsigned get_core_word_length();
    unsigned get_thread_word_length();
    unsigned get_box_max_value();
    unsigned get_board_max_value();
    unsigned get_mailbox_max_value();
    unsigned get_core_max_value();
    unsigned get_thread_max_value();

private:
    unsigned boxWordLength;
    unsigned boardWordLength;
    unsigned mailboxWordLength;
    unsigned coreWordLength;
    unsigned threadWordLength;

    unsigned boxMaxValue;
    unsigned boardMaxValue;
    unsigned mailboxMaxValue;
    unsigned coreMaxValue;
    unsigned threadMaxValue;

    unsigned max_from_length(const unsigned length);
};

#endif
