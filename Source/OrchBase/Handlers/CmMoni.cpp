//------------------------------------------------------------------------------

#include "CmMoni.h"
#include "OrchBase.h"
#include "Pglobals.h"

//==============================================================================

CmMoni::CmMoni(OrchBase * p):par(p)
{
par->fd = stdout;
}

//------------------------------------------------------------------------------

CmMoni::~CmMoni()
{
}

//------------------------------------------------------------------------------

void CmMoni::Dump(FILE * fp)
{
fprintf(fp,"CmMoni+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
if (par==0) fprintf(fp,"OrchBase parent not defined\n");
else fprintf(fp,"Orchbase parent : %s\n",par->FullName().c_str());
fprintf(fp,"CmMoni-------------------------------------------------------\n\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void CmMoni::Show(FILE * fp)
{
fprintf(fp,"\nMoni attributes and state:\n");
fprintf(fp,"\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void CmMoni::Cm_Mdr1()
// Sends a spoof MONI:DEVI:REQ message to the MonServer, as though the monitor
// sent it to request data from the Orchestrator proper. Mostly for testing...
{
    PMsg_p message;
    message.Key(Q::MONI, Q::DEVI, Q::REQ);
    message.Src(par->Urank);  // We're not monsters
    message.Tgt(par->pPmap->U.MonServer);

    message.Mode(1);  // We're pretending to be a Monitor, remember

    message.Zname(0, "ring_test::ring_test_instance");  // Application name.
    message.Zname(1, "0");  // There is a device called '0', imagine.
    message.Zname(2, "Mdr1_test");  // Arbitrary acknowledgement string
    message.Zname(3, "octopus.txt");  // Some file name

    // Configuration
    unsigned unsignedData;
    unsignedData = 1000;
    message.Put<unsigned>(0, &unsignedData, 1);  // Update interval (ms)
    unsignedData = 1;
    message.Put<unsigned>(1, &unsignedData, 1);  // Data type (no idea)
    unsignedData = 2;
    message.Put<unsigned>(4, &unsignedData, 1);  // Data source (Mothership)

    bool boolData;
    boolData = true;
    message.Put<bool>(0, &boolData, 1);  // Start exfiltrating.
    message.Put<bool>(2, &boolData, 1);  // Monitor must write logs.
    message.Put<bool>(3, &boolData, 1);  // Remote must write logs.
    message.Put<bool>(5, &boolData, 1);  // Remote must clobber logs.
    boolData = false;
    message.Put<bool>(4, &boolData, 1);  // Monitor must append to logs.

    // Housekeeping fields
    int data1 = 1;
    int data2;
    void* voidData1 = &data1;
    void* voidData2 = &data2;
    message.Put<void*>(98, &voidData1, 1);
    message.Put<void*>(99, &voidData2, 1);
    message.Put<int>(99, &data1, 1);

    // Timestamp (pretending we are the monitor)
    double timestamp = MPI_Wtime();
    message.Put<double>(-10, &timestamp);

    message.Send();  // And away we go
}

//------------------------------------------------------------------------------

void CmMoni::Cm_Mir1()
// Sends a spoof MONI:INJE:REQ message to the MonServer, as though the monitor
// sent it to inject a command into Root. Mostly for testing...
{
    PMsg_p message;
    message.Key(Q::MONI, Q::INJE, Q::REQ);
    message.Src(par->Urank);  // We're not monsters
    message.Tgt(par->pPmap->U.MonServer);

    message.Zname(3, "Mir1_test");  // Arbitrary acknowledgement string

    // Command to inject into the Orchestrator
    std::string command =
        (std::string("test /echo = \"From monitor /mir1\" ") +
         "// Spoof message produced by monitor /mir1.");
    message.Put<char>(1, &command[0], command.length() + 1);

    // Housekeeping fields
    int data1 = 1;
    int data2;
    void* voidData1 = &data1;
    void* voidData2 = &data2;
    message.Put<void*>(98, &voidData1, 1);
    message.Put<void*>(99, &voidData2, 1);
    message.Put<int>(99, &data1, 1);

    // Timestamp (pretending we are the monitor)
    double timestamp = MPI_Wtime();
    message.Put<double>(-10, &timestamp);

    message.Send();  // And away we go
}

//------------------------------------------------------------------------------

void CmMoni::Cm_Spy()
// Sends a message to the MonServer, commanding it to toggle its Spy. This also
// sends the path for the spy to write to, to the MonServer.
{
    PMsg_p message;
    message.Key(Q::MONI, Q::SPY);
    message.Src(par->Urank);
    message.Tgt(par->pPmap->U.MonServer);
    message.Put(0, &par->pCmPath->pathMonS);
    message.Send();
}

//------------------------------------------------------------------------------

void CmMoni::Cm_Trac()
// Sends a message to the MonServer, commanding it to toggle whether or not it
// tracks and records data from incoming data packets. This also sends the path
// for data to be written to, to the MonServer.
{
    PMsg_p message;
    message.Key(Q::MONI, Q::TRAC);
    message.Src(par->Urank);
    message.Tgt(par->pPmap->U.MonServer);
    message.Put(0, &par->pCmPath->pathMonD);
    message.Send();
}

//------------------------------------------------------------------------------

unsigned CmMoni::operator()(Cli * pC)
// Handle "moni" command from the monkey.
{
//printf("CmMoni operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
  if (strcmp(sCl.c_str(),"mdr1")==0) { Cm_Mdr1(); continue; }
  if (strcmp(sCl.c_str(),"mir1")==0) { Cm_Mir1(); continue; }
  if (strcmp(sCl.c_str(),"spy" )==0) { Cm_Spy();  continue; }
  if (strcmp(sCl.c_str(),"trac" )==0){ Cm_Trac(); continue; }  // Track
  par->Post(25,sCl,"moni");            // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================
