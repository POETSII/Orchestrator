//------------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Monitor.h"
#include "About.h"
#include "Wizard.h"
#include "Misc.h"
#include "flat.h"
#include "Pclient_t.h"
#include "Pglobals.h"
#include "Cli.h"

#include "Winsock.h"

//------------------------------------------------------------------------------

#pragma resource "*.dfm"
TMonitorForm * MonitorForm;            // Give me strength

//==============================================================================

void MonMCB(void * pSkyHook,string message)
{
Cprintf("%s\n",message.c_str());
TMonitorForm * pMoni = (TMonitorForm *)pSkyHook;
pMoni->Memo1->Lines->Add(STL2VCL(message));
}

//------------------------------------------------------------------------------

void MonPCB(int socket,void * _pSky,vector<byte> buffer,int len)
// Client flat callback function
{
                                       // Hook back into Monitor object
TMonitorForm * pMoni = (TMonitorForm *)_pSky;
                                       // Turn payload into a message
Msg_p * pZ = new Msg_p((byte *)&buffer[0]);
//pZ->FDump("MonPCB");
string K0 = hex2str(unsigned(pZ->L(0)));
string K1 = hex2str(unsigned(pZ->L(1)));
string K2 = hex2str(unsigned(pZ->L(2)));
string K3 = hex2str(unsigned(pZ->L(3)));
string Ks = string("| 0x") + K0 + string("| 0x") + K1 + string("| 0x") +
            K2 + string("| 0x") + K3 + string("|");
pMoni->Memo1->Lines->Add(Ks.c_str());
                                       // Do we recognize it?
if (pMoni->FnMap.find(pZ->Key())!=pMoni->FnMap.end()) // Yerp
  (pMoni->*(pMoni->FnMap)[pZ->Key()])(pZ);            // So do it
else {                                                // Nope
  string s = string("Unrecognised message (key :  ") +// So bleat
             Ks + string(" )  from Orchestrator");
  Application->MessageBox(s.c_str(),"POETS Monitor - not good",MB_ICONWARNING);
}
delete pZ;                           // Junk it and absquatulate
}

//==============================================================================

__fastcall TMonitorForm::TMonitorForm(TComponent *Owner) : TForm(Owner)
// Set up the damn main form. There's all sorts of stages: Creation, showing and
// so on. I have no idea what the difference is, so I just shove everything in
// here.
{
Client.SetSkyHook(this);               // Client backpointer
Client.SetMCB(MonMCB);                 // Flat client message callback
Client.SetPCB(MonPCB);                 // Flat client packet callback
Memo1->Clear();                        // Nothing to see ... yet
state = CONNECTED;                     // Get all the right controls alight
SetState(DISCONNECTED);
                                       // Function map. There are so few, it's
                                       // hardly worth it, but.....
FnMap[Msg_p::KEY(Q::MONI,Q::MSG         )] = &TMonitorForm::OnMsg;
FnMap[Msg_p::KEY(Q::MONI,Q::DEVI,Q::ACK )] = &TMonitorForm::OnMoniDeviAck;
FnMap[Msg_p::KEY(Q::MONI,Q::INJE,Q::ACK )] = &TMonitorForm::OnMoniInjeAck;
FnMap[Msg_p::KEY(Q::MONI,Q::MOTH,Q::DATA)] = &TMonitorForm::OnMonixxxxData;
FnMap[Msg_p::KEY(Q::MONI,Q::SOFT,Q::DATA)] = &TMonitorForm::OnMonixxxxData;

T0.Start(1);                           // Start the local RTC
                                       // The only way the children can see
MonitorForm = this; //ADB              // their parent? Can't we do better?
}

//------------------------------------------------------------------------------
          
void __fastcall TMonitorForm::ClearBtnClick(TObject *Sender)
{
Memo1->Clear();
//ShowMessage("The name " );
}

//------------------------------------------------------------------------------

void __fastcall TMonitorForm::ConnectBtnClick(TObject *Sender)
// Connect to the remote (POETS) server
{
                                       // Unload form fields
string name = VCL2STL(MachineEdit->Text);
string port_s = VCL2STL(PortEdit->Text);
unsigned port_u = str2uint(port_s,0);
if (port_u==0) Client.PsMessage(100,"Invalid server port ID");
else                                   // OK, we're good to try:
  if (Client.Start(name,port_u)==0) {  // Yay!
    hostent * p = gethostbyname(name.c_str());
    char * p2 = inet_ntoa( *((in_addr *)p->h_addr) );
    IPEdit->Text = p2;                 // Display the server IP address.
    SetState(CONNECTED);               // Muck about with the buttons
  }
  else {                               // Bugger
    Client.PsMessage(101,"Server startup failed");
    SetState(DISCONNECTED);            // Much about with the buttons
  }
}

//------------------------------------------------------------------------------

void __fastcall TMonitorForm::CreateMDIChild(String Name)
{
TChannelForm * Child;

//--- create a new MDI child window ----
Child = new TChannelForm(Application);
Child->Caption = Name;
//if (FileExists (Name)) Child->Memo1->Lines->LoadFromFile(Name);
}

