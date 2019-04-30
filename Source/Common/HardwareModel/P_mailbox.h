#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_P_MAILBOX_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_P_MAILBOX_H

/* Describes the concept of a "POETS Mailbox" in the model of the POETS
 * hardware stack.
 *
 * The POETS Mailbox services cores, and is contained by an FPGA board on the
 * next level of the hierarchy.
 *
 * See the hardware model documentation for further information POETS
 * mailboxes. */

#include <sstream>

#include "AddressableItem.h"
#include "dumpchan.h"
#include "NameBase.h"
#include "OwnershipException.h"
#include "P_board.h"
#include "P_core.h"

#define MAXIMUM_BREAKER_LENGTH 80

/* Facilitate out-of-order includes. */
class P_board;
class P_core;

class P_mailbox: public AddressableItem, public NameBase, public DumpChan
{
public:
    P_mailbox(std::string name);

    /* Destruction */
    ~P_mailbox();
    void clear();

    /* Mailboxes live in boards; this is the board that this mailbox lives
     * in. */
    P_board* parent = NULL;
    void on_being_contained_hook(P_board* container);

    /* Mailboxes contain cores, mapped by their hardware address components. */
    std::map<AddressComponent, P_core*> P_corem;
    void contain(AddressComponent addressComponent, P_core* core);

    float costCoreCore;
    float costMailboxCore;

    void Dump(FILE* = stdout);
};

#endif
