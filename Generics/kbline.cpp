//------------------------------------------------------------------------------

#include <stdio.h>
#include <conio.h>
#include "kbline.h"
#include "macros.h"
#include <algorithm>
using namespace std;

//==============================================================================

KbLine::KbLine()
{
FnLine = 0;
OnIdle = 0;
OnWizz = 0;
Init();
}

//------------------------------------------------------------------------------

KbLine::~KbLine()
{
}

//------------------------------------------------------------------------------

void KbLine::Blank()
// Overwrite the entire current line on the screen with blanks, making it
// disappear. Note no formatted IO necessary.
{
putchar('\r');
for(unsigned i=0;i<line.size();i++) putchar(' ');
}

//------------------------------------------------------------------------------

void KbLine::C2p()
// Move the screen cursor to the position indicated by the internal character
// pointer. Note no formatted IO.
{
putchar('\r');
for (unsigned j=0;j<cpos;j++) putchar(line[j]);
}

//------------------------------------------------------------------------------

void KbLine::Dump()
// Data structure dump
{
printf("\nKbLine dump---------------------\n");
printf("line = ||%s||, size = %u\n",line.c_str(),line.size());
printf("cpos = %u\n",cpos);
printf("History buffer has %u entries\n",hist.size());
for(unsigned i=0;i<hist.size();i++) printf("%2u ||%s||\n",i,hist[i].c_str());
printf("lpos = %u\n",lpos);
printf("FnLine callback = %p\n",FnLine);
printf("OnIdle callback = %p\n",OnIdle);
printf("OnWizz callback = %p\n",OnWizz);
printf("\nEO KbLine dump------------------\n");
}

//------------------------------------------------------------------------------

void KbLine::GetKey(bool & rischar,char & rc)
// Routine the pulls in a key press. We know there's something there, because
// kbhit() sent us here, but crucially we don't know how many characters are
// in the buffer - some key presses send multiple characters. This is very
// simplistic and could/should be mor complex if we want more characters
// than we do. The only multi-byte characters we *need* here are arrows and
// from the numeric key cluster, and we can disambiguate those with the second
// byte
{
rc = getch();                          // Get a character
rischar = true;                        // Assume it's printable
if (isprint(rc));                      // Do ANSI agree?
else {
  rischar = false;                     // Not printable
  switch (rc) {
    case '\r' : return;                // Return
    case 0x08 : return;                // Backspace
    case 0x1b : return;                // Escape
    default   : rc = getch(); return;  // Multi-byte character
  }
}
}

//------------------------------------------------------------------------------

void KbLine::Go()
{
printf("Do it: ");
char z;
string zz;
for (;;) {
  z=getchar();
  printf("z ||%c||\n",z);
  if (z=='\r') break;
  zz += z;
}
printf("zz ||%s||\n",zz.c_str());




bool ischar;
char c;
for(;;) {                              // And spin...
  if (OnIdle!=0)                       // Go see if anything else is happening
    if (!OnIdle(ptr,line)) return;     // Do I need to abort as a consequence?
  int i = kbhit();                     // Key hit?
  if (i==0) continue;                  // A primate hit a key?
  GetKey(ischar,c);                    // Pull in the key
  printf("\nGot ||%c||\n",c); fflush(stdout);
    switch (Sniffer(c)) {              // Special codes ?
      case sniffEXIT : putchar('\n');  // Exit KbLine completely
                       return;
      case sniffDUMP : Blank();        // Dump data structure to stdio
                       line.clear();
                       cpos=0;
                       Dump();
                       continue;
      case sniffWIZZ : Blank();        // Backdoor callback found?
                       line.clear();
                       cpos=0;
                       if (OnWizz!=0) if (!OnWizz(ptr)) return;
                       continue;
      case sniffNONE : break;          // No secret keywords, then
    }
  if (ischar) {                        // If the character is printable...
    switch (mode) {                    // Insert or overwrite?
      case INS  : line.insert(cpos++,1,c);
                  break;
      case OVER : if (cpos<line.size()) line[cpos++] = c;
                  else line.insert(cpos++,1,c);
                  break;
    }
    Udisp();                           // Actually display the damn character
  }
  else switch (c) {                    // It's a control key
    case '\r' : cpos=0;
                                       // Hand the line out to the user callback
                if (FnLine!=0) if (!FnLine(ptr,line)) return;
                                       // If there was stuff in the buffer, we
                                       // need a new one
                if (!line.empty()) {
                  hist.push_back(line);
                  lpos=hist.size()-1;
                }
                line.clear();
                                       // Lose adjacent duplicates in history
                hist.resize(distance(hist.begin(),
                            unique(hist.begin(),hist.end())));
                putchar('\n');         // And move to the next line 
                break;
    case 0x8  :                        // Backspace
                if (line.empty()) break;          // Nothing there
                if (cpos==0) break;               // Already at LHS
                cpos--;                           // Move char pointer
                Blank();                          // Kill display
                line.erase(line.end()-1);         // Erase the character
                Udisp();                          // Update display
                break;
    case 0x4b :                        // Left arrow
                if (cpos==0) break;               // Already at LHS
                cpos--;                           // Move char pointer
                C2p();                            // Move cursor to char ptr
                break;
    case 0x4d :                        // Right arrow
                if (cpos==line.size()) break;     // Already at RHS
                cpos++;                           // Move char pointer
                C2p();                            // Move cursor to char ptr
                break;
    case 0x48 :                        // Up arrow
                if (hist.empty()) break;          // History empty
                if (lpos==0) break;               // Already at top
                Blank();                          // Kill display
                line = hist[lpos];                // Extract history
                cpos = line.size();               // Move char pointer
                Udisp();                          // Update display
                C2p();                            // Move cursor to char ptr
                lpos--;                           // Move line pointer
                break;
    case 0x50 :                        // Down arrow
                if (hist.empty()) break;          // History empty
                if (lpos==hist.size()-1) break;   // Already at bottom
                Blank();                          // Kill display
                line = hist[lpos];                // Extract history
                cpos = line.size();               // Move char pointer
                Udisp();                          // Update display
                C2p();                            // Move cursor to char ptr
                lpos++;                           // Move line pointer
                break;
    case 0x4f : Init();                // End key - empty history buffer
                break;
    case 0x53 :                        // Del key
                if (cpos==line.size()) break;     // Cursor already at EOL
                Blank();                          // Kill display
                line.erase(cpos,1);               // Extract char from buffer
                Udisp();                          // Update display
                C2p();                            // Move cursor to char ptr
                break;
    case 0x52 : ToggleMode();          // Toggle insert/overwrite mode
                break;
    default   : break;                 // The control keys yet to be defined....
  }
}

}

