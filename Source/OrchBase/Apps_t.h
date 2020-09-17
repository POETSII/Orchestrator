#ifndef __Apps_tH__H
#define __Apps_tH__H

#include <stdio.h>
#include "NameBase.h"
#include "DefRef.h"
class OrchBase;
class DS_XML;
class GraphI_t;
class GraphT_t;

//==============================================================================

class Apps_t : public NameBase, public DefRef
{
public:
                    Apps_t(){}
                    Apps_t(OrchBase *,string);
virtual ~           Apps_t();

static void         DelAll();
static void         DelApp(string);
void                Dump(unsigned = 0,FILE * = stdout);
static void         DumpAll(FILE * =stdout);
static Apps_t *     FindApp(string);
static Apps_t *     FindFile(string);
GraphI_t *          FindGrph(string);
GraphT_t *          FindTree(string);
static void         Show(FILE * = stdout);
static void         WriteBinaryGraph(GraphI_t *);

OrchBase *          par;               // Parent
vector<GraphI_t *>  GraphI_v;          // Device graphs
vector<GraphT_t *>  GraphT_v;          // Declare trees
string              filename;          // XML source from HbD-land
FILE *              fl;                // File load log file
static map<string,Apps_t *> Apps_m;    // Anchor structure
static unsigned     PoLCnt;            // How many PoLs we got?
static unsigned     ExtCnt;            // How many proper applications?
string              sTime;             // Creation timestamp
string              sDate;

struct PoL_t {                         // Orchestrator generated Proof-of-Life
  public:                              // application parameters
  PoL_t();
  void           Dump(unsigned = 0,FILE * = stdout);
  bool           IsPoL;
  string         type;
  vector<string> params;
};

PoL_t * pPoL;                          // The PoL generator - if any

};

//==============================================================================

#endif




