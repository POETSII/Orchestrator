#include "Composer.h"


// Temporary consts (were in build_defs).
//TODO: reconcile
const string GENERATED_PATH = "Generated";
const string GENERATED_H_PATH = GENERATED_PATH+"/inc";
const string GENERATED_CPP_PATH = GENERATED_PATH+"/src";
const string COREMAKE_BASE = "make -j$(nproc --ignore=4) all 2>&1 >> make_errs.txt";


#if (defined __BORLANDC__ || defined _MSC_VER)
const string SYS_COPY = "copy";
const string MAKEDIR = "md";
const string RECURSIVE_CPY = "";
const string REMOVEDIR = "rd /S /Q";
#elif (defined __GNUC__)
const string SYS_COPY = "cp";
const string RECURSIVE_CPY = "-r";
const string PERMISSION_CPY = "-p";
const string MAKEDIR = "mkdir";
const string REMOVEDIR = "rm --force --recursive";
#endif

const unsigned int MAX_RTSBUFFSIZE = 4096;
const unsigned int MIN_RTSBUFFSIZE = 10;



/******************************************************************************
 * ComposerGraphI_t constructors & destructor
 *****************************************************************************/
ComposerGraphI_t::ComposerGraphI_t()
{
    outputDir = "Composer";
    generated = false;
    compiled = false;
}

ComposerGraphI_t::ComposerGraphI_t(GraphI_t* graphIIn)
{
    graphI = graphIIn;
    outputDir = graphI->Name();
    generated = false;
    compiled = false;
}

ComposerGraphI_t::~ComposerGraphI_t()
{
    generated = false;
    compiled = false;
    
    WALKMAP(DevT_t*, devTypStrings_t*, devTStrsMap, devTStrs)
    {
        delete devTStrs->second;
        devTStrsMap.erase(devTStrs);
    }
}

//Safely clear the DevT strings Map
void ComposerGraphI_t::clearDevTStrsMap()
{
    WALKMAP(DevT_t*, devTypStrings_t*, devTStrsMap, devTStrs)
    {
        delete devTStrs->second;
        devTStrsMap.erase(devTStrs);
    }
}


/******************************************************************************
 * Composer constructors 
 *****************************************************************************/
Composer::Composer()
{
    outputPath = "./";
}

Composer::Composer(Placer* plc)
{
    placer = plc;
    outputPath = "./";
}

Composer::~Composer()
{
    
    // Tidy up the map
    WALKMAP(GraphI_t*, ComposerGraphI_t*, graphIMap, graphISrch)
    {
        delete graphISrch->second;
        graphIMap.erase(graphISrch);
    }
}


/******************************************************************************
 * Change the output directory
 *****************************************************************************/
void Composer::setOutputPath(std::string path)
{
    outputPath = path;
    
    // Make sure we end in / for consistency
    if(*outputPath.rbegin() != '/' && *outputPath.rbegin() != '\\')
    {
        outputPath += "/";
    }

    //TODO: invalidate any generated but not compiled 
}


/******************************************************************************
 * Change the placer to a new one
 *****************************************************************************/
void Composer::setPlacer(Placer* plc)
{
    placer = plc;
    //TODO: invalidate any generated but not compiled 
}


/******************************************************************************
 * Public method to generate and compile
 *****************************************************************************/
int Composer::compose(GraphI_t* graphI)
{
    FILE * fd = graphI->par->par->fd;              // Detail output file
    fprintf(fd,"\nComposing %s...\n",graphI->par->Name().c_str());
    
    int ret = generate(graphI);
    
    if(ret) return ret;
    else return compile(graphI);
}

/******************************************************************************
 * Public method to generate source files required for a Graph instance
 *****************************************************************************/
int Composer::generate(GraphI_t* graphI)
{
    ComposerGraphI_t* builderGraphI;
    FILE * fd = graphI->par->par->fd;              // Detail output file
    
    ComposerGraphIMap_t::iterator srch = graphIMap.find(graphI);
    if (srch == graphIMap.end()) 
    {   // The Graph Instance has not been seen before, map it.
        builderGraphI = new ComposerGraphI_t(graphI);
        
        // Insert the GraphI
        std::pair<ComposerGraphIMap_t::iterator, bool> insertedGraphI;
        insertedGraphI = graphIMap.insert(ComposerGraphIMap_t::value_type
                                                (graphI, builderGraphI));
        
    } else {
        builderGraphI = srch->second;
    }
    
    if(builderGraphI->generated)
    {   // Already generated, nothing to do
        fprintf(fd,"\tApplication already generated, skipping\n");
        return 0;
    }
    
    
    // Prepare the directories
    if(prepareDirectories(builderGraphI))
    {
        //Directory prepare failed. TODO: BARF
        fprintf(fd,"\t***FAILED TO PREPARE DIRECTORIES***\n");
        return -1;
    }
    
    // Get the list of cores
    std::map<GraphI_t*, std::set<P_core*> >::iterator giToCoresFinder;
    giToCoresFinder = placer->giToCores.find(graphI);
    if (giToCoresFinder == placer->giToCores.end())
    {   // Something has gone horribly wrong
        //TODO: Barf
        fprintf(fd,"\t***FAILED TO FIND ANY CORES***\n");
        return -2;
    }
    builderGraphI->cores = &(giToCoresFinder->second);
    
    
    //Form Device Type strings for all DevTs in the GraphT
    builderGraphI->clearDevTStrsMap();      // sanity clear
    WALKVECTOR(DevT_t*,graphI->pT->DevT_v,devT)
    {
        if((*devT)->devTyp == 'D')
        {
            formDevTStrings(builderGraphI, (*devT));
        }
    }
    
    
    //Write global properties and message format headers
    std::ofstream props_h, pkt_h;
    
    std::stringstream props_hFName;
    props_hFName << outputPath << builderGraphI->outputDir;
    props_hFName << "/" << GENERATED_PATH;
    props_hFName << "/GlobalProperties.h";
    props_h.open(props_hFName.str().c_str());
    if(props_h.fail()) // Check that the file opened
    {                 // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, vars_hFName.str(), OSFixes::getSysErrorString(errno));
        return -1;
    }
    writeGlobalPropsD(graphI, props_h);
    props_h.close();
    
    std::stringstream pkt_hFName;
    pkt_hFName << outputPath << builderGraphI->outputDir;
    pkt_hFName << "/" << GENERATED_PATH;
    pkt_hFName << "/MessageFormats.h";
    pkt_h.open(pkt_hFName.str().c_str());
    if(pkt_h.fail()) // Check that the file opened
    {                 // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, vars_hFName.str(), OSFixes::getSysErrorString(errno));
        return -1;
    }
    writeMessageTypes(graphI, pkt_h);
    pkt_h.close();
    
    
    //Generate Supervisor, inc Dev> Super map
    
    fprintf(fd,"\tGenerating Supervisor...");
    generateSupervisor(builderGraphI);
    fprintf(fd,"\tDone!\n");
    
    
    P_core* pCore;                      // Core available during iteration.
    devTypStrings_t* dTStrs;
    DevT_t* devT;
    
    fprintf(fd,"\tGenerating code for %lu cores\n",builderGraphI->cores->size());
    
    WALKSET(P_core*,(*(builderGraphI->cores)),coreNode)
    {
        pCore = (*coreNode);
        
        fprintf(fd,"\tCore %lu: ", 
          static_cast<unsigned long>(pCore->get_hardware_address()->as_uint()));
        
        //TODO:
        //mailboxCoreId = pCore->get_hardware_address()->get_mailbox();
        //mailboxCoreId += pCore->get_hardware_address()->get_core();
        
        /*======================================================================
         * Find the device type strings for this core 
         *======================================================================
         * We can abuse two facts to get the DevT:
         *      A core only hosts one device type
         *      Something will be in P_threadm for any core in giToCores
         */
        if(pCore->P_threadm.size() == 0) continue;  // pointless(?) sanity check
        
        std::map<AddressComponent, P_thread*>::iterator coreThread;
        coreThread = pCore->P_threadm.begin();
        
        
        std::list<DevI_t*> devLst = placer->threadToDevices[coreThread->second];
        if(devLst.size() == 0) continue;            // pointless(?) sanity check
        
        DevI_t* devI = *devLst.begin();
        devT = devI->pT;
        
        
        devTStrsMap_t::iterator devTStrsIter;
        devTStrsIter = builderGraphI->devTStrsMap.find(devT);
        
        if (devTStrsIter == builderGraphI->devTStrsMap.end()) 
        {   // Something has gone horribly horribly wrong
            //TODO: Barf
            fprintf(fd,"\t***FAILED TO FIND DevT STRINGS***\n");
            return -3;
        }
        
        dTStrs = devTStrsIter->second;
        
        
        //======================================================================
        // Create empty files for the per-core handlers and variables
        //======================================================================
        std::ofstream vars_h, vars_cpp, handlers_h, handlers_cpp;
        
        
        fprintf(fd,"Files... ");                
        if (createCoreFiles(pCore, builderGraphI->outputDir, vars_h, vars_cpp, 
                            handlers_h, handlers_cpp))
        {
            fprintf(fd,"\t***FAILED TO CREATE CORE FILES***\n");
            return 1;
        }
        
        
        //======================================================================
        // Write per-core source.
        //======================================================================
        fprintf(fd,"Source... "); 
        writeCoreSrc(pCore, dTStrs, vars_h, vars_cpp, handlers_h, handlers_cpp);
        
        handlers_h.close();
        handlers_cpp.close();
        vars_cpp.close();
        
        
        
        //======================================================================
        // Now for the awkward bit: thread variables!
        //======================================================================
        std::ofstream thread_vars_cpp;
        
        WALKMAP(AddressComponent,P_thread*,pCore->P_threadm,threadIterator)
        {   // Iterate over the threads on this core
            
            P_thread* pThread = threadIterator->second;
            
            if (placer->threadToDevices[pThread].size())
            {   // if there are devices placed on the thread, generate source
                
                if (createThreadFile(pThread, builderGraphI->outputDir,
                                        thread_vars_cpp))
                {
                    vars_h.close();
                    return 1;
                }
                
                
                writeThreadVars(builderGraphI,pThread,vars_h,thread_vars_cpp);
                
                thread_vars_cpp.close();
            }
        }
        
        writeCoreVarsFoot(pCore->get_hardware_address()->as_uint(), vars_h);
        vars_h.close();
        
        fprintf(fd,"\n"); 
    }
    
    builderGraphI->generated = true;

    return 0;
}


/******************************************************************************
 * Public method to compile previously generated files for a Graph instance
 *****************************************************************************/
int Composer::compile(GraphI_t* graphI)
{
    ComposerGraphI_t*  builderGraphI;
    FILE * fd = graphI->par->par->fd;              // Detail output file
    
    ComposerGraphIMap_t::iterator srch = graphIMap.find(graphI);
    if (srch == graphIMap.end()) 
    {   // The Graph Instance has not been seen before, barf.
        return -1;
    }
    builderGraphI = srch->second;
    
    
    if(builderGraphI->compiled)
    {   // Already compiled, nothing to do
        fprintf(fd,"\tApplication already compiled, skipping\n");
        return 0;
    }
    
    // Make the bin directory
    std::string taskDir(outputPath + builderGraphI->outputDir);
    std::string mkdirBinPath(taskDir + "/bin");
    if(system((MAKEDIR + " " + mkdirBinPath).c_str()))
    {
        //par->Post(818, mkdirBinPath, OSFixes::getSysErrorString(errno));
        fprintf(fd,"\tFailed to create %s\n",mkdirBinPath.c_str());
        return 1;
    }
    
    
    // Make the magic happen: call make
    std::string buildPath = outputPath + builderGraphI->outputDir;
    buildPath += "/Build";
    
    if(system(("(cd "+buildPath+";"+COREMAKE_BASE+")").c_str()))
    {
        //TODO: barf
        return 1;
    }
    
    builderGraphI->compiled = true;
    
    return 0;
}

/******************************************************************************
 * Stubs for now        TODO
 *****************************************************************************/
void Composer::decompose(GraphI_t* graphI)
{
    
}


void Composer::clean(GraphI_t* graphI)
{   // Tidy up a specific build for a graphI. This invokes make clean
    
}

void Composer::reset()
{
    
}


/******************************************************************************
 * Prepare directories for source code generation and compilation
 *****************************************************************************/