//------------------------------------------------------------------------------

void __fastcall TMonitorForm::DisconnectBtnClick(TObject *Sender)
{
SetState(DISCONNECTED);
Client.Close();
Cprintf("Disconnecting client & closing %d channel windows\n",MDIChildCount);
for(int i=0;i<MDIChildCount;i++) MDIChildren[i]->Close();
}

//------------------------------------------------------------------------------

void __fastcall TMonitorForm::FileExit1Execute(TObject *Sender)
{
Close();
}

//------------------------------------------------------------------------------

void __fastcall TMonitorForm::FileNew1Execute(TObject *Sender)
{
CreateMDIChild("Channel " + IntToStr(MDIChildCount + 1));
}

//------------------------------------------------------------------------------

void __fastcall TMonitorForm::FileOpen1Execute(TObject *Sender)
{
if (OpenDialog->Execute()) CreateMDIChild(OpenDialog->FileName);
}

//------------------------------------------------------------------------------

void __fastcall TMonitorForm::HelpAbout1Execute(TObject *Sender)
{
AboutBox->ShowModal();
}

//------------------------------------------------------------------------------
 
void __fastcall TMonitorForm::LocalBtnClick(TObject *Sender)
{
char localname[1023];
gethostname(localname,1023);           // Who am I?
Cprintf("%s\n",localname);
hostent * p = gethostbyname(localname);   // Bolt the rest of the domain name on
string s(p->h_name);                   // Debug
Cprintf("%s\n",s.c_str());
MachineEdit->Text = p->h_name;         // No idea

/*
serverAddr.sin_port = htons(port);
// Set IP address to localhost
hostname[1023] = "\0";
gethostname(hostname, 1023);
printf("HostName: %s\n", hostname); // this one prints correctly

my_hostent = gethostbyname(hostname);
printf("Host: %s\n", my_hostent->h_addr);
printf("IP: %c\n", inet_ntoa(my_hostent->h_addr));
serverAddr.sin_addr.s_addr = *hostname;

#include <winsock.h>

WORD wVersionRequested;
WSADATA wsaData;

wVersionRequested = MAKEWORD(1, 1);
WSAStartup( wVersionRequested, &wsaData );

hostent *p;
char s[128];
char *p2;

//Get the computer name
gethostname( s, 128 );
p = gethostbyname( s );
Memo1->Lines->Add( p->h_name );

//Get the IpAddress
p2 = inet_ntoa( *((in_addr *)p->h_addr) );
Memo1->Lines->Add( p2 );
*/
}

//------------------------------------------------------------------------------

void __fastcall TMonitorForm::NewchannelBtnClick(TObject *Sender)
{
CreateMDIChild("Channel " + IntToStr(MDIChildCount + 1));
Cprintf("Create new channel %d\n",MDIChildCount);
}

//------------------------------------------------------------------------------

unsigned TMonitorForm::OnMoniDeviAck(Msg_p * pZ)
// Device location acknowledgment.
// Different versions of this come from different Orchestrator processes
// Mode = 1 : MonServer
//      = 2 : Root
//      = 3 : Mothership
{
int cnt;
void ** pV = pZ->Get<void *>(98,cnt);  // Originating child
TChannelForm * pChild = (TChannelForm *)(*pV);
                                       // Extract ack string
string ack_s = "Acknowledge: " + pZ->Zname(2);
string text = ack_s + "\n";
                                       // Extract device name
string dev_s = "Device: " + pZ->Zname(1) + "\n   POETS address: ";
int * pI = pZ->Get<int>(0,cnt);        // ..append device ID (if any)
if (pI!=0) {
  for (int i=0;i<cnt;i++) dev_s += "|" + int2str(pI[i]) + "| ";
  if (pV==0) dev_s += "\nINCOMING ACK MESSAGE CORRUPT";
  else {                               // Update child window
    pChild->BoxEdit->Text      = STL2VCL(int2str(pI[0]));
    pChild->BoardEdit->Text    = STL2VCL(int2str(pI[1]));
    pChild->MailBoxEdit->Text  = STL2VCL(int2str(pI[2]));
    pChild->CoreEdit->Text     = STL2VCL(int2str(pI[3]));
    pChild->ThreadEdit->Text   = STL2VCL(int2str(pI[4]));
    pChild->DeviceEdit->Text   = STL2VCL(int2str(pI[5]));
  }
}
bool * pB = pZ->Get<bool>(0,cnt);  // Start/stop?
bool B = false;
if (pB!=0) B = *pB;
if (!B) dev_s.clear();

text += dev_s;
string caption = "Device locate acknowledgement";   // Caption
switch (pZ->Mode()) {
  case 1  : caption += " from MonServer";
            text += " ** not looked up yet **";
            break;
  case 2  : caption += " from Root";
            pB = pZ->Get<bool>(1,cnt);
            B = false;
            if (pB!=0) B = *pB;
            if (!B) {                   // Didn't find it
              text += " NOT FOUND";
              break;
            }
            text += " found";            // Did find it?

            break;
  case 3  : caption += " from Mothership";
            pB = pZ->Get<bool>(0,cnt);
            B = false;
            if (pB!=0) B = *pB;
            if (B) text += "\nSTART exfiltration";
            else text += "\nSTOP exfiltration";
            break;
  default : ;
}
//text = "\n" + msg;
Application->MessageBox(text.c_str(),caption.c_str(),0);
return 0;
}

