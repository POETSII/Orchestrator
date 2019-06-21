/* Defines behaviour for the dialect 1 functionality of the hardware model file
 * parser (see the accompanying header for further information). */

#include "HardwareFileParser.h"

/* Populates a POETS Engine with information from a previously-loaded hardware
 * description file in dialect 3, after validating it. Arguments:
 *
 * - engine: Pointer to the POETS Engine to populate. Cleared if the input file
 *   is valid.
 *
 * Throws if:
 *
 *  - the input file is semantically invalid. */
void HardwareFileParser::d3_populate_hardware_model(P_engine* engine)
{
    /* Check sections are defined correctly. */
    std::string errorMessage;
    bool failedValidation = false;
    failedValidation |= !d3_validate_sections(&errorMessage);
}

/* Validate that the section occurences in the hardware description input file
 * is semantically-valid in dialect 3.
 *
 * The following sections must be defined exactly once:
 *
 * - header (with an optional context-sensitive argument)
 * - packet_address_format
 * - engine_box
 * - engine_board
 * - core
 *
 * The following sections must be defined once or more:
 *
 * - board (all must have an argument)
 * - mailbox (all must have an argument)
 *
 * The following sections must be defined once or less:
 *
 * - default_types
 *
 * Returns true if the following conditions are all true:
 *
 * - all sections occur the appropriate number of times
 * - no sections exist that don't follow the above convention
 *
 * and false otherwise. Arguments:
 *
 * - errorMessage: string to append error messages to. */
bool HardwareFileParser::d3_validate_sections(std::string* errorMessage)
{return true;}
