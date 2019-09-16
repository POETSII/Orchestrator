#ifndef __SBASE_H__
#define __SBASE_H__

#include "CommonBase.h"
#include "AddressBook.hpp"
using namespace AddressBookNS; 

//==============================================================================
/*

SBase is the common abstraction layer to provide name services to the 
Orchestrator. Its main function is to service Q::NAME messages coming from 
other processes in the system. The Q::NAME message space is itself 
divided into Q::SEND (forward to devices), Q::QRY (ask name services for device
into), Q::RPLY (send a response to a query to some other process), Q::DATA 
(load NameServer data into the database), Q::CFG (configure Motherships at 
startup of a task), Q::CMDC (miscellaneous low-level control functions) and
Q::DUMP (output NameServer data to a file or on-screen)

SBase is in fact a wrapper for 2 classes, CommonBase (containing the MPI 
messaging layer and all its associated machinery) and AddressBook (which 
provides the Name database and query functions. As such it derives from 
both classes. Essentially all the methods of SBase deal with responding
to queries or building the database.
*/
//------------------------------------------------------------------------------
class SBase : public CommonBase, public AddressBook
{

public:
                    SBase(int,char **,string,string);
virtual ~           SBase();

inline virtual void Dump(FILE * f = stdout, string s = "") {AddressBook::Dump(f, s);};

// top-level message dispatchers that handle the main subtypes of message
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

// overloaded Connect for multi-universe support
virtual unsigned    Connect          (string=""); 

// subcommands to set up Motherships
virtual unsigned    ConfigBuild      (PMsg_p *, unsigned);
virtual unsigned    ConfigDelete     (PMsg_p *, unsigned);
virtual unsigned    ConfigDir        (PMsg_p *, unsigned);
virtual unsigned    ConfigDistribute (PMsg_p *, unsigned);
virtual unsigned    ConfigRecall     (PMsg_p *, unsigned);
virtual unsigned    ConfigState      (PMsg_p *, unsigned);
// various types of dumps
virtual unsigned    DumpAll          (PMsg_p *, unsigned);
virtual unsigned    DumpSummary      (PMsg_p *, unsigned);
virtual unsigned    DumpTask         (PMsg_p *, unsigned);
// queries, organised by general type (device, supervisor)
virtual unsigned    QueryDevIAll     (PMsg_p *, unsigned);
virtual unsigned    QueryDevIByName  (PMsg_p *, unsigned);
virtual unsigned    QueryDevIByID    (PMsg_p *, unsigned);
virtual unsigned    QuerySupv        (PMsg_p *, unsigned);
// forwarded messages containing POETS packets. Most of 
// these should do nothing in SBase because the behaviour
// will depend upon the process class
virtual unsigned    SendAttr         (PMsg_p *, unsigned);
virtual unsigned    SendDevIAll      (PMsg_p *, unsigned);
virtual unsigned    SendDevIByName   (PMsg_p *, unsigned);
virtual unsigned    SendDevIByID     (PMsg_p *, unsigned);
virtual unsigned    SendDevT         (PMsg_p *, unsigned);
virtual unsigned    SendExtn         (PMsg_p *, unsigned);
virtual unsigned    SendSupv         (PMsg_p *, unsigned);

// structures needed for CommonBase to register callbacks
typedef unsigned    (SBase::*pMeth)(PMsg_p*, unsigned);
typedef map<unsigned,pMeth> FnMap_t;
vector<FnMap_t*>    FnMapx;
 
private:

// low-level commands - usually called from a subcommand, that
// handle the nuts and bolts of the operations.
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

