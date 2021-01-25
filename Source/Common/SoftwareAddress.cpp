/* Defines software address behaviour (see the accompanying header for further
 * information). */

#include "SoftwareAddress.h"

/* Constructs a software address, and fully populates it. Each argument is a
 * component of the software address. */
SoftwareAddress::SoftwareAddress(
    IsMothershipComponent isMothership,
    IsCncComponent isCnc,
    TaskComponent task,
    OpCodeComponent opCode,
    DeviceComponent device)
{
    definitions = 0;
    raw = 0;

    /* Use the setters to check for validity where appropriate. */
    set_ismothership(isMothership);
    set_iscnc(isCnc);
    set_task(task);
    set_opcode(opCode);
    set_device(device);
}

/* Alternatively, construct a software address from a uint32_t. */
SoftwareAddress::SoftwareAddress(uint32_t address){
    definitions = 10;
    raw = address;
}

/* Alternatively, construct a software address without explicitly priming it
 * with values. */
SoftwareAddress::SoftwareAddress(){
    definitions = 0;
    raw = 0;
}

/* Getters, which read from raw. The return values for the opCode and deviceId
 * getters shrink from the MSB to fit into the return type. */
IsMothershipComponent SoftwareAddress::get_ismothership()
    {return raw >> ISMOTHERSHIP_SHIFT;}
/* Here, the bitwise-and clears the isMothership bit, so the output is either 1
 * or 0. */
IsCncComponent SoftwareAddress::get_iscnc()
    {return ((raw >> ISCNC_SHIFT) & 1);}
/* Here, the bitwise-and clears the isMothership and isCnc bits, so the
 * output will fit in six bits. */
TaskComponent SoftwareAddress::get_task()
    {return (raw >> TASK_SHIFT) & TASK_MAX;}
OpCodeComponent SoftwareAddress::get_opcode()
    {return raw >> OPCODE_SHIFT;}
DeviceComponent SoftwareAddress::get_device(){return raw;}

/* Setters, which set the address component with a value.
 *
 * Also updates the "definitions" word. In the task setter, also raises if the
 * input does not fit within the loaded format. Arguments:
 *
 * - value: Value to set. */
void SoftwareAddress::set_ismothership(IsMothershipComponent value)
{
    if (value)
    {
        raw |= ISMOTHERSHIP_BIT_MASK;
    }
    else
    {
        raw &= ~ISMOTHERSHIP_BIT_MASK;
    }
    set_defined(0);
}

void SoftwareAddress::set_iscnc(IsCncComponent value)
{
    if (value)
    {
        raw |= ISCNC_BIT_MASK;
    }
    else
    {
        raw &= ~ISCNC_BIT_MASK;
    }
    set_defined(1);
}

void SoftwareAddress::set_task(TaskComponent value)
{
    /* Validate */
    if (value > TASK_MAX)
    {
        throw InvalidAddressException(
            dformat("[ERROR] Task component value \"%u\" does not fit the "
                    "software address format. The task component must fit in "
                    "6-bits, and so must be less than %d.", value, TASK_MAX));
    }

    raw &= ~TASK_BIT_MASK; /* Clear */
    raw |= value << TASK_SHIFT; /* Set */
    set_defined(2);
}

void SoftwareAddress::set_opcode(OpCodeComponent value)
{
    /* Note that there is no validation here for non-cnc addresses to
     * facilitate out-of-order address definitions. */
    raw &= ~OPCODE_BIT_MASK; /* Clear */
    raw |= value << OPCODE_SHIFT; /* Set */
    set_defined(3);
}

void SoftwareAddress::set_device(DeviceComponent value)
{
    raw &= ~DEVICE_BIT_MASK; /* Clear */
    raw |= value; /* Set */
    set_defined(4);
}

/* Construct a software address from a uint32_t. */
void SoftwareAddress::from_uint(uint32_t address){
    definitions = 10;
    raw = address;
}

/* Write debug and diagnostic information using dumpchan. Arguments:
 *
 * - file: File to dump to. */
void SoftwareAddress::Dump(unsigned off,FILE* file)
{
    std::string prefix = dformat("%d Software address at %#018lx",
                                 off, (uint64_t) this);
    DumpUtils::open_breaker(file, prefix);

    /* The raw address. */
    fprintf(file, "raw: %" SWA_FMT "\n", raw);

    /* The components. */
    fprintf(file, "isMothership: %s ", get_ismothership() ? "true" : "false");
    fprintf(file, "%s\n", is_ismothership_defined() ? "" : "(not defined)");

    fprintf(file, "isCnc: %s ", get_iscnc() ? "true" : "false");
    fprintf(file, "%s\n", is_iscnc_defined() ? "" : "(not defined)");

    fprintf(file, "task: %u ", get_task());
    fprintf(file, "%s\n", is_task_defined() ? "" : "(not defined)");

    fprintf(file, "opCode: %u ", get_opcode());
    fprintf(file, "%s\n", is_opcode_defined() ? "" : "(not defined)");

    fprintf(file, "device: %u ", get_device());
    fprintf(file, "%s\n", is_device_defined() ? "" : "(not defined)");

    /* Close breaker and flush the dump. */
    DumpUtils::close_breaker(file, prefix);
    fflush(file);
}
