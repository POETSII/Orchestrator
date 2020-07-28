/* Tests the Dialect 3 reader's semantic checking, and output on success.
 *
 * Missing files and files with invalid syntax are tested by the
 * TestHardwareFileReader test suite. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "HardwareFileReader.h"
#include "HardwareModel.h"

TEST_CASE("Aesop example does not raise end-to-end", "[Reader]")
{
    P_engine* engine = new P_engine("Test Engine");
    HardwareFileReader reader("../Tests/StaticResources/Dialect1/aesop_dialect_1.uif",
                              engine);
    delete engine;
}

TEST_CASE("A file with multiple identical valid sections raise a semantics error", "[Reader]")
{
    P_engine* engine = new P_engine("Test Engine");
    HardwareFileReader reader;
    reader.load_file("../Tests/StaticResources/Dialect1/duplicate_section_invalid.uif");
    REQUIRE_THROWS_AS(reader.populate_hardware_model(engine),
                      HardwareSemanticException&);
    delete engine;
}

TEST_CASE("A file with a missing section raises a semantics error", "[Reader]")
{
    P_engine* engine = new P_engine("Test Engine");
    HardwareFileReader reader;
    reader.load_file("../Tests/StaticResources/Dialect1/missing_section_invalid.uif");
    REQUIRE_THROWS_AS(reader.populate_hardware_model(engine),
                      HardwareSemanticException&);
    delete engine;
}

TEST_CASE("A file with an invalid section raises a semantics error", "[Reader]")
{
    P_engine* engine = new P_engine("Test Engine");
    HardwareFileReader reader;
    reader.load_file("../Tests/StaticResources/Dialect1/invalid_section.uif");
    REQUIRE_THROWS_AS(reader.populate_hardware_model(engine),
                      HardwareSemanticException&);
    delete engine;
}
