/* Tests the Dialect 3 reader's semantic checking, and output on success.
 *
 * Missing files and files with invalid syntax are tested by the
 * TestHardwareFileReader test suite. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "HardwareFileReader.h"
#include "HardwareModel.h"

/* C++98 does not have a cross-platform way of listing directories... grumble
 * grumble. */
std::vector<std::string> semanticallyValidInputs = {
    "8_box.uif",
    "valid_dialect_3_mismatched_name.uif",
    "valid_dialect_3_one_thread_per_core.uif",
    "valid_dialect_3_some_types_in_sections.uif",
    "valid_dialect_3_types_everywhere.uif",
    "valid_dialect_3.uif"
};

std::vector<std::string> semanticallyInvalidInputs = {
    "invalid_dialect_3_board_name_too_short.uif",
    "invalid_dialect_3_box_name_too_long.uif",
    "invalid_dialect_3_broken_header_variable_1.uif",
    "invalid_dialect_3_broken_header_variable_2.uif",
    "invalid_dialect_3_duplicate_section.uif",
    "invalid_dialect_3_empty.uif",
    "invalid_dialect_3_floating_dram.uif",
    "invalid_dialect_3_invalid_character_in_type.uif",
    "invalid_dialect_3_invalid_variable_key.uif",
    "invalid_dialect_3_missing_board_type.uif",
    "invalid_dialect_3_missing_box_type.uif",
    "invalid_dialect_3_missing_cost.uif",
    "invalid_dialect_3_missing_mailbox_type.uif",
    "invalid_dialect_3_missing_mandatory_header_item.uif",
    "invalid_dialect_3_missing_mandatory_header_variable_1.uif",
    "invalid_dialect_3_missing_mandatory_header_variable_2.uif",
    "invalid_dialect_3_missing_packet_length.uif",
    "invalid_dialect_3_missing_referenced_board_2.uif",
    "invalid_dialect_3_missing_referenced_board.uif",
    "invalid_dialect_3_missing_referenced_box.uif",
    "invalid_dialect_3_missing_referenced_mailbox_2.uif",
    "invalid_dialect_3_missing_referenced_mailbox.uif",
    "invalid_dialect_3_missing_reverse_edge_board_definition.uif",
    "invalid_dialect_3_missing_reverse_edge_mailbox_definition.uif",
    "invalid_dialect_3_missing_section.uif",
    "invalid_dialect_3_nonfloat_cost.uif",
    "invalid_dialect_3_type_too_long.uif",
    "invalid_dialect_3_type_too_short.uif",
    "invalid_dialect_3_undefined_type.uif"
};

/* Missing files and files with invalid syntax are tested by the Dialect 1
 * reader test suite (they both go through the same syntax-checking bit of
 * source. */
TEST_CASE("Files with valid syntax and semantics do not raise", "[Reader]")
{
    P_engine* engine;
    HardwareFileReader* reader;
    std::vector<std::string>::iterator fileName;
    std::string fullPath;

    /* Create a fresh reader and engine object for each test. */
    for(fileName=semanticallyValidInputs.begin();
        fileName!=semanticallyValidInputs.end();
        fileName++)
    {
        printf("Testing %s...\n", (*fileName).c_str());

        engine = new P_engine("Test Engine");
        reader = new HardwareFileReader();

        fullPath = "../Tests/StaticResources/Dialect3/Valid/" + *fileName;
        reader->load_file(fullPath.c_str());
        reader->populate_hardware_model(engine);

        delete engine;
        delete reader;
    }
}

TEST_CASE("Test each semantically-invalid case in turn", "[Reader]")
{
    P_engine* engine;
    HardwareFileReader* reader;
    std::vector<std::string>::iterator fileName;
    std::string fullPath;

    /* Create a fresh reader and engine object for each test. */
    for(fileName=semanticallyInvalidInputs.begin();
        fileName!=semanticallyInvalidInputs.end();
        fileName++)
    {
        printf("Testing %s (xfail)...\n", (*fileName).c_str());

        engine = new P_engine("Test Engine");
        reader = new HardwareFileReader();

        fullPath = "../Tests/StaticResources/Dialect3/Invalid/" + *fileName;
        reader->load_file(fullPath.c_str());
        REQUIRE_THROWS_AS(reader->populate_hardware_model(engine),
                          HardwareSemanticException&);

        /* Don't need to delete the engine - the reader is responsible for
         * doing that on failure. If a bug has been introduced that causes the
         * reader not to clean up after itself, Valgrind will find it.
         *
         * We do need to delete the reader at the end of the loop though. */
        delete reader;
    }
}
