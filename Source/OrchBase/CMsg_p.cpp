#include "CMsg_p.h"

CMsg_p::CMsg_p(MPI_Comm c):PMsg_p(c){}
CMsg_p::CMsg_p(byte * pb,int l,MPI_Comm c):PMsg_p(pb,l, c){}
CMsg_p::CMsg_p(CMsg_p & r):PMsg_p(r){}
CMsg_p::CMsg_p(PMsg_p & r):PMsg_p(r){}
CMsg_p::~CMsg_p(void){}


template <> void CMsg_p::Dump<pair<unsigned,P_addr_t> >(FILE * fp)
{

}

pair<unsigned,P_addr_t> * CMsg_p::Get(int & cnt)
{
    int numCores[7];
    unsigned* pCIdx = PMsg_p::Get<unsigned>(1,numCores[0]);
    unsigned* pBox = PMsg_p::Get<unsigned>(2,numCores[1]);
    unsigned* pBoard = PMsg_p::Get<unsigned>(3,numCores[2]);
    unsigned* pMailbox = PMsg_p::Get<unsigned>(3,numCores[3]);
    unsigned* pCore = PMsg_p::Get<unsigned>(4,numCores[4]);
    unsigned* pThread = PMsg_p::Get<unsigned>(5,numCores[5]);
    unsigned* pDevice = PMsg_p::Get<unsigned>(6,numCores[6]);
    if ((numCores[6] | numCores[5] | numCores[4] | numCores[3] | numCores[2] | numCores[1]) != numCores[0]) return 0;
    if (!pCIdx || !pBox || !pBoard || !pMailbox || !pCore || !pThread || !pDevice) return 0;
    cnt = numCores[0];
    pair<unsigned,P_addr_t>* pRetCores = new pair<unsigned,P_addr_t>[cnt];
    for (int core=0; core < cnt; core++)
    {
        pRetCores[core].first = pCIdx[core];
        pRetCores[core].second.A_box = pBox[core];
        pRetCores[core].second.A_board = pBoard[core];
        pRetCores[core].second.A_mailbox = pMailbox[core];
        pRetCores[core].second.A_core = pCore[core];
        pRetCores[core].second.A_thread = pThread[core];
        pRetCores[core].second.A_device = pDevice[core];
    }
    return pRetCores;
}

void CMsg_p::Get(vector<pair<unsigned,P_addr_t> > & vP)
{
    vP.clear();
    int len[7];
    unsigned* pCIdx = PMsg_p::Get<unsigned>(1,len[0]);
    unsigned* pBox = PMsg_p::Get<unsigned>(2,len[1]);
    unsigned* pBoard = PMsg_p::Get<unsigned>(3,len[2]);
    unsigned* pMailbox = PMsg_p::Get<unsigned>(3,len[3]);
    unsigned* pCore = PMsg_p::Get<unsigned>(4,len[4]);
    unsigned* pThread = PMsg_p::Get<unsigned>(5,len[5]);
    unsigned* pDevice = PMsg_p::Get<unsigned>(6,len[6]);
    if ((len[6] | len[5] | len[4] | len[3] | len[2] | len[1]) != len[0]);
    if (!pCIdx || !pBox || !pBoard || !pMailbox || !pCore || !pThread || !pDevice);
    for (int core = 0; core < len[0]; core++) vP.push_back(pair<unsigned,P_addr_t>(pCIdx[core],P_addr_t(pBox[core],pBoard[core],pMailbox[core],pCore[core],pThread[core],pDevice[core])));

}

void CMsg_p::Put(pair<unsigned,P_addr_t> * data,int cnt)
{
     unsigned* pCIdx = new unsigned[cnt];
     unsigned* pBox = new unsigned[cnt];
     unsigned* pBoard = new unsigned[cnt];
     unsigned* pMailbox = new unsigned[cnt];
     unsigned* pCore = new unsigned[cnt];
     unsigned* pThread = new unsigned[cnt];
     unsigned* pDevice = new unsigned[cnt];
     for (int core = 0; core < cnt; core++)
     {
         pCIdx[core] = data[core].first;
         pBox[core] = data[core].second.A_box;
         pBoard[core] = data[core].second.A_board;
         pMailbox[core] = data[core].second.A_mailbox;
         pCore[core] = data[core].second.A_core;
         pThread[core] = data[core].second.A_thread;
         pDevice[core] = data[core].second.A_device;
     }
     PMsg_p::Put(1,pCIdx,cnt);
     PMsg_p::Put(2,pBox,cnt);
     PMsg_p::Put(3,pBoard,cnt);
     PMsg_p::Put(4,pMailbox,cnt);
     PMsg_p::Put(5,pCore,cnt);
     PMsg_p::Put(6,pThread,cnt);
     PMsg_p::Put(7,pDevice,cnt);
     delete[] pCIdx;
     delete[] pBox;
     delete[] pBoard;
     delete[] pMailbox;
     delete[] pCore;
     delete[] pThread;
     delete[] pDevice;
}

void CMsg_p::Put(vector<pair<unsigned,P_addr_t> > * data)
{
     int cnt = data->size();
     unsigned* pCIdx = new unsigned[cnt];
     unsigned* pBox = new unsigned[cnt];
     unsigned* pBoard = new unsigned[cnt];
     unsigned* pMailbox = new unsigned[cnt];
     unsigned* pCore = new unsigned[cnt];
     unsigned* pThread = new unsigned[cnt];
     unsigned* pDevice = new unsigned[cnt];
     int idx = 0;
     for (vector<pair<unsigned,P_addr_t> >::iterator core = data->begin(); core != data->end(); core++)
     {
          pCIdx[idx] = core->first;
          pBox[idx] = core->second.A_box;
          pBoard[idx] = core->second.A_board;
          pMailbox[idx] = core->second.A_mailbox;
          pCore[idx] = core->second.A_core;
          pThread[idx] = core->second.A_thread;
          pDevice[idx++] = core->second.A_device;
     }
     PMsg_p::Put(1,pCIdx,cnt);
     PMsg_p::Put(2,pBox,cnt);
     PMsg_p::Put(3,pBoard,cnt);
     PMsg_p::Put(4,pMailbox,cnt);
     PMsg_p::Put(5,pCore,cnt);
     PMsg_p::Put(6,pThread,cnt);
     PMsg_p::Put(7,pDevice,cnt);
     delete[] pCIdx;
     delete[] pBox;
     delete[] pBoard;
     delete[] pMailbox;
     delete[] pCore;
     delete[] pThread;
     delete[] pDevice;
}
