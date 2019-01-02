/* Tests various behaviours of the hardware addressing mechanism. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "HardwareAddressFormat.h"
#include "HardwareAddress.h"

TEST_CASE("Addresses can be created from a format", "[Addressing]")
{
    HardwareAddressFormat myFormat(4, 5, 6, 8, 9);
    HardwareAddress address(&myFormat, 15, 31, 63, 255, 511);
    REQUIRE(address.getHardwareAddress() == 4294967295);
}

TEST_CASE("Invalid addresses cannot be created", "[Addressing]")
{
    HardwareAddressFormat myFormat(4, 5, 6, 8, 9);
    // 21 > 2 ** 4
    REQUIRE_THROWS(HardwareAddress(&myFormat, 21, 31, 63, 255, 511));
}
