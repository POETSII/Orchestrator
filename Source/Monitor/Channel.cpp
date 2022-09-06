//------------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Channel.h"
#include "Misc.h"
#include "timer.h"
#include "Msg_p.hpp"
#include "Pglobals.h"

//------------------------------------------------------------------------------

#pragma resource "*.dfm"

//------------------------------------------------------------------------------

__fastcall TChannelForm::TChannelForm(TComponent *Owner) : TForm(Owner)
{
DeviceInterval.Limits(1.0,50000.0);
DeviceState = STARTED;
DeviceSetState(STOPPED);
}

//------------------------------------------------------------------------------

void __fastcall TChannelForm::BrowseBtnClick(TObject *Sender)
{
AnsiString bfile;
if (MonitorForm->SaveDialog1->Execute())
bfile = MonitorForm->SaveDialog1->FileName;
LogFileEdit->Text = bfile;
}

//------------------------------------------------------------------------------

void TChannelForm::DeviceSetState(Cstate_e _state)
{
if (DeviceState==_state) return;
DeviceState = _state;
switch (DeviceState) {
  case STARTED : DeviceStartBtn->Enabled    = false;
                 DeviceStopBtn->Enabled     = true;
                 ApplicationEdit->Enabled   = false;
                 HostDeviceEdit->Enabled    = false;
                 SoftRadioBtn->Enabled      = false;
                 MothRadioBtn->Enabled      = false;
                 Type1Btn->Enabled          = false;
                 Type2Btn->Enabled          = false;
                 IntervalEdit->Enabled      = false;
                 IntervalUpDown->Enabled    = false;
                 LabelEdit->Enabled         = false;
                 LogFileEdit->Enabled       = false;
                 LocalCheckBox->Enabled     = false;
                 RemoteCheckBox->Enabled    = false;
                 BrowseBtn->Enabled         = false;
                 break;
  case STOPPED : DeviceStartBtn->Enabled    = true;
                 DeviceStopBtn->Enabled     = false;
                 ApplicationEdit->Enabled   = true;
                 HostDeviceEdit->Enabled    = true;
                 SoftRadioBtn->Enabled      = true;
                 MothRadioBtn->Enabled      = true;
                 Type1Btn->Enabled          = true;
                 Type2Btn->Enabled          = true;
                 IntervalEdit->Enabled      = true;
                 IntervalUpDown->Enabled    = true;
                 LabelEdit->Enabled         = true;
                 LogFileEdit->Enabled       = true;
                 LocalCheckBox->Enabled     = true;
                 RemoteCheckBox->Enabled    = true;
                 BrowseBtn->Enabled         = true;
                 break;
  default      : ;
}
}

//------------------------------------------------------------------------------

void __fastcall TChannelForm::DeviceStartBtnClick(TObject *Sender)
// The Monkey has hit the "start sending data" button. Loading this all up seems
// a lot of drama, but the protocol is designed such that (most of) the
// subsequent action takes place by simply adding fields to copies of this.
// It's longer than it would be without error traps, but that's users for you.
// An interesting observation - must fix - there is no (current) way of getting
// dynamic data fields *out* of a Msg_p.
{
Msg_p Z;                               // Start building the first message
Z.Key(Q::MONI,Q::DEVI,Q::REQ );        // Key
Z.Mode(1);                             // Mode: Monitor -> MonServer
if(ApplicationEdit->Text.IsEmpty()) {  // Guess
  ShowMessage("Application field empty");
  return;
}
Z.Zname(0,VCL2STL(ApplicationEdit->Text).c_str()); // POETS app (may not exist?)

if(HostDeviceEdit->Text.IsEmpty()) {   // Guess
  ShowMessage("Device field empty");
  return;
}
Z.Zname(1,VCL2STL(HostDeviceEdit->Text).c_str());  // User device name
Z.Zname(2,VCL2STL(LabelEdit->Text).c_str());       // Acknowledge string
Z.Zname(3,VCL2STL(LogFileEdit->Text).c_str());     // Optional log file

unsigned interval = unsigned(DeviceInterval.dValue());
Z.Put<unsigned>(0,&interval);          // Sampling interval

unsigned typeu = 1;                    // Sample type (1|2|....)
if (Type2Btn->Checked) typeu = 2;
Z.Put<unsigned>(1,&typeu);

UDL = UniU(99);                        // Process unique data label
Z.Put<unsigned>(2,&UDL);

unsigned lfdest = 0;                   // Location of data log file
if (!LocalCheckBox->Checked &&  RemoteCheckBox->Checked) lfdest = 1;
if ( LocalCheckBox->Checked && !RemoteCheckBox->Checked) lfdest = 2;
if ( LocalCheckBox->Checked &&  RemoteCheckBox->Checked) lfdest = 3;
Z.Put<unsigned>(3,&lfdest);

unsigned sampleu = 1;                  // Sample type 1:Soft, 2: Moth
if (Type2Btn->Checked) sampleu = 2;
Z.Put<unsigned>(4,&sampleu);

bool startb = true;                    // START flag
Z.Put<bool>(0,&startb);

void * pChild = (void *)this;          // Address of originating object so the
Z.Put<void *>(98,&pChild);             // corresponding ACK knows where to go

unsigned Wcount = MonitorForm->Wcount; // Test data count from Dummy
Z.Put<unsigned>(666,&Wcount);

double T = MonitorForm->T0.Read(1);    // Leaving Monitor Timestamp
Z.Put<double>(-10,&T);
                                       // So - I think - the form is now as
                                       // bullet-proof as it can be
MonitorForm->Client.Send(Z.Stream_v());// And send the data off to the MonServer
DeviceSetState(STARTED);               // Update the GUI
}

