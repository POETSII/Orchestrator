#ifndef __Apps_tH__H
#define __Apps_tH__H

#include <stdio.h>
#include "NameBase.h"
#include "OrchBase.h"
#include "DefRef.h"

//==============================================================================

class Apps_t : public NameBase, public DefRef
{
public:
                    Apps_t(){}
                    Apps_t(OrchBase *,string);
virtual ~           Apps_t();

void                Clear();
void                Dump(FILE * = stdout);
static Apps_t *     FindApp(string);
GraphI_t *          FindGrph(string);
GraphT_t *          FindTree(string);
bool                IsPoL();
//void                LinkFlag(bool f = true){ linked = f; }

void                Show(FILE * = stdout,unsigned=0);

OrchBase *          par;               // Parent
P_super *           pSup;              // Supervisor device for this graph
vector<GraphI_t *>  GraphI_v;          // Device graphs
vector<GraphT_t *>  GraphT_v;          // Declare trees
string              filename;          // XML source from HbD-land
bool                bValid;            // File passed validation?
FILE *              fl;                // File load log file
static map<string,Apps_t *> Apps_m;    // Anchor structure
static unsigned     PoLCnt;            // How many PoLs we got?
static unsigned     ExtCnt;            // How many proper applications?
string              sTime;             // Creation timestamp
string              sDate;

struct PoL_t {
public:
PoL_t();
void Dump(FILE * = stdout);
bool IsPoL;
string type;
vector<string> params;
};

PoL_t PoL;

};

//==============================================================================

#endif




