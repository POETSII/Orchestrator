//------------------------------------------------------------------------------

#include "Placement.h"
#include "OrchBase.h"
#include "P_core.h"
#include "P_super.h"
#include "P_devtyp.h"
#include "build_defs.h"
#include <algorithm>

//==============================================================================

Placement::Placement(OrchBase * _p):par(_p),pCon(0),pP_graph(0),pD_graph(0)
{
}

//------------------------------------------------------------------------------

Placement::~Placement()
{

}

//------------------------------------------------------------------------------

void Placement::DoLink()
{
}

//------------------------------------------------------------------------------

void Placement::Dump(FILE * fp)
{
fprintf(fp,"Placement+++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"Me,Parent      0x%#08p,0x%#08p\n",this,par);

fprintf(fp,"Placement-----------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

bool Placement::GetNext(P_thread *& prTh, P_stride stride)
// Routine to walk through the available threads....
{
prTh = *Nth;                           // preload the output data pointer

bool wrap = false;                                          // Flag shows when we reset to the start
Nth++;                                                      // Increment thread
if ((Nth==(*Nco)->P_threadv.end()) || (stride > thread)) {  // Thread off the end?
  Nco++;                                                    // Increment core
  if ((Nco==(*Nbd)->P_corev.end()) || (stride > core)) {    // Core off the end?
    Nbd++;                                                  // Increment board
    if ((Nbd==pPbo->P_boardv.end()) || (stride > board)) {  // Board off the end?
      pPbo = par->pP->G.NodeData(++Nbo);                    // Increment box
      if ((Nbo==par->pP->G.NodeEnd()) || (stride == box)) { // Box off the end?
        Nbo = par->pP->G.NodeBegin();                       // Reset the box iterator
        wrap = true;                                        // We've clocked the system....
      }                                                     // Reset the board pointer
      Nbd = par->pP->G.NodeData(Nbo)->P_boardv.begin();
    }
    Nco = (*Nbd)->P_corev.begin();     // Reset the core iterator
  }
  Nth = (*Nco)->P_threadv.begin();     // Reset the thread iterator
}
return wrap;
}

//------------------------------------------------------------------------------

void Placement::Init()
// Initialise 'current' pointers to the first thread/core/board/box
{
Nbo  = par->pP->G.NodeBegin();
pPbo = par->pP->G.NodeData(Nbo);
Nbd  = pPbo->P_boardv.begin();
Nco  = (*Nbd)->P_corev.begin();
Nth  = (*Nco)->P_threadv.begin();
}

//------------------------------------------------------------------------------

bool Placement::Place(P_task * pT)
// Place a task.
{
P_thread * pTh = 0;
 
WALKVECTOR(P_devtyp*,pT->pP_typdcl->P_devtypv,dT)
{
    vector<P_device*> dVs = pT->pD->DevicesOfType(*dT); // get all the devices of this type
    unsigned int devMem = (*dT)->MemPerDevice();
    if (devMem > BYTES_PER_THREAD)
    {
       // even a single device is too big to fit
       par->Post(810, (*dT)->Name(), int2str(devMem), int2str(BYTES_PER_THREAD));
       return true;
    }
    unsigned int devsPerThread = min(BYTES_PER_THREAD/devMem, MAX_DEVICES_PER_THREAD);
    for (unsigned devIdx = 0; devIdx < dVs.size(); devIdx++) // For each device.....
    {
        // if we have packed the existing thread,
        if (!(devIdx%devsPerThread))
        {
           // ...get a thread, checking to see that we don't run out of room
           // - i.e. that we increment past box space and this isn't the last
           // device to place
           if (GetNext(pTh) && !((devIdx == (dVs.size()-1)) && ((dT+1) == pT->pP_typdcl->P_devtypv.end())))
           {
              par->Post(163, pT->Name()); // out of room. Abandon placement.
              return true;
           }
        }
        Xlink(dVs[devIdx],pTh);            // And link thread and device
    }
    // jump to the next core: each core will only have one device type. We do not
    // need to jump if the devices exactly fit on an integral number of cores because in
    // that situation the previous GetNext() function will have incremented the core for us.
    if (dVs.size()%(devsPerThread*THREADS_PER_CORE))
    {
       if (((dT+1) != pT->pP_typdcl->P_devtypv.end()) && GetNext(pTh,Placement::core))
       {
          par->Post(163, pT->Name()); // out of room. Abandon placement.
          return true;
       }
    }
}
return false;
}


//------------------------------------------------------------------------------

void Placement::Xlink(P_device * pDe,P_thread * pTh)
// Actually link a real device to a real thread
{
printf("XLinking device %s to thread %s\n",
        pDe->FullName().c_str(),pTh->FullName().c_str());
fflush(stdout);
pDe->pP_thread = pTh;                  // Device to thread
pTh->P_devicel.push_back(pDe);         // Thread to device
pDe->addr |= pTh->addr;                // Complete P address structure
// The supervisor is already attached to the task; now it needs to be linked to
// the topology. I can't but think there's a cooler way......
pDe->par->par->pSup->Attach(pTh->par->par->par);

}

//==============================================================================



