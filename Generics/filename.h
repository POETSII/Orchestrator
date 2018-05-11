#ifndef __FILENAME__H
#define __FILENAME__H

#include "lex.h"

#include <string>
#include <vector>
using namespace std;

//==============================================================================

class FileName {
public:
                   FileName(string);
virtual ~          FileName(void);
void               Dump();
bool               Err();              // Is there a problem?
string             FNBase();           // Extract base name
void               FNBase(string);     // Set base name
string             FNComplete();       // Extract complete filename
void               FNComplete(string); // Set complete name
string             FNDrive();          // Extract drive letter
void               FNDrive(string);    // Set drive
string             FNExtn();           // Extract extension
void               FNExtn(string);     // Set extension
string             FNName();           // Name : Base.Extn
string             FNPath(vector<string> &);  // Extract full path
void               FNPath(string,int); // Set path
private:
void               Parse();            // Create output strings
void               Derive1();
void               Derive2();

private:                               // Input strings
string             Sip;                // Input data
string             S0;                 // Drive substring
vector<string>     S1;                 // Vector of path strings
string             S2;                 // Extn string
string             S1e;                // Last element of path

                                       // Output strings
string             SC;                 // Complete
string             SD;                 // Drive
string             SP;                 // Path
string             SB;                 // Base
string             SE;                 // Extension

unsigned int       oS1siz;             // Size(S1) before we pulled the end off
bool               problem;            // It's not a filename

Lex                Lx;                 // The input lexer

static const string Xc;
static const string Xs;
static const string Xd;
static const string X0;
static const int   X = -1;             // Cosmic error table entry
static const int   R = -2;             // Cosmic return table entry

};

//==============================================================================

#endif
