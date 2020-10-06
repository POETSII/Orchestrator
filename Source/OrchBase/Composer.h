/* The header GMB sent to MLV on 2020-10-06. Should be replaced with something
 * more up to date. */

#ifndef __ComposerH__H
#define __ComposerH__H

#include "OSFixes.hpp"

#include <string>

#include "DevT_t.h"     // Device Type
#include "DevI_t.h"     // Device Instance
#include "Placer.h"
#include "HardwareAddress.h"

class P_core;

struct ComposerGraphI_t;
struct devTypStrings_t;

typedef std::map<PinI_t*, std::vector<unsigned> >   pinIArcKeyMap_t;
typedef std::map<PinT_t*, unsigned>                 pinTIdxMap_t;
typedef std::map<PinT_t*, unsigned>                 pinTIdxMap_t;
typedef std::map<DevT_t*, devTypStrings_t*>         devTStrsMap_t;
typedef std::map<GraphI_t*, ComposerGraphI_t*>      ComposerGraphIMap_t;

typedef struct ComposerGraphI_t
{
    GraphI_t* graphI;
    std::set<P_core*>* cores;    // Cores used by the GraphInstance

    devTStrsMap_t devTStrsMap;

    pinTIdxMap_t pinTIdxMap;

    //State flags
    bool generated;
    bool compiled;


    // Constructors/Destructors
    ComposerGraphI_t();
    ComposerGraphI_t(GraphI_t*);
    ~ComposerGraphI_t();
} ComposerGraphI_t;


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


struct binaries
{
    std::string data;
    std::string instr;
};


struct UnnamedStruct
{
    GraphI_t* instance;
    std::set<P_core*> cores;    // Cores used by the GraphInstance
    std::string sharedCode;     // Shared Code
    std::string gProps;         // Global Properties
    bool generated;
    bool built;
};


class Composer
{
public:

Composer(Placer*);
Composer();


int         compose(GraphI_t*);    // Generate and compile
int         generate(GraphI_t*); // Generate Source Files
int         compile(GraphI_t*);  // Compile Source Files

void        decompose(GraphI_t*);  // Clear internal strings
void        clean(GraphI_t*);    // Get rid of built files (e.g. a make clean)

void        reset();    // Reset Composer

void        setOutputPath(std::string);
void        setPlacer(Placer*);



private:

/* Commented because some types in here don't make sense.

Placer*     placer;
std::string outputPath;


ComposerGraphIMap_t graphIMap; // Map has an entry for each Graph Instance.


int prepareDirectories(std::string&);




void writeGlobalSharedCode(GraphI_t*, std::ofstream&);
void writeGlobalPropsD(GraphI_t*, std::ofstream&);
void writeGlobalPropsI(GraphI_t*, std::ofstream&);
void writeMessageTypes(GraphI_t*, std::ofstream&);


// Device Type writers
void formDevTStrings(GraphI_t*, DevT_t*);
void populatePinTIdxMap(DevT_t*);
void formHandlerPreamble(devTypStrings_t*);
void formDevTHandlers(devTypStrings_t*);
void formDevTPropsDStateD(devTypStrings_t*);
void formDevTInputPinHandlers(devTypStrings_t* dTypStrs);
void formDevTOutputPinHandlers(devTypStrings_t* dTypStrs);



int createCoreFiles(unsigned, std::string&, std::ofstream&, std::ofstream&,
                    std::ofstream&, std::ofstream&);
int writeCoreSrc(P_core* pCore, devTypStrings_t* dTypStrs,
                    std::ofstream& vars_h, std::ofstream& vars_cpp,
                    std::ofstream& handlers_h, std::ofstream& handlers_cpp);

void writeCoreVarsHead(unsigned, std::ofstream&, std::ofstream&);
void writeCoreVarsFoot(unsigned, std::ofstream&, std::ofstream&);

void writeCoreHandlerHead(unsigned, std::ofstream&, std::ofstream&);
void writeCoreHandlerFoot(unsigned, std::ofstream&, std::ofstream&);


int createThreadFile(P_thread*, std::string&, std::ofstream&);


//unsigned writeThreadVars(
void writeThreadVarsCommon(AddressComponent, AddressComponent,
                            std::ofstream&, std::ofstream&);
void writeThreadContextInitialiser(P_thread*, DevT_t*, size_t,
                            std::ofstream&, std::ofstream&);
void writeDevTDeclInit(AddressComponent, DevT_t*,
                            std::ofstream&, std::ofstream&);
void writeInputPinInit(AddressComponent, DevT_t*,
                            std::ofstream&, std::ofstream&);
void writeOutputPinInit(AddressComponent, DevT_t*,
                            std::ofstream&, std::ofstream&);
void writeDevIDecl(AddressComponent, size_t, std::ofstream&);





void writeDevTSharedCode(DevT_t*, std::ofstream&);


void formHandlerPreamble(devTypStrings_t*);
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
























void writeDevTypeHandlers(P_devtyp*, std::ofstream&, std::ofstream&,
                std::string&, std::string&, std::string&);
void writeInputPinHandlers(P_devtyp*, std::ofstream&, std::ofstream&,
                std::ofstream&, std::string&, std::string&);
void writeOutputPinHandlers(P_devtyp*, std::ofstream&, std::ofstream&,
                std::ofstream&, std::string&, std::string&);


void writeThreadPreamble(unsigned, unsigned, P_thread*,
                            std::ofstream&, std::ofstream&);
void writeThreadCtxInitialiser(unsigned, unsigned, P_thread*,
                            std::ofstream&, std::ofstream&);
void writeDevTypeDeclInit(unsigned, unsigned, P_thread*,
                            std::ofstream&, std::ofstream&);
void writeInputPinInit(unsigned, unsigned, P_thread*,
                            std::ofstream&, std::ofstream&);
void writeOutputPinInit(unsigned, unsigned, P_thread*,
                            std::ofstream&, std::ofstream&);
void writeDevInstInit(unsigned, unsigned, P_thread*,
                            std::ofstream&, std::ofstream&);
*/
};
#endif  // __ComposerH__H
