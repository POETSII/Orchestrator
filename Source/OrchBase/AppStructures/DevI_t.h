#ifndef __DevI_tH__H
#define __DevI_tH__H

#include <stdio.h>
#include "flat.h"
#include "NameBase.h"
#include "SoftwareAddress.h"
#include "DefRef.h"
#include "DumpChan.h"
class Meta_t;
class GraphI_t;
class DevT_t;
class CFrag;
class PinI_t;

//==============================================================================

class DevI_t : public NameBase, protected DumpChan, public DefRef
{
public:
                     DevI_t(GraphI_t * = 0, string = string());
virtual ~            DevI_t();
void                 Dump(unsigned = 0,FILE * = stdout);
static void          DevDat_cb(DevI_t * const &);
static void          DevKey_cb(unsigned const &);
PinI_t *             GetPin(string &);
void                 Par(GraphI_t * _p);

GraphI_t *           par;
SoftwareAddress      addr;
char                 devTyp;
string               tyId;             // Name of type in type tree
DevT_t *             pT;
unsigned             Key;              // Graph key
vector<Meta_t *>     Meta_v;           // MetaData vector
map<string,PinI_t *> Pmap;             // Pin-by-name map

std::string          pPropsI;          // Properties
std::string          pStateI;          // State
};

//==============================================================================

#endif
