/* Defines a configuration for deploying "a Simple engine". */

#include "SimpleDeployer.h"

SimpleDeployer::SimpleDeployer()
{
    /* Properties defined in order in Dialect1Deployer.h (good luck maintaining
     * that...). Costs are all zero and data memory is zero, because these are
     * not known. */

    author = "Mark Vousden";
    datetime = 201901101712;
    version = "0.3.1";
    fileOrigin = "SimpleDeployer.cpp";

    boxWordLength = 0;  /* There's only one box! */
    boardWordLengths.clear();
    boardWordLengths.push_back(2);
    boardWordLengths.push_back(0);
    mailboxWordLengths.clear();
    mailboxWordLengths.push_back(4);
    mailboxWordLengths.push_back(0);
    coreWordLength = 2;
    threadWordLength = 4;

    boxesInEngine = 1;
    boardsInEngine.clear();
    boardsInEngine.push_back(3);
    boardsInEngine.push_back(1);
    boardsAsHypercube = true;
    boardHypercubePeriodicity.clear();
    boardHypercubePeriodicity.push_back(false);
    boardHypercubePeriodicity.push_back(false);
    costExternalBox = 0;

    costBoxBoard = 0;
    costBoardBoard = 8;  /* Informed estimate */
    boxSupervisorMemory = 10240;

    mailboxesInBoard.clear();
    mailboxesInBoard.push_back(16);
    mailboxesInBoard.push_back(1);
    mailboxesAsHypercube = true;
    mailboxHypercubePeriodicity.clear();
    mailboxHypercubePeriodicity.push_back(false);
    mailboxHypercubePeriodicity.push_back(false);
    costBoardMailbox = 0;
    costMailboxMailbox = 0;
    boardSupervisorMemory = 0;
    dram = 4096;

    coresInMailbox = 4;
    costMailboxCore = 0;
    costCoreCore = 0;

    threadsInCore = 16;
    costCoreThread = 0;
    costThreadThread = 0;
    dataMemory = 0;
    instructionMemory = 8;
}
