#ifndef __BHelper__H
#define __BHelper__H

#include "xmlParser.h"
class A_builder;
class PinT_t;
class Apps_t;
class GraphT_t;
class DevT_t;
class PinT_t;
class SupT_t;
class GraphI_t;
class DevI_t;
class PinI_t;
class MsgT_t;
class CFrag;
class Meta_t;
class P_link;
class OrchBase;
#include <vector>
//#include <map>
#include "map2.h"
#include <string>
using namespace std;

//==============================================================================

class BHelper : public xmlParser
{
public:
enum Potype {P_0=0,Po_FILE ,Po_GrS  ,Po_GrT  ,Po_Msg  ,Po_MsTs ,Po_MsT  ,
 Po_DeTs ,Po_DeT  ,Po_CdP  ,Po_CdS  ,Po_CdSh ,Po_ORTS ,Po_OIDL ,Po_PnIn ,
 Po_OREC ,Po_PnOu ,Po_OSND ,Po_GrI  ,Po_DeIs ,Po_DeI  ,Po_EgIs ,Po_EgI  ,
 Po_OCTL ,Po_OHW  ,Po_ExT  ,Po_ExI  ,Po_SuT  ,Po_OPKT ,Po_ORTCL,Po_OSTOP,
 Po_Meta ,Po_XXXX };

                    BHelper(string,OrchBase *);
virtual ~           BHelper(void);
unsigned            BuildApp();
bool                CDATA(const void *,const unsigned &,const string &);
bool                CDATAfix(CFrag **);
bool                Comments(const void *,const string &);
void                Dump(FILE * = stdout);
bool                EndDocument(const void *);
bool                EndElement(const void *,const string &);
bool                Error(const void *,const unsigned &,const unsigned &,
                          const unsigned &,const string &);
void                IntegGraph(GraphI_t *);
void                IntegType(GraphT_t *);
void                MkAtMp(vector<pair<string,string> > &);
void                New_Po_CdP();
void                New_Po_CdS();
void                New_Po_CdSh();
void                New_Po_DeI();
void                New_Po_DeIs();
void                New_Po_DeT();
void                New_Po_DeTs();
void                New_Po_EgI();
void                New_Po_EgIs();
void                New_Po_ExI();
void                New_Po_ExT();
void                New_Po_GrI();
void                New_Po_GrS();
void                New_Po_GrT();
void                New_Po_Meta(const vector<pair<string,string> > &);
void                New_Po_Msg();
void                New_Po_MsT();
void                New_Po_MsTs();
void                New_Po_OCTL();
void                New_Po_OHW();
void                New_Po_OIDL();
void                New_Po_OREC();
void                New_Po_ORTS();
void                New_Po_OSND();
void                New_Po_PnIn();
void                New_Po_PnOu();
void                New_Po_SuT();
static unsigned     PathDecode(string,string &,string &,string &,string &);
DevI_t *            PushNewD(string);
bool                StartDocument(const void *,const string &,
                                 const vector<pair<string,string> > &);
bool                StartElement(const void *,const string &,
                                const vector<pair<string,string> > &);

string              filename;
OrchBase *          par;
map2<Potype,string> Potype_map;

struct context {
  context(BHelper *);
  context(BHelper *,context &);
  ~context(void);
  void Dump(FILE * = stdout,int off=0);
  Apps_t *    pApps;
  GraphT_t *  pGraphT;
  DevT_t *    pDevT;
  PinT_t *    pPinT;
  SupT_t *    pSupT;
  GraphI_t *  pGraphI;
  DevI_t *    pDevI;
  PinI_t *    pPinI;
  P_link *    pEdgI;
  MsgT_t *    pMsgT;
  CFrag *     pCFrag;
  Meta_t *    pMeta;
  Potype      tag;
  Potype      ptag;
  BHelper *   par;
  bool        broken;
};
                                       // FORTRAN COMMON blocks
vector<context *>   context_v;         // Context frame
context *           pTos;              // Top of stack pointer
map<string,string>  attrMap;           // Element attribute map for pTos

};

//==============================================================================

#endif


