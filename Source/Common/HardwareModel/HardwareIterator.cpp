/* Defines hardware iterator behaviour (see the accompanying header for further
 * information). */

#include "HardwareIterator.h"

/* Constructs the hardware iterator. Arguments:
 *
 * - engine: POETS Engine to iterate over
 * - checkEngine: Whether or not to check the engine when constructing the
 *   iterator. It is usually a good idea to do this. */
HardwareIterator::HardwareIterator(P_engine* engine, bool checkEngine):
    engine(engine), isWrapped(false)
{
    /* Namebase setup; the engine is the parent. */
    Name("Iterator");  /* Namebase will print the engine we're pointing to. */
    Npar(engine);

    /* Define the length of the itemChanged vector as four (one for each level
     * of the hardware hierarchy). */
    itemChanged = vector<bool>(4, false);

    /* Check that the engine is full, if desired. */
    if (checkEngine) check_engine();

    /* Checking the engine uses the iterators in this class, so we need to
     * reset them. */
    reset_all_iterators();

    /* Initialise "has the item changed" booleans. */
    for (int i = 0; i < 4; i++){itemChanged[i] = false;}
}

/* Check that the engine is ready for iteration.
 *
 * The engine must contain at least one board, each board must contain at least
 * one mailbox, each mailbox must contain at least one core, and each core must
 * contain at least one thread.
 *
 * Raises IteratorException if the aforementioned statement is not true. */
