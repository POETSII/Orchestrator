/* Tests the hardware description file reader for syntax checking. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "HardwareFileReader.h"
#include "HardwareModel.h"

/* C++98 does not have a cross-platform way of listing directories... grumble
 * grumble. */
std::vector<std::string> badDialectInputs = {
    "invalid_dialect_3_bad_dialect_definition_1.uif",
    "invalid_dialect_3_bad_dialect_definition_2.uif",
    "invalid_dialect_3_bad_dialect_definition_3.uif",
    "invalid_dialect_3_bad_dialect_definition_4.uif"
};

TEST_CASE("Files with a valid syntax do not raise", "[Reader]")
{
    HardwareFileReader reader;
    reader.load_file("../Tests/StaticResources/Dialect1/valid_test_file.uif");
}

TEST_CASE("Files with an invalid syntax raise syntax error", "[Reader]")
{
    HardwareFileReader reader;
    REQUIRE_THROWS_AS(reader.load_file(
        "../Tests/StaticResources/Dialect1/syntactically_invalid_test_file.uif"),
                      HardwareSyntaxException&);
}

TEST_CASE("Files that do not exist raise a file-not-found error", "[Reader]")
{
    HardwareFileReader reader;

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

    REQUIRE_THROWS_AS(reader.load_file(filePath.c_str()),
                      HardwareFileNotFoundException&);
}

TEST_CASE("Attempting to populate before loaded a file raises", "[Reader]")
{
    P_engine* engine = new P_engine("Test Engine");
    HardwareFileReader reader;
    REQUIRE_THROWS_AS(reader.populate_hardware_model(engine),
                      HardwareFileNotLoadedException&);
    delete engine;
}

TEST_CASE("Test each invalid dialect example case in turn", "[Reader]")
{
    P_engine* engine;
    HardwareFileReader* reader;
    std::vector<std::string>::iterator fileName;
    std::string fullPath;

    /* Create a fresh reader and engine object for each test. */
    for(fileName=badDialectInputs.begin();
        fileName!=badDialectInputs.end();
        fileName++)
    {
        engine = new P_engine("Test Engine");
        reader = new HardwareFileReader();

        fullPath = "../Tests/StaticResources/InvalidDialect/" + *fileName;
        reader->load_file(fullPath.c_str());
        REQUIRE_THROWS_AS(reader->populate_hardware_model(engine),
                          HardwareSemanticException&);

        delete engine;
        delete reader;
    }
}
