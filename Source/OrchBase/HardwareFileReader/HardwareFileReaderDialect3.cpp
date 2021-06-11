/* Defines behaviour for the dialect 3 functionality of the hardware model file
 * reader (see the accompanying header for further information). */

#include "HardwareFileReader.h"

/* Catastrophic failure, we out, yo. Well, "catastrophic" is perhaps taking it
 * a bit far. My point is, this error is unrecoverable from the perspective of
 * the file reader.
 *
 * This method constructs an error output string, puts it in the message of an
 * exception, then tosses it over the fence. Arguments:
 *
 * - engine: Partially-populated unsafe engine. */
void HardwareFileReader::d3_catastrophic_failure()
{
    std::string allErrors;
    construct_error_output_string(&allErrors);

    /* Lob over the fence. */
    throw HardwareSemanticException(allErrors.c_str());
}

/* Populates a POETS Engine with information from a previously-loaded hardware
 * description file in dialect 3, after validating it. Arguments:
 *
 * - engine: Pointer to the POETS Engine to populate. Cleared if the input file
 *   is valid.
 *
 * Throws if:
 *
 *  - the input file is semantically invalid. */
void HardwareFileReader::d3_populate_hardware_model(P_engine* engine)
{
    /* Check sections are defined correctly. Not much point continuing if they
     * are not defined correctly. */
    if (!d3_load_validate_sections()){d3_catastrophic_failure();}

    /* Populate the engine with information from the header section. */
    bool passedValidation = true;
    passedValidation &= d3_populate_validate_header(engine);

    /* Populate the hardware address format owned by the engine. */
    passedValidation &= d3_populate_validate_address_format(engine);

    /* Verify that default types, and type fields in sections, are valid, and
     * map to sections that exist. */
    passedValidation &= d3_validate_types_define_cache();

    /* We can't go any further than this without risking a segfault, because if
     * the address format is not defined properly, item address assignment will
     * fail badly. */
    if (!passedValidation){d3_catastrophic_failure();}

    /* Boxes */
    passedValidation &= d3_populate_validate_engine_box(engine);

    /* Boards (and the items beneath) */
    passedValidation &= d3_populate_validate_engine_board_and_below(engine);

    /* Were there any errors? */
    if (!passedValidation){d3_catastrophic_failure();}
}