//------------------------------------------------------------------------------

void __fastcall TChannelForm::DeviceStopBtnClick(TObject *Sender)
// Stop device data exfiltration. A lot of crap here that I just copy over
// for the hell of it.
{
Msg_p Z;                               // Stop exfiltration
Z.Key(Q::MONI,Q::DEVI,Q::REQ );        // Key
Z.Mode(1);                             // Mode

Z.Zname(1,VCL2STL(HostDeviceEdit->Text).c_str());  // User device name
Z.Zname(2,VCL2STL(LabelEdit->Text).c_str());       // Acknowledge string

Z.Put<unsigned>(2,&UDL);               // Process unique data label

bool startb = false;                   // STOP flag
Z.Put<bool>(0,&startb);

void * pChild = (void *)this;          // Address of originating object so the
Z.Put<void *>(98,&pChild);             // corresponding ACK knows where to go

double T = MonitorForm->T0.Read(1);    // Timestamp: Leaving Monitor
Z.Put<double>(-10,&T);

MonitorForm->Client.Send(Z.Stream_v());// And send it off to the MonServer
DeviceSetState(STOPPED);               // Update the GUI
}

//------------------------------------------------------------------------------

void __fastcall TChannelForm::FormClose(TObject *Sender, TCloseAction &Action)
{
Action = caFree;
}

//------------------------------------------------------------------------------

void __fastcall TChannelForm::InjeSendBtnClick(TObject *Sender)
// The "Inject remote command" button has been smote. Smitten? Smut?
{
Msg_p Z;                               // Build us a message to send
Z.Key(Q::MONI,Q::INJE,Q::REQ);         // Key fields
                                       // Pull in the command string
string cmnd = VCL2STL(InjeCommandEdit->Text);
                                       // On the assumption it's a POETS command
                                       // append a comment. If it isn't, Root
                                       // will bark anyway.
cmnd += " // [Remote monitor]";        // Help to keep track of things
                                       // User defined acknowledgement string
string ack_s = VCL2STL(InjeAckEdit->Text);
Z.Zname(3,ack_s);                      // Pack it away
Z.Put<char>(1,&cmnd[0],cmnd.size()+1); // -> <char>[1] 'cos OnKeyb expects it

/* The "+1" is a mystery. The short explanation is that it makes an access
overrun error in TMonitorForm::OnMoniInjeAck(..) go away, and as it's just a
overrun for a read for a GUI display element it's hard to see how it could be
dangerous. Further, it's just one byte, so we can spare it.
More worryingly, I CANNOT reproduce the fault in a test harness, which points to
a more subtle corruption inside Msg_p. Exactly the same construct is used
elsewhere in the Orchestrator with no problems. Given that the internals of
Msg_p scamper about in a large pre-allocated area of memory under the control of
a cut-down memory manager that I wrote, the fault is probably ultimately going
to be mine, but I don't have the bandwidth right now to get in there and find
and fix it.
*/

MonitorForm->Client.Send(Z.Stream_v());// Stream it
}

//------------------------------------------------------------------------------

void __fastcall TChannelForm::IntervalUpDownClick(TObject *Sender,
      TUDBtnType Button)
// Button: 0=up, 1=down
{
DeviceInterval.Click(Button==0);          // Pass the GUI click to the control
IntervalEdit->Text = STL2VCL(DeviceInterval.sValue());     // Display new value
}

//------------------------------------------------------------------------------

