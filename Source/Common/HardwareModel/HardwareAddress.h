#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_HARDWAREADDRESS_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_HARDWAREADDRESS_H

/* Describes hardware addresses, which apply to individual threads in the POETS
   Engine.

   These addresses are comprised of components, which can be used to identify
   the item by traversing the hardware model stack.

   See the hardware model documentation for further information about hardware
   addresses. */

#include "HardwareAddressFormat.h"
#include <math.h>  /* For validating address components. */


/* Components of hardware addresses are unsigned integers. */
typedef unsigned int AddressComponent;

class HardwareAddress: protected DumpChan
{
public:
    HardwareAddress(HardwareAddressFormat* format,
                    AddressComponent boxComponent,
                    AddressComponent boardComponent,
                    AddressComponent mailboxComponent,
                    AddressComponent coreComponent,
                    AddressComponent threadComponent);

    /* Stores a reference to the format so that lengths can be obtained. */
    HardwareAddressFormat* format;

    /* Address components for this hardware address object, from which the full
       hardware address word can be assembled. */
    AddressComponent boxComponent;
    AddressComponent boardComponent;
    AddressComponent mailboxComponent;
    AddressComponent coreComponent;
    AddressComponent threadComponent;

    unsigned getHardwareAddress();
    void dump(FILE* = stdout);
};

#endif