int Composer::prepareDirectories(ComposerGraphI_t* builderGraphI)
{
    GraphI_t* graphI = builderGraphI->graphI;
    FILE * fd = graphI->par->par->fd;              // Detail output file
    
    //TODO: a lot of this system() calling needs to be made more cross-platform, and should probably be moved to OSFixes.hpp.
    //TODO: fix the path specifiers for system() calls throughout as they are currently inconsistent on what has a "/" at the end.
    //TODO: make the system() path specifiers cross-platform - these will fall over on Windows.

    //==========================================================================
    // Remove the task directory and recreate it to clean any previous build.
    //==========================================================================
    //TODO: make this safer. Currently the remove uses an "rm -rf" without any safety.
    std::string taskDir(outputPath + builderGraphI->outputDir); 
    if(system((REMOVEDIR+" "+taskDir).c_str())) // Check that the directory deleted
    {                                  // if it didn't, tell logserver and exit
        //par->Post(817, task_dir, OSFixes::getSysErrorString(errno));
        fprintf(fd,"\tFailed to remove %s\n",taskDir.c_str());
        return 1;
    }

    if(system((MAKEDIR+" "+ taskDir).c_str()))// Check that the directory created
    {                                 // if it didn't, tell logserver and exit
        //par->Post(818, (task_dir), OSFixes::getSysErrorString(errno));
        fprintf(fd,"\tFailed to create %s\n",taskDir.c_str());
        return 1;
    }
    
    
    //==========================================================================
    // Create the directory for the generated code and the src and inc dirs
    // below it. If any of these fail, tell the logserver and bail.
    //==========================================================================
    std::string mkdirGenPath(taskDir + "/" + GENERATED_PATH);
    if(system((MAKEDIR + " " + mkdirGenPath).c_str()))
    {
        //par->Post(818, mkdirGenPath, OSFixes::getSysErrorString(errno));
        fprintf(fd,"\tFailed to create %s\n",mkdirGenPath.c_str());
        return 1;
    }
    
    std::string mkdirGenHPath(taskDir + "/" + GENERATED_H_PATH);
    if(system((MAKEDIR + " " + mkdirGenHPath).c_str()))
    {
        //par->Post(818, mkdirGenHPath, OSFixes::getSysErrorString(errno));
        fprintf(fd,"\tFailed to create %s\n",mkdirGenHPath.c_str());
        return 1;
    }
    
    std::string mkdirGenCPath(taskDir + "/" + GENERATED_CPP_PATH);
    if(system((MAKEDIR + " " + mkdirGenCPath).c_str()))
    {
        //par->Post(818, mkdirGenCPath, OSFixes::getSysErrorString(errno));
        fprintf(fd,"\tFailed to create %s\n",mkdirGenCPath.c_str());
        return 1;
    }
    
    std::string mkdirSoftswitchPath(taskDir + "/Softswitch");
    if(system((MAKEDIR + " " + mkdirSoftswitchPath).c_str()))
    {
        //par->Post(818, mkdirSoftswitchPath, OSFixes::getSysErrorString(errno));
        fprintf(fd,"\tFailed to create %s\n",mkdirSoftswitchPath.c_str());
        return 1;
    }
    
    std::string mkdirBuildPath(taskDir + "/Build");
    if(system((MAKEDIR + " " + mkdirBuildPath).c_str()))
    {
        //par->Post(818, mkdirBuildPath, OSFixes::getSysErrorString(errno));
        fprintf(fd,"\tFailed to create %s\n",mkdirBuildPath.c_str());
        return 1;
    }
    
    
    //==========================================================================
    // Copy static source
    //
    // We do not copy Softswitch, Tinsel or Orch bits - we will use them from
    // the parent directory
    //==========================================================================
    
    // Copy the default supervisor code into the right place.
    std::stringstream cpCmd;
    
    cpCmd << SYS_COPY << " ";
    cpCmd << outputPath << "Supervisor.* ";                     // Source
    cpCmd << taskDir << "/" << GENERATED_PATH;                  // Destination
    if(system(cpCmd.str().c_str()))
    {
        //par->Post(807, (task_dir+GENERATED_PATH), OSFixes::getSysErrorString(errno));
        fprintf(fd,"\tFailed to execute %s\n",cpCmd.str().c_str());
        return 1;
    }
    
    // Copy Makefile
    cpCmd.str("");
    cpCmd << SYS_COPY << " ";
    cpCmd << outputPath << "Softswitch/Makefile ";
    cpCmd << taskDir << "/Build";
    if(system(cpCmd.str().c_str()))
    {
        //par->Post(807, (task_dir+GENERATED_PATH), OSFixes::getSysErrorString(errno));
        fprintf(fd,"\tFailed to execute %s\n",cpCmd.str().c_str());
        return 1;
    }
    
    return 0;
}

/******************************************************************************
 * Generate the source code for the Supervisor(s).
 *
 * One supervisor is required for each BOX. For now, this is single box
 *****************************************************************************/
int Composer::generateSupervisor(ComposerGraphI_t* builderGraphI)
{
    std::string taskDir = builderGraphI->outputDir;
    GraphI_t* graphI = builderGraphI->graphI;
    
    //Create the graph instance-specific ones
    std::ofstream supervisor_cpp, supervisor_h;
    
    
    // Create the Supervisor header
    std::stringstream supervisor_hFName;
    supervisor_hFName << outputPath << taskDir << "/" << GENERATED_PATH;
    supervisor_hFName << "/supervisor_generated.h";
    
    supervisor_h.open(supervisor_hFName.str().c_str());
    if(supervisor_h.fail()) // Check that the file opened
    {                 // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, vars_hFName.str(), OSFixes::getSysErrorString(errno));
        return -1;
    }
    
    
    // Create the Supervisor source
    std::stringstream supervisor_cppFName;
    supervisor_cppFName << outputPath << taskDir << "/" << GENERATED_PATH;
    supervisor_cppFName << "/supervisor_generated.cpp";
    
    supervisor_cpp.open(supervisor_cppFName.str().c_str());
    if(supervisor_cpp.fail()) // Check that the file opened
    {                 // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, vars_hFName.str(), OSFixes::getSysErrorString(errno));
        return -1;
    }
    
    
    supervisor_h << "#ifndef __SupervisorGeneratedH__H\n";
    supervisor_h << "#define __SupervisorGeneratedH__H\n\n";
    
    supervisor_cpp << "#include \"supervisor_generated.h\"\n";
    supervisor_cpp << "#include \"supervisor.h\"\n\n";
    
    
    // Write the static member initialisors (Supervisor::init and the Device Vector)
    supervisor_cpp << "bool Supervisor::__SupervisorInit = false;\n";
    
    // As part of this, we need to generate an edge index for each device on
    // this supervisor. For now, that is all devices so we (ab)use Digraph.
    // TODO: change for multi supervisor.
    int devIdx = 0; // Faster than using std::distance
    builderGraphI->supevisorDevTVect.clear();       // Sanity clear
    builderGraphI->devISuperIdxMap.clear();         // sanity clear
    
    supervisor_cpp << "const std::vector<SupervisorDeviceInstance_t> ";
    supervisor_cpp << "Supervisor::DeviceVector = {";
    WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,graphI->G,i)
    {
        DevI_t* devI = graphI->G.NodeData(i);
        
        if(devI->pT->devTyp != 'D') continue;
        
        builderGraphI->supevisorDevTVect.push_back(devI);
        builderGraphI->devISuperIdxMap.insert(devISuperIdxMap_t::value_type(devI,devIdx));
        
        // Get the Thread for the HW addr
        std::map<DevI_t*, P_thread*>::iterator threadSrch;
        threadSrch = placer->deviceToThread.find(devI);
        if (threadSrch == placer->deviceToThread.end()) 
        {   // Something has gone terribly terribly wrong
            //TODO: Barf
            
            return -1;
        }
        P_thread* thread = threadSrch->second;
        
    // Write the initialiser directly: {HwAddr, SwAddr, Name}
        if(devIdx%100 == 0) supervisor_cpp << "\n\t";     // Add some line splitting
        supervisor_cpp << "{" << thread->get_hardware_address()->as_uint();
        supervisor_cpp << "," << devI->addr.as_uint();
        supervisor_cpp << ",\"" << devI->Name() <<"\"";
        supervisor_cpp << "},";
        
        devIdx++;
    }
    supervisor_cpp.seekp(-1,ios_base::cur); // Rewind one place to remove the stray ","
    supervisor_cpp << "\n};\n\n";               // properly terminate the initialiser
    
    // Add references for static class members
    supervisor_cpp << "SupervisorProperties_t* ";
    supervisor_cpp << "Supervisor::__SupervisorProperties;\n";
    supervisor_cpp << "SupervisorState_t* Supervisor::__SupervisorState;\n\n";
    
    // Make a global pointer for Supervisot properties and State
    supervisor_cpp << "SupervisorProperties_t* supervisorProperties = ";
    supervisor_cpp << "Supervisor::__SupervisorProperties;\n";
    
    supervisor_cpp << "SupervisorState_t* supervisorState = ";
    supervisor_cpp << "Supervisor::__SupervisorState;\n\n";
    
    
    // Default Supervisor handler strings
    std::string supervisorOnInitHandler = "";
    std::string supervisorOnStopHandler = "";
    std::string supervisorOnImplicitHandler = "return -1;";
    std::string supervisorOnPktHandler = "return -1;";
    std::string supervisorOnCtlHandler = "return 0;";
    std::string supervisorOnIdleHandler = "return 0;";
    std::string supervisorOnRTCLHandler = "return 0;";
    
    
    // Default Properteis, state and message struct content
    std::string supervisorPropertiesBody = "\tbool dummy;";
    std::string supervisorStateBody = "\tbool dummy;";
    std::string supervisorRecvMsgBody = "\tuint8_t pyld[56];";
    
    if(graphI->pT->pSup)    // If we have a non-default Supervisor, build it.
    {        
        SupT_t* supType = graphI->pT->pSup;
        
        supervisor_h << "#define _APPLICATION_SUPERVISOR_ 1\n\n";
        supervisor_h << "#include \"GlobalProperties.h\"\n";
        supervisor_h << "#include \"MessageFormats.h\"\n\n";
        
        
        // Global properties initialiser
        writeGlobalPropsI(graphI, supervisor_cpp);
        
        // Global shared code - written directly
        if(supType->par->pShCd)
        {
            supervisor_cpp << "// =============================== Shared Code ";
            supervisor_cpp << "=================================\n";
            supervisor_cpp << supType->par->pShCd->C_src();
            supervisor_cpp << "\n\n";
        }
 
        // Code section - written directly
        if(supType->pShCd)
        {
            supervisor_cpp << "// =================================== Code ";
            supervisor_cpp << "====================================\n";
            supervisor_cpp << supType->pShCd->C_src();
            supervisor_cpp << "\n\n";
        }
        
        supervisor_cpp << "// ================================= Handlers ";
        supervisor_cpp << "==================================\n";
        
        // Properties
        if(supType->pOnInit)
        {
            supervisorPropertiesBody = supType->pPropsD->C_src();
        }
        
        // State
        if(supType->pOnInit)
        {
            supervisorStateBody = supType->pStateD->C_src();
        }
        
        // Init Handler
        if(supType->pOnInit)
        {
            supervisorOnInitHandler = supType->pOnInit->C_src();
        }
        
        // OnStop handler
        if(supType->pOnStop)
        {
            supervisorOnStopHandler = supType->pOnStop->C_src();
        }
        
        // Implicit receive handler
        if(supType->pPinTSI)
        {
            // Get the body of the Supervisor message
            supervisorRecvMsgBody = supType->pPinTSI->pMsg->pPropsD->C_src();
            
            supervisorOnImplicitHandler = supType->pPinTSI->pHandl->C_src();
        }
        
        // General receive handler
        if(supType->pOnPkt)
        {
            //TODO
        }
        
        // MPI RX handler, essentially stub for now
        if(supType->pOnCTL)
        {
            supervisorOnCtlHandler = supType->pOnCTL->C_src();
        }
        
        // OnSupervisorIdle
        if(supType->pOnDeId)
        {
            supervisorOnIdleHandler = supType->pOnDeId->C_src();
        }
        
        // OnRTCL
        if(supType->pOnRTCL)
        {
            supervisorOnRTCLHandler = supType->pOnRTCL->C_src();
        }
        
    }
    
    // Write out the handlers and structs we just formed
    
    // Supervisor Properties
    supervisor_h << "typedef struct SupervisorProperties_t\n{\n";
    supervisor_h << supervisorPropertiesBody;
    supervisor_h << "\n} SupervisorProperties_t;\n\n";
    
    //State
    supervisor_h << "typedef struct SupervisorState_t\n{\n";
    supervisor_h << supervisorStateBody;
    supervisor_h << "\n} SupervisorState_t;\n\n";
    
    //Implicit Message Type
    supervisor_h << "typedef struct SupervisorImplicitRecvMessage_t\n{\n";
    supervisor_h << supervisorRecvMsgBody;
    supervisor_h << "\n} SupervisorImplicitRecvMessage_t;\n\n";
    
    // Init Handler
    supervisor_cpp << "int Supervisor::OnInit()\n{\n";
    supervisor_cpp << "\tif(__SupervisorInit) return -1;\n";
    supervisor_cpp << "\t__SupervisorProperties = new SupervisorProperties_t;\n";
    supervisor_cpp << "\t__SupervisorState = new SupervisorState_t;\n\n";
    supervisor_cpp << supervisorOnInitHandler;
    supervisor_cpp << "\n\n\t__SupervisorInit = true;\n";
    supervisor_cpp << "\treturn 0;\n";
    supervisor_cpp << "}\n\n";
    
    // Stop handler
    supervisor_cpp << "int Supervisor::OnStop()\n{\n";
    supervisor_cpp << supervisorOnStopHandler;
    supervisor_cpp << "\n\n\tdelete __SupervisorProperties;\n";
    supervisor_cpp << "\tdelete __SupervisorState;\n";
    supervisor_cpp << "\treturn 0;\n";
    supervisor_cpp << "}\n\n";
    
    // OnImplicit
    supervisor_cpp << "int Supervisor::OnImplicit(P_Pkt_t* inMsg){";
    
    supervisor_cpp << "\t const SupervisorProperties_t* SupervisorProperties";
    supervisor_cpp << " OS_ATTRIBUTE_UNUSED= __SupervisorProperties;\n";
    supervisor_cpp << "\tOS_PRAGMA_UNUSED(SupervisorProperties)\n";
    
    supervisor_cpp << "\t SupervisorState_t* SupervisorState";
    supervisor_cpp << " OS_ATTRIBUTE_UNUSED= __SupervisorState;\n";
    supervisor_cpp << "\tOS_PRAGMA_UNUSED(SupervisorState)\n";
    
    supervisor_cpp << "\t const SupervisorImplicitRecvMessage_t* message";
    supervisor_cpp << " OS_ATTRIBUTE_UNUSED= ";
    supervisor_cpp << "static_cast<const SupervisorImplicitRecvMessage_t*>(";
    supervisor_cpp << "static_cast<const void*>(inMsg->payload));\n";
    supervisor_cpp << "\tOS_PRAGMA_UNUSED(message)\n\n";
    
    supervisor_cpp << supervisorOnImplicitHandler;
    supervisor_cpp << "\n\n\treturn 0;\n";
    supervisor_cpp << "}\n\n";
    
    
    // OnPkt
    supervisor_cpp << "int Supervisor::OnPkt(P_Pkt_t* inMsg){";
    
    supervisor_cpp << "\t const SupervisorProperties_t* SupervisorProperties";
    supervisor_cpp << " OS_ATTRIBUTE_UNUSED= __SupervisorProperties;\n";
    supervisor_cpp << "\tOS_PRAGMA_UNUSED(SupervisorProperties)\n";
    
    supervisor_cpp << "\t SupervisorState_t* SupervisorState";
    supervisor_cpp << " OS_ATTRIBUTE_UNUSED= __SupervisorState;\n";
    supervisor_cpp << "\tOS_PRAGMA_UNUSED(SupervisorState)\n";
    
    supervisor_cpp << "\t const SupervisorImplicitRecvMessage_t* message";
    supervisor_cpp << " OS_ATTRIBUTE_UNUSED= ";
    supervisor_cpp << "static_cast<const SupervisorImplicitRecvMessage_t*>(";
    supervisor_cpp << "static_cast<const void*>(inMsg->payload));\n";
    supervisor_cpp << "\tOS_PRAGMA_UNUSED(message)\n\n";
    
    supervisor_cpp << supervisorOnPktHandler;
    supervisor_cpp << "\n\n\treturn 0;\n";
    supervisor_cpp << "}\n\n";
    
    // OnCTL
    supervisor_cpp << "int Supervisor::OnCtl(){";
    supervisor_cpp << supervisorOnCtlHandler;
    supervisor_cpp << "\n\n\treturn 0;\n";
    supervisor_cpp << "}\n\n";
    
    // OnIdle
    supervisor_cpp << "int Supervisor::OnIdle(){";
    supervisor_cpp << supervisorOnIdleHandler;
    supervisor_cpp << "\n\n\treturn 0;\n";
    supervisor_cpp << "}\n\n";
    
    //OnRTCL
    supervisor_cpp << "int Supervisor::OnRTCL(){";
    supervisor_cpp << supervisorOnRTCLHandler;
    supervisor_cpp << "\n\n\treturn 0;\n";
    supervisor_cpp << "}\n\n";
    
    
    // Close Supervisor files.
    supervisor_cpp << "\n";
    supervisor_cpp.close();
    
    supervisor_h << "\n#endif\n";
    supervisor_h.close();
    
    return 0;
}


