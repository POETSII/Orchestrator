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

    /* Initialise iterators. */
    initialise_board_iterator();
    initialise_mailbox_iterator();
    initialise_core_iterator();
    initialise_thread_iterator();

    /* Initialise "has the item changed" booleans. */
    for (int i = 0; i < 4; i++){itemChanged.push_back(false)}
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

/* Methods for (re)initialising iterators, for when they overrun. */
void HardwareIterator::initialise_board_iterator()
{
    boardIterator = engine->G.NodeBegin();
}

void HardwareIterator::initialise_mailbox_iterator()
{
    mailboxIterator = boardIterator->second.data->G.NodeBegin();
}

void HardwareIterator::initialise_core_iterator()
{
    coreIterator = mailboxIterator->second.data->P_corem.begin();
}

void HardwareIterator::initialise_thread_iterator()
{
    threadIterator = coreIterator->second->P_threadm.begin();
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
