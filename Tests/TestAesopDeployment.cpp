/* Tests the deployment of Aesop for integrity. Does not test that the
   configuration in AesopDeployer.cpp is accurate. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "AesopDeployer.h"
#include "HardwareAddressFormat.h"
#include "HardwareModel.h"

TEST_CASE("Deployment to an empty engine", "[Aesop]")
{
    PoetsEngine engine("Engine000");
    HardwareAddressFormat addressFormat;
    AesopDeployer deployer;
    deployer.deploy(&engine, &addressFormat);

    SECTION("Check engine contains the correct number of boxes")
    {
        /* Check there is a value for each logical address component. */
        REQUIRE(engine.PoetsBoxes.size() == deployer.boxesInEngine);
        for (AddressComponent boxIndex=0; boxIndex < deployer.boxesInEngine;
             boxIndex++)
        {
            REQUIRE(engine.PoetsBoxes.find(boxIndex++) !=
                    engine.PoetsBoxes.end());
        }
    }

    SECTION("Check each box is unique")
    {
        std::set<PoetsBox*> uniqueBoxes;
        std::map<AddressComponent, PoetsBox*>::iterator boxIterator;
        for (boxIterator=engine.PoetsBoxes.begin();
             boxIterator!=engine.PoetsBoxes.end(); boxIterator++)
        {
            uniqueBoxes.insert(boxIterator->second);
        }
        REQUIRE(uniqueBoxes.size() == deployer.boxesInEngine);
    }

    /* Compute number of boards total in the engine. This accumulation assumes
     * there is at least one board... */
    unsigned boardCount = std::accumulate(deployer.boardsInEngine.begin(),
                                          deployer.boardsInEngine.end(), 1,
                                          std::multiplies<unsigned>());

    SECTION("Check engine contains the correct number of boards")
    {
        REQUIRE(engine.PoetsBoards.SizeNodes() == boardCount);
    }

    SECTION("Check each board is unique")
    {
        std::set<PoetsBoard*> uniqueBoards;
        WALKPDIGRAPHNODES(AddressComponent, PoetsBoard*,
                          unsigned int, float,
                          unsigned int, unsigned int,
                          engine.PoetsBoards, boardIterator)
        {
            uniqueBoards.insert(*(boardIterator->second));
        }
        REQUIRE(uniqueBoards.size() == boardCount);
    }

    SECTION("Check each box contains the correct number of boards")
    {
        unsigned boardsPerBox = boardCount / engine.PoetsBoxes.size();
        std::map<AddressComponent, PoetsBox*>::iterator boxIterator;
        for (boxIterator=engine.PoetsBoxes.begin();
             boxIterator!=engine.PoetsBoxes.end(); boxIterator++)
        {
            REQUIRE(boxIterator->second->PoetsBoards.size() == boardsPerBox);
        }
    }

    SECTION("Dump for fun")
    {
        engine.dump();
    }
}