/******************************************************************************
 * Write Global Shared Code
 *****************************************************************************/
void writeGlobalSharedCode(GraphI_t* graphI, std::stringstream& handlers_cpp)
{
    GraphT_t* graphT = graphI->pT;
    
    // There may be some global shared code
    if(graphT->pShCd)
    {
        handlers_cpp << (graphT->pShCd->C_src()) << "\n";
    }
}


/******************************************************************************
 * Write the Global Properties common header
 *****************************************************************************/
void Composer::writeGlobalPropsD(GraphI_t* graphI, std::ofstream& props_h)
{
    GraphT_t* graphT = graphI->pT;
    
    props_h << "#ifndef _GLOBALPROPS_H_\n";
    props_h << "#define _GLOBALPROPS_H_\n\n";
    
    props_h << "#include <cstdint>\n";
    //props_h << "#include \"softswitch_common.h\"\n\n";
    
    if(graphT->pPropsD)
    {
        //Definition comes from the Graph Type
        props_h << "typedef struct " << graphT->Name() << "_properties_t \n{\n";
        props_h << graphT->pPropsD->C_src();   // These are the actual properties
        props_h << "\n} global_props_t;\n\n";
        props_h << "extern const global_props_t graphProperties;\n\n";
    }
    
    props_h << "#endif /*_GLOBALPROPS_H_*/\n\n";
}


/******************************************************************************
 * Write the Global Properties initialiser
 *****************************************************************************/
void Composer::writeGlobalPropsI(GraphI_t* graphI, std::ofstream& props_cpp)
{
    //There may be an initialiser in the Graph Instance
    if(graphI->pPropsI)
    {
        props_cpp << "const global_props_t graphProperties ";
        props_cpp << "OS_ATTRIBUTE_UNUSED= {";
        props_cpp << graphI->pPropsI->C_src() << "};\n";
        props_cpp << "OS_PRAGMA_UNUSED(graphProperties)\n";
    }
} 


/******************************************************************************
 * Write the Message structs to a common header
 *****************************************************************************/
void Composer::writeMessageTypes(GraphI_t* graphI, std::ofstream& pkt_h)
{
    GraphT_t* graphT = graphI->pT;
    
    pkt_h << "#ifndef _MESSAGETYPES_H_\n";
    pkt_h << "#define _MESSAGETYPES_H_\n\n";
    
    pkt_h << "#include <cstdint>\n";
    //pkt_h << "#include \"softswitch_common.h\"\n\n";
    
    WALKVECTOR(MsgT_t*,graphT->MsgT_v,msg)
    {
        pkt_h << "typedef struct " << graphT->Name() << "_" << (*msg)->Name();
        pkt_h << "_message_t \n{\n";
        pkt_h << (*msg)->pPropsD->C_src();       // The message struct
        pkt_h << "\n} pkt_" << (*msg)->Name() << "_pyld_t;;\n\n";
    }
    
    pkt_h << "#endif /*_MESSAGETYPES_H_*/\n\n";
}







/******************************************************************************
 * Form the common handler strings for a device type if they do not exist
 *****************************************************************************/
void Composer::formDevTStrings(ComposerGraphI_t* builderGraphI, DevT_t* devT)
{
    GraphI_t* graphI = builderGraphI->graphI;
    
    devTStrsMap_t::iterator srch = builderGraphI->devTStrsMap.find(devT);
    if (srch == builderGraphI->devTStrsMap.end()) 
    {
        // Device type does not have existing strings, form them.
        devTypStrings_t* dTypStrs = new devTypStrings_t;
        
        dTypStrs->devT = devT;
        dTypStrs->graphI = graphI;
        
        formHandlerPreamble(dTypStrs);
        formDevTHandlers(dTypStrs);
        formDevTPropsDStateD(dTypStrs);
        formDevTInputPinHandlers(dTypStrs);
        formDevTOutputPinHandlers(dTypStrs);
        
        builderGraphI->devTStrsMap.insert(devTStrsMap_t::value_type(devT, dTypStrs));
    }
}

/******************************************************************************
 * Add map entries for the indicies of pin types in the device type's in PinTI_v
 *****************************************************************************/
/*void Composer::populatePinTIdxMap(DevT_t* devT)
{
    unsigned pinIdx = 0;        // Quicker than std::distance
    
    WALKVECTOR(PinT_t*,devT->PinTI_v,pinI)
    {
        // Sanity check that we don't alreadyy exist
        pinTIdxMap_t::iterator srch = pinTIdxMap.find((*pinI));
        if (srch == pinTIdxMap.end()) 
        {
            //Cool, add the Pin Type.
            pinTIdxMap.insert(dpinTIdxMap_t::value_type((*pinI), pinIdx));
        }
        else
        {
            // Somehow the PinT_t is already in the map. IMPOSSIBLE!
            //TODO: Barf
        }
        
        pinIdx++;
    }
}*/

/******************************************************************************
 * Form the common handler preamble (deviceProperties & deviceState)
 *****************************************************************************/
void Composer::formHandlerPreamble(devTypStrings_t* dTypStrs)
{
    DevT_t* devT = dTypStrs->devT;  // grab a local copy of the devtype
    std::string devTName = devT->Name();  // grab a local copy of the name
    
    std::stringstream handlerPreamble_SS("");
    std::stringstream handlerPreambleS_SS("");        // "normal" state
    std::stringstream handlerPreambleCS_SS("");       // const state
    
    handlerPreamble_SS << "{\n";

    if (devT->pPropsD)
    {
        handlerPreamble_SS << "    const global_props_t* graphProperties ";
        handlerPreamble_SS << "OS_ATTRIBUTE_UNUSED= ";
        handlerPreamble_SS << "static_cast<const global_props_t*>";
        handlerPreamble_SS << "(graphProps);\n";
        handlerPreamble_SS << "    OS_PRAGMA_UNUSED(graphProperties)\n";
    }
    handlerPreamble_SS << "   PDeviceInstance* deviceInstance ";
    handlerPreamble_SS << "OS_ATTRIBUTE_UNUSED= ";
    handlerPreamble_SS << "static_cast<PDeviceInstance*>(device);\n";
    handlerPreamble_SS << "   OS_PRAGMA_UNUSED(deviceInstance)\n";

    // deviceProperties (with unused variable handling)
    if (devT->pPropsD)
    {
        handlerPreamble_SS << "    const devtyp_" << devTName;
        handlerPreamble_SS << "_props_t* deviceProperties ";
        handlerPreamble_SS << "OS_ATTRIBUTE_UNUSED= ";
        handlerPreamble_SS << "static_cast<const devtyp_";
        handlerPreamble_SS << devTName;
        handlerPreamble_SS << "_props_t*>(deviceInstance->properties);\n";
        handlerPreamble_SS << "    OS_PRAGMA_UNUSED(deviceProperties)\n";
    }

    // deviceState (with unused variable handling)
    if (devT->pStateD)
    {
        // Const-protected state
        handlerPreambleCS_SS << "    const devtyp_" << devTName;
        handlerPreambleCS_SS << "_state_t* deviceState ";
        handlerPreambleCS_SS << "OS_ATTRIBUTE_UNUSED= ";
        handlerPreambleCS_SS << "static_cast<devtyp_";
        handlerPreambleCS_SS << devTName;
        handlerPreambleCS_SS << "_state_t*>(deviceInstance->state);\n";
        handlerPreambleCS_SS << "    OS_PRAGMA_UNUSED(deviceState)\n";
        
        // "normal" state
        handlerPreambleS_SS << "    devtyp_" << devTName;
        handlerPreambleS_SS << "_state_t* deviceState ";
        handlerPreambleS_SS << "OS_ATTRIBUTE_UNUSED= ";
        handlerPreambleS_SS << "static_cast<devtyp_";
        handlerPreambleS_SS << devTName;
        handlerPreambleS_SS << "_state_t*>(deviceInstance->state);\n";
        handlerPreambleS_SS << "    OS_PRAGMA_UNUSED(deviceState)\n";
    }
    
    dTypStrs->handlerPreamble = handlerPreamble_SS.str();
    dTypStrs->handlerPreambleS = handlerPreambleS_SS.str();
    dTypStrs->handlerPreambleCS = handlerPreambleCS_SS.str();
}


/******************************************************************************
 * Form Device Type Handler strings
 *****************************************************************************/
