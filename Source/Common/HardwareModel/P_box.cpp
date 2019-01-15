/* Defines POETS Box behaviour (see the accompanying header for further
   information). */

#include "P_box.h"

/* Constructs a POETS Box. Arguments:
   - name: Name of this box object (see namebase)
*/
P_box::P_box(std::string name)
{
    Name(name);
}

P_box::~P_box(){clear();}

/* Clears the dynamically-allocated elements of the data structure of this
   box, deleting all contained components recursively.
*/
void P_box::clear()
{
    /* Clear all boards that this box knows about. This should clear
       recursively. */
    WALKMAP(AddressComponent,P_board*,P_boards,iterator)
    {
        delete iterator->second;
    }
    P_boards.clear();
}

/* Donates an uncontained board to this box. Arguments:

   - addressComponent: Used to index the board in this box.
   - board: Pointer to the board object to contain. Must not already have a
     parent.
*/
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

    P_boards[addressComponent] = board;
}

/* Write debug and diagnostic information about the POETS box, recursively,
   using dumpchan. Arguments:

   - file: File to dump to.
*/
void P_box::dump(FILE* file)
{
    std::string fullName = FullName();  /* Name of this from namebase. */

    /* About this object and its parent, if any. */
    char breaker[MAXIMUM_BREAKER_LENGTH + 1];
    int breakerLength = sprintf(breaker, "P_box %s ", fullName.c_str());
    for(int index=breakerLength; index<MAXIMUM_BREAKER_LENGTH - 1;
        breaker[index++]='+');
    breaker[MAXIMUM_BREAKER_LENGTH - 1] = '\n';
    breaker[MAXIMUM_BREAKER_LENGTH] = '\0';
    fprintf(file, "%s", breaker);
    NameBase::Dump(file);

    /* About contained items, if any. */
    fprintf(file, "Boards in this box %s\n", std::string(60, '+').c_str());
    if (P_boards.empty())
        fprintf(file, "The board map is empty.\n");
    else
    {
        WALKMAP(AddressComponent,P_board*,P_boards,iterator)
        {
            P_board* iterBoard = iterator->second;
            /* Print information from the map. */
            fprintf(file, "%u: %s (%p)\n",
                    iterator->first,
                    iterBoard->FullName().c_str(), iterBoard);
            /* Recursive-dump. */
            iterBoard->dump();
        }
    }
    fprintf(file, "Boards in this box %s\n", std::string(60, '-').c_str());

    /* Close breaker and flush the dump. */
    for(int index=breakerLength; index<MAXIMUM_BREAKER_LENGTH - 1;
        breaker[index++]='-');
    fprintf(file, "%s", breaker);
    fflush(file);
}

/* Hook that a container calls to contain this object. Arguments:
   - container: Address of the engine that contains this box.
*/
void P_box::on_being_contained_hook(P_engine* container)
{
    parent = container;
    Npar(container);
}
