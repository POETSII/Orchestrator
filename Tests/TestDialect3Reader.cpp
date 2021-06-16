/* Tests the Dialect 3 reader's semantic checking, and output on success.
 *
 * Missing files and files with invalid syntax are tested by the
 * TestHardwareFileReader test suite. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "HardwareFileReader.h"
#include "HardwareModel.h"

/* Missing files and files with invalid syntax are tested by the Dialect 1
 * reader test suite (they both go through the same syntax-checking bit of
 * source. */
TEST_CASE("Files with valid syntax and semantics do not raise", "[Reader]")
{
   /* C++98 does not have a cross-platform way of listing
    * directories... grumble grumble. */
    std::vector<std::string> semanticallyValidInputs;
    semanticallyValidInputs.push_back("1_box.uif");
    semanticallyValidInputs.push_back("6_box.uif");
    semanticallyValidInputs.push_back("8_box.uif");
    semanticallyValidInputs.push_back("valid_dialect_3_mismatched_name.uif");
    semanticallyValidInputs.push_back("valid_dialect_3_mpi.uif");
    semanticallyValidInputs.push_back("valid_dialect_3_one_thread_per_core.uif");
    semanticallyValidInputs.push_back("valid_dialect_3_some_types_in_sections.uif");
    semanticallyValidInputs.push_back("valid_dialect_3_types_everywhere.uif");
    semanticallyValidInputs.push_back("valid_dialect_3.uif");
    semanticallyValidInputs.push_back("valid_dialect_3_weird_names.uif");
    semanticallyValidInputs.push_back("valid_dialect_3_with_hostnames.uif");

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
    /* C++98 does not have a cross-platform way of listing
     * directories... grumble grumble. */
    std::vector<std::string> semanticallyInvalidInputs;
    semanticallyInvalidInputs.push_back("invalid_dialect_3_board_name_too_short.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_box_name_too_long.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_broken_header_variable_1.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_broken_header_variable_2.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_duplicate_section.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_empty.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_floating_dram.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_invalid_character_in_type.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_invalid_variable_key.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_board_type.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_box_type.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_cost.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_mailbox_type.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_mandatory_header_item.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_mandatory_header_variable_1.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_mandatory_header_variable_2.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_packet_length.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_referenced_board_2.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_referenced_board.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_referenced_box.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_referenced_mailbox_2.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_referenced_mailbox.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_reverse_edge_board_definition.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_reverse_edge_mailbox_definition.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_missing_section.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_nonfloat_cost.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_odd_paired_cores.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_type_too_long.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_type_too_short.uif");
    semanticallyInvalidInputs.push_back("invalid_dialect_3_undefined_type.uif");

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

        delete engine;
        delete reader;
    }
}
