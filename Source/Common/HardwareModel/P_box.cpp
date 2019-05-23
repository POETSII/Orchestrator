/* Defines POETS Box behaviour (see the accompanying header for further
 * information). */

#include "P_box.h"

/* Constructs a POETS Box. Arguments:
 *
 * - name: Name of this box object (see namebase) */
P_box::P_box(std::string name)
{
    Name(name);
}

P_box::~P_box(){clear();}

/* Clears the dynamically-allocated elements of the data structure of this box,
 * deleting all contained components recursively. NB: Does not delete
 * supervisors; this is handled by D_graph. */
void P_box::clear()
{
    /* Clear all boards that this box knows about. This should clear
     * recursively. */
    WALKVECTOR(P_board*,P_boardv,iterator){delete *iterator;}
    P_boardv.clear();
}

/* Donates an uncontained board to this box. Arguments:
 *
 * - addressComponent: Used to index the board in this box.
 * - board: Pointer to the board object to contain. Must not already have a
 *   parent. */
void P_box::contain(AddressComponent addressComponent, P_board* board)
{
    /* Verify that the board is unowned. */
    if (board->parent != NULL)
    {
        std::stringstream errorMessage;
        errorMessage << "Board \"" << board->Name()
                     << "\" is already owned by box \""
                     << board->parent->Name()
                     << "\". The board's full name is \"" << board->FullName()
                     << "\".";
        throw OwnershipException(errorMessage.str());
    }

    /* Claim it. */
    board->on_being_contained_hook(this);

    /* Verify that the board is now owned by us. */
    if (board->parent != this)
    {
        std::stringstream errorMessage;
        errorMessage << "Board \"" << board->Name()
                     << "\" not owned by this box (" << Name()
                     << ") after being claimed. The board's full name is \""
                     << board->FullName() << "\".";
        throw OwnershipException(errorMessage.str());
    }

    P_boardv.push_back(board);

    /* Define a hardware address for the board, if we have a hardware
     * address. */
    if (isAddressBound)
    {
        HardwareAddress* boardHardwareAddress = copy_hardware_address();
        boardHardwareAddress->set_board(addressComponent);
        board->set_hardware_address(boardHardwareAddress);
    }
}

/* Write debug and diagnostic information about the POETS box, recursively,
 * using dumpchan. Arguments:
 *
 * - file: File to dump to. */
void P_box::Dump(FILE* file)
{
    std::string prefix = dformat("P_box %s", FullName().c_str());
    HardwareDumpUtils::open_breaker(file, prefix);

    /* About this object and its parent, if any. */
    NameBase::Dump(file);

    /* About contained items, if any. */
    HardwareDumpUtils::open_breaker(file, "Boards in this box");
    if (P_boardv.empty()){fprintf(file, "The board vector is empty.\n");}
    else
    {
        WALKVECTOR(P_board*, P_boardv, iterator)
        {
            /* Recursive-dump. */
            (*iterator)->Dump(file);
        }
    }
    HardwareDumpUtils::close_breaker(file, "Boards in this box");

    /* Close breaker and flush the dump. */
    HardwareDumpUtils::close_breaker(file, prefix);
    fflush(file);
}

/* Hook that a container calls to contain this object. Arguments:
 *
 * - container: Address of the engine that contains this box. */
void P_box::on_being_contained_hook(P_engine* container)
{
    parent = container;
    Npar(container);
}