void Composer::formDevTHandlers(devTypStrings_t* dTypStrs)
{
    DevT_t* devT = dTypStrs->devT;  // grab a local copy of the devtype
    GraphT_t* graphT = devT->par;
    std::string devTName = devT->Name();  // grab a local copy of the name
    
    std::stringstream handlers_h("");
    std::stringstream handlers_cpp("");
    
    
    
    // There may be some global shared code
    if(graphT->pShCd)
    {
        handlers_cpp << graphT->pShCd->C_src() << "\n";
    }
    
    // There may be some shared code
    if(devT->pShCd)
    {
        handlers_cpp << devT->pShCd->C_src() << "\n";
    }
    
    // Now write the Device Type handlers
    
    // ReadyToSend 
    handlers_h << "uint32_t devtyp_" << devTName;
    handlers_h << "_RTS_handler (const void* graphProps, ";
    handlers_h << "void* device, uint32_t* readyToSend);\n";

    handlers_cpp << "uint32_t devtyp_" << devTName;
    handlers_cpp << "_RTS_handler (const void* graphProps, ";
    handlers_cpp << "void* device, uint32_t* readyToSend)\n";
    handlers_cpp << dTypStrs->handlerPreamble;
    handlers_cpp << dTypStrs->handlerPreambleCS;
    handlers_cpp << devT->pOnRTS->C_src() << "\n";
    // we assume here the return value is intended to be an RTS bitmap.
    handlers_cpp << "    return *readyToSend;\n"; 
    handlers_cpp << "}\n\n";
    
    
    // OnInit
    handlers_h << "uint32_t devtyp_" << devTName;
    handlers_h << "_OnInit_handler (const void* graphProps, ";
    handlers_h << "void* device);\n";

    handlers_cpp << "uint32_t devtyp_" << devTName;
    handlers_cpp << "_OnInit_handler (const void* graphProps, ";
    handlers_cpp << "void* device)\n";
    handlers_cpp << dTypStrs->handlerPreamble;
    handlers_cpp << dTypStrs->handlerPreambleS << "\n";

    if (devT->pOnInit) // insert the OnHWIdle handler if there is one
    {
        handlers_cpp << devT->pOnInit->C_src() << "\n";
    }
    else handlers_cpp << "    return 0;\n"; // or a stub if not
    
    handlers_cpp << "}\n\n";
    
    
    // OnIdle
    handlers_h << "uint32_t devtyp_" << devTName;
    handlers_h << "_OnIdle_handler (const void* graphProps, ";
    handlers_h << "void* device);\n";

    handlers_cpp << "uint32_t devtyp_" << devTName;
    handlers_cpp << "_OnIdle_handler (const void* graphProps, ";
    handlers_cpp << "void* device)\n";
    handlers_cpp << dTypStrs->handlerPreamble;
    handlers_cpp << dTypStrs->handlerPreambleS << "\n";

    if (devT->pOnDeId) // insert the OnIdle handler if there is one
    {
        handlers_cpp << devT->pOnDeId->C_src() << "\n";
        handlers_cpp << "    return 1;\n";  // Default return 1
    }
    else handlers_cpp << "    return 0;\n"; // or a stub if not
    handlers_cpp << "}\n\n";
    
    
    // OnHWIdle
    handlers_h << "uint32_t devtyp_" << devTName;
    handlers_h << "_OnHWIdle_handler (const void* graphProps, ";
    handlers_h << "void* device);\n";

    handlers_cpp << "uint32_t devtyp_" << devTName;
    handlers_cpp << "_OnHWIdle_handler (const void* graphProps, ";
    handlers_cpp << "void* device)\n";
    handlers_cpp << dTypStrs->handlerPreamble;
    handlers_cpp << dTypStrs->handlerPreambleS << "\n";

    if (devT->pOnHWId) // insert the OnHWIdle handler if there is one
    {
        handlers_cpp << devT->pOnHWId->C_src() << "\n";
        handlers_cpp << "    return 1;\n";  // Default return 1
    }
    else handlers_cpp << "    return 0;\n"; // or a stub if not
    handlers_cpp << "}\n\n";
    
    
    // OnCtl
    handlers_h << "uint32_t devtyp_" << devTName;
    handlers_h << "_OnCtl_handler (const void* graphProps, ";
    handlers_h << "void* device, uint8_t opcide, const void* pkt);\n";

    handlers_cpp << "uint32_t devtyp_" << devTName;
    handlers_cpp << "_OnCtl_handler (const void* graphProps, ";
    handlers_cpp << "void* device, uint8_t opcide, const void* pkt)\n";
    handlers_cpp << dTypStrs->handlerPreamble;
    handlers_cpp << dTypStrs->handlerPreambleS << "\n";
    
    /*
    if (devT->pOnCtl) // insert the OnCtl handler if there is one
    {
        handlers_cpp << devT->pOnCtl->C_src() << "\n";
        handlers_cpp << "    return 1;\n";  // Default return 1
    }
    else handlers_cpp << "   return 0;\n"; // or a stub if not
    */
    handlers_cpp << "    return 0;\n"; // or a stub if not
    
    handlers_cpp << "}\n\n";
    
    
    
    // Append to the strings
    dTypStrs->handlersH += handlers_h.str();
    dTypStrs->handlersC += handlers_cpp.str();
}


/******************************************************************************
 * Form Device Type Properties and State declarations
 *****************************************************************************/
void Composer::formDevTPropsDStateD(devTypStrings_t* dTypStrs)
{
    DevT_t* devT = dTypStrs->devT;  // grab a local copy of the devtype
    std::string devTName = devT->Name();  // grab a local copy of the name
    
    std::stringstream vars_h("");
    
    // Write Properties declaration
    if (devT->pPropsD)
    {
        vars_h << "typedef struct " << devTName << "_properties_t \n{\n";
        vars_h << devT->pPropsD->C_src();
        vars_h << "\n} devtyp_" << devTName << "_props_t;\n\n";
    }
    
    // Write State declaration
    if (devT->pStateD)
    {
        vars_h << "typedef struct " << devTName << "_state_t \n{\n";
        vars_h << devT->pStateD->C_src();
        vars_h << "\n} devtyp_" << devTName << "_state_t;\n\n";
    }
    
    // Append to the strings
    dTypStrs->varsHCommon += vars_h.str();
}


/******************************************************************************
 * Form Device Type Input Pin handler strings
 *****************************************************************************/
void Composer::formDevTInputPinHandlers(devTypStrings_t* dTypStrs)
{
    DevT_t* devT = dTypStrs->devT;  // grab a local copy of the devtype
    std::string devTName = devT->Name();  // grab a local copy of the name
    
    std::stringstream handlers_h("");
    std::stringstream handlers_cpp("");
    std::stringstream vars_h("");
    
    
    
    WALKVECTOR(PinT_t*,devT->PinTI_v,pinI)
    {
        std::string pinIName = (*pinI)->Name();

        handlers_h << "uint32_t devtyp_" << devTName;
        handlers_h << "_InPin_" << pinIName;
        handlers_h << "_Recv_handler (const void* graphProps, ";
        handlers_h << "void* device, void* edge, const void* pkt);\n";

        handlers_cpp << "uint32_t devtyp_" << devTName;
        handlers_cpp << "_InPin_" << pinIName;
        handlers_cpp << "_Recv_handler (const void* graphProps, ";
        handlers_cpp << "void* device, void* edge, const void* pkt)\n";
        handlers_cpp << dTypStrs->handlerPreamble;
        handlers_cpp << dTypStrs->handlerPreambleS;
        handlers_cpp << "   inEdge_t* edgeInstance ";
        handlers_cpp << "OS_ATTRIBUTE_UNUSED= ";
        handlers_cpp << "static_cast<inEdge_t*>(edge);\n";
        handlers_cpp << "OS_PRAGMA_UNUSED(edgeInstance)\n";

        if ((*pinI)->pPropsD)
        {
            vars_h << "typedef struct " << devTName;
            vars_h << "_InPin_" << pinIName << "_edgeproperties_t \n{\n";
            vars_h << (*pinI)->pPropsD->C_src();
            vars_h << "\n} devtyp_" << devTName;
            vars_h << "_InPin_" << pinIName << "_props_t;\n\n";

            handlers_cpp << "   const devtyp_" << devTName;
            handlers_cpp << "_InPin_" << pinIName;
            handlers_cpp << "_props_t* edgeProperties ";
            handlers_cpp << "OS_ATTRIBUTE_UNUSED= ";
            handlers_cpp << "static_cast<const devtyp_" << devTName; 
            handlers_cpp << "_InPin_" << pinIName;
            handlers_cpp << "_props_t*>(edgeInstance->properties);\n";
            handlers_cpp << "OS_PRAGMA_UNUSED(edgeProperties)\n";
        }

        if ((*pinI)->pStateD)
        {
            vars_h << "typedef struct " << devTName;
            vars_h << "_InPin_" << pinIName << "_edgestate_t \n{\n";
            vars_h << (*pinI)->pStateD->C_src();
            vars_h << "\n} devtyp_" << devTName;
            vars_h << "_InPin_" << pinIName << "_state_t;\n\n";

            handlers_cpp << "   devtyp_" << devTName;
            handlers_cpp << "_InPin_" << pinIName;
            handlers_cpp << "_state_t* edgeState ";
            handlers_cpp << "OS_ATTRIBUTE_UNUSED= ";
            handlers_cpp << "static_cast<devtyp_" << devTName;
            handlers_cpp << "_InPin_"<< pinIName; 
            handlers_cpp << "_state_t*>(edgeInstance->state);\n";
            handlers_cpp << "OS_PRAGMA_UNUSED(edgeState)\n";
        }

        if ((*pinI)->pMsg->pPropsD) 
        {
            handlers_cpp << "   const pkt_" << (*pinI)->pMsg->Name();
            handlers_cpp << "_pyld_t* message = ";
            handlers_cpp << "static_cast<const pkt_" << (*pinI)->pMsg->Name();
            handlers_cpp << "_pyld_t*>(pkt);\n";
        }

        handlers_cpp << (*pinI)->pHandl->C_src() << "\n";

        // return type is indicated as uint32_t yet in the handlers we see from DBT 
        // no return value is set. Is something expected here?
        handlers_cpp << "   return 0;\n";
        handlers_cpp << "}\n\n";
    }
    
    // Append to the strings
    dTypStrs->handlersH += handlers_h.str();
    dTypStrs->handlersC += handlers_cpp.str();
    dTypStrs->varsHCommon += vars_h.str();
}


/******************************************************************************
 * Form Device Type Output Pin handler strings
 *****************************************************************************/
void Composer::formDevTOutputPinHandlers(devTypStrings_t* dTypStrs)
{
    DevT_t* devT = dTypStrs->devT;  // grab a local copy of the devtype
    std::string devTName = devT->Name();  // grab a local copy of the name
    
    std::stringstream handlers_h("");
    std::stringstream handlers_cpp("");
    std::stringstream vars_h("");
    
    // Device Type has a Supervisor output pin
    if(devT->pPinTSO)
    {
        // To make things simple, we include it in the Output pin array as the
        // LAST entry.
        handlers_h << "const uint32_t RTS_SUPER_IMPLICIT_SEND_INDEX";
        handlers_h << " = " << devT->PinTO_v.size() << ";\n";
        
        handlers_h << "const uint32_t RTS_SUPER_IMPLICIT_SEND_FLAG";
        handlers_h << " = 0x1 << " << devT->PinTO_v.size() << ";\n";
        
        handlers_h << "uint32_t devtyp_" << devTName;
        handlers_h << "_Supervisor_Implicit_OutPin";
        handlers_h << "_Send_handler (const void* graphProps, ";
        handlers_h << "void* device, void* pkt);\n";
        
        handlers_cpp << "uint32_t devtyp_" << devTName;
        handlers_cpp << "_Supervisor_Implicit_OutPin";
        handlers_cpp << "_Send_handler (const void* graphProps, ";
        handlers_cpp << "void* device, void* pkt)\n";

        handlers_cpp << dTypStrs->handlerPreamble;
        handlers_cpp << dTypStrs->handlerPreambleS;
        handlers_cpp << "\n";
        
        if (devT->pPinTSO->pMsg->pPropsD)
        {
            handlers_cpp << "   pkt_" << devT->pPinTSO->pMsg->Name();
            handlers_cpp << "_pyld_t* message = static_cast<pkt_";
            handlers_cpp << devT->pPinTSO->pMsg->Name() << "_pyld_t*>(pkt);\n";
        }
        handlers_cpp << devT->pPinTSO->pHandl->C_src() << "\n";
        
        handlers_cpp << "   return 0;\n";
        handlers_cpp << "}\n\n";
    }
    
    // Other output pins
    WALKVECTOR(PinT_t*,devT->PinTO_v,pinO)
    {
        std::string pinOName = (*pinO)->Name();
        uint32_t pinIdx = (*pinO)->Idx;

        handlers_h << "const uint32_t RTS_INDEX_" << pinOName;
        handlers_h << " = " << pinIdx << ";\n";

        handlers_h << "const uint32_t RTS_FLAG_" << pinOName;
        handlers_h << " = 0x1 << " << pinIdx << ";\n";

        handlers_h << "uint32_t devtyp_" << devTName;
        handlers_h << "_OutPin_" << pinOName;
        handlers_h << "_Send_handler (const void* graphProps, ";
        handlers_h << "void* device, void* pkt);\n";

        handlers_cpp << "uint32_t devtyp_" << devTName;
        handlers_cpp << "_OutPin_" << pinOName;
        handlers_cpp << "_Send_handler (const void* graphProps, ";
        handlers_cpp << "void* device, void* pkt)\n";

        handlers_cpp << dTypStrs->handlerPreamble;
        handlers_cpp << dTypStrs->handlerPreambleS;
        handlers_cpp << "\n";

        if ((*pinO)->pMsg->pPropsD)
        {
            handlers_cpp << "   pkt_" << (*pinO)->pMsg->Name();
            handlers_cpp << "_pyld_t* message = static_cast<pkt_";
            handlers_cpp << (*pinO)->pMsg->Name() << "_pyld_t*>(pkt);\n";
        }
        handlers_cpp << (*pinO)->pHandl->C_src() << "\n";

        // same thing: what is this function expected to return?
        handlers_cpp << "   return 0;\n";
        handlers_cpp << "}\n\n";
    }
    
    // Append to the strings
    dTypStrs->handlersH += handlers_h.str();
    dTypStrs->handlersC += handlers_cpp.str();
    dTypStrs->varsHCommon += vars_h.str();
}


