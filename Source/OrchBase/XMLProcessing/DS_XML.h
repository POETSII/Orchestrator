#ifndef __DS_XMLH__H
#define __DS_XMLH__H

//==============================================================================
/*
This class does all the heavy lifting associated with translating the type-
agnostic symbol tree spat out by the XMLparser/validator into the strongly typed
POETS data structure. The high level design idea is that if we ever want another
route in (a binary format, for example), we simply write another DS_XXXX class.
The way in is via PBuild, which walks the XML node tree, recognising the POETS
node type from the element name and looking up an enumerated type from DS_map.
(DS_ here stands for data structure __, if you hadn't already worked that out)
The *assumes* the validator has done its job, but - trust but verify - each
builder routine is effectively a switch with a default clause that calls an
Unrecoverable error.
Apart from the low-level XML elements (devices and paths) there are very few
comments, because - I think - it's all pretty obvious......
*/
//==============================================================================

#include "XMLTreeDump.h"
#include <string>
using namespace std;
class OrchBase;
class GraphT_t;
class GraphI_t;
class CFrag;
class Apps_t;
class Meta_t;
class MsgT_t;
class DevT_t;
class DevI_t;
class PinT_t;
class SupT_t;

//==============================================================================

class DS_XML
{
public:
typedef       xmlTreeDump::node xnode;
              DS_XML(OrchBase *);
virtual ~     DS_XML(void);

void          _Apps_t(xnode *);
CFrag *       _CFrag(xnode *);
DevI_t *      _DevI_t(GraphI_t *,xnode *);
void          _DevI_ts(GraphI_t *,xnode *);
DevT_t *      _DevT_t(GraphT_t *,xnode *);
void          _DevT_ts(GraphT_t *,xnode *);
void          _EdgeI_t(GraphI_t *,xnode *);
void          _EdgeI_ts(GraphI_t *,xnode *);
void          _ExtI_t(GraphI_t *,xnode *);
GraphI_t *    _GraphI_t(Apps_t *,xnode *);
GraphT_t *    _GraphT_t(Apps_t *,xnode *);
Meta_t *      _Meta(xnode *);
MsgT_t *      _MsgT_t(GraphT_t *,xnode *);
void          _MsgT_ts(GraphT_t *,xnode *);
PinT_t *      _PinT_t(DevT_t *,xnode *);
SupT_t *      _SupT_t(GraphT_t *,xnode *);
void          Dump(unsigned = 0,FILE * = stdout);
void          PBuild(xnode *);
void          Show(FILE *);

enum Xtyp {c0 = 0,cCDATA,cCode,cDeviceType,cDeviceTypes,cExternalType,
                cGraphInstance,cGraphs,cGraphType,cInputPin,cMessageType,
                cMessageTypes,cMetaData,cOnCTL,cOnDeviceIdle,cDevI,
                cDeviceInstances,cEdgeI,cEdgeInstances,cExtI,cOnHardwareIdle,
                cOnInit,cOnReceive,cOnRTCL,cOnSend,cOnStop,cOnSupervisorIdle,
                cOutputPin,cProperties,cReadyToSend,cSharedCode,cState,
                cSupervisorInPin,cSupervisorOutPin,cSupervisorType,cXML,cXXXX};
map<string,Xtyp> DS_map;
OrchBase *    par;
long          t0;                      // Wallclock time

};

//==============================================================================

#endif
