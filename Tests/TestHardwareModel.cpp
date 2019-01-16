/* Tests the connectivity mechanism of the hardware model. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "HardwareAddressFormat.h"
#include "HardwareAddress.h"
#include "HardwareModel.h"

TEST_CASE("A full stack of hardware can be connected", "[Items]")
{
    P_engine engine("Engine000");
    engine.addressFormat = HardwareAddressFormat(4, 5, 6, 8, 9);
    P_box* box;
    P_board* board;
    P_mailbox* firstMailbox;
    P_mailbox* secondMailbox;
    P_core* core;
    P_thread* thread;

    box = new P_box("Box000");
    board = new P_board("Board000");
    firstMailbox = new P_mailbox("Mailbox000");
    secondMailbox = new P_mailbox("Mailbox001");
    core = new P_core("Core000");
    thread = new P_thread("Thread000");

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
    P_core core("Core000");
    P_thread* thread;
    thread = new P_thread("Thread000");

    core.contain(123, thread);
    REQUIRE_THROWS_AS(core.contain(124, thread), OwnershipException&);
}

TEST_CASE("Cores cannot be claimed multiple times", "[Items]")
{
    P_mailbox mailbox("Mailbox000");
    P_core* core;
    core = new P_core("Core000");

    mailbox.contain(10, core);
    REQUIRE_THROWS_AS(mailbox.contain(124, core), OwnershipException&);
}

TEST_CASE("Mailboxes cannot be claimed multiple times", "[Items]")
{
    P_board board("Board000");
    P_mailbox* mailbox;
    mailbox = new P_mailbox("Mailbox000");

    board.contain(0, mailbox);
    REQUIRE_THROWS_AS(board.contain(124, mailbox), OwnershipException&);
}

TEST_CASE("Boards cannot be claimed multiple times by boxes", "[Items]")
{
    P_box box("Box000");
    P_board* board;
    board = new P_board("Board000");

    box.contain(17, board);
    REQUIRE_THROWS_AS(box.contain(124, board), OwnershipException&);
}

TEST_CASE("Boards cannot be claimed multiple times by engines", "[Items]")
{
    P_engine engine("Engine000");
    P_box* box;
    P_board* board;
    box = new P_box("Box000");
    board = new P_board("Board000");

    engine.addressFormat = HardwareAddressFormat(4, 5, 6, 8, 9);
    engine.contain(4, box);
    box->contain(17, board);
    engine.contain(17, board);
    REQUIRE_THROWS_AS(engine.contain(17, board), OwnershipException&);
}

TEST_CASE("Boards that are not contained in boxes in the engine cannot be claimed", "[Items]")
{
    P_engine engine("Engine000");
    P_board* board;
    board = new P_board("Board000");

    REQUIRE_THROWS_AS(engine.contain(2, board), OwnershipException&);
    delete board;  /* Otherwise we're leaking. */
}

TEST_CASE("Boxes cannot be claimed multiple times", "[Items]")
{
    P_engine engine("Engine000");
    P_box* box;
    box = new P_box("Box000");

    engine.addressFormat = HardwareAddressFormat(4, 5, 6, 8, 9);
    engine.contain(4, box);
    REQUIRE_THROWS_AS(engine.contain(124, box), OwnershipException&);
}

TEST_CASE("Engines are empty when initialised", "[Emptiness]")
{
    P_engine engine("Engine000");
    REQUIRE(engine.is_empty());
}

TEST_CASE("Engines are not empty when populated", "[Emptiness]")
{
    P_engine engine("Engine000");
    P_box* box;
    box = new P_box("Box000");
    engine.contain(0, box);
    REQUIRE(engine.is_empty() == false);
}

TEST_CASE("Engines are empty when populated and then cleared", "[Emptiness]")
{
    P_engine engine("Engine000");
    P_box* box;
    box = new P_box("Box000");
    engine.contain(0, box);
    engine.clear();
    REQUIRE(engine.is_empty());
}

TEST_CASE("Addresses can be assigned to boxes", "[Address Assignment]")
{
    P_box box("quack");
    HardwareAddressFormat myFormat(4, 5, 6, 8, 9);
    HardwareAddress* address = new HardwareAddress(&myFormat);
    box.set_hardware_address(address);
    REQUIRE(box.get_hardware_address() == address);

    /* A change in the address object should be reflected in the box's
       address. */
    address->set_box(1);
    REQUIRE(box.get_hardware_address()->get_box() == 1);
}

TEST_CASE("Addresses can be assigned to boards", "[Address Assignment]")
{
    P_board board("quack");
    HardwareAddressFormat myFormat(4, 5, 6, 8, 9);
    HardwareAddress* address = new HardwareAddress(&myFormat);
    board.set_hardware_address(address);
    REQUIRE(board.get_hardware_address() == address);

    /* A change in the address object should be reflected in the board's
       address. */
    address->set_board(2);
    REQUIRE(board.get_hardware_address()->get_board() == 2);
}

