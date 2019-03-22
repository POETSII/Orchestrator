/* Tests the hardware iterator with a variety of engine configurations. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "HardwareIterator.h"

TEST_CASE("Check that iterating over invalid engines throws.")
{
    P_engine engine("Engine000");
    P_box* box = new P_box("Box000");
    P_board* board = new P_board("Board000");
    P_mailbox* mailbox = new P_mailbox("Mailbox000");
    P_core* core = new P_core("Core000");
    P_thread* thread = new P_thread("Thread000");

    engine.addressFormat = HardwareAddressFormat(4, 4, 4, 4, 4);

    /* Check that trying to iterator over an empty engine throws an
     * IteratorException. */
    REQUIRE_THROWS_AS(HardwareIterator(&engine), IteratorException&);

    engine.contain(0, box);
    box->contain(0, board);
    engine.contain(0, board);

    /* Check that trying to iterate over an engine with an empty board throws
     * an IteratorException. */
    REQUIRE_THROWS_AS(HardwareIterator(&engine), IteratorException&);

    board->contain(0, mailbox);

    /* Check that trying to iterate over an engine with an empty mailbox throws
     * an IteratorException. */
    REQUIRE_THROWS_AS(HardwareIterator(&engine), IteratorException&);

    mailbox->contain(0, core);

    /* Check that trying to iterate over an engine with an empty core throws
     * an IteratorException. */
    REQUIRE_THROWS_AS(HardwareIterator(&engine), IteratorException&);

    core->contain(0, thread);

    /* Check that a fully-defined engine does not cause the iterator to
     * throw. */
    HardwareIterator iterator(&engine);
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
        box->contain(boardIndex, board);
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

    /* Check that the iterator doesn't fall over during initialisation. */
    HardwareIterator iterator(&engine);

    /* Placeholder variables to hold the first of each item in the engine. */
    P_board* firstBoard;
    P_mailbox* firstMailbox;
    P_core* firstCore;
    P_thread* firstThread;

    /* Check that the iterator is initialised on the first item. */
    firstBoard = engine.G.NodeBegin()->second.data;
    firstMailbox = firstBoard->G.NodeBegin()->second.data;
    firstCore = firstMailbox->P_corem.begin()->second;
    firstThread = firstCore->P_threadm.begin()->second;

    REQUIRE(iterator.get_board() == firstBoard);
    REQUIRE(iterator.get_mailbox() == firstMailbox);
    REQUIRE(iterator.get_core() == firstCore);
    REQUIRE(iterator.get_thread() == firstThread);

    /* Check that, on initialisation, none of the items have changed. */
    REQUIRE(iterator.has_board_changed() == false);
    REQUIRE(iterator.has_mailbox_changed() == false);
    REQUIRE(iterator.has_core_changed() == false);
    REQUIRE(iterator.has_thread_changed() == false);

    /* Placeholder variables used to define values expected from the test. */
    P_board* nextBoard;
    P_mailbox* nextMailbox;
    P_core* nextCore;
    P_thread* nextThread;

    /* Placeholder iterators. */
    std::map<AddressComponent, P_thread*>::iterator threadIterator;
    std::map<AddressComponent, P_core*>::iterator coreIterator;
    pdigraph<AddressComponent, P_mailbox*,
             unsigned int, P_link*,
             unsigned int, P_port*>::TPn_it mailboxIterator;
    pdigraph<AddressComponent, P_board*,
             unsigned int, P_link*,
             unsigned int, P_port*>::TPn_it boardIterator;

    /* Check that the thread advances when the iterator is incremented. */
    threadIterator = firstCore->P_threadm.begin();
    nextThread = (++threadIterator)->second;
    REQUIRE(nextThread == iterator.next_thread());
    REQUIRE(nextThread == iterator.get_thread());

    /* Check that the iterator tracks the changed thread, but only once. */
    REQUIRE(iterator.has_thread_changed() == true);
    REQUIRE(iterator.has_thread_changed() == false);

    /* Check that a core-level iteration changes the thread to be the first
     * thread on that core. */
    coreIterator = firstMailbox->P_corem.begin();

    /* Test that the thread and core haven't changed yet. */
    REQUIRE(iterator.has_core_changed() == false);
    REQUIRE(iterator.has_thread_changed() == false);

    /* Check that the core increments. */
    nextCore = (++coreIterator)->second;
    REQUIRE(nextCore == iterator.next_core());

    /* Check that the current thread is the first thread on that core. */
    nextThread = nextCore->P_threadm.begin()->second;
    REQUIRE(nextThread == iterator.get_thread());

    /* Test that the thread and core changed after being incremented. */
    REQUIRE(iterator.has_core_changed() == true);
    REQUIRE(iterator.has_thread_changed() == true);

    /* Check mailbox-level iteration in the same way. */
    mailboxIterator = firstBoard->G.NodeBegin();

    /* Test that the mailbox, thread, and core haven't changed yet. */
    REQUIRE(iterator.has_mailbox_changed() == false);
    REQUIRE(iterator.has_core_changed() == false);
    REQUIRE(iterator.has_thread_changed() == false);

    /* Check that the mailbox increments. */
    nextMailbox = (++mailboxIterator)->second.data;
    REQUIRE(nextMailbox == iterator.next_mailbox());

    /* Check that the current core is the first core on that mailbox. */
    nextCore = nextMailbox->P_corem.begin()->second;
    REQUIRE(nextCore == iterator.get_core());

    /* Check that the current thread is the first thread on that core. */
    nextThread = nextCore->P_threadm.begin()->second;
    REQUIRE(nextThread == iterator.get_thread());

    /* Test that the mailbox, thread, and core changed after being
     * incremented. */
    REQUIRE(iterator.has_mailbox_changed() == true);
    REQUIRE(iterator.has_core_changed() == true);
    REQUIRE(iterator.has_thread_changed() == true);

    /* Check board-level iteration in the same way. */
    boardIterator = engine.G.NodeBegin();

    /* Test that the board, mailbox, thread, and core haven't changed yet. */
    REQUIRE(iterator.has_board_changed() == false);
    REQUIRE(iterator.has_mailbox_changed() == false);
    REQUIRE(iterator.has_core_changed() == false);
    REQUIRE(iterator.has_thread_changed() == false);

    /* Check that the board increments. */
    nextBoard = (++boardIterator)->second.data;
    REQUIRE(nextBoard == iterator.next_board());

    /* Check that the current mailbox is the first mailbox on that board. */
    nextMailbox = nextBoard->G.NodeBegin()->second.data;
    REQUIRE(nextMailbox == iterator.get_mailbox());

    /* Check that the current core is the first core on that mailbox. */
    nextCore = nextMailbox->P_corem.begin()->second;
    REQUIRE(nextCore == iterator.get_core());

    /* Check that the current thread is the first thread on that core. */
    nextThread = nextCore->P_threadm.begin()->second;
    REQUIRE(nextThread == iterator.get_thread());

    /* Test that the board, mailbox, thread, and core changed after being
     * incremented. */
    REQUIRE(iterator.has_board_changed() == true);
    REQUIRE(iterator.has_mailbox_changed() == true);
    REQUIRE(iterator.has_core_changed() == true);
    REQUIRE(iterator.has_thread_changed() == true);

    /* Check that resetting the iterator points the sub-iterators to the first
     * of each item in the engine. */
    iterator.reset_all_iterators();
    REQUIRE(firstBoard == iterator.get_board());
    REQUIRE(firstMailbox == iterator.get_mailbox());
    REQUIRE(firstCore == iterator.get_core());
    REQUIRE(firstThread == iterator.get_thread());

    /* Test that the board, mailbox, core, and thread all changed (since we
     * were in no-man's land after the previous test). */
    REQUIRE(iterator.has_board_changed() == true);
    REQUIRE(iterator.has_mailbox_changed() == true);
    REQUIRE(iterator.has_core_changed() == true);
    REQUIRE(iterator.has_thread_changed() == true);

    /* Check that iterating the thread over a core boundary also iterates the
     * core. */

    /* We've just reset the iterator. Push the iterator to a core boundary. */
    for (unsigned threadIndex = 1; threadIndex < threadsInCore;
         threadIndex++) iterator.next_thread();

    /* Check that the thread has changed, but nothing else. */
    REQUIRE(iterator.has_board_changed() == false);
    REQUIRE(iterator.has_mailbox_changed() == false);
    REQUIRE(iterator.has_core_changed() == false);
    REQUIRE(iterator.has_thread_changed() == true);

    /* Push it over the edge. */
    iterator.next_thread();

    /* Check that both the core and thread have changed, but nothing else. */
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

    /* Check that iterating the final thread in the engine wraps around to the
     * beginning. */
    iterator.reset_all_iterators();
    unsigned requiredIncrements = (threadsInCore *
                                   coresInMailbox *
                                   mailboxesInBoard *
                                   boardsInEngine);
    /* Iterate to the last core. */
    for (unsigned index = 1; index < requiredIncrements; index++)
    {
        iterator.next_thread();
    }

    /* Check everything has changed. */
    REQUIRE(iterator.has_board_changed() == true);
    REQUIRE(iterator.has_mailbox_changed() == true);
    REQUIRE(iterator.has_core_changed() == true);
    REQUIRE(iterator.has_thread_changed() == true);

    /* Dumping test included for fun, if you are so inclined. Uncomment to get
     * a dump of the iterator at its end. */
    // printf("Here's a dump of the hardware iterator just before it iterates "
    //        "over the edge of the hardware stack:\n");
    // iterator.Dump();

    /* Push over the edge. */
    iterator.next_thread();

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

    /* Dumping test included for fun, if you are so inclined. Uncomment to get
     * a dump of the iterator while it's "fresh". */
    // printf("\nHere's a dump of a \"fresh\" hardware iterator:\n");
    // iterator.Dump();
}
