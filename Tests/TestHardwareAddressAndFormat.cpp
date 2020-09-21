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
    HardwareAddressFormat myFormat(4, 5, 6, 8, 9);
    HardwareAddress address(&myFormat, 15, 31, 63, 255, 511);
    REQUIRE(address.get_hardware_address() == 4294967295);  /* Sorry!
    * // If you want to know what that bizarre number is, try dumping with
    * address.Dump(); */
    REQUIRE(address.is_fully_defined() == true);

    /* While we're here, test that HardwareAddress::as_uint() is a synonym for
       HardwareAddress::get_hardware_address(). */
    REQUIRE(address.get_hardware_address() == address.as_uint());
}

TEST_CASE("Invalid address components cannot be set", "[Addressing]")
{
    HardwareAddressFormat myFormat(4, 5, 6, 8, 9);
    // 21 > 2 ** 4
    REQUIRE_THROWS_AS(HardwareAddress(&myFormat, 21, 31, 63, 255, 511),
                      InvalidAddressException&);
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