void HardwareIterator::check_engine()
{
    std::string exceptionHeader = dformat("Cannot construct iterator for "
                                          "engine \"%s\"",
                                          engine->FullName().c_str());

    /* Check for boards. */
    if (engine->G.SizeNodes() == 0)
    {
        throw IteratorException(dformat("%s because it contains no boards.",
                                        exceptionHeader.c_str()));
    }

    for (boardIterator = engine->G.NodeBegin();
         boardIterator != engine->G.NodeEnd(); boardIterator++)
    {
        /* Check all boards have mailboxes. */
        P_board* currentBoard = get_board();
        if (currentBoard->G.SizeNodes() == 0)
        {
            throw IteratorException(dformat("%s because board \"%s\" contains "
                                            "no mailboxes.",
                                            exceptionHeader.c_str(),
                                            currentBoard->FullName().c_str()));
        }

        for (mailboxIterator = currentBoard->G.NodeBegin();
             mailboxIterator != currentBoard->G.NodeEnd();
             mailboxIterator++)
        {

            /* Check all mailboxes have cores. */
            P_mailbox* currentMailbox = get_mailbox();
            if (currentMailbox->P_corem.empty())
            {
                throw IteratorException(
                    dformat("%s because mailbox \"%s\" contains no cores.",
                            exceptionHeader.c_str(),
                            currentMailbox->FullName().c_str()));
            }

            for (coreIterator = currentMailbox->P_corem.begin();
                 coreIterator != currentMailbox->P_corem.end(); coreIterator++)
            {
                /* Check all cores have threads. */
                P_core* currentCore = get_core();
                if (currentCore->P_threadm.empty())
                {
                    throw IteratorException(
                        dformat("%s because core \"%s\" contains no threads.",
                                exceptionHeader.c_str(),
                                currentCore->FullName().c_str()));
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
    return output;
}

/* Incrementers! These all take no arguments, iterate to the next level of the
 * hierarchy, and set all lower levels to the first value of the item in that
 * container. Read the disclaimer in the header. */
P_thread* HardwareIterator::next_thread()
{
    threadIterator++;

    /* If we've run out of threads, increment the core. */
    if (threadIterator == get_core()->P_threadm.end())
    {
        next_core();
    }

    itemChanged[3] = true;

    return get_thread();
}

P_core* HardwareIterator::next_core()
{
    coreIterator++;

    /* If we've run out of cores, increment the mailbox. */
    if (coreIterator == get_mailbox()->P_corem.end())
    {
        next_mailbox();
    }

    /* Everything us-and-below has changed. */
    reset_thread_iterator();
    itemChanged[2] = true;
    itemChanged[3] = true;

    return get_core();
}

P_mailbox* HardwareIterator::next_mailbox()
{
    mailboxIterator++;

    /* If we've run out of mailboxes, increment the board. */
    if (mailboxIterator == get_board()->G.NodeEnd())
    {
        next_board();
    }

    /* Everything us-and-below has changed. */
    reset_core_iterator();
    reset_thread_iterator();
    itemChanged[1] = true;
    itemChanged[2] = true;
    itemChanged[3] = true;

    return get_mailbox();
}

P_board* HardwareIterator::next_board()
{
    boardIterator++;

    /* If we've run out of boards, cycle around naively. */
    if (boardIterator == engine->G.NodeEnd())
    {
        isWrapped = true;
        reset_board_iterator();
    }

    /* Everything has changed. */
    reset_mailbox_iterator();
    reset_core_iterator();
    reset_thread_iterator();
    itemChanged[0] = true;
    itemChanged[1] = true;
    itemChanged[2] = true;
    itemChanged[3] = true;

    return get_board();
}

/* Methods for resetting (re)initialising iterators. Each reset method sets
 * it's respective itemChanged appropriately. */
void HardwareIterator::reset_all_iterators()
{
    /* Order is important. */
    reset_board_iterator();
    reset_mailbox_iterator();
    reset_core_iterator();
    reset_thread_iterator();
}

void HardwareIterator::reset_board_iterator()
{
    BoardIterator initialValue = engine->G.NodeBegin();
    itemChanged[0] = (boardIterator != initialValue);
    boardIterator = initialValue;
}

void HardwareIterator::reset_mailbox_iterator()
{
    MailboxIterator initialValue = boardIterator->second.data->G.NodeBegin();
    itemChanged[1] = (mailboxIterator != initialValue);
    mailboxIterator = initialValue;
}

void HardwareIterator::reset_core_iterator()
{
    CoreIterator initialValue = mailboxIterator->second.data->P_corem.begin();
    itemChanged[2] = (coreIterator != initialValue);
    coreIterator = initialValue;
}

void HardwareIterator::reset_thread_iterator()
{
    ThreadIterator initialValue = coreIterator->second->P_threadm.begin();
    itemChanged[3] = (threadIterator != initialValue);
    threadIterator = initialValue;
}

/* Write debug and diagnostic information about this iterator using
 * dumpchan. Arguments:
 *
 * - file: File to dump to. */
void HardwareIterator::Dump(FILE* file)
{
    std::string prefix = FullName();
    HardwareDumpUtils::open_breaker(file, prefix);

    /* About this object. */
    NameBase::Dump(file);

    /* Various truths about the internal state. */
    fprintf(file, "Current board memory address:     %#018lx\n",
            (uint64_t) get_board());
    fprintf(file, "Current board hardware address:   %u\n",
            get_board()->get_hardware_address()->as_uint());
    fprintf(file, "Current mailbox memory address:   %#018lx\n",
            (uint64_t) get_mailbox());
    fprintf(file, "Current mailbox hardware address: %u\n",
            get_mailbox()->get_hardware_address()->as_uint());
    fprintf(file, "Current core memory address:      %#018lx\n",
            (uint64_t) get_core());
    fprintf(file, "Current core hardware address:    %u\n",
            get_core()->get_hardware_address()->as_uint());
    fprintf(file, "Current thread memory address:    %#018lx\n",
            (uint64_t) get_thread());
    fprintf(file, "Current thread hardware address:  %u\n",
            get_thread()->get_hardware_address()->as_uint());

    /* Print whether or not each iterator has changed since the last time it
     * was polled. */
    std::vector<std::string> itemTypes;
    itemTypes.insert(itemTypes.end(), {"board", "mailbox", "core", "thread"});
    for (long unsigned typeIndex = 0; typeIndex < itemTypes.size();
         typeIndex++)
    {
        fprintf(file, "The %s has%s changed since "
                      "\"has_%s_changed\" was last called.\n",
                itemTypes[typeIndex].c_str(),
                itemChanged[typeIndex] ? "" : " not",
                itemTypes[typeIndex].c_str());
    }

    /* Print whether or not the iterator has wrapped since it was last
     * polled. */
    fprintf(file, "This iterator has%s wrapped.\n", isWrapped ? "" : " not");

    /* Close breaker and flush the dump. */
    HardwareDumpUtils::close_breaker(file, prefix);
    fflush(file);
}