/******************************************************************************
 * Create the per-core header and source files.
 *
 * Arguments are the core number and four references to std::ofstream objects.
 * 
 * Returns 0 for a successful open and -1 (with a logserver Post) on failure.
 *
 * If creation of any of the files fails, all opened ones are closed.
 *****************************************************************************/
int Composer::createCoreFiles(P_core* pCore,
                                std::string& taskDir,
                                std::ofstream& vars_h,
                                std::ofstream& vars_cpp,
                                std::ofstream& handlers_h,
                                std::ofstream& handlers_cpp)
{
    uint32_t coreAddr = pCore->get_hardware_address()->as_uint();
    FILE * fd = pCore->parent->parent->parent->parent->parent->fd;  // Microlog
    
    // Create the vars header
    std::stringstream vars_hFName;
    vars_hFName << outputPath << taskDir << "/" << GENERATED_H_PATH;
    vars_hFName << "/vars_" << coreAddr << ".h";
    
    vars_h.open(vars_hFName.str().c_str());
    
    if(vars_h.fail()) // Check that the file opened
    {                 // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, vars_hFName.str(), OSFixes::getSysErrorString(errno));
        fprintf(fd,"\t\tFailed to open %s\n",vars_hFName.str().c_str());
        return -1;
    }
    
    
    // Create the vars source
    std::stringstream vars_cppFName;
    vars_cppFName << outputPath << taskDir << "/" << GENERATED_CPP_PATH;
    vars_cppFName << "/vars_" << coreAddr << ".cpp";
    
    vars_cpp.open(vars_cppFName.str().c_str());
    
    if(vars_cpp.fail()) // Check that the file opened
    {                   // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, vars_cppFName.str(), OSFixes::getSysErrorString(errno));
        vars_h.close(); // Close the header
        fprintf(fd,"\t\tFailed to open %s\n",vars_cppFName.str().c_str());
        return -1;
    }
    
    // Create the vars header
    std::stringstream handlers_hFName;
    handlers_hFName << outputPath << taskDir << "/" << GENERATED_H_PATH;
    handlers_hFName << "/handlers_" << coreAddr << ".h";
    
    handlers_h.open(handlers_hFName.str().c_str());
    
    if(handlers_h.fail())   // Check that the file opened
    {                       // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, handlers_hFName.str(), OSFixes::getSysErrorString(errno));
        vars_h.close(); // Close the vars header
        vars_cpp.close(); // Close the vars source
        fprintf(fd,"\t\tFailed to open %s\n",handlers_hFName.str().c_str());
        return -1;
    }
    
    
    // Create the vars source
    std::stringstream handlers_cppFName;
    handlers_cppFName << outputPath << taskDir << "/" << GENERATED_CPP_PATH;
    handlers_cppFName << "/handlers_" << coreAddr << ".cpp";
    
    handlers_cpp.open(handlers_cppFName.str().c_str());
    
    if(handlers_cpp.fail()) // Check that the file opened
    {                       // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, handlers_cppFName.str(), OSFixes::getSysErrorString(errno));
        vars_h.close(); // Close the vars header
        vars_cpp.close(); // Close the vars source
        handlers_h.close(); // Close the handler header
        fprintf(fd,"\t\tFailed to open %s\n",handlers_cppFName.str().c_str());
        return -1;
    }
    
    return 0;
}

void Composer::writeCoreSrc(P_core* pCore, devTypStrings_t* dTypStrs,
                         std::ofstream& vars_h, std::ofstream& vars_cpp,
                         std::ofstream& handlers_h, std::ofstream& handlers_cpp)
{
    uint32_t coreAddr = pCore->get_hardware_address()->as_uint();
    
    writeCoreVarsHead(coreAddr, vars_h, vars_cpp);
    writeCoreHandlerHead(coreAddr, handlers_h, handlers_cpp);
    
    
    // Global properties initialiser
    writeGlobalPropsI(dTypStrs->graphI, vars_cpp);
    
    // Write out the common strings for the device type.
    vars_h << dTypStrs->varsHCommon;
    handlers_h << dTypStrs->handlersH;
    handlers_cpp << dTypStrs->handlersC;
    
    writeCoreHandlerFoot(coreAddr, handlers_h, handlers_cpp);
    vars_cpp << "\n";
    
}

/******************************************************************************
 * Write the common header lines of the per-core vars header and source.
 *****************************************************************************/
void Composer::writeCoreVarsHead(AddressComponent coreAddr, 
                                    std::ofstream& vars_h,
                                    std::ofstream& vars_cpp)
{
    vars_h << "#ifndef _VARS_" << coreAddr << "_H_\n";
    vars_h << "#define _VARS_" << coreAddr << "_H_\n";
    vars_h << "#include <cstdint>\n";
    vars_h << "#include \"softswitch_common.h\"\n";
    vars_h << "#include \"MessageFormats.h\"\n";
    vars_h << "#include \"GlobalProperties.h\"\n\n";
    
    vars_cpp << "#include \"vars_" << coreAddr << ".h\"\n";
}

/******************************************************************************
 * Write the common footer lines of the per-core vars header and source.
 *****************************************************************************/
void Composer::writeCoreVarsFoot(AddressComponent coreAddr, 
                                    std::ofstream& vars_h)
{
    vars_h << "#endif /*_VARS_" << coreAddr << "_H_*/\n\n";
}

/******************************************************************************
 * Write the common header lines of the per-core handler header and source.
 *****************************************************************************/
void Composer::writeCoreHandlerHead(AddressComponent coreAddr, 
                                        std::ofstream& handlers_h, 
                                        std::ofstream& handlers_cpp)
{
    handlers_h << "#ifndef _HANDLERS_" << coreAddr << "_H_\n";
    handlers_h << "#define _HANDLERS_" << coreAddr << "_H_\n";
    handlers_h << "#include \"vars_" << coreAddr << ".h\"\n";
    
    handlers_cpp << "#include \"handlers_" << coreAddr << ".h\"\n";
}

/******************************************************************************
 * Write the common footer lines of the per-core handler header and source.
 *****************************************************************************/
void Composer::writeCoreHandlerFoot(AddressComponent coreAddr, 
                                        std::ofstream& handlers_h, 
                                        std::ofstream& handlers_cpp)
{
    handlers_h << "#endif /*_HANDLERS_" << coreAddr << "_H_*/\n\n";
}




/******************************************************************************
 * Create the per-thread vars source file.
 *
 * Arguments are the core number and four references to std::ofstream objects.
 * 
 * Returns 0 for a successful open and -1 (with a logserver Post) on failure.
 *
 * If creation of any of the files fails, all opened ones are closed.
 *****************************************************************************/
int Composer::createThreadFile(P_thread* pThread, std::string& taskDir,
                                std::ofstream& tvars_cpp)
{
    uint32_t coreAddr = pThread->parent->get_hardware_address()->as_uint();
    uint32_t threadAddr = pThread->get_hardware_address()->get_thread();
    
    // Create the vars source
    std::stringstream tvars_cppFName;
    tvars_cppFName << outputPath << taskDir << "/" << GENERATED_CPP_PATH;
    tvars_cppFName << "/vars_" << coreAddr;
    tvars_cppFName << "_" << threadAddr << ".cpp";
    
    tvars_cpp.open(tvars_cppFName.str().c_str());
    
    if(tvars_cpp.fail()) // Check that the file opened
    {                       // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, tvars_cppFName.str(), OSFixes::getSysErrorString(errno));
        return -1;
    }
    
    return 0;
}

/******************************************************************************
 * Write the common footer lines of the per-core handler header and source.
 *****************************************************************************/
unsigned Composer::writeThreadVars(ComposerGraphI_t* builderGraphI,
                                    P_thread* thread,
                                    ofstream& vars_h, ofstream& thread_vars_cpp)
{
    AddressComponent coreAddr = thread->parent->get_hardware_address()->as_uint();
    AddressComponent threadAddr = thread->get_hardware_address()->get_thread();
    
    DevI_t* dev = (*(placer->threadToDevices.at(thread).begin()));
    DevT_t* devT = dev->pT;
    
    
    std::list<DevI_t*>::size_type numberOfDevices =
        placer->threadToDevices.at(thread).size();  // Get the thread dev count
    
    
    
    writeThreadVarsCommon(coreAddr, threadAddr, vars_h, thread_vars_cpp);
    
    writeThreadContextInitialiser(thread, devT, vars_h, thread_vars_cpp);
    
    writeDevTDeclInit(threadAddr, devT, vars_h, thread_vars_cpp);
    
    writeInputPinInit(threadAddr, devT, vars_h, thread_vars_cpp);
    
    writeOutputPinInit(threadAddr, devT, vars_h, thread_vars_cpp);
    
    writeDevIDecl(threadAddr, numberOfDevices, vars_h);
    
    writeThreadDevIDefs(builderGraphI, thread, numberOfDevices,
                        vars_h, thread_vars_cpp);
    
    return 0;
}

/******************************************************************************
 * Write the common header lines of the per-thread vars header.
 *****************************************************************************/
void Composer::writeThreadVarsCommon(AddressComponent coreAddr,
                                        AddressComponent threadAddr,
                                        std::ofstream& vars_h,
                                        std::ofstream& thread_vars_cpp)
{
    vars_h << "\n";
    vars_h << "//-------------------- ";
    vars_h << "Core " << coreAddr << " Thread " << threadAddr << " variables";
    vars_h << " --------------------\n";
    vars_h << "extern ThreadCtxt_t Thread_" << threadAddr << "_Context;\n";
    
    
    thread_vars_cpp << "#include \"vars_" << coreAddr << ".h\"\n";
    thread_vars_cpp << "#include \"handlers_" << coreAddr << ".h\"\n\n";
    thread_vars_cpp << "//-------------------- Core " << coreAddr;
    thread_vars_cpp << " Thread " << threadAddr;
    thread_vars_cpp << " variables --------------------\n";
}


/******************************************************************************
 * Generate the ThreadContext initialiser. PThreadContext/ThreadCtxt_t struct
 *****************************************************************************/
