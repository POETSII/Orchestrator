#ifndef __ORCHESTRATOR_SOURCE_COMMON_SOFTWAREADDRESS_H
#define __ORCHESTRATOR_SOURCE_COMMON_SOFTWAREADDRESS_H

/* Describes software addresses, which apply to devices in the POETS Engine
 * that have been placed.
 *
 * A software address is sufficient to identify a device deployed in the POETS
 * stack, either to a thread on a Tinsel core, or something else. In addition,
 * a software address contains information about the type of device, and a
 * variety of other quantities. Unlike hardware addresses, software address
 * formats are fixed at compile-time.
 *
 * Software addresses are fundamentally 32-bit integers, where (MSB-first):
 *
 * - Bit 0 (isMothership): Is 1 if the device is reached through a Mothership,
 *   and 0 otherwise.
 *
 * - Bit 1 (isCnc): Is 1 if the device is a Supervisor device (if isMothership
 *   is also 1), and 0 otherwise.
 *
 *   The combination of these first two bits defines the type of device being
 *   addressed, according to this table:
 *
 *   +------------+-----+-------------------------------+
 *   |isMothership|isCnc|        Device Category        |
 *   +------------+-----+-------------------------------+
 *   | 0          | 0   | Normal device                 |
 *   | 1          | 0   | External device               |
 *   | 1          | 1   | Supervisor device             |
 *   | 0          | 1   | Normal device (OnCtl handler) |
 *   +------------+-----+-------------------------------+
 *
 * - Bits 2 to 7 (task): Identifies the task that the device is associated
 *   with (particularly relevant for supervisor devices).
 *
 * - Bits 8 to 15 (opCode): Identifies an opCode to be passed to OnCtl. Is
 *   identically zero if isCnc is zero, but may also be zero if isCnc is one.
 *
 * - Bits 16 to 31 (device): Identifies the device uniquely given the other
 *   address components. Is identically zero if isCnc and isMothership are both
 *   one (i.e. if it's a supervisor device).
 *
 * Setters on software address components operate on a complete address,
 * which is sliced when getters are called.
 *
 * Software addresses also track which of their components have been defined -
 * developers can query this state calling SoftwareAddress::is_fully_defined().
 *
 * Unlike hardware addresses, software addresses are not hierarchical.
 *
 * See the software address documentation for further information about
 * software addresses. */

#include "DumpUtils.h"
#include "InvalidAddressException.h"
#include "OSFixes.hpp"

#define TASK_MAX 63  /* Where 63 = 0b111111 is the greatest value to fit in six
                      * bits. */

/* Define some common shifts that are applied to the member "raw" to obtain
 * components. */
#define ISMOTHERSHIP_SHIFT 31
#define ISCNC_SHIFT 30
#define TASK_SHIFT 24
#define OPCODE_SHIFT 16
#define DEVICE_SHIFT 0

/* Define masks for each section. The parentheses are important. */
#define ISMOTHERSHIP_BIT_MASK (1 << ISMOTHERSHIP_SHIFT)
#define ISCNC_BIT_MASK (1 << ISCNC_SHIFT)
#define TASK_BIT_MASK (TASK_MAX << TASK_SHIFT)
#define OPCODE_BIT_MASK (255 << OPCODE_SHIFT)
#define DEVICE_BIT_MASK (65535)  /* = 2 ^ 16 - 1 */

/* Define masks for setting the 'definitions' variable. */
#define SOFTWARE_ADDRESS_FULLY_DEFINED_MASK 31  /* 0b11111 */
#define ISMOTHERSHIP_DEFINED_MASK 1             /* 0b00001 */
#define ISCNC_DEFINED_MASK 2                    /* 0b00010 */
#define TASK_DEFINED_MASK 4                     /* 0b00100 */
#define OPCODE_DEFINED_MASK 8                   /* 0b01000 */
#define DEVICE_DEFINED_MASK 16                  /* 0b10000 */

/* Software address integer representation. */
typedef uint32_t SoftwareAddressInt;

/* Component representations (for setters and getters, not for storage) */
typedef bool IsMothershipComponent;
typedef bool IsCncComponent;
typedef uint8_t TaskComponent;
typedef uint8_t OpCodeComponent;
typedef uint16_t DeviceComponent;

class SoftwareAddress: public DumpChan
{
public:
    SoftwareAddress(IsMothershipComponent isMothership, IsCncComponent isCnc,
                    TaskComponent task, OpCodeComponent opCode,
                    DeviceComponent device);
    SoftwareAddress();

    /* Getters and setters. */
    IsMothershipComponent get_ismothership();
    IsCncComponent get_iscnc();
    TaskComponent get_task();
    OpCodeComponent get_opcode();
    DeviceComponent get_device();

    void set_ismothership(IsMothershipComponent value);
    void set_iscnc(IsCncComponent value);
    void set_task(TaskComponent value);
    void set_opcode(OpCodeComponent value);
    void set_device(DeviceComponent value);

    /* Access */
    inline SoftwareAddressInt get_software_address(){return raw;}
    inline SoftwareAddressInt as_uint(){return get_software_address();}
    void Dump(FILE* = stdout);

    /* Defines whether or not the software address is fully defined. */
    inline bool is_fully_defined()
        {return definitions == SOFTWARE_ADDRESS_FULLY_DEFINED_MASK;}
    inline bool is_ismothership_defined()
        {return (definitions & ISMOTHERSHIP_DEFINED_MASK) > 0;}
    inline bool is_iscnc_defined()
        {return (definitions & ISCNC_DEFINED_MASK) > 0;}
    inline bool is_task_defined()
        {return (definitions & TASK_DEFINED_MASK) > 0;}
    inline bool is_opcode_defined()
        {return (definitions & OPCODE_DEFINED_MASK) > 0;}
    inline bool is_device_defined()
        {return (definitions & DEVICE_DEFINED_MASK) > 0;}

private:
    SoftwareAddressInt raw;

    /* Binary word which stores which components of the address have been
     * defined. Bits are 0 if not defined, and 1 if defined, according to the
     * map (LSB-first):
     *
     * - Bit 0: isMothership
     * - Bit 1: isCnc
     * - Bit 2: task
     * - Bit 3: opCode
     * - Bit 4: device
     *
     * Functionally this acts as an array of booleans, but uses less memory. */
    uint8_t definitions;

    /* Convenience method for defining bits of definitions. */
    inline void set_defined(unsigned index){definitions |= (1 << index);};
};
#endif
