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

    SECTION("Check engine contains the correct number of boxes", "[Aesop]")
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

    SECTION("Check each box is unique", "[Aesop]")
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

    SECTION("Check each box has correct static properties", "[Aesop]")
    {
        std::map<AddressComponent, PoetsBox*>::iterator boxIterator;
        for (boxIterator=engine.PoetsBoxes.begin();
             boxIterator!=engine.PoetsBoxes.end(); boxIterator++)
        {
            REQUIRE(boxIterator->second->supervisorMemory ==
                    deployer.boxSupervisorMemory);
            REQUIRE(boxIterator->second->costBoxBoard -
                    deployer.costBoxBoard < 1e-8);
            REQUIRE(boxIterator->second->costBoxBoard -
                    deployer.costBoxBoard > -1e-8);
        }
    }

    /* Compute number of boards total in the engine according to the
     * deployer. This accumulation assumes there is at least one board... */
    unsigned boardCount = std::accumulate(deployer.boardsInEngine.begin(),
                                          deployer.boardsInEngine.end(), 1,
                                          std::multiplies<unsigned>());

    SECTION("Check engine contains the correct number of boards", "[Aesop]")
    {
        REQUIRE(engine.PoetsBoards.SizeNodes() == boardCount);
    }

    SECTION("Check each board is unique", "[Aesop]")
    {
        std::set<PoetsBoard*> uniqueBoards;
        WALKPDIGRAPHNODES(AddressComponent, PoetsBoard*,
                          unsigned int, float,
                          unsigned int, unsigned int,
                          engine.PoetsBoards, boardIterator)
        {
            uniqueBoards.insert(boardIterator->second.data);
        }
        REQUIRE(uniqueBoards.size() == boardCount);
    }

    SECTION("Check each box contains the correct number of boards", "[Aesop]")
    {
        unsigned boardsPerBox = boardCount / engine.PoetsBoxes.size();
        std::map<AddressComponent, PoetsBox*>::iterator boxIterator;
        for (boxIterator=engine.PoetsBoxes.begin();
             boxIterator!=engine.PoetsBoxes.end(); boxIterator++)
        {
            REQUIRE(boxIterator->second->PoetsBoards.size() == boardsPerBox);
        }
    }

    /* Compute number of mailboxes in each board according to the
     * deployer. This accumulation assumes there is at least one board and one
     * mailbox... */
    unsigned mailboxCount = std::accumulate(deployer.mailboxesInBoard.begin(),
                                            deployer.mailboxesInBoard.end(),
                                            1, std::multiplies<unsigned>());

    SECTION("Check each board contains the correct number of mailboxes", "[Aesop]")
    {
        WALKPDIGRAPHNODES(AddressComponent, PoetsBoard*,
                          unsigned int, float,
                          unsigned int, unsigned int,
                          engine.PoetsBoards, boardIterator)
        {
            REQUIRE(boardIterator->second.data->PoetsMailboxes.SizeNodes() ==
                    mailboxCount);
        }
    }

    SECTION("Check each board has correct static properties", "[Aesop]")
    {
        WALKPDIGRAPHNODES(AddressComponent, PoetsBoard*,
                          unsigned int, float,
                          unsigned int, unsigned int,
                          engine.PoetsBoards, boardIterator)
        {
            REQUIRE(boardIterator->second.data->dram == deployer.dram);
            REQUIRE(boardIterator->second.data->supervisorMemory ==
                    deployer.boardSupervisorMemory);
            REQUIRE(boardIterator->second.data->costBoardMailbox -
                    deployer.costBoardMailbox < 1e-8);
            REQUIRE(boardIterator->second.data->costBoardMailbox -
                    deployer.costBoardMailbox > -1e-8);
        }
    }

    /* Gather all mailboxes in the engine for implementation elegance later. */
    std::set<PoetsMailbox*> uniqueMailboxes;
    std::set<PoetsMailbox*>::iterator uniqueMailboxIterator;

    /* For each board... */
    WALKPDIGRAPHNODES(AddressComponent, PoetsBoard*,
                      unsigned int, float,
                      unsigned int, unsigned int,
                      engine.PoetsBoards, boardIterator)
    {
        /* For each mailbox in that board... */
        WALKPDIGRAPHNODES(AddressComponent, PoetsMailbox*,
                          unsigned int, float,
                          unsigned int, unsigned int,
                          boardIterator->second.data->PoetsMailboxes,
                          mailboxIterator)
        {
            /* Track if unique. */
            uniqueMailboxes.insert(&(*(mailboxIterator)->second.data));
        }
    }

    SECTION("Check each mailbox in the engine is unique", "[Aesop]")
    {
        /* Right-hand side is number of boards in engine * number of mailboxes
         * in board. */
        REQUIRE(uniqueMailboxes.size() == boardCount * mailboxCount);
    }

    SECTION("Check each mailbox has correct static properties", "[Aesop]")
    {
        for (uniqueMailboxIterator=uniqueMailboxes.begin();
             uniqueMailboxIterator!=uniqueMailboxes.end();
             uniqueMailboxIterator++)
        {
            REQUIRE((*uniqueMailboxIterator)->costMailboxCore -
                    deployer.costMailboxCore < 1e-8);
            REQUIRE((*uniqueMailboxIterator)->costMailboxCore -
                    deployer.costMailboxCore > -1e-8);
        }
    }

    SECTION("Check each mailbox contains the correct number of cores", "[Aesop]")
    {
        for (uniqueMailboxIterator=uniqueMailboxes.begin();
             uniqueMailboxIterator!=uniqueMailboxes.end();
             uniqueMailboxIterator++)
        {
            REQUIRE((*uniqueMailboxIterator)->PoetsCores.size() ==
                    deployer.coresInMailbox);
        }
    }

    /* Gather all cores in the engine for implementation elegance later. */
    std::set<PoetsCore*> uniqueCores;
    std::set<PoetsCore*>::iterator uniqueCoreIterator;

    std::map<AddressComponent, PoetsCore*>::iterator coreIterator;
    for (uniqueMailboxIterator=uniqueMailboxes.begin();
         uniqueMailboxIterator!=uniqueMailboxes.end();
         uniqueMailboxIterator++)
    {
        for (coreIterator=(*uniqueMailboxIterator)->PoetsCores.begin();
             coreIterator!=(*uniqueMailboxIterator)->PoetsCores.end();
             coreIterator++)
        {
            uniqueCores.insert(coreIterator->second);
        }
    }

    SECTION("Check each core in the engine is unique", "[Aesop]")
    {
        /* Right-hand side is number of boards in engine * number of mailboxes
         * in board * number of cores in mailbox. */
        REQUIRE(uniqueCores.size() ==
                boardCount * mailboxCount * deployer.coresInMailbox);
    }

    SECTION("Check each core has correct static properties", "[Aesop]")
    {
        for (uniqueCoreIterator=uniqueCores.begin();
             uniqueCoreIterator!=uniqueCores.end(); uniqueCoreIterator++)
        {
            REQUIRE((*uniqueCoreIterator)->costCoreThread -
                    deployer.costCoreThread < 1e-8);
            REQUIRE((*uniqueCoreIterator)->costCoreThread -
                    deployer.costCoreThread > -1e-8);
        }
    }

    SECTION("Check each core contains the correct number of threads", "[Aesop]")
    {
        for (uniqueCoreIterator=uniqueCores.begin();
             uniqueCoreIterator!=uniqueCores.end(); uniqueCoreIterator++)
        {
            REQUIRE((*uniqueCoreIterator)->PoetsThreads.size() ==
                    deployer.threadsInCore);
        }
    }

    SECTION("Check each thread in the engine is unique", "[Aesop]")
    {
        std::set<PoetsThread*> uniqueThreads;
        std::map<AddressComponent, PoetsThread*>::iterator threadIterator;
        for (uniqueCoreIterator=uniqueCores.begin();
             uniqueCoreIterator!=uniqueCores.end();
             uniqueCoreIterator++)
        {
            for (threadIterator=(*uniqueCoreIterator)->PoetsThreads.begin();
                 threadIterator!=(*uniqueCoreIterator)->PoetsThreads.end();
                 threadIterator++)
            {
                uniqueThreads.insert(threadIterator->second);
            }
        }

        /* Right-hand side is number of boards in engine * number of mailboxes
         * in board * number of cores in mailbox * number of threads in a
         * core. */
        REQUIRE(uniqueThreads.size() ==
                boardCount * mailboxCount * deployer.coresInMailbox
                * deployer.threadsInCore);
    }

    SECTION("Dump for fun", "[Aesop]")
    {
        engine.dump();
    }
}