void Composer::writeThreadContextInitialiser(P_thread* thread, DevT_t* devT,
                                        std::ofstream& vars_h,
                                        std::ofstream& vars_cpp)
{
    AddressComponent threadAddr = thread->get_hardware_address()->get_thread();
    std::list<DevI_t*>::size_type numberOfDevices =
        placer->threadToDevices.at(thread).size();  // Get the thread dev count
        
    size_t outTypCnt = devT->PinTO_v.size();      // Number of output pins
    
    vars_cpp << "ThreadCtxt_t Thread_" << threadAddr << "_Context ";              
    vars_cpp << "__attribute__((section (\".thr" << threadAddr << "_base\"))) ";
    vars_cpp << "= {";
    vars_cpp << "1,";                                                 // numDevT
    vars_cpp << "Thread_" << threadAddr << "_DeviceTypes,";           // devTs
    vars_cpp << numberOfDevices <<  ",";                              // numDevI
    vars_cpp << "Thread_" << threadAddr << "_Devices,";               // devIs
    vars_cpp << ((devT->par->pPropsD)?"&graphProperties,":"PNULL,");  // props


    /* Work out the required size for rtsBuffSize: The size of the RTS buffer is
    * dependant on the number of connected output pins hosted on the Softswitch.
    * The size is set to 1 + <number of connected pins> + <number of connected
    * devices (if supervisor pin connected), as long as this is less than
    * MAX_RTSBUFFSIZE, so that each connected pin can have a pending send.
    *
    * The additional slot is required to ensure that the crude, simple wrapping
    * mechanism for the circular buffer does not set rtsEnd to be the same as
    * rtsStart when adding to the buffer. If this occurs, softswitch_IsRTSReady
    * will always return false (as it simply checks that rtsStart != rtsEnd) and
    * no further application-generated packets will be sent by the softswitch 
    * (as softswitch_onRTS will only alter rtsEnd if it adds an entry to the
    * buffer, which it wont do in this case as all pins will already be marked 
    * as send pending).
    *
    * If the buffer size is constrained to MAX_RTSBUFFSIZE, a warning is
    * generated - if this occurs frequently, more graceful handling of rtsBuf
    * overflowing may be required.
    */
    uint32_t outputCount = 1;     // Intentionally 1 to cope with wrapping.
    
    if(devT->pPinTSO)
    {   // There is a supervisor output, increase the output count by dev count.
        outputCount += numberOfDevices;
    }
    
    if (outTypCnt)  // If we have output pins
    {               // Iterate through devices counting connected output pins.
        WALKLIST(DevI_t*, placer->threadToDevices.at(thread), dev)
        {   
            WALKVECTOR(PinT_t*, devT->PinTO_v, pin)
            {
                // Check that we have a connection
                std::map<std::string,PinI_t *>::iterator pinSrch;
                pinSrch = (*dev)->Pmap.find(devT->Name());
                if(pinSrch != (*dev)->Pmap.end())
                {
                    if(pinSrch->second->Key_v.size())
                    {
                        outputCount++;
                        
                        // If 0, we have overflowed & there is no point cont
                        if (outputCount==0) break;    
                    }
                }
            }
            if (outputCount==0)
            {
                outputCount = MAX_RTSBUFFSIZE;
                break;
            }
        }
    }
    
    if (outputCount > MAX_RTSBUFFSIZE)
    { // If we have too many pins for one buffer entry per ping, set to max &warn.
    // This may need a check adding to the Softswitch to stop buffer overflow.
        outputCount = MAX_RTSBUFFSIZE;
        //TODO: Barf
        //par->Post(819,int2str(thread_num),int2str(coreNum),
        //            int2str(MAX_RTSBUFFSIZE), int2str(MAX_RTSBUFFSIZE));
    }
    else if (outputCount < MIN_RTSBUFFSIZE)
    {
        outputCount = MIN_RTSBUFFSIZE;
    }
    
    
    vars_cpp << outputCount << ",";                                   // rtsBuffSize

    vars_cpp << "PNULL,";                                             // rtsBuf
    vars_cpp << "0,";                                                 // rtsStart
    vars_cpp << "0,";                                                 // rtsEnd
    vars_cpp << "0,";                                                 // idleStart
    vars_cpp << "0,";                                                 // ctlEnd

    // Instrumentation
    vars_cpp << "0,";                                 // lastCycles
    vars_cpp << "0,";                                 // pendCycles
    vars_cpp << "0,";                                 // txCount
    vars_cpp << "0,";                                 // superCount
    vars_cpp << "0,";                                 // rxCount
    vars_cpp << "0,";                                 // txHandlerCount
    vars_cpp << "0,";                                 // rxHandlerCount
    vars_cpp << "0,";                                 // idleCount
    vars_cpp << "0,";                                 // idleHandlerCount
    vars_cpp << "0,";                                 // blockCount
    vars_cpp << "0";                                  // cycleIdx
    vars_cpp << "};\n";
}

/******************************************************************************
 * Generate the device type declaration and initialiser.
 *
 * Currently, there is one device-type per thread. This restriction may be 
 * removed in future so we do this to make it easier.
 *
 *****************************************************************************/
void Composer::writeDevTDeclInit(AddressComponent threadAddr, DevT_t* devT,
                                std::ofstream& vars_h, std::ofstream& vars_cpp)
{
    size_t inTypCnt = devT->PinTI_v.size();       // Number of Input pins
    size_t outTypCnt = devT->PinTO_v.size();      // Number of output pins
    
    if(devT->pPinTSO)
    {   // There is a supervisor output, increase the output count by one.
        outTypCnt++;
    }
    
    vars_h << "//------------------------------ Device Type Tables ";
    vars_h << "------------------------------\n";
    vars_h << "extern struct PDeviceType Thread_" << threadAddr;
    vars_h << "_DeviceTypes[1];\n";

    vars_cpp << "devTyp_t Thread_" << threadAddr << "_DeviceTypes[1] ";         //set devTyp_t name
    
    
    vars_cpp << "= {";
    vars_cpp << "&devtyp_" << devT->Name() << "_RTS_handler,";                  // RTS_Handler
    vars_cpp << "&devtyp_" << devT->Name() << "_OnInit_handler,";               // OnInit_Handler
    vars_cpp << "&devtyp_" << devT->Name() << "_OnIdle_handler,";               // OnIdle_Handler
    vars_cpp << "&devtyp_" << devT->Name() << "_OnHWIdle_handler,";             // OnHWIdle_Handler
    vars_cpp << "&devtyp_" << devT->Name() << "_OnCtl_handler,";                // OnCtl_Handler
    
    //TODO: Add v4 handlers
    
    if(devT->pPropsD)
        vars_cpp << "sizeof(devtyp_" << devT->Name() << "_props_t),";           // sz_props
    else vars_cpp << "0,";
    if(devT->pStateD)
        vars_cpp << "sizeof(devtyp_" << devT->Name() << "_state_t),";           // sz_state
    else vars_cpp << "0,";

    vars_cpp << inTypCnt << ",";                                                // numInputTypes
    if(inTypCnt) vars_cpp << "Thread_" << threadAddr << "_DevTyp_0_InputPins,"; // inputTypes
    else vars_cpp << "PNULL,";

    vars_cpp << outTypCnt << ",";                                               // numOutputTypes
    if(outTypCnt) vars_cpp << "Thread_" << threadAddr << "_DevTyp_0_OutputPins";// outputTypes
    else vars_cpp << "PNULL,";

    vars_cpp << "};\n";
}

/******************************************************************************
 * Form the initialiser(s) for the input pins array if we have input pins
 *****************************************************************************/
void Composer::writeInputPinInit(AddressComponent threadAddr, DevT_t* devT,
                                std::ofstream& vars_h, std::ofstream& vars_cpp)
{
    unsigned int inTypCnt = devT->PinTI_v.size();       // Number of Input pins
    std::stringstream initialiser;

    vars_h << "//------------------------------ Pin Type Tables ";
    vars_h << "-------------------------------\n";
    
    if (inTypCnt)
    {
        // Add declaration for the input pins array to relevant vars header
        vars_h << "extern in_pintyp_t Thread_" << threadAddr;       
        vars_h << "_DevTyp_0_InputPins[" << inTypCnt << "];\n";
        
        // Build the dev type input pin name string
        std::string dTypInPin = std::string("devtyp_");                             
        dTypInPin += devT->Name() + std::string("_InPin_");
        
        initialiser.str("");    // Clear the initialiser
        
        initialiser << "{";
        WALKVECTOR(PinT_t*, devT->PinTI_v, ipin) // Build pin initialiser
        {
            initialiser << "{";
            initialiser << "&" << dTypInPin;
            initialiser << (*ipin)->Name() << "_Recv_handler,"; // Recv_handler
           
            if ((*ipin)->pMsg->pPropsD)
            {
                initialiser << "sizeof(pkt_";
                initialiser << (*ipin)->pMsg->Name() << "_pyld_t),"; // sz_pkt
            }
            else initialiser << "0,";
            
           
            if ((*ipin)->pPropsD)
            {
                initialiser << "sizeof(" << dTypInPin;               // sz_props
                initialiser << (*ipin)->Name() << "_props_t),";
            }
            else initialiser << "0,";
           
            if ((*ipin)->pStateD)                                    // sz_state
            {
                initialiser << "sizeof(" << dTypInPin;
                initialiser << (*ipin)->Name() << "_state_t)";
            }
            else initialiser << "0";
            initialiser << "},";
        }
        initialiser.seekp(-1,ios_base::cur);    // Rewind one place to remove the stray ","
        initialiser << "}";                     // properly terminate the initialiser
        
        // Add the initialiser to vars cpp.
        vars_cpp << "in_pintyp_t Thread_" << threadAddr << "_DevTyp_0_InputPins";
        vars_cpp << "[" << inTypCnt << "] = " << initialiser.str() << ";\n";
    }
    vars_cpp << "\n";
}


/******************************************************************************
 * Form the initialiser(s) for the output pins array if we have output pins
 *****************************************************************************/
void Composer::writeOutputPinInit(AddressComponent threadAddr, DevT_t* devT,
                                std::ofstream& vars_h, std::ofstream& vars_cpp)
{
    size_t outTypCnt = devT->PinTO_v.size();      // Number of output pins
    std::string devTName = devT->Name();  // grab a local copy of the name
    
    if(devT->pPinTSO)
    {   // There is a supervisor output, increase the output count by one.
        outTypCnt++;
    }
    
    std::stringstream initialiser;
    
    if (outTypCnt)
    {
        // Add declaration for the output pins array to relevant vars header
        vars_h << "extern out_pintyp_t Thread_" << threadAddr;
        vars_h << "_DevTyp_0_OutputPins[" << outTypCnt << "];\n";
        
        // Build the dev type output pin name string
        std::string dTypOutPin = std::string("devtyp_");
        dTypOutPin += devT->Name() + std::string("_OutPin_");
        
        initialiser.str("");     // Clear the initialiser
        initialiser << "{";
        
        WALKVECTOR(PinT_t*, devT->PinTO_v, opin) // Build pin initialiser
        {
            initialiser << "{";
            initialiser << "&" << dTypOutPin;
            initialiser << (*opin)->Name() << "_Send_handler,";  // Send_Handler
            
            if ((*opin)->pMsg->pPropsD)
            {
                initialiser << "sizeof(pkt_";
                initialiser << (*opin)->pMsg->Name() << "_pyld_t)"; // sz_pkt
            }
            else initialiser << "0";
            initialiser << "},";
        }
        
        if(devT->pPinTSO)
        {   // There is a supervisor output, add it to the initialiser
            initialiser << "{";
            initialiser << "&devtyp_" << devTName;
            initialiser << "_Supervisor_Implicit_OutPin";
            initialiser << "_Send_handler,";  // Send_Handler
            
            if (devT->pPinTSO->pMsg->pPropsD)
            {
                initialiser << "sizeof(pkt_" << devT->pPinTSO->pMsg->Name();
                initialiser << "_pyld_t)"; // sz_pkt
            }
            else initialiser << "0";
            initialiser << "},";
        }
        
        initialiser.seekp(-1,ios_base::cur);   // Rewind one place to remove the stray ","
        initialiser << "}";                    // properly terminate the initialiser
        
        // Add the initialiser to vars cpp.
        vars_cpp << "out_pintyp_t Thread_" << threadAddr << "_DevTyp_0_OutputPins";
        vars_cpp << "[" << outTypCnt << "] = " << initialiser.str() << ";\n";
    }
    vars_cpp << "\n";
}

/******************************************************************************
 * Add the device instances declaration to vars h
 *****************************************************************************/
void Composer::writeDevIDecl(AddressComponent threadAddr, 
                                size_t numberOfDevices, std::ofstream& vars_h)
{
    vars_h << "//--------------------------- Device Instance Tables ";
    vars_h << "---------------------------\n";
    vars_h << "extern devInst_t Thread_" << threadAddr;
    vars_h << "_Devices[" << numberOfDevices << "];\n";
}



/******************************************************************************
 * Add the edge instances properties array declaration to vars h
 *****************************************************************************/
void Composer::writePinPropsDecl(PinI_t* pinI, std::string& thrDevName,
                                    size_t edgeCount, std::ofstream& vars_h)
{   
    vars_h << "extern devtyp_" << pinI->pT->par->Name();
    vars_h << "_InPin_" << pinI->pT->Name() << "_props_t ";
    vars_h << thrDevName << "_Pin_" << pinI->pT->Name();
    vars_h << "_InEdgeProps[" << edgeCount << "];\n\n";
}


/******************************************************************************
 * Add the edge instances states array declaration to vars h
 *****************************************************************************/
void Composer::writePinStateDecl(PinI_t* pinI, std::string& thrDevName,
                                    size_t edgeCount, std::ofstream& vars_h)
{   
    vars_h << "extern devtyp_" << pinI->pT->par->Name();
    vars_h << "_InPin_" << pinI->pT->Name() << "_state_t ";
    vars_h << thrDevName << "_Pin_" << pinI->pT->Name();
    vars_h << "_InEdgeStates[" << edgeCount << "];\n\n";
}


/******************************************************************************
 * Write the declaration for the device's Input Pins
 *****************************************************************************/
void Composer::writeDevIInPinsDecl(std::string& thrDevName, 
                                size_t inTypCnt, std::ofstream& vars_h)
{
    vars_h << "//-----------------------";
    vars_h << " Input Pin (Associative) Tables ";
    vars_h << "-----------------------\n";
    vars_h << "extern inPin_t " << thrDevName;
    vars_h << "_InputPins[" << inTypCnt << "];\n";
}

/******************************************************************************
 * Write the declaration for the device's Output Pins
 *****************************************************************************/
void Composer::writeDevIOutPinsDecl(std::string& thrDevName, 
                                size_t outTypCnt, std::ofstream& vars_h)
{
    vars_h << "//------------------------------";
    vars_h << " Output Pin Tables ";
    vars_h << "-----------------------------\n";
    vars_h << "extern outPin_t Thread_" << thrDevName;
    vars_h << "_OutputPins[" << outTypCnt << "];\n";
}


