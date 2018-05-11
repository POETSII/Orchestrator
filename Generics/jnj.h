#ifndef __JNJ__H
#define __JNJ__H

#include "flat.h"
#include "uif.h"

//==============================================================================
// I never really liked the INI interface in STL+, and attempts to modify it
// to fit UIF never really fitted comfortably; I *always* had to look stuff up,
// which to my mind meant it was a cumbersome interface.
// This is an attempt at a different kind of interface, based on the idea of
// 'UIF handles' (which are actually typedefed consted Node addresses).
// I might make them mutable if it turns out to be a cool idea.
// The JNJ datastructure is a tree; broadly speaking, you go down with Get...,
// up with Fnd... and along with Loc...
//==============================================================================

typedef UIF::Node * hJNJ;              // Define the handle
typedef vector<hJNJ> vH;               // Vector of handles

class JNJ : public UIF {
public:
          JNJ();                       // Empty constructor
          JNJ(int,char **);            // Off-piste again - command line decoder
          JNJ(string);                 // From file
virtual ~ JNJ(void);

hJNJ FndRecd(hJNJ);                    // Given name handle, find parent record
void FndRecdLabl(hJNJ,string,vH &);    // Given section and ??? string, find
void FndRecdValu(hJNJ,string,vH &);    // parent(s)
void FndRecdVari(hJNJ,string,vH &);
void FndSect(string,vH &);             // Given name, find section(s)
hJNJ FndSect(hJNJ);                    // Given name handle, find parent section
void GetAExpr(hJNJ,vH &);              // Find expressions ... in the attributes
void GetALabl(hJNJ,vH &);              // Find labels ... in the attributes
void GetAVari(hJNJ,vH &);              // Find variables ... in the attributes
void GetLabl(hJNJ,vH &);               // Given section/record, find label names
void GetNames(vH &);                   // Get all section names in the object
void GetNames(hJNJ,vH &);              // Get all section names of section
void GetRecd(hJNJ,vH &);               // Get all the records in a section
void GetSect(vH &);                    // Get all the sections in the object
void GetSub(hJNJ,vH &,int=1);          // Given a name node, get the subnames
void GetValu(hJNJ,vH &);               // Given section/record, find value names
void GetVari(hJNJ,vH &);               // Given ..., find variable names
void LocLabl(hJNJ,vH &);               // Given name, find label set
                                       // Locate instances of the string
void LocName(vH &,string,vH &);
void LocName(vH &,char *,vH &);
hJNJ LocRecd(hJNJ,hJNJ=0);
hJNJ LocSect(hJNJ=0);
void LocValu(hJNJ,vH &);               // Given name, find value set
void RecdSort(vH &);                   // Sort the record variable names
void SectSort(vH &);                   // Sort the section names

#define PRINTV JNJ::printv
void printv(vH &);                     // Diagnostic vector print

private:
void GetAA(hJNJ,vH &,Notype);
void GetAAA(hJNJ,vH &,Notype);
void GetXX(hJNJ,vH &,Notype);
void GetXXX(hJNJ,vH &,Notype);
};

//==============================================================================

double str2dble(hJNJ);
int    str2int(hJNJ);

//==============================================================================

#endif