//------------------------------------------------------------------------------

unsigned TMonitorForm::OnMoniInjeAck(Msg_p * pZ)
// Injected command acknowledgment
{
string ack_s = pZ->Zname(3);           // Ack string
                                       // Mode == 1 => from MonServer
string ack_1 = "[From MonServer] " + ack_s;
Memo1->Lines->Add(ack_1.c_str());
if (pZ->Mode()==1) return 0;
                                       // Mode == 2 => from Root
string caption = "Command injection acknowledgement";
string ack_2 = "[From Root] " + ack_s;
Memo1->Lines->Add(ack_2.c_str());      // -> memo pad
int cnt;                               // Pull out the original command
char * buf = pZ->Get<char>(1,cnt);
string cmnd = string(buf,cnt);
Memo1->Lines->Add(cmnd.c_str());       // -> memo pad
ack_s = ack_s + "\n" + cmnd;           // Decorate and put in the Monkeys face
Application->MessageBox(ack_s.c_str(),caption.c_str(),0);
return 0;
}

//------------------------------------------------------------------------------

unsigned TMonitorForm::OnMonixxxxData(Msg_p * pZ)
// Real data. Park a timestamp in the memo for now.
{
static unsigned count = 0;
double T = MonitorForm->T0.Read(1);    // Timestamp: entering Monitor
pZ->Put<double>(-1,&T);
string Sdata = uint2str(count++) + " [" + dbl2str(T) + "] : |MONI|xxxx|DATA|";
Memo1->Lines->Add(Sdata.c_str());
int cnt;
void ** pV = pZ->Get<void *>(98,cnt);  // Originating child
if (pV==0) {
  Application->MessageBox("Target child window address missing",
                          "Corrupt data packet",0);
  return 0;
}
if (*pV==0) {
  Application->MessageBox("Target child window address zero",
                          "Corrupt data packet",0);
  return 0;
}
TChannelForm * pChild = (TChannelForm *)(*pV);
if (count%2==0) pChild->POLPanel->Color = clAqua;
else pChild->POLPanel->Color = clYellow;
return 0;
}

//------------------------------------------------------------------------------

unsigned TMonitorForm::OnMsg(Msg_p * pZ)
// Incoming message handler FROM POETS server:
// Put a message on the GUI....
{
string text    = pZ->Zname(0);         // Box body
string caption = pZ->Zname(1);         // Box header
int count;                             // I am reliably informed (by me) that
int * pflags   = pZ->Get<int>(1,count);// this is a cool API
int flags = 0;                         // Default no icon
if (pflags!=0) flags = *pflags;        // Natty little icon
string mtext = caption + " : " + text; // Concatenate strings for the memo pad
                                       // Destination?
unsigned * pdest = pZ->Get<unsigned>(0,count);
unsigned dest = 3;                     // Default to both
if (pdest!=0) dest = *pdest;           // Destination specified
switch (dest) {                        // And the destination is.....
  case 0  : Application->MessageBox(text.c_str(),caption.c_str(),flags);
            break;
  case 1  : Memo1->Lines->Add(mtext.c_str());
            break;
  default : Memo1->Lines->Add(mtext.c_str());
            Application->MessageBox(text.c_str(),caption.c_str(),flags);
            break;
}
return 0;
}

//------------------------------------------------------------------------------

void __fastcall TMonitorForm::PortBtnClick(TObject *Sender)
{
PortEdit->Text = "28755";
}

//------------------------------------------------------------------------------

void TMonitorForm::SetState(Mstate_e _state)
// Guess
{
if (state==_state) return;             // Nothing to do?
state = _state;
switch (state) {
  case CONNECTED    : ConnectBtn->Enabled    = false;
                      DisconnectBtn->Enabled = true;
                      NewchannelBtn->Enabled = true;
                      ConnectionEdit->Text   = "CONNECTED";
                      break;
  case DISCONNECTED : ConnectBtn->Enabled    = true;
                      DisconnectBtn->Enabled = false;
                      NewchannelBtn->Enabled = false;
                      ConnectionEdit->Text   = "DISCONNECTED";
                      break;
  default           : ;
}
}

//------------------------------------------------------------------------------

void __fastcall TMonitorForm::TestPortClick(TObject *Sender)
{
PortEdit->Text = "9876";
}

//------------------------------------------------------------------------------

void __fastcall TMonitorForm::WizardTextClick(TObject *Sender)
{
Cprintf("Wizard click\n");
Application->MessageBox("Are you sure?",
             "The Monkey activates the Wizards back passage...",MB_ICONWARNING);
WizardForm->ShowModal();
Wcount = WizardForm->Wcount;
}

//------------------------------------------------------------------------------


