//----------------------------------------------------------------------------
#ifndef MonitorH
#define MonitorH
//----------------------------------------------------------------------------
#include "Channel.h"
#include <vcl\ComCtrls.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\Messages.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\Dialogs.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Menus.hpp>
#include <vcl\Controls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Graphics.hpp>
#include <vcl\Classes.hpp>
#include <vcl\SysUtils.hpp>
#include <vcl\Windows.hpp>
#include <vcl\System.hpp>
#include <ActnList.hpp>
#include <ImgList.hpp>
#include <StdActns.hpp>
#include <ToolWin.hpp>

#include "Pclient_t.h"
#include "timer.h";
#include "Msg_p.hpp"
#include <map.h>
using namespace std;

//----------------------------------------------------------------------------
class TMonitorForm : public TForm
{
__published:
	TMainMenu *MainMenu1;
	TMenuItem *File1;
	TMenuItem *FileNewItem;
	TMenuItem *FileOpenItem;
	TMenuItem *FileCloseItem;
	TMenuItem *Window1;
	TMenuItem *Help1;
	TMenuItem *N1;
	TMenuItem *FileExitItem;
	TMenuItem *WindowCascadeItem;
	TMenuItem *WindowTileItem;
	TMenuItem *WindowArrangeItem;
	TMenuItem *HelpAboutItem;
	TOpenDialog *OpenDialog;
	TMenuItem *FileSaveItem;
	TMenuItem *FileSaveAsItem;
	TMenuItem *Edit1;
	TMenuItem *CutItem;
	TMenuItem *CopyItem;
	TMenuItem *PasteItem;
	TMenuItem *WindowMinimizeItem;
        TStatusBar *StatusBar;
        TActionList *ActionList1;
        TEditCut *EditCut1;
        TEditCopy *EditCopy1;
        TEditPaste *EditPaste1;
        TAction *FileNew1;
        TAction *FileSave1;
        TAction *FileExit1;
        TAction *FileOpen1;
        TAction *FileSaveAs1;
        TWindowCascade *WindowCascade1;
        TWindowTileHorizontal *WindowTileHorizontal1;
        TWindowArrange *WindowArrangeAll1;
        TWindowMinimizeAll *WindowMinimizeAll1;
        TAction *HelpAbout1;
        TWindowClose *FileClose1;
        TWindowTileVertical *WindowTileVertical1;
        TMenuItem *WindowTileItem2;
        TToolBar *ToolBar2;
        TToolButton *ToolButton1;
        TToolButton *ToolButton2;
        TToolButton *ToolButton3;
        TToolButton *ToolButton4;
        TToolButton *ToolButton5;
        TToolButton *ToolButton6;
        TToolButton *ToolButton7;
        TToolButton *ToolButton8;
        TToolButton *ToolButton9;
        TToolButton *ToolButton10;
        TToolButton *ToolButton11;
        TImageList *ImageList1;
        TPanel *Panel1;
        TMemo *Memo1;
        TLabel *Label1;
        TLabel *Label2;
        TEdit *MachineEdit;
        TLabel *Label3;
        TEdit *PortEdit;
        TButton *ConnectBtn;
        TButton *DisconnectBtn;
        TButton *NewchannelBtn;
        TLabel *Label4;
        TLabel *Label5;
        TEdit *ConnectionEdit;
        TEdit *UserEdit;
        TStaticText *StaticText1;
        TPanel *Panel2;
        TButton *ClearBtn;
        TButton *LocalBtn;
        TButton *PortBtn;
        TLabel *Label6;
        TLabel *Label7;
        TEdit *IPEdit;
        TPanel *WizardPanel;
        TButton *TestPort;
        TLabel *Label8;
        TSaveDialog *SaveDialog1;
        void __fastcall FileNew1Execute(TObject *Sender);
        void __fastcall FileOpen1Execute(TObject *Sender);
        void __fastcall HelpAbout1Execute(TObject *Sender);
        void __fastcall FileExit1Execute(TObject *Sender);
        void __fastcall NewchannelBtnClick(TObject *Sender);
        void __fastcall ClearBtnClick(TObject *Sender);
        void __fastcall LocalBtnClick(TObject *Sender);
        void __fastcall PortBtnClick(TObject *Sender);
        void __fastcall WizardTextClick(TObject *Sender);
        void __fastcall ConnectBtnClick(TObject *Sender);
        void __fastcall DisconnectBtnClick(TObject *Sender);
        void __fastcall TestPortClick(TObject *Sender);
private:
	void __fastcall CreateMDIChild(const String Name);
public:
	virtual __fastcall TMonitorForm(TComponent *Owner);

Pclient_t            Client;
Timer_t              T0;               // POETS equivalent of MPI_WTime()
enum                 Mstate_e {CONNECTED=0,DISCONNECTED} state;
unsigned             Wcount;

void                 SetState(Mstate_e);
typedef unsigned     (TMonitorForm::*pMeth)(Msg_p *);
map<unsigned,pMeth>  FnMap;
unsigned             OnMsg(Msg_p *);
unsigned             OnMoniDeviAck (Msg_p *);
unsigned             OnMoniInjeAck (Msg_p *);
unsigned             OnMonixxxxData(Msg_p *);
};

//------------------------------------------------------------------------------

//extern TChannelForm * __fastcall MDIChildCreate(void);
extern TMonitorForm * MonitorForm;

//------------------------------------------------------------------------------
#endif
