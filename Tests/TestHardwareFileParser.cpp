/* Tests various behaviours of the hardware addressing mechanism. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "HardwareFileParser.h"
#include "HardwareModel.h"

TEST_CASE("Files with a valid syntax do not raise", "[Parser]")
{
    HardwareFileParser parser;
    parser.load_file("../Tests/StaticResources/valid_test_file.uif");
}

TEST_CASE("Files with an invalid syntax raise syntax error", "[Parser]")
{
    HardwareFileParser parser;
    REQUIRE_THROWS_AS(parser.load_file(
        "../Tests/StaticResources/syntactically_invalid_test_file.uif"),
                      HardwareSyntaxException&);
}

TEST_CASE("Files that do not exist raise a file-not-found error", "[Parser]")
{
    HardwareFileParser parser;

    /* Find a file that doesn't exist by starting from "x", and adding "x"s
     * until we hit a non-existent file. Would be better with POSIX's stat. */
    std::string filePath;
    FILE* testFile;
    while (true)
    {
        testFile = fopen(filePath.c_str(), "r");
        if (testFile)
        {
            fclose(testFile);
            filePath.append("x");
        }
        else {break;}
    }

    REQUIRE_THROWS_AS(parser.load_file(filePath.c_str()),
                      HardwareFileNotFoundException&);
}

TEST_CASE("Attempting to populate before loaded a file raises", "[Parser]")
{
    P_engine* engine = new P_engine("Test Engine");
    HardwareFileParser parser;
    REQUIRE_THROWS_AS(parser.populate_hardware_model(engine),
                      HardwareFileNotLoadedException&);
    delete engine;
}

TEST_CASE("Aesop example does not raise end-to-end", "[Parser]")
{
    P_engine* engine = new P_engine("Test Engine");
    HardwareFileParser parser("../Tests/StaticResources/aesop_dialect_1.uif",
                              engine);
    delete engine;
}

TEST_CASE("A file with multiple identical valid sections raise a semantics error", "[Parser]")
{
    P_engine* engine = new P_engine("Test Engine");
    HardwareFileParser parser;
    parser.load_file("../Tests/StaticResources/duplicate_section_invalid.uif");
    REQUIRE_THROWS_AS(parser.populate_hardware_model(engine),
                      HardwareSemanticException&);
    delete engine;
}

TEST_CASE("A file with a missing section raises a semantics error", "[Parser]")
{
    P_engine* engine = new P_engine("Test Engine");
    HardwareFileParser parser;
    parser.load_file("../Tests/StaticResources/missing_section_invalid.uif");
    REQUIRE_THROWS_AS(parser.populate_hardware_model(engine),
                      HardwareSemanticException&);
    delete engine;
}

TEST_CASE("A file with an invalid section raises a semantics error", "[Parser]")
{
    P_engine* engine = new P_engine("Test Engine");
    HardwareFileParser parser;
    parser.load_file("../Tests/StaticResources/invalid_section.uif");
    REQUIRE_THROWS_AS(parser.populate_hardware_model(engine),
                      HardwareSemanticException&);
    delete engine;
}
