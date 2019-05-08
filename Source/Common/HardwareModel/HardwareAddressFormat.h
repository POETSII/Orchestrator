#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_HARDWAREADDRESSFORMAT_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_HARDWAREADDRESSFORMAT_H

/* Describes the format of hardware addresses for a given POETS Engine.
 *
 * This format defines the length of the constituent fields of hardware
 * addresses in the POETS Engine.
 *
 * See the hardware model documentation for further information about the
 * format of hardware addresses. */

#include "HardwareDumpUtils.h"

class HardwareAddressFormat: public DumpChan
{
public:
    HardwareAddressFormat(unsigned boxWordLength,
                          unsigned boardWordLength,
                          unsigned mailboxWordLength,
                          unsigned coreWordLength,
                          unsigned threadWordLength);
    HardwareAddressFormat();

    unsigned boxWordLength;
    unsigned boardWordLength;
    unsigned mailboxWordLength;
    unsigned coreWordLength;
    unsigned threadWordLength;

    void Dump(FILE* = stdout);
};

#endif
