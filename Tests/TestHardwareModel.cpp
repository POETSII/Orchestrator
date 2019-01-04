/* Tests the connectivity mechanism of the hardware model. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "HardwareModel.h"

TEST_CASE("A full stack of hardware can be connected", "[Items]")
{
    PoetsEngine engine("Engine000");
    PoetsBox* box;
    PoetsBoard* board;
    PoetsMailbox* firstMailbox;
    PoetsMailbox* secondMailbox;
    PoetsCore* core;
    PoetsThread* thread;

    box = new PoetsBox("Box000");
    board = new PoetsBoard("Board000");
    firstMailbox = new PoetsMailbox("Mailbox000");
    secondMailbox = new PoetsMailbox("Mailbox001");
    core = new PoetsCore("Core000");
    thread = new PoetsThread("Thread000");

    engine.contain(4, box);
    box->contain(17, board);
    engine.contain(17, board);
    board->contain(0, firstMailbox);
    board->contain(1, secondMailbox);
    board->connect(0, 1, 10);
    firstMailbox->contain(10, core);
    core->contain(123, thread);
}

TEST_CASE("Threads cannot be claimed multiple times", "[Items]")
{
    PoetsCore core("Core000");
    PoetsThread* thread;
    thread = new PoetsThread("Thread000");

    core.contain(123, thread);
    REQUIRE_THROWS_AS(core.contain(124, thread), OwnershipException&);
}

TEST_CASE("Cores cannot be claimed multiple times", "[Items]")
{
    PoetsMailbox mailbox("Mailbox000");
    PoetsCore* core;
    core = new PoetsCore("Core000");

    mailbox.contain(10, core);
    REQUIRE_THROWS_AS(mailbox.contain(124, core), OwnershipException&);
}

TEST_CASE("Mailboxes cannot be claimed multiple times", "[Items]")
{
    PoetsBoard board("Board000");
    PoetsMailbox* mailbox;
    mailbox = new PoetsMailbox("Mailbox000");

    board.contain(0, mailbox);
    REQUIRE_THROWS_AS(board.contain(124, mailbox), OwnershipException&);
}

TEST_CASE("Boards cannot be claimed multiple times by boxes", "[Items]")
{
    PoetsBox box("Box000");
    PoetsBoard* board;
    board = new PoetsBoard("Board000");

    box.contain(17, board);
    REQUIRE_THROWS_AS(box.contain(124, board), OwnershipException&);
}

TEST_CASE("Boards cannot be claimed multiple times by engines", "[Items]")
{
    PoetsEngine engine("Engine000");
    PoetsBox* box;
    PoetsBoard* board;
    box = new PoetsBox("Box000");
    board = new PoetsBoard("Board000");

    engine.contain(4, box);
    box->contain(17, board);
    engine.contain(17, board);
    REQUIRE_THROWS_AS(engine.contain(17, board), OwnershipException&);
}

TEST_CASE("Boards that are not contained in boxes in the engine cannot be claimed", "[Items]")
{
    PoetsEngine engine("Engine000");
    PoetsBoard* board;
    board = new PoetsBoard("Board000");

    REQUIRE_THROWS_AS(engine.contain(2, board), OwnershipException&);
    delete board;  /* Otherwise we're leaking. */
}

TEST_CASE("Boxes cannot be claimed multiple times", "[Items]")
{
    PoetsEngine engine("Engine000");
    PoetsBox* box;
    box = new PoetsBox("Box000");

    engine.contain(4, box);
    REQUIRE_THROWS_AS(engine.contain(124, box), OwnershipException&);
}

TEST_CASE("Engines are empty when initialised", "[Emptiness]")
{
    PoetsEngine engine("Engine000");
    REQUIRE(engine.is_empty());
}

TEST_CASE("Engines are not empty when populated", "[Emptiness]")
{
    PoetsEngine engine("Engine000");
    PoetsBox* box;
    box = new PoetsBox("Box000");
    engine.contain(0, box);
    REQUIRE(engine.is_empty() == false);
}

TEST_CASE("Engines are empty when populated and then cleared", "[Emptiness]")
{
    PoetsEngine engine("Engine000");
    PoetsBox* box;
    box = new PoetsBox("Box000");
    engine.contain(0, box);
    engine.clear();
    REQUIRE(engine.is_empty());
}