TEST_CASE("Addresses can be assigned to mailboxes", "[Address Assignment]")
{
    P_mailbox mailbox("quack");
    HardwareAddressFormat myFormat(4, 5, 6, 8, 9);
    HardwareAddress* address = new HardwareAddress(&myFormat);
    mailbox.set_hardware_address(address);
    REQUIRE(mailbox.get_hardware_address() == address);

    /* A change in the address object should be reflected in the mailbox's
       address. */
    address->set_mailbox(3);
    REQUIRE(mailbox.get_hardware_address()->get_mailbox() == 3);
}

TEST_CASE("Addresses can be assigned to cores", "[Address Assignment]")
{
    P_core core("quack");
    HardwareAddressFormat myFormat(4, 5, 6, 8, 9);
    HardwareAddress* address = new HardwareAddress(&myFormat);
    core.set_hardware_address(address);
    REQUIRE(core.get_hardware_address() == address);

    /* A change in the address object should be reflected in the core's
       address. */
    address->set_core(4);
    REQUIRE(core.get_hardware_address()->get_core() == 4);
}

TEST_CASE("Addresses can be assigned to threads", "[Address Assignment]")
{
    P_thread thread("quack");
    HardwareAddressFormat myFormat(4, 5, 6, 8, 9);
    HardwareAddress* address = new HardwareAddress(&myFormat);
    thread.set_hardware_address(address);
    REQUIRE(thread.get_hardware_address() == address);

    /* A change in the address object should be reflected in the thread's
       address. */
    address->set_thread(5);
    REQUIRE(thread.get_hardware_address()->get_thread() == 5);
}

TEST_CASE("Metadata can be assigned to POETS engines", "[Metadata]")
{
    P_engine engine("Engine000");
    engine.author = "Some Body";
    engine.datetime = 20190107162455;
    engine.version = "0.3.1~sandwich";
    engine.fileOrigin = "TestHardwareModel.cpp fake";
}

TEST_CASE("Items in the hardware stack have meaningful addresses", "[Address Assignment]")
{
    P_engine engine("Engine000");
    P_box* box = new P_box("Box000");
    P_board* board = new P_board("Board000");
    P_mailbox* mailbox = new P_mailbox("Mailbox000");
    P_core* core = new P_core("Core000");
    P_thread* thread = new P_thread("Thread000");

    AddressComponent boxComponent = 0;
    AddressComponent boardComponent = 1;
    AddressComponent mailboxComponent = 2;
    AddressComponent coreComponent = 3;
    AddressComponent threadComponent = 4;

    engine.addressFormat = HardwareAddressFormat(0, 1, 2, 2, 3);
    engine.contain(boxComponent, box);
    box->contain(boardComponent, board);
    engine.contain(boardComponent, board);
    board->contain(mailboxComponent, mailbox);
    mailbox->contain(coreComponent, core);
    core->contain(threadComponent, thread);

    HardwareAddress* address;

    /* Check box address has the box component, and is not fully defined. */
    REQUIRE(box->get_hardware_address()->get_box() == boxComponent);
    REQUIRE(box->get_hardware_address()->is_fully_defined() == false);

    /* Check board address has the box and board components, and is not fully
     * defined. */
    REQUIRE(board->get_hardware_address()->get_box() == boxComponent);
    REQUIRE(board->get_hardware_address()->get_board() == boardComponent);
    REQUIRE(board->get_hardware_address()->is_fully_defined() == false);

    /* And so on. */
    REQUIRE(mailbox->get_hardware_address()->get_box() == boxComponent);
    REQUIRE(mailbox->get_hardware_address()->get_board() == boardComponent);
    REQUIRE(mailbox->get_hardware_address()->get_mailbox() == mailboxComponent);
    REQUIRE(mailbox->get_hardware_address()->is_fully_defined() == false);

    REQUIRE(core->get_hardware_address()->get_box() == boxComponent);
    REQUIRE(core->get_hardware_address()->get_board() == boardComponent);
    REQUIRE(core->get_hardware_address()->get_mailbox() == mailboxComponent);
    REQUIRE(core->get_hardware_address()->get_core() == coreComponent);
    REQUIRE(core->get_hardware_address()->is_fully_defined() == false);

    REQUIRE(thread->get_hardware_address()->get_box() == boxComponent);
    REQUIRE(thread->get_hardware_address()->get_board() == boardComponent);
    REQUIRE(thread->get_hardware_address()->get_mailbox() == mailboxComponent);
    REQUIRE(thread->get_hardware_address()->get_core() == coreComponent);
    REQUIRE(thread->get_hardware_address()->get_thread() == threadComponent);

    /* However, thread-addresses are supposed to be fully defined. */
    REQUIRE(thread->get_hardware_address()->is_fully_defined() == true);

    SECTION("Check that the addresses are truly unique.")
    {
        REQUIRE(board->get_hardware_address() != core->get_hardware_address());

        /* We're really trying to break it. */
        board->set_hardware_address(
            new HardwareAddress(&(engine.addressFormat)));
        REQUIRE(core->get_hardware_address()->get_box() == boxComponent);
    }
}
