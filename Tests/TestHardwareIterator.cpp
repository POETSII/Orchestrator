/* Tests the hardware iterator with a variety of engine configurations. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "HardwareAddressFormat.h"
#include "HardwareAddress.h"
#include "HardwareIterator.h"
#include "HardwareModel.h"

TEST_CASE("Check that iterating over invalid engines throws.")
{
    SECTION("Empty engine")
    {
        P_engine engine("Engine000");
        engine.addressFormat = HardwareAddressFormat(4, 4, 4, 4, 4);
        REQUIRE_THROWS_AS(HardwareIterator iterator(&engine),
                          IteratorException&);
    }

    SECTION("Empty board")
    {
        P_box* box = new P_box("Box000");
        P_board* board = new P_board("Board000");
        engine.contain(0, box);
        box->contain(0, board);
        engine.contain(0, board);

        REQUIRE_THROWS_AS(HardwareIterator iterator(&engine),
                          IteratorException&);
    }

    SECTION("Empty mailbox")
    {
        P_mailbox* mailbox = new P_mailbox("Mailbox000");
        board->contain(0, mailbox);

        REQUIRE_THROWS_AS(HardwareIterator iterator(&engine),
                          IteratorException&);
    }

    SECTION("Empty core")
    {
        P_core* core = new P_core("Core000");
        mailbox->contain(0, core);

        REQUIRE_THROWS_AS(HardwareIterator iterator(&engine),
                          IteratorException&);
    }

    SECTION("But if we put a thread in the core, it works.")
    {
        P_thread* thread = new P_thread("Thread000");
        core->contain(0, thread);
        HardwareIterator iterator(&engine);
    }
}

TEST_CASE("Iteration behaviour with a known valid engine configuration.")
{
    unsigned boardsInEngine = 2;
    unsigned mailboxesInBoard = 3;
    unsigned coresInMailbox = 4;
    unsigned threadsInCore = 5;  /* Must be greater than four. */

    /* Assembling an engine with known topology. */
    P_engine engine("Engine000");

    /* One box, for the sake of argument. */
    engine.addressFormat = HardwareAddressFormat(4, 4, 4, 4, 4);
    P_box* box = new P_box("Box000");
    engine.contain(0, box);

    /* Adding items... */
    P_board* board;
    P_mailbox* mailbox;
    P_core* core;
    P_thread* thread;

    for (unsigned boardIndex = 0; boardIndex < boardsInEngine; boardIndex++)
    {
        /* Add boards. */
        board = new P_board(dformat("Board%03u", boardIndex));
        engine.contain(boardIndex, board);

        for (unsigned mailboxIndex = 0; mailboxIndex < mailboxesInBoard;
             mailboxIndex++)
        {
            /* Add mailboxes. */
            mailbox = new P_mailbox(dformat("Mailbox%03u", mailboxIndex));
            board->contain(mailboxIndex, mailbox);

            for (unsigned coreIndex = 0; coreIndex < coresInMailbox;
                 coreIndex++)
            {
                /* Add cores. */
                core = new P_core(dformat("Core%03u", coreIndex));
                mailbox->contain(coreIndex, core);

                for (unsigned threadIndex = 0; threadIndex < threadsInCore;
                     threadIndex++)
                {
                    /* Add threads. */
                    thread = new P_thread(dformat("Thread%03u",
                                                  threadIndex));
                    core->contain(threadIndex, thread);
                }
            }
        }
    } /* Phew */

    HardwareIterator iterator;

    SECTION("Check that the iterator doesn't fall over during initialisation.")
    {
        iterator = HardwareIterator(&engine);
    }

    /* Placeholder variables to hold the first of each item in the engine. */
    P_board* firstBoard;
    P_mailbox* firstMailbox;
    P_core* firstCore;
    P_thread* firstThread;

    SECTION("Check that the iterator is initialised on the first item.")
    {
        firstBoard = engine->G.NodeBegin()->second;
        firstMailbox = firstBoard->G.NodeBegin()->second;
        firstCore = firstMailbox->P_corem.begin()->second;
        firstThread = firstCore->P_threadm.begin()->second;

        REQUIRE(iterator.get_board() == firstBoard);
        REQUIRE(iterator.get_mailbox() == firstMailbox);
        REQUIRE(iterator.get_core() == firstCore);
        REQUIRE(iterator.get_thread() == firstThread);
    }

    SECTION("Check that, on initialisation, none of the items have changed.")
    {
        REQUIRE(iterator.has_board_changed() == false);
        REQUIRE(iterator.has_mailbox_changed() == false);
        REQUIRE(iterator.has_core_changed() == false);
        REQUIRE(iterator.has_thread_changed() == false);
    }

    /* Placeholder variables used to define values expected from the test. */
    P_board* nextBoard;
    P_mailbox* nextMailbox;
    P_core* nextCore;
    P_thread* nextThread;

    std::map<AddressComponent, P_thread*>::iterator threadIterator;

    SECTION("Check that the thread advances when the iterator is incremented.")
    {
        threadIterator = firstCore->P_threadm.begin();

        /* Test with incremeting. */
        nextThread = (++threadIterator)->second;
        REQUIRE(nextThread == iterator++);
        REQUIRE(nextThread == iterator.get_thread());

        threadIterator++;
        nextThread = threadIterator->second;
        REQUIRE(nextThread == iterator++);
        REQUIRE(nextThread == iterator.get_thread());

        /* Test with direct method call. */
        nextThread = (++threadIterator)->second;
        REQUIRE(nextThread == iterator.next_thread());
        REQUIRE(nextThread == iterator.get_thread());
    }

    SECTION("Check that the iterator tracks the changed thread, but only once.")
    {
        REQUIRE(iterator.has_thread_changed() == true);
        REQUIRE(iterator.has_thread_changed() == false);
    }

    std::map<AddressComponent, P_core*>::iterator coreIterator;

    SECTION("Check that a core-level iteration changes the thread to be the first thread on that core.")
    {
        coreIterator = firstMailbox->P_corem.begin();

        /* Test that the thread and core haven't changed yet. */
        REQUIRE(iterator.has_core_changed() == false);
        REQUIRE(iterator.has_thread_changed() == false);

        /* Check that the core increments. */
        nextCore = (++coreIterator)->second;
        REQUIRE(nextCore == iterator.next_core());

        /* Check that the current thread is the first thread on that core. */
        nextThread = nextCore->P_threadm.begin()->second
        REQUIRE(nextThread == iterator.get_thread());nnn

        /* Test that the thread and core changed after being incremented. */
        REQUIRE(iterator.has_core_changed() == true);
        REQUIRE(iterator.has_thread_changed() == true);

    }

    std::map<AddressComponent, P_mailbox*>::iterator mailboxIterator;

    SECTION("Check mailbox-level iteration in the same way.")
    {
        mailboxIterator = firstBoard->G.NodeBegin();

        /* Test that the mailbox, thread, and core haven't changed yet. */
        REQUIRE(iterator.has_mailbox_changed() == false);
        REQUIRE(iterator.has_core_changed() == false);
        REQUIRE(iterator.has_thread_changed() == false);

        /* Check that the mailbox increments. */
        nextMailbox = (++mailboxIterator)->second;
        REQUIRE(nextMailbox == iterator.next_mailbox());

        /* Check that the current core is the first core on that mailbox. */
        nextCore = nextMailbox->P_corem.begin()->second;
        REQUIRE(nextCore == iterator.get_core());

        /* Check that the current thread is the first thread on that core. */
        nextThread = nextCore->P_threadm.begin()->second
        REQUIRE(nextThread == iterator.get_thread());

        /* Test that the mailbox, thread, and core changed after being
         * incremented. */
        REQUIRE(iterator.has_mailbox_changed() == true);
        REQUIRE(iterator.has_core_changed() == true);
        REQUIRE(iterator.has_thread_changed() == true);

    }

    std::map<AddressComponent, P_board*>::iterator boardIterator;

    SECTION("Check board-level iteration in the same way.")
    {
        boardIterator = engine->G.NodeBegin();

        /* Test that the board, mailbox, thread, and core haven't changed
         * yet. */
        REQUIRE(iterator.has_board_changed() == false);
        REQUIRE(iterator.has_mailbox_changed() == false);
        REQUIRE(iterator.has_core_changed() == false);
        REQUIRE(iterator.has_thread_changed() == false);

        /* Check that the board increments. */
        nextBoard = (++boardIterator)->second;
        REQUIRE(nextMailbox == iterator.next_board());

        /* Check that the current mailbox is the first mailbox on that
         * board. */
        nextMailbox = nextBoard->G.NodeBegin()->second;
        REQUIRE(nextMailBox == iterator.get_mailbox());

        /* Check that the current core is the first core on that mailbox. */
        nextCore = nextMailbox->P_corem.begin()->second;
        REQUIRE(nextCore == iterator.get_core());

        /* Check that the current thread is the first thread on that core. */
        nextThread = nextCore->P_threadm.begin()->second
        REQUIRE(nextThread == iterator.get_thread());

        /* Test that the board, mailbox, thread, and core changed after being
         * incremented. */
        REQUIRE(iterator.has_board_changed() == true);
        REQUIRE(iterator.has_mailbox_changed() == true);
        REQUIRE(iterator.has_core_changed() == true);
        REQUIRE(iterator.has_thread_changed() == true);
    }

    SECTION("Check that resetting the iterator points the sub-iterators to the first of each item in the engine.")
    {
        iterator.reset_all_iterators();
        REQUIRE(firstBoard = iterator.get_board());
        REQUIRE(firstMailbox = iterator.get_mailbox());
        REQUIRE(firstCore = iterator.get_core());
        REQUIRE(firstThread = iterator.get_thread());

        /* Test that the board, mailbox, core, and thread all changed (since we
         * were in no-man's land after the previous test). */
        REQUIRE(iterator.has_board_changed() == true);
        REQUIRE(iterator.has_mailbox_changed() == true);
        REQUIRE(iterator.has_core_changed() == true);
        REQUIRE(iterator.has_thread_changed() == true);
    }

    SECTION("Check that iterating the thread over a core boundary also iterates the core.")
    {
        /* We've just reset the iterator. Push the iterator to a core
         * boundary. */
        for (unsigned threadIndex = 1; threadIndex < threadsInCore;
             threadIndex++; iterator++);

        /* Check that the thread has changed, but nothing else. */
        REQUIRE(iterator.has_board_changed() == false);
        REQUIRE(iterator.has_mailbox_changed() == false);
        REQUIRE(iterator.has_core_changed() == false);
        REQUIRE(iterator.has_thread_changed() == true);

        /* Push it over the edge. */
        iterator++;

        /* Check that both the core and thread have changed, but nothing
         * else. */
        REQUIRE(iterator.has_board_changed() == false);
        REQUIRE(iterator.has_mailbox_changed() == false);
        REQUIRE(iterator.has_core_changed() == true);
        REQUIRE(iterator.has_thread_changed() == true);

        /* Check that the iterator is now pointing to the first thread on the
         * second core of the engine. */
        coreIterator = firstMailbox->P_corem.begin();
        nextCore = (++coreIterator)->second;
        REQUIRE(nextCore == iterator.get_core());
        REQUIRE(nextCore->P_threadm.begin()->second == iterator.get_thread());
    }

    SECTION("Check that iterating the final thread in the engine wraps around to the beginning.")
    {
        iterator.reset_all_iterators();
        unsigned requiredIncrements = (threadsInCore *
                                       coresInMailbox *
                                       mailboxesInBoard *
                                       boardsInEngine);
        /* Iterate to the last core. */
        for (int index = 1; index < requiredIncrements; index++; iterator++);

        /* Check everything has changed. */
        REQUIRE(iterator.has_board_changed() == true);
        REQUIRE(iterator.has_mailbox_changed() == true);
        REQUIRE(iterator.has_core_changed() == true);
        REQUIRE(iterator.has_thread_changed() == true);

        /* Push over the edge. */
        iterator++;

        /* Check that we're back at the start. */
        REQUIRE(firstBoard == iterator.get_board());
        REQUIRE(firstMailbox == iterator.get_mailbox());
        REQUIRE(firstCore == iterator.get_core());
        REQUIRE(firstThread == iterator.get_thread());

        /* Check everything has changed again. */
        REQUIRE(iterator.has_board_changed() == true);
        REQUIRE(iterator.has_mailbox_changed() == true);
        REQUIRE(iterator.has_core_changed() == true);
        REQUIRE(iterator.has_thread_changed() == true);

        /* Check we've wrapped, but that the information is lost when read. */
        REQUIRE(iterator.has_wrapped() == true);
        REQUIRE(iterator.has_wrapped() == false);
    }

    /* Dumping test included for fun, if you are so inclined. Uncomment to get
       a dump of the iterator. /*
    SECTION("Dump for fun")
    {
        iterator.Dump();
    } */
}
