#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_HARDWAREADDRESSFORMAT_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_HARDWAREADDRESSFORMAT_H

/* Describes the format of hardware addresses for a given POETS Engine.

   This format defines the length of the constituent fields of hardware
   addresses in the POETS Engine, and enforces that the length of the address
   (specifically, the sum of all of these lengths) must be equal to 32 (bits).

   See the hardware model documentation for further information about the
   format of hardware addresses. */

#include "dumpchan.h"
#define HARDWARE_ADDRESS_LENGTH 32

class HardwareAddressFormat: protected DumpChan
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

    void dump(FILE* = stdout);
};

#endif
