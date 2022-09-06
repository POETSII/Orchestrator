//------------------------------------------------------------------------------

#include "P_super.h"
#include "P_link.h"
#include "P_port.h"

//==============================================================================

P_super::P_super():DevI_t()
{

}

//------------------------------------------------------------------------------

P_super::P_super(string s):DevI_t(0,s)
{

}

//------------------------------------------------------------------------------

P_super::~P_super()
{

}

//------------------------------------------------------------------------------

void P_super::Attach(P_board* targetBoard)
// Attaches this supervisor to a given board, and to the box it contains if the
// box doesn't already have it (e.g. from another board).
{
    // Verify this supervisor is not already contained in the target board, by
    // checking P_super's memory.
    WALKVECTOR(P_board*, P_boardv, boardIterator)
    {
        if (*boardIterator == targetBoard) {return;}
    }

    // It's safe to attach now. Attachment has three steps:
    //
    //  1. Track the target board in this (P_super) object.
    //
    //  2. Store the supervisor in the box, if it's not already in the
    //     box. Either way, get the index of the supervisor in the box.
    //
    //  3. Store the index of the supervisor in the box, in the board (so that
    //     the board can obtain its supervisor by querying the box it's
    //     contained in.

    // Add this board to P_super's memory.
    P_boardv.push_back(targetBoard);

    // Get the index of this supervisor in the box that contains this board.
    P_box* parentBox = targetBoard->parent;
    std::vector<P_super*>::iterator supervisorIterator;
    supervisorIterator = std::find(parentBox->P_superv.begin(),
                                   parentBox->P_superv.end(), this);
    unsigned supervisorIndex;
    // We found it! Grab the index.
    if (supervisorIterator != parentBox->P_superv.end())
    {
        supervisorIndex = supervisorIterator - parentBox->P_superv.begin();
    }
    // We didn't find it... so add it and save the index (which is the last
    // element, thanks to push_back).
    else
    {
        parentBox->P_superv.push_back(this);
        supervisorIndex = parentBox->P_superv.size() - 1;
    }

    // Store the index in the board, so the board can access the supervisor by
    // querying its parent box.
    targetBoard->sup_offv.push_back(supervisorIndex);
}

//------------------------------------------------------------------------------

void P_super::Detach(P_board* targetBoard)
// Remove this supervisor from a given board, if it is attached to that board.
{
    // Get the index of this supervisor in the box that contains this board.
    P_box* parentBox = targetBoard->parent;
    std::vector<P_super*>::iterator supervisorIterator;
    supervisorIterator = std::find(parentBox->P_superv.begin(),
                                   parentBox->P_superv.end(), this);

    // If we didn't find it, it's already been cleared. Let's not worry then.
    if (supervisorIterator == parentBox->P_superv.end()){return;}

    // Remove the supervisor from the board, by its index in the box. NB: Can't
    // use std::remove because (in Borland), it clashes with cstdio's remove
    // for some mysterious reason. Note that this replacement is woefully
    // inefficient, but sup_offv is small anyway. Here's the equivalent
    // std::remove line.
    //
    // std::remove(targetBoard->sup_offv.begin(), targetBoard->sup_offv.end(),
    //             superIndex);
    unsigned superIndex = supervisorIterator - parentBox->P_superv.begin();
    bool removed;
    do
    {
        removed = false;
        WALKVECTOR(unsigned, targetBoard->sup_offv, indexIterator)
        {
            if (*indexIterator == superIndex)
            {
                targetBoard->sup_offv.erase(indexIterator);
                removed = true;
                break;  // Iterator now invalid - start again.
            }
        }
    } while (!removed);  // Removes all instances of superIndex, if many exist.

    // Remove the board from this P_super object.
    WALKVECTOR(P_board*, P_boardv, boardIterator)
    {
        if ((*boardIterator) == targetBoard) {
            P_boardv.erase(boardIterator);
            // We've got to go now, because the iterator is now invalid. This
            // is fine, because a board is only stored once in a P_super.
            return;
        }
    }
}

//------------------------------------------------------------------------------

void P_super::Detach()
// Removes this supervisor from all boards, if this supervisor is attached to a
// board.
{
    // This is done by going through each of the boards tracked in this P_super
    // object, finding the index for supervisor in it's contained box, and then
    // removing the entry in the P_board level.
    //
    // Fortunately, Detach does this for one board, so we iterate through each
    // of the boards that we know we're attached to. It doesn't matter if a box
    // contains multiple boards that we're attached to, because Detach is
    // idempotent.
    WALKVECTOR(P_board*, P_boardv, boardIterator){Detach(*boardIterator);}

    // Clear the record in P_super, since we must be empty by this point.
    P_boardv.clear();
}

//------------------------------------------------------------------------------

void P_super::Dump(unsigned off,FILE * fp)
{
string s(off,' ');
const char * os = s.c_str();
fprintf(fp,"\n%sP_super+++++++++++++++++++++++++++++++++\n",os);
fprintf(fp,"%sSupervisor attached to :\n",os);
WALKVECTOR(P_board*, P_boardv, i)
  fprintf(fp,"%s%s\n",os,(*i)->FullName().c_str());
DevI_t::Dump(off+2,fp);
fprintf(fp,"%sP_super---------------------------------\n\n",os);
fflush(fp);
}

//==============================================================================
