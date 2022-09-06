//------------------------------------------------------------------------------
#ifndef ChannelH
#define ChannelH
//------------------------------------------------------------------------------
#include <vcl\Controls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Graphics.hpp>
#include <vcl\Classes.hpp>
#include <vcl\Windows.hpp>
#include <vcl\System.hpp>
#include <StdCtrls.hpp>
#include <ComCtrls.hpp>

#include "Monitor.h"
#include "LogUpDn.h"
#include <ExtCtrls.hpp>

//------------------------------------------------------------------------------

class TChannelForm : public TForm
{
__published:
        TPageControl *ChannelPageCtl;
        TTabSheet *DeviceTabSheet;
        TTabSheet *InjeTabSheet;
        TLabel *Label1;
        TEdit *HostDeviceEdit;
        TCheckBox *RemoteCheckBox;
        TEdit *LabelEdit;
        TLabel *Label2;
        TButton *DeviceStartBtn;
        TButton *DeviceStopBtn;
        TButton *InjeSendBtn;
        TLabel *Label5;
        TEdit *InjeCommandEdit;
        TEdit *InjeAckEdit;
        TLabel *Label6;
        TEdit *ApplicationEdit;
        TLabel *Label7;
        TEdit *IntervalEdit;
        TUpDown *IntervalUpDown;
        TLabel *Label8;
        TLabel *Label11;
        TGroupBox *GroupBox1;
        TLabel *Label12;
        TEdit *LogFileEdit;
        TLabel *Label13;
        TLabel *Label14;
        TLabel *Label15;
        TLabel *Label16;
        TLabel *Label17;
        TLabel *Label18;
        TEdit *BoxEdit;
        TEdit *BoardEdit;
        TEdit *MailBoxEdit;
        TEdit *CoreEdit;
        TEdit *ThreadEdit;
        TEdit *DeviceEdit;
        TButton *BrowseBtn;
        TPanel *POLPanel;
        TPanel *Panel1;
        TRadioButton *SoftRadioBtn;
        TRadioButton *MothRadioBtn;
        TPanel *Panel2;
        TRadioButton *Type1Btn;
        TRadioButton *Type2Btn;
        TGroupBox *Local;
        TCheckBox *LocalCheckBox;
        TRadioButton *LocalOverwrite;
        TRadioButton *LocalAppend;
        TGroupBox *GroupBox2;
        TRadioButton *RemoteAppend;
        TRadioButton *RemoteOverwrite;
void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
void __fastcall IntervalUpDownClick(TObject *Sender, TUDBtnType Button);
void __fastcall DeviceStartBtnClick(TObject *Sender);
void __fastcall InjeSendBtnClick(TObject *Sender);
void __fastcall DeviceStopBtnClick(TObject *Sender);
        void __fastcall BrowseBtnClick(TObject *Sender);

private:

public:
enum               Cstate_e {STARTED=0, STOPPED} DeviceState;
void               DeviceSetState(Cstate_e);

virtual __fastcall TChannelForm(TComponent *Owner);
LogUpDn            DeviceInterval;
unsigned           UDL;
};


//------------------------------------------------------------------------------

#endif
