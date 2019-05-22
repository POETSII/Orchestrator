/* Tests various behaviours of the software addressing mechanism. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "SoftwareAddress.h"

TEST_CASE("Addresses can be created empty and undefined")
{
    SoftwareAddress address;
    REQUIRE(address.get_ismothership() == false);
    REQUIRE(address.get_iscnc() == false);
    REQUIRE(address.get_task() == 0);
    REQUIRE(address.get_opcode() == 0);
    REQUIRE(address.get_device() == 0);

    REQUIRE(!address.is_ismothership_defined());
    REQUIRE(!address.is_iscnc_defined());
    REQUIRE(!address.is_task_defined());
    REQUIRE(!address.is_opcode_defined());
    REQUIRE(!address.is_device_defined());

    REQUIRE(!address.is_fully_defined());
}

TEST_CASE("Addresses can be fully populated on construction")
{
    /* This type of device is not valid, because external devices must have a
     * zero opCode, but this is fine for testing. */
    bool isMothership = true;
    bool isCnc = false;
    uint8_t task = 14;
    uint8_t opCode = 4;
    uint16_t device = 65535;

    SoftwareAddress address(isMothership, isCnc, task, opCode, device);
    REQUIRE(address.get_ismothership() == isMothership);
    REQUIRE(address.get_iscnc() == isCnc);
    REQUIRE(address.get_task() == task);
    REQUIRE(address.get_opcode() == opCode);
    REQUIRE(address.get_device() == device);

    REQUIRE(address.is_ismothership_defined());
    REQUIRE(address.is_iscnc_defined());
    REQUIRE(address.is_task_defined());
    REQUIRE(address.is_opcode_defined());
    REQUIRE(address.is_device_defined());

    REQUIRE(address.is_fully_defined());
}

TEST_CASE("Empty addresses are full of zero (or false) components")
{
    SoftwareAddress address;
    REQUIRE(address.get_ismothership() == false);
    REQUIRE(address.get_iscnc() == false);
    REQUIRE(address.get_task() == 0);
    REQUIRE(address.get_opcode() == 0);
    REQUIRE(address.get_device() == 0);
}

TEST_CASE("Empty addresses can be populated in stages")
{
    /* This is for the OnCtl handler of normal device 102 operating as part
     * Orchestrator task 1 on the target thread defined by the hardware address
     * (not included here). It will run the command-and-control operation 255
     * (used by the OnCtl handler, or not). */
    bool isMothership = false;
    bool isCnc = true;
    uint8_t task = 1;
    uint8_t opCode = 255;
    uint16_t device = 102;

    SoftwareAddress address;

    /* Populate with isMothersip and device only */
    address.set_ismothership(isMothership);
    address.set_device(device);

    REQUIRE(address.get_ismothership() == isMothership);
    REQUIRE(address.get_device() == device);

    REQUIRE(address.is_ismothership_defined());
    REQUIRE(address.is_device_defined());

    /* Nothing else should be defined, and should return zero-ish on being
     * gotten. */
    REQUIRE(address.get_iscnc() == false);
    REQUIRE(address.get_task() == 0);
    REQUIRE(address.get_opcode() == 0);
    REQUIRE(!address.is_iscnc_defined());
    REQUIRE(!address.is_task_defined());
    REQUIRE(!address.is_opcode_defined());

    REQUIRE(!address.is_fully_defined());

    /* Define the rest of them. */
    address.set_iscnc(isCnc);
    address.set_task(task);
    address.set_opcode(opCode);

    /* Everything should now be defined. */
    REQUIRE(address.get_ismothership() == isMothership);
    REQUIRE(address.get_iscnc() == isCnc);
    REQUIRE(address.get_task() == task);
    REQUIRE(address.get_opcode() == opCode);
    REQUIRE(address.get_device() == device);

    REQUIRE(address.is_ismothership_defined());
    REQUIRE(address.is_iscnc_defined());
    REQUIRE(address.is_task_defined());
    REQUIRE(address.is_opcode_defined());
    REQUIRE(address.is_device_defined());

    REQUIRE(address.is_fully_defined());
}

TEST_CASE("Address components can be redefined")
{
    bool isMothership = true;
    bool isCnc = false;
    uint8_t task = 14;
    uint8_t opCode = 0;
    uint16_t device = 65535;
    SoftwareAddress address(isMothership, isCnc, task, opCode, device);

    /* Now we redefine the components. */
    isMothership = !isMothership;
    isCnc = !isCnc;
    task = 17;
    opCode = 2;
    device = 0;

    address.set_ismothership(isMothership);
    address.set_iscnc(isCnc);
    address.set_task(task);
    address.set_opcode(opCode);
    address.set_device(device);

    /* Everything should now be defined according to the second definition. */
    REQUIRE(address.get_ismothership() == isMothership);
    REQUIRE(address.get_iscnc() == isCnc);
    REQUIRE(address.get_task() == task);
    REQUIRE(address.get_opcode() == opCode);
    REQUIRE(address.get_device() == device);

    REQUIRE(address.is_ismothership_defined());
    REQUIRE(address.is_iscnc_defined());
    REQUIRE(address.is_task_defined());
    REQUIRE(address.is_opcode_defined());
    REQUIRE(address.is_device_defined());

    REQUIRE(address.is_fully_defined());
}

TEST_CASE("An invalid task input cannot be set")
{
    SoftwareAddress address;
    /* Can't be > 63. */
    REQUIRE_THROWS_AS(address.set_task(64), InvalidAddressException&);

    /* Test with 0b11111111, which is also > 63. */
    REQUIRE_THROWS_AS(address.set_task(-1), InvalidAddressException&);
}

TEST_CASE("Fully-defined software addresses can be dumped as a 32-bit string")
{
    /* This is a supervisor device for Orchestrator task 59. It will run the
     * command-and-control operation 47 (used by the supervisor handler, or
     * not). */
    bool isMothership = true;
    bool isCnc = true;
    uint8_t task = 59;
    uint8_t opCode = 47;
    uint16_t device = 0;

    SoftwareAddress address(isMothership, isCnc, task, opCode, device);
    REQUIRE(address.is_fully_defined());

    /* This should be 1 1 11,1011 0010,1111 0000,0000,0000,0000 = 4214161408 */
    uint32_t expectedResult = 4214161408;
    REQUIRE(address.as_uint() == expectedResult);
    REQUIRE(address.get_software_address() == expectedResult);
}

TEST_CASE("Partially-defined software addresses can also be dumped")
{
    /* This is a normal device. Since non-OnCtl packets to normal devices have
       a zero opcode, we can simply forgo defining it. */
    bool isMothership = false;
    bool isCnc = false;
    uint8_t task = 1;
    uint16_t device = 0;

    SoftwareAddress address;
    address.set_ismothership(isMothership);
    address.set_iscnc(isCnc);
    address.set_task(task);
    address.set_device(device);
    REQUIRE(!address.is_fully_defined());

    /* This should be 0 0 00,0001 0000,0000 0000,0000,0000,0000 = 16777216 */
    uint32_t expectedResult = 16777216;
    REQUIRE(address.as_uint() == expectedResult);
    REQUIRE(address.get_software_address() == expectedResult);
}
