#ifndef __MothershipDummy_H__
#define __MothershipDummy_H__

#include "SBase.h"
#include "PMsg_p.hpp"
#include <string>

class MothershipDummy : public SBase
{
   public:
             MothershipDummy(int,char **,string);
   virtual  ~MothershipDummy();

   unsigned              Connect(string="");

   private:
   
   unsigned              CmLoad(string);
   unsigned              CmRun(string);
   unsigned              CmStop(string);
   unsigned              OnCfg(PMsg_p *, unsigned);
   unsigned              OnCmnd(PMsg_p *,unsigned);
   unsigned              OnDump(PMsg_p *, unsigned);
   unsigned              OnName(PMsg_p *,unsigned);
   unsigned              OnSuper(PMsg_p *,unsigned);
   unsigned              OnSyst(PMsg_p *,unsigned);
   unsigned              SystHW(const vector<string>&);
   unsigned              SystKill();
   unsigned              SystShow();
   unsigned              SystTopo();

   #include              "SDecode.cpp"
   void                  Dump(FILE * = stdout, string = "");

   // unsigned              ConfigDir(PMsg_p *, unsigned);
   unsigned              ConfigDistribute(PMsg_p *, unsigned);
   unsigned              ConfigRecall(PMsg_p *, unsigned);
   unsigned              ConfigState(PMsg_p *, unsigned);
   unsigned              DumpAll(PMsg_p *, unsigned);
   unsigned              DumpSummary(PMsg_p *, unsigned);
   unsigned              DumpTask(PMsg_p *, unsigned);

   typedef unsigned    (MothershipDummy::*pMeth)(PMsg_p *, unsigned);
   typedef map<unsigned,pMeth> FnMap_t;
   vector<FnMap_t*> FnMapx;
   
};

#endif
