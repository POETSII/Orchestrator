#ifndef __ComposerH__H
#define __ComposerH__H

#include "OSFixes.hpp"

#include <string>

#include "Placer.h"
#include "poets_pkt.h"
#include "HardwareAddress.h"
#include "SoftwareAddress.h"
#include "SoftwareAddressDefs.h"
#include "CFrag.h"
#include "MsgT_t.h"
#include "Meta_t.h"

#include "GraphT_t.h"
#include "DevT_t.h"
#include "PinT_t.h"
#include "SupT_t.h"

#include "GraphI_t.h"
#include "DevI_t.h"
#include "PinI_t.h"
#include "EdgeI_t.h"


//class P_core;

struct ComposerGraphI_t;
struct devTypStrings_t;


typedef std::map<PinI_t*, std::vector<unsigned> >   pinIArcKeyMap_t;
typedef std::map<PinT_t*, unsigned>                 pinTIdxMap_t;
typedef std::map<DevT_t*, devTypStrings_t*>         devTStrsMap_t;
typedef std::map<GraphI_t*, ComposerGraphI_t*>      ComposerGraphIMap_t;

typedef std::map<DevI_t*, uint32_t>                 devISuperIdxMap_t;


typedef struct devTypStrings_t
{
    DevT_t* devT;
    GraphI_t* graphI;
    std::string handlerPreamble;
    std::string handlerPreambleS;
    std::string handlerPreambleCS;
    
    std::string handlersH;
    std::string handlersC;
    std::string varsHCommon;
} devTypStrings_t;


typedef struct ComposerGraphI_t
{
    GraphI_t* graphI;
    std::set<P_core*>* cores;    // Cores used by the GraphInstance
    
    // Maps/vector filled during generation. These are only valid if "generated"
    devTStrsMap_t devTStrsMap;              // Common strings for each DeVT
    std::vector<DevI_t*> supevisorDevTVect; // Supervisor Edge Index handling. 
    devISuperIdxMap_t   devISuperIdxMap;    // Supervisor Edge Index handling. 
    // TODO: This is inherently single-supervisor for now and needs modifying to
    // have one vector per supervisor.
    
    std::string outputDir;
    
    //State flags
    bool generated;
    bool compiled;
    
    
    // Constructors/Destructors
    ComposerGraphI_t();
    ComposerGraphI_t(GraphI_t*);
    ~ComposerGraphI_t();
    
    void clearDevTStrsMap();
} ComposerGraphI_t;



struct binaries
{
    std::string data;
    std::string instr;
};


