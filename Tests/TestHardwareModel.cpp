/* Tests the connectivity mechanism of the hardware model. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "HardwareModel.h"

TEST_CASE("A full stack of hardware can be connected", "[Items]")
{
    PoetsEngine myEngine("Engine000");
    PoetsBox myBox("Box000");
    PoetsBoard myBoard0("Board000");
    PoetsMailbox myMailbox0("Mailbox000");
    PoetsMailbox myMailbox1("Mailbox001");
    PoetsCore myCore("Core000");
    PoetsThread myThread("Thread000");

    myEngine.contain(4, &myBox);
    myBox.contain(17, &myBoard0);
    myEngine.contain(17, &myBoard0);
    myBoard0.contain(0, &myMailbox0);
    myBoard0.contain(1, &myMailbox1);
    myBoard0.connect(0, 1, 10);
    myMailbox0.contain(10, &myCore);
    myCore.contain(123, &myThread);
}

TEST_CASE("Threads cannot be claimed multiple times", "[Items]")
{
    PoetsCore myCore("Core000");
    PoetsThread myThread("Thread000");
    myCore.contain(123, &myThread);
    REQUIRE_THROWS_AS(myCore.contain(124, &myThread), OwnershipException&);
}

TEST_CASE("Cores cannot be claimed multiple times", "[Items]")
{
    PoetsMailbox myMailbox0("Mailbox000");
    PoetsCore myCore("Core000");
    myMailbox0.contain(10, &myCore);
    REQUIRE_THROWS_AS(myMailbox0.contain(124, &myCore), OwnershipException&);
}

TEST_CASE("Mailboxes cannot be claimed multiple times", "[Items]")
{
    PoetsBoard myBoard0("Board000");
    PoetsMailbox myMailbox0("Mailbox000");
    myBoard0.contain(0, &myMailbox0);
    REQUIRE_THROWS_AS(myBoard0.contain(124, &myMailbox0), OwnershipException&);
}

TEST_CASE("Boards cannot be claimed multiple times by boxes", "[Items]")
{
    PoetsBox myBox("Box000");
    PoetsBoard myBoard0("Board000");
    myBox.contain(17, &myBoard0);
    REQUIRE_THROWS_AS(myBox.contain(124, &myBoard0), OwnershipException&);
}

TEST_CASE("Boards cannot be claimed multiple times by engines", "[Items]")
{
    PoetsEngine myEngine("Engine000");
    PoetsBox myBox("Box000");
    PoetsBoard myBoard0("Board000");
    myEngine.contain(4, &myBox);
    myBox.contain(17, &myBoard0);
    myEngine.contain(17, &myBoard0);
    REQUIRE_THROWS_AS(myEngine.contain(17, &myBoard0), OwnershipException&);
}

TEST_CASE("Boards that are not contained in boxes in the engine cannot be claimed", "[Items]")
{
    PoetsEngine myEngine("Engine000");
    PoetsBoard myBoard0("Board000");
    REQUIRE_THROWS_AS(myEngine.contain(2, &myBoard0), OwnershipException&);
}

TEST_CASE("Boxes cannot be claimed multiple times", "[Items]")
{
    PoetsEngine myEngine("Engine000");
    PoetsBox myBox("Box000");
    myEngine.contain(4, &myBox);
    REQUIRE_THROWS_AS(myEngine.contain(124, &myBox), OwnershipException&);
}