void Composer::writeThreadDevIDefs(ComposerGraphI_t* builderGraphI, 
                                P_thread* thread, size_t numberOfDevices, 
                                std::ofstream& vars_h, std::ofstream& vars_cpp)
{
    // Device, state and properties initialisers. 
    AddressComponent threadAddr = thread->get_hardware_address()->get_thread();
    
    GraphI_t* graphI = builderGraphI->graphI;
    
    // devInst_t initialiser
    std::vector<std::string> devIIStrs(numberOfDevices, "{},");      
    std::stringstream devII;
    
    
    // devtyp_X_props_t initialiser
    std::vector<std::string> devPIStrs(numberOfDevices, "{},");
    std::stringstream devPI;
    
    // devtyp_X_props_t initialiser
    std::vector<std::string> devSIStrs(numberOfDevices, "{},");
    std::stringstream devSI;
    
    // Abuse the fact that all devices have the same type
    DevT_t* devT = PNULL;
    
    // Iterate through all of the devices
    WALKLIST(DevI_t*, placer->threadToDevices.at(thread), dev)
    {
        unsigned devIdx = (*dev)->addr.get_device();
        
        devT = (*dev)->pT;
        DevI_t* devI = (*dev);
        
        size_t inTypCnt = devT->PinTI_v.size();   // # Input pins
        size_t outTypCnt = devT->PinTO_v.size();  // # output pins
        
        if(devI->pT->pPinTSO)
        {   // There is a supervisor output, increase the output count by one.
            outTypCnt++;
        }
        
        
        // Form the common "Thread_X_Device_Y" string for this device
        std::string thrDevName = std::string("Thread_");
        thrDevName += TO_STRING(threadAddr) + std::string("_Device_");
        thrDevName += devI->Name();
        
        std::stringstream iPinH, iPinCPP;
        std::stringstream oPinH, oPinCPP;
        std::stringstream iPinII, oPinII;
        
        
        devII.str("");
        devPI.str("");
        devSI.str("");
        
        // Form the first part of the PDeviceInstance/devInst_t initialiser
        devII << "{";
        devII << "&Thread_" << threadAddr << "_Context,";          // thread
        devII << "&Thread_" << threadAddr << "_DeviceTypes[0],";   // devType
        devII << devIdx << ",";                  // deviceID
        
        
        // Find all of the arcs that involve this device.
        std::vector<unsigned> arcKeysIn;
        std::vector<unsigned> arcKeysOut;
        graphI->G.FindArcs(devI->Key, arcKeysIn, arcKeysOut);
        
        // Write the pin definitions/initialisers
        
        writeDevIInputPinDefs(graphI, devT, threadAddr, thrDevName,
                                arcKeysIn, vars_h, vars_cpp);
        writeDevIOutputPinDefs(builderGraphI, devI, threadAddr, thrDevName, 
                                arcKeysOut, vars_h, vars_cpp);
        

        if (inTypCnt)
        {   // Input pin array Declaration
            writeDevIInPinsDecl(thrDevName, inTypCnt, vars_h);
            
            devII << inTypCnt << ",";               // numInputs
            devII << thrDevName << "_InputPins,";   // inputPins
        }
        else
        {
            devII << "0,";                          // numInputs
            devII << "PNULL,";                      // inputPins
        }
        
        
        if (outTypCnt)
        {   // Output pin array declaration
            writeDevIOutPinsDecl(thrDevName, outTypCnt, vars_h);
            
            devII << outTypCnt << ",";               // numOutputs
            devII << thrDevName << "_OutputPins,";   // outputPins
        }
        else
        {
            devII << "0,";                          // numOutputs
            devII << "PNULL,";                      // inputPins
        }
        
        
        // Add the device properties
        if(devI->pT->pPropsD)
        {
            devII << "&Thread_" << threadAddr << "_DeviceProperties[";
            devII << devIdx << "],";
            
            if(devI->pPropsI)
            {
                devPIStrs[devIdx] = "{" + devI->pPropsI->C_src() + "},";
            }
        }
        else
        {
            devII << "PNULL,";
        }
        
        // Add the device state 
        if(devI->pT->pStateD)
        {
            devII << "&Thread_" << threadAddr << "_DeviceState[";
            devII << devIdx << "]},";
            
            if(devI->pStateI)
            {
                devSIStrs[devIdx] = "{" + devI->pStateI->C_src() + "},";
            }
        }
        else
        {
            devII << "PNULL},";
        }
        
        // populate the DevIIStrs with the initialiser
        devIIStrs[devIdx] = devII.str();
    }
    
    
    
    
    
    // Process the individual initialisers into a coherent string
    
    // Start the devInst_t initialiser
    devII.str("");
    devII << "devInst_t Thread_" << threadAddr << "_Devices[";
    devII << numberOfDevices << "] = {";
    
    
    // Start the devtyp_X_props_t initialiser
    devPI.str("");
    devPI << "devtyp_" << devT->Name() << "_props_t Thread_" << threadAddr;
    devPI << "_DeviceProperties[" << numberOfDevices << "] = {";
    
    // Start the devtyp_X_state_t initialiser
    devSI.str("");
    devSI << "devtyp_" << devT->Name() << "_state_t Thread_" << threadAddr;
    devSI << "_DeviceState[" << numberOfDevices << "] = {";
    
    
    // Fill in the initialiser strings
    for(size_t i=0; i<numberOfDevices; i++)
    {
        devII << devIIStrs[i];
        devPI << devPIStrs[i];
        devSI << devSIStrs[i];
    }
    
    // Finish off the devInst_t initialiser
    devII.seekp(-1, ios_base::cur);   // Remove the stray ,
    devII << "};\n\n";
    
    // Finish off the devtyp_X_props_t initialiser
    devPI.seekp(-1, ios_base::cur);   // Remove the stray ,
    devPI << "};\n\n";
    
    // Finish off the devtyp_X_state_t initialiser
    devSI.seekp(-1, ios_base::cur);   // Remove the stray ,
    devSI << "};\n\n";
    
    
    
    // Write the initialisers to the vars.cpp
    vars_cpp << devII.rdbuf();
    
    if(devT->pStateD)
    {
        vars_cpp << devPI.rdbuf();
    }
    
    if(devT->pStateD)
    {
        vars_cpp << devSI.rdbuf();
    }
    
    // Make sure the properties and state decls are there.
    vars_h << "extern devtyp_" << devT->Name() << "_props_t Thread_";
    vars_h << threadAddr << "_DeviceProperties[" << numberOfDevices << "];\n";

    vars_h << "extern devtyp_" << devT->Name() << "_state_t Thread_";
    vars_h << threadAddr << "_DeviceState[" << numberOfDevices << "];\n\n";

}


/******************************************************************************
 * Write the definition for each input pin
 *****************************************************************************/
void Composer::writeDevIInputPinDefs(GraphI_t* graphI, DevT_t* devT, 
                                        AddressComponent threadAddr,
                                        std::string& thrDevName,
                                        std::vector<unsigned>& arcKeysIn,
                                        std::ofstream& vars_h,
                                        std::ofstream& vars_cpp)
{
    pinIArcKeyMap_t iPinIArcKMap;
    size_t inTypCnt = devT->PinTI_v.size();  // # output pins
    
    // Walk through the input arcs and find their associated PinI_t
    WALKVECTOR(unsigned, arcKeysIn, arc)
    {
        PinI_t* oPin;
        PinI_t* iPin;
        unsigned oPinKey, iPinKey;
        
        //  Get the pin data for the edge
        graphI->G.FindArcPins((*arc), oPinKey, oPin, iPinKey, iPin);
        
        pinIArcKeyMap_t::iterator srch = iPinIArcKMap.find(iPin);
        if (srch == iPinIArcKMap.end()) 
        {   // First time seeing this instance, add to the map
            std::vector<unsigned> arcKey;
            arcKey.push_back((*arc));
            iPinIArcKMap.insert(pinIArcKeyMap_t::value_type(iPin, arcKey));
        }
        else
        {   // Add the edge key to the entry
            srch->second.push_back((*arc));
        }
    }
    
    // Now need to generate initialisers for all of the input pins and their 
    // edges. Start by filling array with default, then overwrite from map.
    std::vector<std::string> inPinTIStrs; 
    for(size_t pinIdx = 0; pinIdx < inTypCnt; pinIdx++)
    {
        std::stringstream inPinInit;
        inPinInit << "{PNULL,&Thread_" << threadAddr;
        inPinInit << "_DevTyp_0_InputPins[" << pinIdx << "],";
        inPinInit << "0,PNULL},";
        inPinTIStrs.push_back(inPinInit.str());
    }
     
    WALKMAP(PinI_t*, std::vector<unsigned>, iPinIArcKMap, pinIItr)
    {
        unsigned pinIdx;
        unsigned edgeCnt = pinIItr->first->Key_v.size();
       
        std::stringstream inPinInit;
       
        // Find the pinIdx
        pinIdx = pinIItr->first->pT->Idx;
        
        // inPin_t initialiser
        inPinInit << "{PNULL,&Thread_" << threadAddr;
        inPinInit << "_DevTyp_0_InputPins[" << pinIdx << "],";
       
        if(edgeCnt)
        {
            inPinInit << edgeCnt << "," << thrDevName << "_Pin_";
            inPinInit << pinIItr->first->pT->Name() << "_InEdges},";
        }
        else
        {   // Excessively paranoid sanity check
            inPinInit << "0,PNULL},";
        }
        
        inPinTIStrs[pinIdx] = inPinInit.str();
        
        
        // Descend into the edges
        writeDevIInputPinEdgeDefs(graphI, pinIItr->first, threadAddr,
                                thrDevName, pinIItr->second, vars_h, vars_cpp);
    }
    
    //Form the inPin_t definition
    std::stringstream inPinTI;
    inPinTI << "inPin_t " << thrDevName;
    inPinTI << "_InputPins[" << inTypCnt << "] = {";
    
    for(size_t i=0; i<inTypCnt; i++)
    {
        inPinTI << inPinTIStrs[i];
    }
    
    inPinTI.seekp(-1, ios_base::cur);   // Remove the stray ,
    inPinTI<< "};\n\n";
    
    // Dump it to file
    vars_cpp << inPinTI.rdbuf();
}


/******************************************************************************
 * Write the definition for each input edge
 *****************************************************************************/
