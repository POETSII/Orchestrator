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

void CmMoni::Cm_Inj1()
// Sends a spoof MONI:INJE:REQ message to the MonServer, as though the monitor
// sent it to inject a command into Root. Mostly for testing...
{
    PMsg_p message;
    message.Key(Q::MONI, Q::INJE, Q::REQ);
    message.Src(par->Urank);  // We're not monsters
    message.Tgt(par->pPmap->U.MonServer);

    message.Zname(3, "Inj1_test");  // Arbitrary acknowledgement string

    // Command to inject into the Orchestrator
    std::string command =
        (std::string("test /echo = \"From monitor /inj1\" ") +
         "// Spoof message produced by monitor /inj1.");
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

unsigned CmMoni::operator()(Cli * pC)
// Handle "moni" command from the monkey.
{
//printf("CmMoni operator() splitter for ....\n");
//fflush(stdout);
if (pC==0) return 0;                   // Paranoia
WALKVECTOR(Cli::Cl_t,pC->Cl_v,i) {     // Walk the clause list
  string sCl = (*i).Cl;                // Pull out clause name
  if (strcmp(sCl.c_str(),"inj1")==0) { Cm_Inj1(); continue; }
  if (strcmp(sCl.c_str(),"spy" )==0) { Cm_Spy();  continue; }
  par->Post(25,sCl,"moni");            // Unrecognised clause
}
return 0;                              // Legitimate command exit
}

//==============================================================================
