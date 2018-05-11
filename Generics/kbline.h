#ifndef __KBLINE__H
#define __KBLINE__H

#include <vector>
#include <string>
using namespace std;

//==============================================================================

class KbLine {
public:
                   KbLine();
virtual ~          KbLine(void);
void               Dump();
void               Go();
void               SetFnLine(bool(*pf)(void *,string &)){ FnLine = pf; }
void               SetOnIdle(bool(*pf)(void *,string &)){ OnIdle = pf; }
void               SetOnWizz(bool(*pf)(void *)){ OnWizz = pf; }
void               SetPar(void * p) { ptr = p; }

private:
enum sniff_t       {sniffNONE,sniffEXIT,sniffDUMP,sniffWIZZ};
void               Blank();
void               C2p();
void               GetKey(bool &,char &);
void               Init();
sniff_t            Sniffer(char);
void               ToggleMode();
void               Udisp();

bool               (*FnLine)(void *,string &);
bool               (*OnIdle)(void *,string &);
bool               (*OnWizz)(void *);

vector<string>     hist;               // History vector
string             line;               // Line buffer
unsigned           lpos;               // Line position in history vector
unsigned           cpos;               // Character position in line buffer
enum mode_t        {OVER,INS} mode;    // Overwrite/insert mode
void *             ptr;                // Callback pointer

};

//==============================================================================

#endif
