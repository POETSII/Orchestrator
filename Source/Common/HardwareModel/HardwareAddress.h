#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_HARDWAREADDRESS_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_HARDWAREADDRESS_H

/* Describes hardware addresses, which apply to individual threads in the POETS
 * Engine.
 *
 * These addresses are comprised of components, which can be used to identify
 * the item by traversing the hardware model stack.
 *
 * This class can also be (ab)used to address components at a higher level of
 * the hierarchy by using the HardwareAddressFormat* constructor, and only
 * setting the components you need.
 *
 * For elegance, this class, and the HardwareAddressFormat class should be
 * refactored to store their components in an array to reduce code
 * repitition. But it doesn't.
 *
 * Note that, other than the hardware address format (format), this class
 * contains no dynamic members. This is to facilitate easy copying when
 * populating POETS engines.
 *
 * See the hardware model documentation for further information about hardware
 * addresses. */

#include "dfprintf.h"
#include "HardwareAddressFormat.h"
#include "InvalidAddressException.h"
#include "P_addr.h"
#include <cmath>  /* For validating address components. */

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
    HardwareAddress(HardwareAddressFormat* format);

    /* Stores a reference to the format so that lengths can be obtained. */
    HardwareAddressFormat* format;

    /* Getters and setters. */
    inline AddressComponent get_box(){return boxComponent;}
    inline AddressComponent get_board(){return boardComponent;}
    inline AddressComponent get_mailbox(){return mailboxComponent;}
    inline AddressComponent get_core(){return coreComponent;}
    inline AddressComponent get_thread(){return threadComponent;}

    void set_box(AddressComponent value);
    void set_board(AddressComponent value);
    void set_mailbox(AddressComponent value);
    void set_core(AddressComponent value);
    void set_thread(AddressComponent value);

    /* Access */
    unsigned get_hardware_address();
    void populate_a_software_address(P_addr* target, bool resetFirst = true);
    void populate_from_software_address(P_addr* source);
    void dump(FILE* = stdout);

    /* Defines whether or not the hardware address is fully defined. No binary
     * literal in C++98 (yuck). */
    inline bool is_fully_defined(){return definitions == 31;}  /* 0b11111 */
    inline bool is_box_defined(){return (definitions & 1) > 0;}
    inline bool is_board_defined(){return (definitions & 2) > 0;}
    inline bool is_mailbox_defined(){return (definitions & 4) > 0;}
    inline bool is_core_defined(){return (definitions & 8) > 0;}
    inline bool is_thread_defined(){return (definitions & 16) > 0;}

private:

    /* Address components for this hardware address object, from which the full
     * hardware address word can be assembled. */
    AddressComponent boxComponent;
    AddressComponent boardComponent;
    AddressComponent mailboxComponent;
    AddressComponent coreComponent;
    AddressComponent threadComponent;

    /* Binary word which stores which components of the address have been
     * defined. Bits are 0 if not defined, and 1 if defined, according to the
     * map:
     *
     * - Bit 0: box
     * - Bit 1: board
     * - Bit 2: mailbox
     * - Bit 3: core
     * - Bit 4: thread
     *
     * Functionally this acts as an array of booleans, but uses less memory. */
    unsigned definitions;

    /* Convenience method for defining bits of definitions. */
    void set_defined(unsigned index);
};
#endif
