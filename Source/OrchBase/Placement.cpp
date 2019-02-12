//------------------------------------------------------------------------------

#include "Placement.h"
#include "Constraints.h"
#include "OrchBase.h"
#include "P_core.h"
#include "P_super.h"
#include "P_devtyp.h"
#include "build_defs.h"
#include <algorithm>

//==============================================================================

Placement::Placement(OrchBase * _p):par(_p),pCon(0),pD_graph(0)
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
// Gets the next thread in the engine, and sets it to prTh. Returns true if a
// new thread could not be found (i.e. we wrapped around). If we wrapped
// around, set all of the iterators, and prTh, to their starting value.
{
    bool didWeWrap = false;

    // Increment the thread, then check to see if we've run out of threads on
    // the current core.
    threadIterator++;
    if (threadIterator == coreIterator->second->P_threadm.end())
    {
        // Increment the core, then check to see if we've run out of cores on
        // the current mailbox.
        coreIterator++;
        if (coreIterator == mailboxIterator->second.data->P_corem.end())
        {
            // Increment the mailbox, then check to see if we've run out of
            // mailboxes on the current board.
            mailboxIterator++;
            if (mailboxIterator ==
                boardIterator->second.data->G.NodeEnd())
            {
                // Increment the board, then check to see if we've run out of
                // boards in the current engine.
                if (boardIterator == par->pE->G.NodeEnd())
                {
                    // Out of boards, we're done here.
                    boardIterator = par->pE->G.NodeBegin();
                    didWeWrap = true;
                }
                // Now we have a new board, set the mailbox iterator to point
                // to the first mailbox in that board.
                mailboxIterator =
                    boardIterator->second.data->G.NodeBegin();
            }

            // Now we have a new mailbox, set the core iterator to point to the
            // first core in that mailbox.
            coreIterator = mailboxIterator->second.data->P_corem.end();
        }

        // Now we have a new core, set the thread iterator to point to the
        // first thread on that core.
        threadIterator = coreIterator->second->P_threadm.begin();
    }

    // Set thread pointer and return.
    prTh = threadIterator->second;
    return didWeWrap;
}

//------------------------------------------------------------------------------

void Placement::Init()
// Initialise 'current' pointers to the first thread/core/mailbox/board
{
    boardIterator = par->pE->G.NodeBegin();
    mailboxIterator = boardIterator->second.data->G.NodeBegin();
    coreIterator = mailboxIterator->second.data->P_corem.begin();
    threadIterator = coreIterator->second->P_threadm.begin();
}

//------------------------------------------------------------------------------

bool Placement::Place(P_task * pT)
// Place a task.
{
P_thread * pTh = 0;

WALKVECTOR(P_devtyp*,pT->pP_typdcl->P_devtypv,dT)
{
    if ((*dT)->pOnRTS) // don't need to place if it's a supervisor - easily identified by lack of RTS handler
    {
    vector<P_device*> dVs = pT->pD->DevicesOfType(*dT); // get all the devices of this type
    unsigned int devMem = (*dT)->MemPerDevice();
    if (devMem > BYTES_PER_THREAD)
    {
       // even a single device is too big to fit
       par->Post(810, (*dT)->Name(), int2str(devMem), int2str(BYTES_PER_THREAD));
       return true;
    }
    // place according to constraints found
    if (!pCon) pCon = new Constraints();
    if (pCon->Constraintm.find("DevicesPerThread") == pCon->Constraintm.end()) pCon->Constraintm["DevicesPerThread"] = min(BYTES_PER_THREAD/devMem, MAX_DEVICES_PER_THREAD);
    if (pCon->Constraintm.find("ThreadsPerCore") == pCon->Constraintm.end()) pCon->Constraintm["ThreadsPerCore"] = THREADS_PER_CORE;
    for (unsigned devIdx = 0; devIdx < dVs.size(); devIdx++) // For each device.....
    {
        // if we have packed the existing thread,
        if (!(devIdx%pCon->Constraintm["DevicesPerThread"]))
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
        dVs[devIdx]->addr.SetDevice(devIdx%pCon->Constraintm["DevicesPerThread"]); // insert the device's internal address (thread index)
        Xlink(dVs[devIdx],pTh);            // And link thread and device
    }

    // jump to the next core: each core will only have one device type. We do not
    // need to jump if the devices exactly fit on an integral number of cores because in
    // that situation the previous GetNext() function will have incremented the core for us.
    if (dVs.size()%(pCon->Constraintm["DevicesPerThread"]*pCon->Constraintm["ThreadsPerCore"]))
    {
       if (((dT+1) != pT->pP_typdcl->P_devtypv.end()) && GetNext(pTh,Placement::core))
       {
          par->Post(163, pT->Name()); // out of room. Abandon placement.
          return true;
       }
    }
    // current tinsel architecture shares I-memory between pairs of cores, so
    // for a new device type, if the postincremented core number is odd, we
    // need to increment again to get an even boundary.
    if ((coreIterator->first & 0x1) && GetNext(pTh,Placement::core))
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
printf("XLinking device %s with id %d to thread %s\n",
        pDe->FullName().c_str(), pDe->addr.A_device, pTh->FullName().c_str());
fflush(stdout);
pDe->pP_thread = pTh;                  // Device to thread
pTh->P_devicel.push_back(pDe);         // Thread to device
// The supervisor is already attached to the task; now it needs to be linked to
// the topology. I can't but think there's a cooler way......
pDe->par->par->pSup->Attach(pTh->parent->parent->parent);
}

//==============================================================================