class Composer
{
public:

Composer(Placer*);
Composer();
~Composer();    

int         compose(GraphI_t*);    // Generate and compile
int         generate(GraphI_t*); // Generate Source Files
int         compile(GraphI_t*);  // Compile Source Files

void        decompose(GraphI_t*);  // Clear internal strings
void        clean(GraphI_t*);    // Get rid of built files (e.g. a make clean)

void        reset();    // Reset Composer

void        setOutputPath(std::string);
void        setPlacer(Placer*);



private:
    
Placer*     placer;
std::string outputPath;    
std::string supervisorSendPinName;
    

ComposerGraphIMap_t graphIMap; // Map has an entry for each Graph Instance.


int prepareDirectories(ComposerGraphI_t*);

int generateSupervisor(ComposerGraphI_t*);

void writeGlobalSharedCode(GraphI_t*, std::ofstream&);
void writeGlobalPropsD(GraphI_t*, std::ofstream&);
void writeGlobalPropsI(GraphI_t*, std::ofstream&);
void writeMessageTypes(GraphI_t*, std::ofstream&);


// Device Type writers
void formDevTStrings(ComposerGraphI_t*, DevT_t*);
void populatePinTIdxMap(DevT_t*);
void formHandlerPreamble(devTypStrings_t*);
void formDevTHandlers(devTypStrings_t*);
void formDevTPropsDStateD(devTypStrings_t*);
void formDevTInputPinHandlers(devTypStrings_t* dTypStrs);
void formDevTOutputPinHandlers(devTypStrings_t* dTypStrs);



int createCoreFiles(P_core*, std::string&, std::ofstream&,
                    std::ofstream&, std::ofstream&, std::ofstream&);
void writeCoreSrc(P_core*, devTypStrings_t*, std::ofstream&, 
                    std::ofstream&, std::ofstream&, std::ofstream&);

void writeCoreVarsHead(unsigned, std::ofstream&, std::ofstream&);
void writeCoreVarsFoot(unsigned, std::ofstream&);

void writeCoreHandlerHead(unsigned, std::ofstream&, std::ofstream&);
void writeCoreHandlerFoot(unsigned, std::ofstream&, std::ofstream&);


int createThreadFile(P_thread*, std::string&, std::ofstream&);

unsigned writeThreadVars(ComposerGraphI_t*, P_thread*, ofstream&, ofstream&);
void writeThreadVarsCommon(AddressComponent, AddressComponent, 
                            std::ofstream&, std::ofstream&);
void writeThreadContextInitialiser(P_thread*, DevT_t*,
                            std::ofstream&, std::ofstream&);
void writeDevTDeclInit(AddressComponent, DevT_t*,
                            std::ofstream&, std::ofstream&);
void writeInputPinInit(AddressComponent, DevT_t*, 
                            std::ofstream&, std::ofstream&);
void writeOutputPinInit(AddressComponent, DevT_t*,
                            std::ofstream&, std::ofstream&);
void writeDevIDecl(AddressComponent, size_t, std::ofstream&);


void writeThreadDevIDefs(ComposerGraphI_t*, P_thread*, size_t,
                                std::ofstream&, std::ofstream&);
void writeDevIInputPinDefs(GraphI_t*, DevT_t*, AddressComponent threadAddr,
                                std::string&, std::vector<unsigned>&,
                                std::ofstream&, std::ofstream&);
void writeDevIInputPinEdgeDefs(GraphI_t*, PinI_t*, AddressComponent,
                                std::string&, std::vector<unsigned>&,
                                std::ofstream&, std::ofstream&);
void writePinPropsDecl(PinI_t*, std::string&, size_t, std::ofstream&);
void writePinStateDecl(PinI_t*, std::string&, size_t, std::ofstream&);
void writeDevIInPinsDecl(std::string&, size_t, std::ofstream&);
void writeDevIOutPinsDecl(std::string&, size_t, std::ofstream&);


void writeDevIOutputPinDefs(ComposerGraphI_t*, DevI_t*, AddressComponent,
                                std::string&, std::vector<unsigned>&,
                                std::ofstream&, std::ofstream&);
void writeDevIOutputPinEdgeDefs(GraphI_t*, PinI_t*, std::string&, 
                                std::vector<unsigned>&,
                                std::ofstream&, std::ofstream&);


void writeDevTSharedCode(DevT_t*, std::ofstream&);


void writeDevTOnDeIdHandler(devTypStrings_t*, std::ofstream&, std::ofstream&);
void writeDevTOnHWIdHandler(devTypStrings_t*, std::ofstream&, std::ofstream&);
void writeDevTOnRTSHandler(devTypStrings_t*, std::ofstream&, std::ofstream&);
void writeDevTOnInitHandler(devTypStrings_t*, std::ofstream&, std::ofstream&);
              
void writeDevTPropsDStateD(DevT_t*, std::ofstream&);




void writeDevIPropsI(DevI_t*, std::ofstream&);
void writeDevIStateI(DevI_t*, std::ofstream&);


void writeGlobalProps(DevT_t*, P_thread*, std::ofstream&, std::ofstream&);
void writeGlobalSharedCode(DevT_t*, std::ofstream&);
void writeMessageTypedefs(DevT_t*, std::ofstream&);
void writeGeneralHandlers(DevT_t*, std::ofstream&);
void writeDeviceTypeDecls(DevT_t*, std::ofstream&);
    
};
#endif  // __ComposerH__H
