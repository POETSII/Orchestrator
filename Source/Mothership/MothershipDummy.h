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
   unsigned              OnCfg(PMsg_p *,unsigned);
   unsigned              OnCmnd(PMsg_p *,unsigned);
   unsigned              OnDump(PMsg_p *,unsigned);
   unsigned              OnName(PMsg_p *,unsigned);
   unsigned              OnReply(PMsg_p *, unsigned);
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
   unsigned              ReplyAttrs(PMsg_p*, unsigned);
   unsigned              ReplyDevice(PMsg_p*, unsigned);
   unsigned              ReplyDevices(PMsg_p*, unsigned);
   unsigned              ReplyDevSuper(PMsg_p*, unsigned);
   unsigned              ReplyDevTypes(PMsg_p*, unsigned);
   unsigned              ReplyList(PMsg_p*, unsigned);
   unsigned              ReplyNotFound(PMsg_p*, unsigned);
   unsigned              ReplySupers(PMsg_p*, unsigned);
   unsigned              ReplyTask(PMsg_p*, unsigned);

   static void           MapValue2Key(unsigned value, vector<byte>* key);     
   string                QueryType(unsigned);

   typedef unsigned    (MothershipDummy::*pMeth)(PMsg_p *, unsigned);
   typedef map<unsigned,pMeth> FnMap_t;
   vector<FnMap_t*> FnMapx;
   
   map<int,unsigned>     qryMap;
};

#endif
