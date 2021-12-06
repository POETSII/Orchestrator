/* Tests the Simple deployer for integrity. Does not test that the
   configuration in SimpleDeployer.cpp is accurate. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "HardwareAddressFormat.h"
#include "HardwareModel.h"
#include "MultiSimpleDeployer.h"
#include "SimpleDeployer.h"

TEST_CASE("Simple deployment to an empty engine", "[Simple]")
{
    P_engine engine("Engine000");
    SimpleDeployer deployer;
    deployer.deploy(&engine);

    SECTION("Check the hardware address format in the engine is populated correctly", "[Simple]")
    {
        REQUIRE(engine.addressFormat.get_box_word_length() ==
                deployer.boxWordLength);
        REQUIRE(engine.addressFormat.get_core_word_length() ==
                deployer.coreWordLength);
        REQUIRE(engine.addressFormat.get_thread_word_length() ==
                deployer.threadWordLength);

        /* Sum-reduce the formats that are defined as vectors. */
        REQUIRE(engine.addressFormat.get_board_word_length() ==
            std::accumulate(deployer.boardWordLengths.begin(),
                            deployer.boardWordLengths.end(), 0));
        REQUIRE(engine.addressFormat.get_mailbox_word_length() ==
            std::accumulate(deployer.mailboxWordLengths.begin(),
                            deployer.mailboxWordLengths.end(), 0));
    }

    SECTION("Check engine contains the correct number of boxes", "[Simple]")
    {
        /* Check there is a value for each logical address component. */
        REQUIRE(engine.P_boxm.size() == deployer.boxesInEngine);
        for (AddressComponent boxIndex=0; boxIndex < deployer.boxesInEngine;
             boxIndex++)
        {
            REQUIRE(engine.P_boxm.find(boxIndex++) !=
                    engine.P_boxm.end());
        }
    }

    SECTION("Check each box is unique", "[Simple]")
    {
        std::set<P_box*> uniqueBoxes;
        std::map<AddressComponent, P_box*>::iterator boxIterator;
        for (boxIterator=engine.P_boxm.begin();
             boxIterator!=engine.P_boxm.end(); boxIterator++)
        {
            uniqueBoxes.insert(boxIterator->second);
        }
        REQUIRE(uniqueBoxes.size() == deployer.boxesInEngine);
    }

    SECTION("Check each box has correct static properties", "[Simple]")
    {
        std::map<AddressComponent, P_box*>::iterator boxIterator;
        for (boxIterator=engine.P_boxm.begin();
             boxIterator!=engine.P_boxm.end(); boxIterator++)
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

    SECTION("Check engine contains the correct number of boards", "[Simple]")
    {
        REQUIRE(engine.G.SizeNodes() == boardCount);
    }

    SECTION("Check each board is unique", "[Simple]")
    {
        std::set<P_board*> uniqueBoards;
        WALKPDIGRAPHNODES(AddressComponent, P_board*,
                          unsigned, P_link*,
                          unsigned, P_port*,
                          engine.G, boardIterator)
        {
            uniqueBoards.insert(boardIterator->second.data);
        }
        REQUIRE(uniqueBoards.size() == boardCount);
    }

    SECTION("Check each box contains the correct number of boards", "[Simple]")
    {
        unsigned boardsPerBox = boardCount / engine.P_boxm.size();
        std::map<AddressComponent, P_box*>::iterator boxIterator;
        for (boxIterator=engine.P_boxm.begin();
             boxIterator!=engine.P_boxm.end(); boxIterator++)
        {
            REQUIRE(boxIterator->second->P_boardv.size() == boardsPerBox);
        }
    }

    /* Compute number of mailboxes in each board according to the
     * deployer. This accumulation assumes there is at least one board and one
     * mailbox... */
    unsigned mailboxCount = std::accumulate(deployer.mailboxesInBoard.begin(),
                                            deployer.mailboxesInBoard.end(),
                                            1, std::multiplies<unsigned>());

    SECTION("Check each board contains the correct number of mailboxes", "[Simple]")
    {
        WALKPDIGRAPHNODES(AddressComponent, P_board*,
                          unsigned, P_link*,
                          unsigned, P_port*,
                          engine.G, boardIterator)
        {
            REQUIRE(boardIterator->second.data->G.SizeNodes() ==
                    mailboxCount);
        }
    }

    SECTION("Check each board has correct static properties", "[Simple]")
    {
        WALKPDIGRAPHNODES(AddressComponent, P_board*,
                          unsigned, P_link*,
                          unsigned, P_port*,
                          engine.G, boardIterator)
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
    std::set<P_mailbox*> uniqueMailboxes;
    std::set<P_mailbox*>::iterator uniqueMailboxIterator;

    /* For each board... */
    WALKPDIGRAPHNODES(AddressComponent, P_board*,
                      unsigned, P_link*,
                      unsigned, P_port*,
                      engine.G, boardIterator)
    {
        /* For each mailbox in that board... */
        WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                          unsigned, P_link*,
                          unsigned, P_port*,
                          boardIterator->second.data->G,
                          mailboxIterator)
        {
            /* Track if unique. */
            uniqueMailboxes.insert(&(*(mailboxIterator)->second.data));
        }
    }

    SECTION("Check each mailbox in the engine is unique", "[Simple]")
    {
        /* Right-hand side is number of boards in engine * number of mailboxes
         * in board. */
        REQUIRE(uniqueMailboxes.size() == boardCount * mailboxCount);
    }

    SECTION("Check each mailbox has correct static properties", "[Simple]")
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

    SECTION("Check each mailbox contains the correct number of cores", "[Simple]")
    {
        for (uniqueMailboxIterator=uniqueMailboxes.begin();
             uniqueMailboxIterator!=uniqueMailboxes.end();
             uniqueMailboxIterator++)
        {
            REQUIRE((*uniqueMailboxIterator)->P_corem.size() ==
                    deployer.coresInMailbox);
        }
    }

    /* Gather all cores in the engine for implementation elegance later. */
    std::set<P_core*> uniqueCores;
    std::set<P_core*>::iterator uniqueCoreIterator;

    std::map<AddressComponent, P_core*>::iterator coreIterator;
    for (uniqueMailboxIterator=uniqueMailboxes.begin();
         uniqueMailboxIterator!=uniqueMailboxes.end();
         uniqueMailboxIterator++)
    {
        for (coreIterator=(*uniqueMailboxIterator)->P_corem.begin();
             coreIterator!=(*uniqueMailboxIterator)->P_corem.end();
             coreIterator++)
        {
            uniqueCores.insert(coreIterator->second);
        }
    }

    SECTION("Check each core in the engine is unique", "[Simple]")
    {
        /* Right-hand side is number of boards in engine * number of mailboxes
         * in board * number of cores in mailbox. */
        REQUIRE(uniqueCores.size() ==
                boardCount * mailboxCount * deployer.coresInMailbox);
    }

    SECTION("Check each core has correct static properties", "[Simple]")
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

    SECTION("Check each core contains the correct number of threads", "[Simple]")
    {
        for (uniqueCoreIterator=uniqueCores.begin();
             uniqueCoreIterator!=uniqueCores.end(); uniqueCoreIterator++)
        {
            REQUIRE((*uniqueCoreIterator)->P_threadm.size() ==
                    deployer.threadsInCore);
        }
    }

    SECTION("Check each thread in the engine is unique", "[Simple]")
    {
        std::set<P_thread*> uniqueThreads;
        std::map<AddressComponent, P_thread*>::iterator threadIterator;
        for (uniqueCoreIterator=uniqueCores.begin();
             uniqueCoreIterator!=uniqueCores.end();
             uniqueCoreIterator++)
        {
            for (threadIterator=(*uniqueCoreIterator)->P_threadm.begin();
                 threadIterator!=(*uniqueCoreIterator)->P_threadm.end();
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

    /* Dumping test included for fun, if you are so inclined. Uncomment to get
       a dump of the Simple configuration. */ /*
    SECTION("Dump for fun", "[Simple]")
    {
        engine.Dump();
    } */
}

TEST_CASE("MultiSimple(4) deployment to an empty engine", "[MultiSimple]")
{
    unsigned multiple = 4;

    P_engine engine("Engine000");
    MultiSimpleDeployer deployer(4);
    SimpleDeployer comparisonDeployer;
    deployer.deploy(&engine);

    SECTION("Check engine contains the correct number of boxes", "[MultiSimple]")
    {
        REQUIRE(engine.P_boxm.size() ==
                comparisonDeployer.boxesInEngine * multiple);
    }

    /* Compute number of boards total in the engine according to the
     * deployer. This accumulation assumes there is at least one board... */
    unsigned comparisonBoardCount = std::accumulate(
        comparisonDeployer.boardsInEngine.begin(),
        comparisonDeployer.boardsInEngine.end(), 1,
        std::multiplies<unsigned>());

    SECTION("Check engine contains the correct number of boards", "[MultiSimple]")
    {
        REQUIRE(engine.G.SizeNodes() == comparisonBoardCount * multiple);
    }
}
