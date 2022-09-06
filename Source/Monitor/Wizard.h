//------------------------------------------------------------------------------

#ifndef WizardH
#define WizardH

//------------------------------------------------------------------------------

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>

//------------------------------------------------------------------------------

class TWizardForm : public TForm
{
__published:                           // IDE-managed Components
TLabel *        Label1;
TEdit *         LaunchCountEdit;
TUpDown *       LaunchCountUpDown;
TButton *       DoneBtn;
void __fastcall DoneBtnClick(TObject * Sender);
void __fastcall LaunchCountUpDownClick(TObject * Sender,TUDBtnType Button);
private:                               // User declarations
public:                                // User declarations
__fastcall      TWizardForm(TComponent * Owner);
unsigned        Wcount;
};

//------------------------------------------------------------------------------

extern PACKAGE TWizardForm *WizardForm;

//------------------------------------------------------------------------------
#endif
