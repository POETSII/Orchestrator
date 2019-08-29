#ifndef __FILENAME__H
#define __FILENAME__H

#include "lex.h"

#include <string>
#include <vector>
using namespace std;

//==============================================================================

class FileName {
public:
               FileName(string = string());
virtual ~      FileName(void);
void           Dump(FILE * = stdout);
bool           Err();                  // Is there a problem?
string         FNBase();               // Extract base name
void           FNBase(string);         // Set base name
string         FNComplete();           // Extract complete filename
void           FNComplete(string);     // Set complete name
string         FNDrive();              // Extract drive letter
void           FNDrive(string);        // Set drive
string         FNExtn();               // Extract extension
void           FNExtn(string);         // Set extension
char           FNMode();               // Extract relative/absolute mode
void           FNMode(char);           // Set relative/absolute mode
string         FNName();               // Extract name : Base.Extn
vector<string> FNPath();               // Extract full path
void           FNPath(vector<string>); // Set path
string         FnSepn();               // Extract separator
void           FnSepn(string);         // Set separator
private:
void           EOParse();              // Build internal data elements
void           Parse();                // Parse input string

private:                               // Input strings
string         sIP;                    // Input data
string         sD;                     // Drive substring
vector<string> sP;                     // Vector of path strings
string         sB;                     // Base string
string         sE;                     // Extension string
string         sS;                     // Seperator string

vector<string> sbuf;                   // Buffer for all strings in filename
int            aD;                     // Address in buffer of drive string
int            aP[2];                  // Address in buffer of path subset
int            aB;                     // Address in buffer of base string
int            aE;                     // Address in buffer of extension string
char           RAmode;                 // Relative or absolute path?
bool           problem;                // It's not a filename
bool           dirty;                  // Monkey been fiddling? 
Lex            Lx;                     // The input lexer

static const int   X = -1;             // Cosmic error table entry
static const int   R = -2;             // Cosmic return table entry

};

//==============================================================================

#endif
