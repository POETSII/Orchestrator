/* Defines the behaviour of items in the POETS hardware stack that can be
 * addressed (see the accompanying header for more information). */

#include "AddressableItem.h"

AddressableItem::AddressableItem()
{
    isAddressBound = false;
}

AddressableItem::~AddressableItem()
{
    clear_hardware_address();
}

/* Clears the hardware address, if one has been stored. */
void AddressableItem::clear_hardware_address()
{
    if (isAddressBound) delete hardwareAddress;
}

/* Shallow-copies the hardware address belonging to this object, and writes the
 * dynamically-allocated copy to the pointer returned. */
HardwareAddress* AddressableItem::copy_hardware_address()
{
    return new HardwareAddress(*hardwareAddress);
}

/* Hardware address getter and setter, maintains the isAddressBound record. Any
 * address passed to this setter will be deleted when this AddressableItem
 * object is destroyed, so allocate this dynamically, e.g.
 *
 *     ...
 *     AddressableItem duck;
 *     HardwareAddress* address;
 *     address = new HardwareAddress(...);
 *     AddressableItem.set_hardware_address(address)
 *     ... */
HardwareAddress* AddressableItem::get_hardware_address()
{
    if (!isAddressBound)
    {
        throw MissingAddressException(
            dformat("[ERROR] Item at %#018lx has no hardware address, so it "
                    "cannot be retreived.", this));
    }
    return hardwareAddress;
}

void AddressableItem::set_hardware_address(HardwareAddress* value)
{
    clear_hardware_address();
    hardwareAddress = value;
    isAddressBound = true;
}