//------------------------------------------------------------------------------

void KbLine::Init()
// Initialise the data structure. Used in startup AND at user command
{
hist.clear();                          // Kill the history buffer (if any)
hist.push_back(string());              // Re-instate one line for current input
lpos = 0;                              // Line pointer (only one to point to...)
cpos = 0;                              // Character pointer
mode = INS;                            // Guess
}

//------------------------------------------------------------------------------

KbLine::sniff_t KbLine::Sniffer(char c)
// The sniffer that sits astride the incoming byte stream. It's just a set of
// pattern recognisers. This affords a functionality that is not intended to be
// exposed to the "average" user
{
// Look for <esc>exit'\r'. This forces an exit from the keyboard hit loop
static int state1 = 0;
switch (state1) {
  case 0 : if (c==0x1b) state1 = 1; else state1 = 0; break;
  case 1 : if (c=='e' ) state1 = 2; else state1 = 0; break;
  case 2 : if (c=='x' ) state1 = 3; else state1 = 0; break;
  case 3 : if (c=='i' ) state1 = 4; else state1 = 0; break;
  case 4 : if (c=='t' ) state1 = 5; else state1 = 0; break;
  case 5 : if (c=='\r') state1 = 6; else state1 = 0; break;
}
if (state1==6) {
  state1 = 0;
  return sniffEXIT;
}

// Look for <esc>dump'\r'. This forces an datastucture dump to stdio
static int state2 = 0;
switch (state2) {
  case 0 : if (c==0x1b) state2 = 1; else state2 = 0; break;
  case 1 : if (c=='d' ) state2 = 2; else state2 = 0; break;
  case 2 : if (c=='u' ) state2 = 3; else state2 = 0; break;
  case 3 : if (c=='m' ) state2 = 4; else state2 = 0; break;
  case 4 : if (c=='p' ) state2 = 5; else state2 = 0; break;
  case 5 : if (c=='\r') state2 = 6; else state2 = 0; break;
}
if (state2==6) {
  state2 = 0;
  return sniffDUMP;
}

// Look for <esc>[]]]'\r'. This is the key to the Wizards backdoor
static int state3 = 0;
switch (state3) {
  case 0 : if (c==0x1b) state3 = 1; else state3 = 0; break;
  case 1 : if (c=='[' ) state3 = 2; else state3 = 0; break;
  case 2 : if (c==']' ) state3 = 3; else state3 = 0; break;
  case 3 : if (c==']' ) state3 = 4; else state3 = 0; break;
  case 4 : if (c==']' ) state3 = 5; else state3 = 0; break;
  case 5 : if (c=='\r') state3 = 6; else state3 = 0; break;
}
if (state3==6) {
  state3 = 0;
  return sniffWIZZ;
}
return sniffNONE;                      // Found nothing
}

//------------------------------------------------------------------------------

void KbLine::ToggleMode()
// Toggle between insert and overwrite mode. The cursor type changing routine
// is not ANSI - it's in conio.h in Windoze and curses.h in UNIX.
// This routine entirely encapsulates the useage - comment it out if you can't
// find _setcursortype
{
switch (mode) {
  case OVER : mode = INS;  _setcursortype(_NORMALCURSOR); return;
  case INS  : mode = OVER; _setcursortype(_SOLIDCURSOR);  return;
}
}

//------------------------------------------------------------------------------

void KbLine::Udisp()
// Overwrite the current displayed line with the current buffer line.
// Note no formatted IO
{
putchar('\r');
for(unsigned i=0;i<line.size();i++) putchar(line[i]);
}

//------------------------------------------------------------------------------
