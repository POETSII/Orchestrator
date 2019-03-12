/* Defines hardware iterator behaviour (see the accompanying header for further
 * information). */

#include "HardwareIterator.h"

/* Constructs the hardware iterator. Arguments:
 *
 * - engine: POETS Engine to iterate over. */
HardwareIterator::HardwareIterator(P_engine* engine):
    engine(engine), isWrapped(false)
{
    /* Namebase setup; the engine is the parent. */
    Name("Iterator for engine \"%s\"", P_engine->FullName().c_str());
    Npar(engine);

    check_engine();
    reset_all_iterators();

    /* Initialise "has the item changed" booleans. */
    for (int i = 0; i < 4; i++){itemChanged.push_back(false)}
}

/* Check that the engine is ready for iteration.
 *
 * The engine must contain at least one board, each board must contain at least
 * one mailbox, each mailbox must contain at least one core, and each core must
 * contain at least one thread.
 *
 * Raises IteratorException if the aforementioned statement is not true. */
bool check_engine()
{
    std::string exceptionHeader = dformat("Cannot construct iterator for "
                                          "engine \"%s\"",
                                          P_engine->FullName().c_str());

    /* Check for boards. */
    if (engine->G.SizeNodes() == 0)
    {
        throw IteratorException("%s because it contains no boards.",
                                exceptionHeader.c_str());
    }

    for (boardIterator = engine->G.index_n.begin();
         boardIterator != engine->G.index_n.end(); boardIterator++)
    {
        /* Check all boards have mailboxes. */
        P_board* currentBoard = get_board();
        if (currentBoard->G.SizeNodes() == 0)
        {
            throw IteratorException("%s because board \"%s\" contains no "
                                    "mailboxes.",
                                    currentBoard->FullName().c_str());
        }

        for (mailboxIterator = currentBoard->G.index_n.begin();
             mailboxIterator != currentBoard->G.index_n.end();
             mailboxIterator++)
        {

            /* Check all mailboxes have cores. */
            P_mailbox* currentMailbox = get_mailbox();
            if (currentMailbox->P_corem.empty())
            {
                throw IteratorException("%s because mailbox \"%s\" contains "
                                        "no cores.",
                                        currentMailbox->FullName().c_str());
            }

            for (coreIterator = currentMailbox->P_corem.begin();
                 coreIterator != currentMailbox->P_corem.end(); coreIterator++)
            {
                /* Check all cores have threads. */
                P_core* currentCore = get_core();
                if (currentCore->P_threadm.empty())
                {
                    throw IteratorException("%s because core \"%s\" contains "
                                            "no threads.",
                                            currentCore->FullName().c_str());
                }
            }
        }
    }
}


/* Getters, each return the address of the item they get. */
P_board* HardwareIterator::get_board()
{
    return engine->G.NodeData(boardIterator);
}

P_mailbox* HardwareIterator::get_mailbox()
{
    return get_board()->G.NodeData(mailboxIterator);
}

P_core* HardwareIterator::get_core()
{
    return coreIterator->second;
}

P_thread* HardwareIterator::get_thread()
{
    return threadIterator->second;
}

/* Convenience change-detection method. */
bool HardwareIterator::has_item_changed(int itemIndex)
{
    bool output = itemChanged[itemIndex];
    itemChanged[itemIndex] = false;
    return output;
}

/* Returns whether or not the iterator has wrapped since the last time this
 * method was called. */
bool HardwareIterator::has_wrapped()
{
    bool output = isWrapped;
    isWrapped = false;
    return isWrapped;
}

/* Incrementers! These all take no arguments, iterate to the next level of the
 * hierarchy, and set all lower levels to the first value of the item in that
 * container. Read the disclaimer in the header. */
P_thread* HardwareIterator::next_thread()
{
    threadIterator++;

    /* If we've run out of threads, increment the core. */
    if (threadIterator == get_core()->P_threadm.end()){next_core();}

    itemChanged[3] = true;

    return get_thread();
}

P_core* HardwareIterator::next_core()
{
    coreIterator++;

    /* If we've run out of cores, increment the mailbox. */
    if (coreIterator == get_mailbox()->P_corem.end()){next_mailbox();}

    /* Everything us-and-below has changed. */
    initialise_thread_iterator();
    itemChanged[2] = true;
    itemChanged[3] = true;

    return get_core();
}

