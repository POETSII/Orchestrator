#ifndef __SBASE_H__
#define __SBASE_H__

#include "CommonBase.h"
#include "AddressBook.hpp"
using namespace AddressBookNS; 

//==============================================================================

class SBase : public CommonBase, public AddressBook
{

public:
                    SBase(int,char **,string,string);
virtual ~           SBase();

inline virtual void Dump(FILE * f = stdout, string s = "") {AddressBook::Dump(f, s);};

unsigned            OnCfg   (PMsg_p *, unsigned);
unsigned            OnCmd   (PMsg_p *, unsigned);
unsigned            OnData  (PMsg_p *, unsigned);
unsigned            OnDump  (PMsg_p *, unsigned);
unsigned            OnQuery (PMsg_p *, unsigned); 
unsigned            OnReply (PMsg_p *, unsigned);
unsigned            OnSend  (PMsg_p *, unsigned);

static string       State2Str (TaskState_t);
static TaskState_t  Str2State (const string&);

protected:

virtual unsigned    Connect          (string="");

virtual unsigned    ConfigBuild      (PMsg_p *, unsigned);
virtual unsigned    ConfigDelete     (PMsg_p *, unsigned);
virtual unsigned    ConfigDir        (PMsg_p *, unsigned);
virtual unsigned    ConfigDistribute (PMsg_p *, unsigned);
virtual unsigned    ConfigRecall     (PMsg_p *, unsigned);
virtual unsigned    ConfigState      (PMsg_p *, unsigned);
virtual unsigned    DumpAll          (PMsg_p *, unsigned);
virtual unsigned    DumpSummary      (PMsg_p *, unsigned);
virtual unsigned    DumpTask         (PMsg_p *, unsigned);
virtual unsigned    QueryDevIAll     (PMsg_p *, unsigned);
virtual unsigned    QueryDevIByName  (PMsg_p *, unsigned);
virtual unsigned    QueryDevIByID    (PMsg_p *, unsigned);
virtual unsigned    QuerySupv        (PMsg_p *, unsigned);
virtual unsigned    SendAttr         (PMsg_p *, unsigned);
virtual unsigned    SendDevIAll      (PMsg_p *, unsigned);
virtual unsigned    SendDevIByName   (PMsg_p *, unsigned);
virtual unsigned    SendDevIByID     (PMsg_p *, unsigned);
virtual unsigned    SendDevT         (PMsg_p *, unsigned);
virtual unsigned    SendExtn         (PMsg_p *, unsigned);
virtual unsigned    SendSupv         (PMsg_p *, unsigned);

typedef unsigned    (SBase::*pMeth)(PMsg_p*, unsigned);
typedef map<unsigned,pMeth> FnMap_t;
vector<FnMap_t*>    FnMapx;
 
private:

unsigned            CmdIntegrity       (PMsg_p *); 
unsigned            DataTask           (PMsg_p *);
unsigned            DataDevType        (PMsg_p *);
unsigned            DataDevice         (PMsg_p *);
unsigned            DataDeviceExternal (PMsg_p *);
unsigned            DataExternal       (PMsg_p *);
unsigned            DataSupervisor     (PMsg_p *);
unsigned            QueryAttr          (PMsg_p *, unsigned);
unsigned            QueryDevT          (PMsg_p *, unsigned);
unsigned            QueryExtn          (PMsg_p *, unsigned);
unsigned            QueryList          (PMsg_p *, unsigned);
unsigned            QueryTask          (PMsg_p *, unsigned);

};

//==============================================================================

#endif

