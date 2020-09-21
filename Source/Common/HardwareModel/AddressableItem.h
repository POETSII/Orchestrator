#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_ADDRESSABLEITEM_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_ADDRESSABLEITEM_H

/* Describes the behaviour of items in the POETS hardware stack that can be
 * addressed.
 *
 * The addresses of items can be derived from the keys of the objects that
 * contain them higher-up the hierarchy. This subclassed behaviour allows
 * superclasses to accept and manipulate address objects, so that certain tasks
 * can be made easier to implement. For example, if the programmer has ready
 * access to this object, the address can be determined by a single dereference
 * as opposed to several lookups. */

#include "HardwareAddress.h"
#include "MissingAddressException.h"

class AddressableItem
{
    /* The actual address is protected, and can be interfaced with using the
     * getter and setter. This is to aid destruction. */
public:
    AddressableItem();
    ~AddressableItem();
    void clear_hardware_address();
    HardwareAddress* copy_hardware_address();
    HardwareAddress* get_hardware_address();
    void set_hardware_address(HardwareAddress* value);

protected:
    HardwareAddress* hardwareAddress;  /* Holds the address. */
    bool isAddressBound;  /* Defines whether or not an address has been
                           * assigned. */
};

#endif