void Composer::writeDevIInputPinEdgeDefs(GraphI_t* graphI, PinI_t* pinI,
                                            AddressComponent threadAddr,
                                            std::string& thrDevName,
                                            std::vector<unsigned>& arcV,
                                            std::ofstream& vars_h,
                                            std::ofstream& vars_cpp)
{
    size_t edgeCount = arcV.size();
    
    if(edgeCount)
    {   // Probably a pointless sanity check

        if(pinI->pT->pPropsD)
        {   // Write the edge's properties array declaration
            writePinPropsDecl(pinI, thrDevName, edgeCount, vars_h);
        }
        
        if(pinI->pT->pStateD)
        {   // Write the edge's states array declaration
            writePinStateDecl(pinI, thrDevName, edgeCount, vars_h);
        }
        
        // inEdge_t array initialiser
        std::vector<std::string> inEdgeTIStrs(edgeCount, "{},");
        
        // devtyp_X_InPin_Y_state_t array initialiser
        std::vector<std::string> inEdgeStatesIStrs(edgeCount, "{},");
        
        // devtyp_X_InPin_Y_props_t array initialiser
        std::vector<std::string> inEdgePropsIStrs(edgeCount, "{},");
        
        
        WALKVECTOR(unsigned, arcV, arcKey)
        {   // Walk the vector of Digraph pin keys
            std::stringstream inEdgeInit;
            
            EdgeI_t* edgeI;
            
            // Get the Edge instance
            edgeI = *(pinI->par->G.FindArc((*arcKey)));
            
            // {PNULL,1,2,PNULL,PNULL}
            inEdgeInit << "{PNULL,";
            
            if(pinI->pT->pPropsD)   // Properties exist in Pin Type, not edge
            {
                inEdgeInit << "&" << thrDevName << "_Pin_" << pinI->pT->Name();
                inEdgeInit << "_InEdgeProps[" << edgeI->Idx << "],";
                
                if(edgeI->pPropsI)
                {
                    inEdgePropsIStrs[edgeI->Idx] = 
                                        "{" + edgeI->pPropsI->C_src() + "},";
                }
            }
            else
            {
                inEdgeInit << "PNULL,";
            }
            
            if(pinI->pT->pStateD)   // State exists in Pin Type, not edge
            {
                inEdgeInit << "&" << thrDevName << "_Pin_" << pinI->pT->Name();
                inEdgeInit << "_InEdgeStates[" << edgeI->Idx << "]},";
                
                if(edgeI->pStateI)
                {
                    inEdgeStatesIStrs[edgeI->Idx] = 
                                        "{" + edgeI->pStateI->C_src() + "},";
                }
            }
            else
            {
                inEdgeInit << "PNULL},";
            }
            
            inEdgeTIStrs[edgeI->Idx] = inEdgeInit.str();
        }
        
        
        // Process the individual initialiser fragments into coherent strings
        // We do them all at the same time (even if there is no props or state)
        // so that we only iterate once.
        
        // Start the inEdge_t initialiser
        std::stringstream inEdgeTI;
        std::stringstream inEdgeStatesI;
        std::stringstream inEdgePropsI;
        
        inEdgeTI.str("");
        inEdgeTI << "inEdge_t " << thrDevName << "_Pin_" << pinI->pT->Name();
        inEdgeTI << "_InEdges[" << edgeCount << "] = {";
        
        // Start the devtyp_X_InPin_Y_state_t initialiser
        inEdgeStatesI.str("");
        inEdgeStatesI << "devtyp_" << pinI->pT->par->Name();
        inEdgeStatesI << "_InPin_" << pinI->pT->Name();
        inEdgeStatesI << "_state_t ";
        inEdgeStatesI << thrDevName << "_Pin_" << pinI->pT->Name();
        inEdgeStatesI << "_InEdgeStates[" << edgeCount << "] = {";
        
        // Start the devtyp_X_InPin_Y_props_t initialiser
        inEdgePropsI.str("");
        inEdgePropsI << "devtyp_" << pinI->pT->par->Name();
        inEdgePropsI << "_InPin_" << pinI->pT->Name();
        inEdgePropsI << "_props_t Thread_ ";
        inEdgePropsI << thrDevName << "_Pin_" << pinI->pT->Name();
        inEdgePropsI << "_InEdgeProps[" << edgeCount << "] = {";
        
        
        // Fill in the initialiser strings
        for(size_t i=0; i<edgeCount; i++)
        {
            inEdgeTI << inEdgeTIStrs[i];
            inEdgeStatesI << inEdgeStatesIStrs[i];
            inEdgePropsI << inEdgePropsIStrs[i];
        }
        
        // Finish off the inEdge_t initialiser
        inEdgeTI.seekp(-1, ios_base::cur);   // Remove the stray ,
        inEdgeTI << "};\n";
        
        // Finish off the devtyp_X_InPin_Y_state_t initialiser
        inEdgeStatesI.seekp(-1, ios_base::cur);   // Remove the stray ,
        inEdgeStatesI << "};\n";
        
        // Finish off the devtyp_X_InPin_Y_props_t initialiser
        inEdgePropsI.seekp(-1, ios_base::cur);   // Remove the stray ,
        inEdgePropsI << "};\n\n";
        
        
        
        // Write the initialisers to the vars.cpp
        vars_cpp << inEdgeTI.rdbuf();
        
        if(pinI->pT->pPropsD)
        {
            vars_cpp << inEdgePropsI.rdbuf();
        }
        if(pinI->pT->pStateD)
        {
            vars_cpp << inEdgeStatesI.rdbuf();
        }
    }
}


/******************************************************************************
 * Write the definition for each output pin
 *****************************************************************************/
void Composer::writeDevIOutputPinDefs(ComposerGraphI_t* builderGraphI,
                                        DevI_t* devI,
                                        AddressComponent threadAddr,
                                        std::string& thrDevName,
                                        std::vector<unsigned>& arcKeysOut,
                                        std::ofstream& vars_h,
                                        std::ofstream& vars_cpp)
{
    GraphI_t* graphI = builderGraphI->graphI;
    DevT_t* devT = devI->pT;
    
    pinIArcKeyMap_t oPinIArcKMap;
    size_t outTypCnt = devT->PinTO_v.size();  // # output pins
    
    
    if(devT->pPinTSO)
    {   // There is a supervisor output, increase the output count by one.
        outTypCnt++;
    }
    
    // Walk through the input arcs and find their associated PinI_t
    WALKVECTOR(unsigned, arcKeysOut, arc)
    {
        PinI_t* oPin;
        PinI_t* iPin;
        unsigned oPinKey, iPinKey;
        
        //  Get the pin data for the edge
        graphI->G.FindArcPins((*arc), oPinKey, oPin, iPinKey, iPin);
        
        pinIArcKeyMap_t::iterator srch = oPinIArcKMap.find(oPin);
        if (srch == oPinIArcKMap.end()) 
        {   // First time seeing this instance, add to the map
            std::vector<unsigned> arcKey;
            arcKey.push_back((*arc));
            oPinIArcKMap.insert(pinIArcKeyMap_t::value_type(oPin, arcKey));
        }
        else
        {   // Add the edge key to the entry
            srch->second.push_back((*arc));
        }
    }
    
    // Now need to generate initialisers for all of the output pins and their 
    // edges. Start by filling array with default, then overwrite from map.
    std::vector<std::string> outPinTIStrs; 
    for(size_t pinIdx = 0; pinIdx < outTypCnt; pinIdx++)
    {
        std::stringstream outPinInit;
        outPinInit << "{PNULL,&Thread_" << threadAddr;
        outPinInit << "_DevTyp_0_OutputPins[" << pinIdx << "],";
        outPinInit << "0,PNULL,0,0},";
        outPinTIStrs.push_back(outPinInit.str());
    }    
    
    
    WALKMAP(PinI_t*, std::vector<unsigned>, oPinIArcKMap, pinIItr)
    {
        unsigned pinIdx;
        unsigned edgeCnt = pinIItr->first->Key_v.size();
       
        std::stringstream outPinInit;
       
        // Find the pinIdx
        pinIdx = pinIItr->first->pT->Idx;
        
        // {PNULL,&Thread_0_DevTyp_0_OutputPins[0],2,
        //  Thread_0_Device_c_0_1_OutPin_out_Tgts,0,0}
        
        // outPin_t initialiser
        outPinInit << "{PNULL,&Thread_" << threadAddr;
        outPinInit << "_DevTyp_0_OutputPins[" << pinIdx << "],";
        
        if(edgeCnt)
        {
            outPinInit << edgeCnt << "," << thrDevName << "_OutPin_";
            outPinInit << pinIItr->first->pT->Name() << "_Tgts,0,0},";
        }
        else
        {   // Excessively paranoid sanity check
            outPinInit << "0,PNULL,0,0},";
        }
        
        outPinTIStrs[pinIdx] = outPinInit.str();
        
        
        // Descend into the edges
        writeDevIOutputPinEdgeDefs(graphI, pinIItr->first, thrDevName, 
                                    pinIItr->second, vars_h, vars_cpp);
    }
    
    
    // Now add the implicit Supervisor send pin if it exists
    if(devT->pPinTSO)
    {   
        std::stringstream outPinInit;
        
        unsigned pinIdx = devT->PinTO_v.size();      // Last pin

        // outPin_t initialiser
        outPinInit << "{PNULL,&Thread_" << threadAddr;
        outPinInit << "_DevTyp_0_OutputPins[" << pinIdx << "],";
        
        // ALWAYS have a single edge.
        outPinInit << "1," << thrDevName;
        outPinInit << "_Supervisor_Implicit_OutPin_Tgts,0,0},";
        
        // Start the outEdge_t initialiser
        std::stringstream outEdgeTI;
        outEdgeTI << "outEdge_t " << thrDevName;
        outEdgeTI  << "_Supervisor_Implicit_OutPin_Tgts[1] = {";
        
        // Only a single, static edge so we will do it manually.
        // {PNULL,<hwAddr>,<swAddr>,<pinAddr>}
        outEdgeTI << "{PNULL,";
        
        // Hardware address is replaced in the softswitch by a call to tinsel.
        outEdgeTI << DEST_BROADCAST << ",";
        
        // SW address needs to be set. Default 0 for most vakues, but left
        // configurable.
        TaskComponent task = 0;
        OpCodeComponent opCode = P_CNC_IMPL;
        DeviceComponent device = 0;
        SoftwareAddress swAddr(true, true, task, opCode, device);
        outEdgeTI << swAddr.as_uint() << ",";
        
        // Pin header: retrieve the idx from the Supervisor map
        devISuperIdxMap_t::iterator devISrch;
        devISrch = builderGraphI->devISuperIdxMap.find(devI);
        if (devISrch == builderGraphI->devISuperIdxMap.end()) 
        {   // Something has gone wrong that we are going to ignore.
            //TODO: Barf
            outEdgeTI << "0";
        }
        else
        {
            outEdgeTI << devISrch->second;
        }
        
        
        outEdgeTI << "0";
        
        // Finish off the inEdge_t initialiser
        outEdgeTI << "}};\n\n";
        
        // Dump it to file
        vars_cpp << outEdgeTI.rdbuf();
        
        outPinTIStrs[pinIdx] = outPinInit.str();
    }
    
    //Form the outPin_t definition
    std::stringstream outPinTI;
    outPinTI << "outPin_t " << thrDevName;
    outPinTI << "_OutputPins[" << outTypCnt << "] = {";
    
    for(size_t i=0; i<outTypCnt; i++)
    {
        outPinTI << outPinTIStrs[i];
    }
    
    outPinTI.seekp(-1, ios_base::cur);   // Remove the stray ,
    outPinTI<< "};\n\n";
    
    // Dump it to file
    vars_cpp << outPinTI.rdbuf();
    
}


/******************************************************************************
 * Write the definition for each output edge (target)
 *****************************************************************************/
void Composer::writeDevIOutputPinEdgeDefs(GraphI_t* graphI, PinI_t* pinI,
                                            std::string& thrDevName,
                                            std::vector<unsigned>& arcV,
                                            std::ofstream& vars_h,
                                            std::ofstream& vars_cpp)
{
    size_t edgeCount = arcV.size();
    
    if(edgeCount)
    {   // Probably a pointless sanity check
        
        // ioutEdge_t array initialiser
        std::vector<std::string> outEdgeTIStrs;
        
        std::stringstream outEdgeTI;
        
        WALKVECTOR(unsigned, arcV, arcKey)
        {   // Walk the vector of Digraph pin keys
            std::stringstream outEdgeInit;
            
            EdgeI_t* edgeI;
            
            unsigned inNodeKey = 0, outNodeKey = 0;
            DevI_t* outDevI;
            
            unsigned inPinKey, outPinKey;
            PinI_t* inPinI = PNULL;
            PinI_t* outPinI = PNULL;
            
            P_thread* farThread;
            
            // Get the Edge instance
            edgeI = *(graphI->G.FindArc((*arcKey)));
            
            // Get the DevI_t* for the far end
            graphI->G.FindNodes((*arcKey), outNodeKey, inNodeKey);
            outDevI = *(graphI->G.FindNode(outNodeKey));
            
            // Get the pinI_t of both ends
            graphI->G.FindArcPins((*arcKey), outPinKey, outPinI, inPinKey, inPinI);
            
            // Get the Thread of the far end
            // This is probably slow, may be faster to have a field in DevI_t?
            std::map<DevI_t*, P_thread*>::iterator threadSrch;
            threadSrch = placer->deviceToThread.find(outDevI);
            
            if (threadSrch == placer->deviceToThread.end()) 
            {   // Something has gone terribly terribly wrong
                //TODO: Barf
            }
            farThread = threadSrch->second;
            
            // {PNULL,<hwAddr>,<swAddr>,<pinAddr>}
            outEdgeInit << "{PNULL,";
            
            // Get the hardware address of the other end.
            uint32_t hwAddr = farThread->get_hardware_address()->as_uint();
            
            
            // Form the SW address for the pin
            SoftwareAddress swAddr = outDevI->addr;
            
            //TODO: set_task, etc. etc.
            
            // Get the pin and edge indicies for the other side.
            uint32_t pinAddr = 0;
            
            // Edge Index
            pinAddr |= ((edgeI->Idx << P_HD_DESTEDGEINDEX_SHIFT) 
                        & P_HD_DESTEDGEINDEX_MASK);
            
            // Pin Index
            if(outPinI != PNULL)
            {   // Pointless sanity check?
                pinAddr |= (((outPinI->pT->Idx) << P_HD_TGTPIN_SHIFT)
                            & P_HD_TGTPIN_MASK);
            }
            else
            {   // Barf?
                
            }
            
            
            // Make the magic happen
            outEdgeInit << hwAddr << ",";
            outEdgeInit << swAddr.as_uint() << ",";
            outEdgeInit << pinAddr << "},";
            
            outEdgeTIStrs.push_back(outEdgeInit.str());
        }
        
        
        
        // Process the individual initialiser fragments into coherent strings
        
        // Start the outEdge_t initialiser
        outEdgeTI.str("");
        outEdgeTI << "outEdge_t " << thrDevName << "_OutPin_" << pinI->pT->Name();
        outEdgeTI << "_Tgts[" << edgeCount << "] = {";
        
        // Fill in the initialiser strings
        for(size_t i=0; i<edgeCount; i++)
        {
            outEdgeTI << outEdgeTIStrs[i];
        }
        
        // Finish off the inEdge_t initialiser
        outEdgeTI.seekp(-1, ios_base::cur);   // Remove the stray ,
        outEdgeTI << "};\n\n";
        
        // Write the initialisers to the vars.cpp
        vars_cpp << outEdgeTI.rdbuf();
    }
    
}

