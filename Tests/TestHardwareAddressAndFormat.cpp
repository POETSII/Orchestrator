/* Tests various behaviours of the hardware addressing mechanism. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "HardwareAddressFormat.h"
#include "HardwareAddress.h"

TEST_CASE("Addresses can be created from a format and no values", "[Addressing]")
{
    HardwareAddressFormat myFormat(4, 5, 6, 8, 9);
    HardwareAddress address(&myFormat);
    REQUIRE(address.get_hardware_address() == 0);
}

TEST_CASE("Underdefined addresses can be differentiated from fully-defined addresses", "[Addressing]")
{
    HardwareAddressFormat myFormat(4, 5, 6, 8, 9);
    HardwareAddress address(&myFormat);
    REQUIRE(address.is_fully_defined() == false);
}

TEST_CASE("Fully-defined addresses can be created from a format and all values", "[Addressing]")
{
    /* The behaviour of this test is dependent on whether or not we are
     * ignoring the box component. */
    HardwareAddressFormat myFormat(4, 5, 6, 8, 9);
    HardwareAddress address(&myFormat, 15, 31, 63, 255, 511);

    /* Sorry about these bizarre numbers! If you want to understand them, try
     * dumping the address object with address.Dump(). I've even commented it
     * for you here: */
    /* address.Dump(); */

    if (IGNORE_BOX_COMPONENT)
    {
        REQUIRE(address.get_hardware_address() == 268435455);
    }
    else
    {
        REQUIRE(address.get_hardware_address() == 4294967295);
    }

    /* The address is fully defined in either case. */
    REQUIRE(address.is_fully_defined() == true);
}

TEST_CASE("Invalid address components cannot be set", "[Addressing]")
{
    HardwareAddressFormat myFormat(4, 5, 6, 8, 9);
    /* 33 > 2 ** 5 */
    REQUIRE_THROWS_AS(HardwareAddress(&myFormat, 15, 33, 63, 255, 511),
                      InvalidAddressException&);

    /* Also test that the box component does/not throw when we are/not
     * considering the box component. 17 > 2 ** 4 */
    if (IGNORE_BOX_COMPONENT)
    {
        HardwareAddress(&myFormat, 17, 31, 63, 255, 511);
    }
    else
    {
        REQUIRE_THROWS_AS(HardwareAddress(&myFormat, 17, 31, 63, 255, 511),
                          InvalidAddressException&);
    }
}

TEST_CASE("Test that as_uint() is a synonym of get_hardware_address()", "[Addressing]")
{
    HardwareAddressFormat myFormat(4, 5, 6, 8, 9);

    /* Test when values have not been defined. */
    HardwareAddress undefinedAddress(&myFormat);
    REQUIRE(undefinedAddress.get_hardware_address() == \
            undefinedAddress.as_uint());

    /* Test when values have been defined. */
    HardwareAddress definedAddress(&myFormat, 0, 1, 2, 3, 4);
    REQUIRE(definedAddress.get_hardware_address() == definedAddress.as_uint());
}
