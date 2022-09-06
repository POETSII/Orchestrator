
/* The reason for the rather bizarre way of defining this method - via the
#include in the .h file - is because it's part of several derived classes, and
FnMapx and this are different in each derived class, and I can't find a way
of doing this without breaking syntax. Rather, I can and have, and this is it,
but I really, really don't like it.

Perfection is the enemy of progress, and all that. This works.

DON'T PUT ANY #include IN THIS FILE

It's virtual here because this makes it an invalid translation unit, so if you
try to compile it, the compiler will squeak.
*/

//==============================================================================

virtual unsigned Decode(PMsg_p * pPkt)//, unsigned cIdx)
// Routine to decode - that is, send to the correct handler - incoming messages
// Look at the message key, and use the map to call the appropriate (derived
// class) member. If it's not in the derived class map, try the base class map.
// If it's not there either, it gets junked.
{
                                       // Handler in the derived class?
if (FnMap.find(pPkt->Key())!=FnMap.end()) {
  return (this->*FnMap[pPkt->Key()])(pPkt);
}
                                       // Nope. Base class?
if (CommonBase::FnMap.find(pPkt->Key())!=CommonBase::FnMap.end()) {
  return (this->*CommonBase::FnMap[pPkt->Key()])(pPkt);//,cIdx);
}
                                       // Nope. Kick.
                                       // Pull out the unknown key and post what
                                       // little we know to the LogServer
Post(101,Sderived,int2str(pPkt->Src()),pPmap->vPmap[pPkt->Src()].P_class,int2str(pPkt->Tgt()),
     pPmap->vPmap[pPkt->Tgt()].P_class,hex2str(pPkt->Key()));
return 0;                              // Return "keep going" value
}

//==============================================================================