P_core* HardwareIterator::next_mailbox()
{
    mailboxIterator++;

    /* If we've run out of mailboxes, increment the board. */
    if (mailboxIterator == get_board()->G.NodeEnd()){next_board();}

    /* Everything us-and-below has changed. */
    initialise_core_iterator();
    initialise_thread_iterator();
    itemChanged[1] = true;
    itemChanged[2] = true;
    itemChanged[3] = true;

    return get_mailbox();
}

P_core* HardwareIterator::next_board()
{
    boardIterator++;

    /* If we've run out of boards, cycle around naively. */
    if (mailboxIterator == get_board()->G.NodeEnd())
    {
        isWrapped = true;
        initialise_board_iterator();
    }

    /* Everything has changed. */
    initialise_mailbox_iterator();
    initialise_core_iterator();
    initialise_thread_iterator();
    itemChanged[0] = true;
    itemChanged[1] = true;
    itemChanged[2] = true;
    itemChanged[3] = true;

    return get_board();
}

/* Methods for resetting (re)initialising iterators. */
void HardwareIterator::reset_all_iterators()
{
    reset_board_iterator();
    reset_mailbox_iterator();
    reset_core_iterator();
    reset_thread_iterator();
}

void HardwareIterator::reset_board_iterator()
{
    boardIterator = engine->G.NodeBegin();
}

void HardwareIterator::reset_mailbox_iterator()
{
    mailboxIterator = boardIterator->second.data->G.NodeBegin();
}

void HardwareIterator::reset_core_iterator()
{
    coreIterator = mailboxIterator->second.data->P_corem.begin();
}

void HardwareIterator::reset_thread_iterator()
{
    threadIterator = coreIterator->second->P_threadm.begin();
}

/* Write debug and diagnostic information about this iterator using
 * dumpchan. Arguments:
 *
 * - file: File to dump to. */
void P_engine::Dump(FILE* file)
{
    std::string fullName = FullName();
    std::string breakerHead = fullName + " ";
    std::string breakerTail;
    if (breakerHead.size() >= MAXIMUM_BREAKER_LENGTH)
    {
        breakerTail.assign("+");
    }
    else
    {
        breakerTail.assign(MAXIMUM_BREAKER_LENGTH - breakerHead.size() - 1,
                           '+');
    }
    fprintf(file, "%s%s\n", breakerHead.c_str(), breakerTail.c_str());

    /* About this object. */
    NameBase::Dump(file);

    /* Various truths about the internal state. */
    fprintf("Current board memory address:     %#018lx\n",
            (uint64_t) get_board());
    fprintf("Current board hardware address:   %u\n",
            get_board()->get_hardware_address()->get_hardware_address());
    fprintf("Current mailbox memory address:   %#018lx\n",
            (uint64_t) get_mailbox());
    fprintf("Current mailbox hardware address: %u\n",
            get_mailbox()->get_hardware_address()->get_hardware_address());
    fprintf("Current core memory address:      %#018lx\n",
            (uint64_t) get_core());
    fprintf("Current core hardware address:    %u\n",
            get_core()->get_hardware_address()->get_hardware_address());
    fprintf("Current thread memory address:    %#018lx\n",
            (uint64_t) get_thread());
    fprintf("Current thread hardware address:  %u\n",
            get_thread()->get_hardware_address()->get_hardware_address());

    /* Print whether or not each iterator has changed since the last time it
     * was polled. */
    std::vector<std::string> itemTypes;
    itemTypes.insert(itemTypes.end(), {"board", "mailbox", "core", "thread"});
    for (long typeIndex = 0; typeIndex < itemTypes.size(); typeIndex++)
    {
        fprintf("The %s has%s changed since "
                "\"has_%s_changed\" was last called.\n",
                itemTypes[typeIndex],
                itemChanged[typeIndex] ? "" : " not",
                itemTypes[typeIndex]);
    }

    /* Print whether or not the iterator has wrapped since it was last
     * polled. */
    fprintf("This iterator has%s wrapped.\n", isWrapped ? "" : " not");

    /* Close breaker and flush the dump. */
    std::replace(breakerTail.begin(), breakerTail.end(), '+', '-');
    fprintf(file, "%s%s\n", breakerHead.c_str(), breakerTail.c_str());
    fflush(file);
}
