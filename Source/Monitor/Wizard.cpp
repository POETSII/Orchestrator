//------------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include "Wizard.h"
#include "flat.h"
#include "misc.h"
#include <string>
using namespace std;

//------------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
TWizardForm * WizardForm;

//------------------------------------------------------------------------------

__fastcall TWizardForm::TWizardForm(TComponent* Owner) : TForm(Owner)
{
Wcount = 0;
}

//------------------------------------------------------------------------------

void __fastcall TWizardForm::DoneBtnClick(TObject *Sender)
{
Close();
}

//------------------------------------------------------------------------------

void __fastcall TWizardForm::LaunchCountUpDownClick(TObject *Sender,
      TUDBtnType Button)
{
string Wcount_s = VCL2STL(LaunchCountEdit->Text);
Wcount = str2uint(Wcount_s);
}

//------------------------------------------------------------------------------
