#ifndef __ORCHESTRATOR_SOURCE_COMMON_SOFTWAREADDRESSDEFS_H
#define __ORCHESTRATOR_SOURCE_COMMON_SOFTWAREADDRESSDEFS_H

#include "OSFixes.hpp"

/* Software Address definitions, masks and shifts.
 * Separated from SoftwareAddress.h to stop Softswitch bloat. */

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

#endif
