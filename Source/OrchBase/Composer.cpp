#include "Composer.h"

// Temporary consts (were in build_defs).
//TODO: reconcile
const string GENERATED_PATH = "Generated";
const string GENERATED_H_PATH = GENERATED_PATH+"/inc";
const string GENERATED_CPP_PATH = GENERATED_PATH+"/src";

//TODO: cross-platform this
const string COREMAKE = "make -j$(nproc --ignore=4) all ";
const string COREMAKEPOST = "> make_errs.txt 2>&1";
const string COREMAKECLEAN = "make clean 2>&1 >> clean_errs.txt";



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
    
    // Softswitch control.
    rtsBuffSizeMax = MAX_RTSBUFFSIZE;
    bufferingSoftswitch = false;        // Default to a non-buffering softswitch
    softswitchInstrumentation = true;   // Default to enable instrumentation
    softswitchLogHandler = trivial;     // Default to the trivial log handler
    softswitchLogLevel = 2;             // Default to a log level of 2
    softswitchLoopMode = standard;      // Default to the standard loop mode
    softswitchRequestIdle = true;       // Default to respecting requestIdle
    
    compilationFlags = "";
    provenanceCache = "";
}

ComposerGraphI_t::ComposerGraphI_t(GraphI_t* graphIIn, std::string& outputPath)
{
    graphI = graphIIn;
    outputDir = outputPath;
    outputDir += graphI->GetCompoundName(true);
    generated = false;
    compiled = false;
    
    // Softswitch control.
    rtsBuffSizeMax = MAX_RTSBUFFSIZE;
    bufferingSoftswitch = false;        // Default to a non-buffering softswitch
    softswitchInstrumentation = true;   // Default to enable instrumentation
    softswitchLogHandler = trivial;     // Default to the trivial log handler 
    softswitchLogLevel = 2;             // Default to a log level of 2
    softswitchLoopMode = standard;      // Default to the standard loop mode
    softswitchRequestIdle = true;       // Default to respecting requestIdle
    
    compilationFlags = "";
    provenanceCache = "";
}

ComposerGraphI_t::~ComposerGraphI_t()
{
    generated = false;
    compiled = false;

    WALKMAP(DevT_t*, devTypStrings_t*, devTStrsMap, devTStrs)
    {
        delete devTStrs->second;
    }
    devTStrsMap.clear();
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
 * Dump a ComposerGraphI_t to file for debugging
 *****************************************************************************/
void ComposerGraphI_t::Dump(unsigned off,FILE* file)
{
    std::string prefix = dformat("%d Composer Graph Instance at %" PTR_FMT ,
                                 off, OSFixes::getAddrAsUint(this));
    DumpUtils::open_breaker(file, prefix);
    
    fprintf(file, "Graph instance name:          %s \n",graphI->Name().c_str());
    fprintf(file, "Output directory:             %s \n",outputDir.c_str());
    fprintf(file, "Generated:                    %s \n",
                                                generated ? "true" : "false");
    fprintf(file, "Compiled:                     %s \n",
                                                compiled ? "true" : "false");
    
    fprintf(file, "\nSoftswitch generation/compilation control:\n");
    fprintf(file, "  Maximum RTS buffer size:    %lu \n",rtsBuffSizeMax);
    fprintf(file, "  Buffering Softswitch:       %s \n",
                        bufferingSoftswitch ? "true" : "false");
    fprintf(file, "  Softswitch Instrumentation: %s \n",
                        softswitchInstrumentation ? "true" : "false");
    fprintf(file, "  Softswitch requestIdle: %s \n",
                        softswitchRequestIdle ? "true" : "false");
                        
    fprintf(file, "  Softswitch log handler:     ");
    switch(softswitchLogHandler)
    {
        case disabled:  fprintf(file, "none\n");                          break;
        case trivial:   fprintf(file, "trivial\n");                       break;
        default:        fprintf(file, "**INVALID**\n");
    }
    
    fprintf(file, "  Softswitch log level:       %lu\n",softswitchLogLevel);
    
    fprintf(file, "  Softswitch loop mode:       ");
    switch(softswitchLoopMode)
    {
        case standard:  fprintf(file, "standard\n");                      break;
        case priInstr:  fprintf(file, "prioritise instrumentation\n");    break;
        default:        fprintf(file, "**INVALID**\n");
    }
    
    
    fprintf(file, "  Hardware Idle Instructions: %s\n",
                    idleInstructionBinary.c_str());
    fprintf(file, "  Hardware Idle Data:         %s\n",idleDataBinary.c_str());
    
    fprintf(file, "\nNitty gritty details:\n");
    fprintf(file, "  Device type strs map size:  %lu \n",
                        static_cast<unsigned long>(devTStrsMap.size()));
    fprintf(file, "  supevisorDevIVect size:     %lu \n",
                        static_cast<unsigned long>(supevisorDevIVect.size()));
    fprintf(file, "  Provenance string cache:\n%s \n\n",provenanceCache.c_str());
    
    
    // Form the Softswitch compilation control string
    std::string makeArgs = "";
    if(bufferingSoftswitch)
    {   // Softswitch needs to be built in buffering mode
        makeArgs += "SOFTSWITCH_BUFFERING=1 ";
    }
    if(!(softswitchRequestIdle))
    {   // Softswitch needs to be built without requestIdle
        makeArgs += "SOFTSWITCH_NOREQUESTIDLE=1 ";
    }
    if(!(softswitchInstrumentation))
    {   // Softswitch needs to be built with instrumentation disabled
        makeArgs += "SOFTSWITCH_DISABLE_INSTRUMENTATION=1 ";
    }
    switch(softswitchLogHandler)
    {   // Control the log handler 
        case trivial:   makeArgs += "SOFTSWITCH_TRIVIAL_LOG_HANDLER=1 ";
                        break;
        
        default:        break;  // De Nada!
    }
    switch(softswitchLoopMode)
    {   // Control the main loop order
        case priInstr:  makeArgs += "SOFTSWITCH_PRIORITISE_INSTRUMENTATION=1 ";
                        break;
        
        default:        break;  // De Nada!
    }
    // Set the softswitch log level
    makeArgs += "SOFTSWITCH_LOGLEVEL=";
    makeArgs += TO_STRING(softswitchLogLevel);
    makeArgs += " ";
    // Add the user-supplied flags if they exist
    if(compilationFlags.size())
    {
        makeArgs += "CM_CFLAGS=\"";
        makeArgs += compilationFlags;
        makeArgs += "\" ";
    }
    // Print the Make invocation
    fprintf(file, "  Make invocation:\n\t%s%s%s\n\n",   COREMAKE.c_str(),
                                                        makeArgs.c_str(),
                                                        COREMAKEPOST.c_str());

    /* Close breaker and flush the dump. */
    DumpUtils::close_breaker(file, prefix);
    fflush(file);
}

/******************************************************************************
 * Composer constructors
 *****************************************************************************/
Composer::Composer()
{
    placer = PNULL;
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
    }
    graphIMap.clear();
}

/******************************************************************************
 * Show some summary info about a Composer instance
 *****************************************************************************/
void Composer::Show(FILE* file)
{
    fprintf(file, "Output Path:                  %s \n",outputPath.c_str());
    fprintf(file, "Number of graph instances:    %lu \n",
                                static_cast<unsigned long>(graphIMap.size()));
    
    WALKMAP(GraphI_t*, ComposerGraphI_t*, graphIMap, builderGraphI)
    {
        fprintf(file, "Instance Name: %s\tGenerated: %s\tCompiled: %s\n",
                        builderGraphI->second->graphI->Name().c_str(),
                        builderGraphI->second->generated ? "true" : "false",
                        builderGraphI->second->compiled ? "true" : "false");
    }
}

/******************************************************************************
 * Dump a Composer instance to file for debugging
 *****************************************************************************/
void Composer::Dump(unsigned off,FILE* file)
{
    std::string prefix = dformat("%d Composer Instance at %" PTR_FMT ,
                                 off, OSFixes::getAddrAsUint(this));
    DumpUtils::open_breaker(file, prefix);
    
    fprintf(file, "Placer instance:              %" PTR_FMT "\n",
                                                OSFixes::getAddrAsUint(placer));
    fprintf(file, "Output Path:                  %s \n",outputPath.c_str());
    fprintf(file, "Number of graph instances:    %lu \n",
                                static_cast<unsigned long>(graphIMap.size()));
    
    WALKMAP(GraphI_t*, ComposerGraphI_t*, graphIMap, builderGraphI)
    {
        fprintf(file, "\n");
        builderGraphI->second->Dump(off+2,file);
    }
    
    
    /* Close breaker and flush the dump. */
    DumpUtils::close_breaker(file, prefix);
    fflush(file);
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
    
    // Changing the placer invalidates everything. Cleanup!
    WALKMAP(GraphI_t*, ComposerGraphI_t*, graphIMap, graphISrch)
    {
        delete graphISrch->second;
    }
    graphIMap.clear();
}


/******************************************************************************
 * Set the buffering mode for the softswitch
 *
 * Changing this requires a compiled app to be recompiled.
 *****************************************************************************/
int Composer::setBuffMode(GraphI_t* graphI, bool buffMode)
{
    ComposerGraphI_t* builderGraphI;
    //FILE * fd = graphI->par->par->fd;              // Detail output file

    ComposerGraphIMap_t::iterator srch = graphIMap.find(graphI);
    if (srch == graphIMap.end())
    {   // The Graph Instance has not been seen before, map it.
        builderGraphI = new ComposerGraphI_t(graphI, outputPath);

        // Insert the GraphI
        std::pair<ComposerGraphIMap_t::iterator, bool> insertedGraphI;
        insertedGraphI = graphIMap.insert(ComposerGraphIMap_t::value_type
                                                (graphI, builderGraphI));

    } else {
        builderGraphI = srch->second;
    }
    
    if(builderGraphI->compiled)
    {   // Already compiled, need to decompile
        clean(graphI);
    }
    
    builderGraphI->bufferingSoftswitch = buffMode;
    
    return 0;
}

/******************************************************************************
 * Set the size of the RTS Buffer
 *
 * Changing this requires an app to be regenerated and recompiled.
 *****************************************************************************/
int Composer::setRTSSize(GraphI_t* graphI, unsigned long rtsSize)
{
    ComposerGraphI_t* builderGraphI;
    //FILE * fd = graphI->par->par->fd;              // Detail output file

    ComposerGraphIMap_t::iterator srch = graphIMap.find(graphI);
    if (srch == graphIMap.end())
    {   // The Graph Instance has not been seen before, map it.
        builderGraphI = new ComposerGraphI_t(graphI, outputPath);

        // Insert the GraphI
        std::pair<ComposerGraphIMap_t::iterator, bool> insertedGraphI;
        insertedGraphI = graphIMap.insert(ComposerGraphIMap_t::value_type
                                                (graphI, builderGraphI));

    } else {
        builderGraphI = srch->second;
    }
    
    if(builderGraphI->compiled)
    {   // Already compiled, need to decompile
        clean(graphI);
    }
    
    if(builderGraphI->generated)
    {   // Already generated, need to degenerate
        degenerate(graphI, false);
    }
    
    builderGraphI->rtsBuffSizeMax = rtsSize;
    
    return 0;
}

/******************************************************************************
 * Set the buffering mode for the softswitch
 *
 * Changing this requires a compiled app to be recompiled.
 *****************************************************************************/
int Composer::setReqIdleMode(GraphI_t* graphI, bool reqIdleMode)
{
    ComposerGraphI_t* builderGraphI;
    //FILE * fd = graphI->par->par->fd;              // Detail output file

    ComposerGraphIMap_t::iterator srch = graphIMap.find(graphI);
    if (srch == graphIMap.end())
    {   // The Graph Instance has not been seen before, map it.
        builderGraphI = new ComposerGraphI_t(graphI, outputPath);

        // Insert the GraphI
        std::pair<ComposerGraphIMap_t::iterator, bool> insertedGraphI;
        insertedGraphI = graphIMap.insert(ComposerGraphIMap_t::value_type
                                                (graphI, builderGraphI));

    } else {
        builderGraphI = srch->second;
    }
    
    if(builderGraphI->compiled)
    {   // Already compiled, need to decompile
        clean(graphI);
    }
    
    builderGraphI->softswitchRequestIdle = reqIdleMode;
    
    return 0;
}

/******************************************************************************
 * Set whether instrumentation will be enabled or disabled within a softswitch.
 *
 * Changing this requires a compiled app to be recompiled.
 *****************************************************************************/
int Composer::enableInstr(GraphI_t* graphI, bool ssInstr)
{
    ComposerGraphI_t* builderGraphI;
    //FILE * fd = graphI->par->par->fd;              // Detail output file

    ComposerGraphIMap_t::iterator srch = graphIMap.find(graphI);
    if (srch == graphIMap.end())
    {   // The Graph Instance has not been seen before, map it.
        builderGraphI = new ComposerGraphI_t(graphI, outputPath);

        // Insert the GraphI
        std::pair<ComposerGraphIMap_t::iterator, bool> insertedGraphI;
        insertedGraphI = graphIMap.insert(ComposerGraphIMap_t::value_type
                                                (graphI, builderGraphI));

    } else {
        builderGraphI = srch->second;
    }
    
    if(builderGraphI->compiled)
    {   // Already compiled, need to decompile
        clean(graphI);
    }
    
    builderGraphI->softswitchInstrumentation = ssInstr;
    
    return 0;
}

/******************************************************************************
 * Set loghandler mode for a softswitch
 *
 * Changing this requires a compiled app to be recompiled.
 *****************************************************************************/
int Composer::setLogHandler(GraphI_t* graphI, ssLogHandler_t logHandler)
{
    ComposerGraphI_t* builderGraphI;
    //FILE * fd = graphI->par->par->fd;              // Detail output file

    ComposerGraphIMap_t::iterator srch = graphIMap.find(graphI);
    if (srch == graphIMap.end())
    {   // The Graph Instance has not been seen before, map it.
        builderGraphI = new ComposerGraphI_t(graphI, outputPath);

        // Insert the GraphI
        std::pair<ComposerGraphIMap_t::iterator, bool> insertedGraphI;
        insertedGraphI = graphIMap.insert(ComposerGraphIMap_t::value_type
                                                (graphI, builderGraphI));

    } else {
        builderGraphI = srch->second;
    }
    
    if(builderGraphI->compiled)
    {   // Already compiled, need to decompile
        clean(graphI);
    }
    
    builderGraphI->softswitchLogHandler = logHandler;
    
    return 0;
}

int Composer::setLogLevel(GraphI_t* graphI, unsigned long level)
{
    ComposerGraphI_t* builderGraphI;
    //FILE * fd = graphI->par->par->fd;              // Detail output file

    ComposerGraphIMap_t::iterator srch = graphIMap.find(graphI);
    if (srch == graphIMap.end())
    {   // The Graph Instance has not been seen before, map it.
        builderGraphI = new ComposerGraphI_t(graphI, outputPath);

        // Insert the GraphI
        std::pair<ComposerGraphIMap_t::iterator, bool> insertedGraphI;
        insertedGraphI = graphIMap.insert(ComposerGraphIMap_t::value_type
                                                (graphI, builderGraphI));

    } else {
        builderGraphI = srch->second;
    }
    
    if(builderGraphI->compiled)
    {   // Already compiled, need to decompile
        clean(graphI);
    }
    
    builderGraphI->softswitchLogLevel = level;
    
    return 0;
}

/******************************************************************************
 * Set loop mode for a softswitch
 *
 * Changing this requires a compiled app to be recompiled.
 *****************************************************************************/
int Composer::setLoopMode(GraphI_t* graphI, ssLoopMode_t loopMode)
{
    ComposerGraphI_t* builderGraphI;
    //FILE * fd = graphI->par->par->fd;              // Detail output file

    ComposerGraphIMap_t::iterator srch = graphIMap.find(graphI);
    if (srch == graphIMap.end())
    {   // The Graph Instance has not been seen before, map it.
        builderGraphI = new ComposerGraphI_t(graphI, outputPath);

        // Insert the GraphI
        std::pair<ComposerGraphIMap_t::iterator, bool> insertedGraphI;
        insertedGraphI = graphIMap.insert(ComposerGraphIMap_t::value_type
                                                (graphI, builderGraphI));

    } else {
        builderGraphI = srch->second;
    }
    
    if(builderGraphI->compiled)
    {   // Already compiled, need to decompile
        clean(graphI);
    }
    
    builderGraphI->softswitchLoopMode = loopMode;
    
    return 0;
}


/******************************************************************************
 * Add C flags to the compilation
 *
 * Changing this requires a compiled app to be recompiled.
 *****************************************************************************/
int Composer::addFlags(GraphI_t* graphI, std::string& flags)
{
    ComposerGraphI_t* builderGraphI;
    //FILE * fd = graphI->par->par->fd;              // Detail output file

    ComposerGraphIMap_t::iterator srch = graphIMap.find(graphI);
    if (srch == graphIMap.end())
    {   // The Graph Instance has not been seen before, map it.
        builderGraphI = new ComposerGraphI_t(graphI, outputPath);

        // Insert the GraphI
        std::pair<ComposerGraphIMap_t::iterator, bool> insertedGraphI;
        insertedGraphI = graphIMap.insert(ComposerGraphIMap_t::value_type
                                                (graphI, builderGraphI));

    } else {
        builderGraphI = srch->second;
    }
    
    if(builderGraphI->compiled)
    {   // Already compiled, need to decompile
        clean(graphI);
    }
    
    builderGraphI->compilationFlags += flags;
    builderGraphI->compilationFlags += " ";
    
    return 0;
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
        builderGraphI = new ComposerGraphI_t(graphI, outputPath);

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
    if(placer == PNULL)
    {   // No placer set, we cannot continue.
        fprintf(fd,"\t***No placer set. Please gently shout at an Orchestrator developer.***\n");
        return -3;
    }
    std::map<GraphI_t*, std::set<P_core*> >::iterator giToCoresFinder;
    giToCoresFinder = placer->giToCores.find(graphI);
    if (giToCoresFinder == placer->giToCores.end())
    {   // Something has gone horribly wrong
        fprintf(fd,"\t***FAILED TO FIND ANY CORES***\n");
        return -2;
    }
    builderGraphI->cores = &(giToCoresFinder->second);
    
    
    // Cache the file provenance info
    formFileProvenance(builderGraphI);
    
    
    //==========================================================================
    // Write device structures header header
    //==========================================================================
    std::ofstream devst_h;

    std::stringstream devst_hFName;
    devst_hFName << builderGraphI->outputDir;
    devst_hFName << "/" << GENERATED_PATH;
    devst_hFName << "/DeviceStructs.h";
    
    std::string devst_hFNameStr = devst_hFName.str();

    devst_h.open(devst_hFNameStr.c_str());
    if(devst_h.fail()) // Check that the file opened
    {                 // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, vars_hFName.str(), OSFixes::getSysErrorString(errno));
        return -1;
    }
    writeFileProvenance(devst_hFNameStr, builderGraphI, devst_h);
    writeDeviceStructTypesPreamble(devst_h);
    
    //Form Device Type strings for all DevTs in the GraphT and write the device
    //structs
    builderGraphI->clearDevTStrsMap();      // sanity clear
    WALKVECTOR(DevT_t*,graphI->pT->DevT_v,devT)
    {
        if((*devT)->devTyp == 'D')
        {
            formDevTStrings(builderGraphI, (*devT));
        }
        writeDeviceStructTypes((*devT), devst_h);
    }
    writeDeviceStructTypesPostamble(devst_h);
    devst_h.close();
    
    
    //==========================================================================
    // Write global properties header
    //==========================================================================
    std::ofstream props_h;

    std::stringstream props_hFName;
    props_hFName << builderGraphI->outputDir;
    props_hFName << "/" << GENERATED_PATH;
    props_hFName << "/GlobalProperties.h";

    std::string props_hFNameStr = props_hFName.str();

    props_h.open(props_hFNameStr.c_str());
    if(props_h.fail()) // Check that the file opened
    {                 // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, vars_hFName.str(), OSFixes::getSysErrorString(errno));
        return -1;
    }
    writeFileProvenance(props_hFNameStr, builderGraphI, props_h);
    writeGlobalPropsD(graphI, props_h);
    props_h.close();
    
    
    //==========================================================================
    // Write message format header
    //==========================================================================
    std::ofstream pkt_h;
    std::stringstream pkt_hFName;
    pkt_hFName << builderGraphI->outputDir;
    pkt_hFName << "/" << GENERATED_PATH;
    pkt_hFName << "/MessageFormats.h";

    std::string pkt_hFNameStr = pkt_hFName.str();

    pkt_h.open(pkt_hFNameStr.c_str());
    if(pkt_h.fail()) // Check that the file opened
    {                 // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, vars_hFName.str(), OSFixes::getSysErrorString(errno));
        return -1;
    }
    writeFileProvenance(pkt_hFNameStr, builderGraphI, pkt_h);
    writeMessageTypes(graphI, pkt_h);
    pkt_h.close();


    //==========================================================================
    // Generate Supervisor, inc Dev> Super map
    //==========================================================================
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
        if (createCoreFiles(pCore, builderGraphI,
                            vars_h, vars_cpp,
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

                if (createThreadFile(pThread, builderGraphI, thread_vars_cpp))
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
 * Public method to check the generation state of a Graph instance
 *****************************************************************************/
bool Composer::isGenerated(GraphI_t* graphI)
{
    ComposerGraphI_t* builderGraphI;

    ComposerGraphIMap_t::iterator srch = graphIMap.find(graphI);
    if (srch == graphIMap.end())
    {   // The Graph Instance has not been seen before, so not generated.
        return false;

    } else {
        builderGraphI = srch->second;
    }

    return builderGraphI->generated;
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
        fprintf(fd,"\tAttempt to compile a non-existant application.\n");
        return -1;
    }
    builderGraphI = srch->second;


    if(builderGraphI->compiled)
    {   // Already compiled, nothing to do
        fprintf(fd,"\tApplication already compiled, skipping.\n");
        return 0;
    }
    
    if(!(builderGraphI->generated))
    {   // Application not generated, barf
        fprintf(fd,"\tAttempt to compile a non-generated application.\n");
        return -1;
    }

    // Make the bin directory
    std::string taskDir(builderGraphI->outputDir);
    std::string elfPath(taskDir + "/bin");
    if(system((MAKEDIR + " " + elfPath).c_str()))
    {
        //par->Post(818, mkdirBinPath, OSFixes::getSysErrorString(errno));
        fprintf(fd,"\tFailed to create %s\n",elfPath.c_str());
        return 1;
    }


    // Make the magic happen: call make
    std::string buildPath(builderGraphI->outputDir);
    buildPath += "/Build";
    
    
    // Form the Softswitch compilation control string
    //TODO: Make these const strings at the top
    std::string makeArgs = "";
	
    //TODO: make this configurable
    makeArgs += "SOFTSWITCH_HWIDLE_BARRIER=1 ";

    if(builderGraphI->bufferingSoftswitch)
    {   // Softswitch needs to be built in buffering mode
        makeArgs += "SOFTSWITCH_BUFFERING=1 ";
    }
    
    if(!(builderGraphI->softswitchRequestIdle))
    {   // Softswitch needs to be built without requestIdle
        makeArgs += "SOFTSWITCH_NOREQUESTIDLE=1 ";
    }
    
    if(!(builderGraphI->softswitchInstrumentation))
    {   // Softswitch needs to be built with instrumentation disabled
        makeArgs += "SOFTSWITCH_DISABLE_INSTRUMENTATION=1 ";
    }
    
    switch(builderGraphI->softswitchLogHandler)
    {   // Control the log handler 
        case trivial:   makeArgs += "SOFTSWITCH_TRIVIAL_LOG_HANDLER=1 ";
                        break;
        
        default:        break;  // De Nada!
    }
    
    switch(builderGraphI->softswitchLoopMode)
    {   // Control the main loop order
        case priInstr:  makeArgs += "SOFTSWITCH_PRIORITISE_INSTRUMENTATION=1 ";
                        break;
        
        default:        break;  // De Nada!
    }
    
    // Set the softswitch log level
    makeArgs += "SOFTSWITCH_LOGLEVEL=";
    makeArgs += TO_STRING(builderGraphI->softswitchLogLevel);
    makeArgs += " ";
    
    // Add the user-supplied flags if they exist
    if(builderGraphI->compilationFlags.size())
    {
        makeArgs += "CM_CFLAGS=\"";
        makeArgs += builderGraphI->compilationFlags;
        makeArgs += "\" ";
    }
    
    
    fprintf(fd,"\tMake called with %s%s%s\n",COREMAKE.c_str(),makeArgs.c_str(),
                                                COREMAKEPOST.c_str());
    if(system(("(cd "+buildPath+";"+COREMAKE+makeArgs+COREMAKEPOST+")").c_str()))
    {   // The build failed. Shout about it!
    
        std::string prefix = dformat("Compilation failed! make_errs.txt dump");
        DumpUtils::open_breaker(fd, prefix);
        fprintf(fd,"\n");
        
        // Copy make_errs.txt into the microlog.
        std::string makeErrsFName = buildPath;
        makeErrsFName += "/make_errs.txt";
        char MEBuff [100];
        
        FILE * makeErrsF = fopen(makeErrsFName.c_str(), "r");
        if(makeErrsF == PNULL)
        {
            fprintf(fd,"\tThe make errors file cannot be opened. Sorry!\n");
        }
        else
        {
            while(!feof(makeErrsF))
            {
                if(fgets(MEBuff , 100 , makeErrsF) == PNULL) break;
                fputs(MEBuff , fd);
            }
            fclose(makeErrsF);
        }
        
        fprintf(fd,"\n");
        DumpUtils::close_breaker(fd, prefix);
        fflush(fd);
        
        return 1;
    }

    if(checkBinaries(builderGraphI) != 0) return -1;

    // Mark that we have compiled
    builderGraphI->compiled = true;
    builderGraphI->graphI->built = true;

    return 0;
}

/******************************************************************************
 * Public method to check the compilation state of a Graph instance
 *****************************************************************************/
bool Composer::isCompiled(GraphI_t* graphI)
{
    ComposerGraphI_t* builderGraphI;

    ComposerGraphIMap_t::iterator srch = graphIMap.find(graphI);
    if (srch == graphIMap.end())
    {   // The Graph Instance has not been seen before, so not compiled.
        return false;

    } else {
        builderGraphI = srch->second;
    }

    return builderGraphI->compiled;
}

/******************************************************************************
 * Invoke a clean and then a degenerate
 *****************************************************************************/
int Composer::decompose(GraphI_t* graphI)
{
    FILE * fd = graphI->par->par->fd;              // Detail output file
    fprintf(fd,"\nDecomposing %s...\n",graphI->par->Name().c_str());

    int ret = clean(graphI);

    if(ret) return ret;
    else return degenerate(graphI, true);
}


/******************************************************************************
 * Undo the generation step by removing any generated files and removing the
 * graphIMap entry.
 *****************************************************************************/
int Composer::degenerate(GraphI_t* graphI, bool del)
{
    ComposerGraphI_t*  builderGraphI;
    FILE * fd = graphI->par->par->fd;              // Detail output file

    ComposerGraphIMap_t::iterator graphISrch = graphIMap.find(graphI);
    if (graphISrch == graphIMap.end())
    {   // The Graph Instance has not been seen before, barf.
        return -1;
    }
    builderGraphI = graphISrch->second;

    if(builderGraphI->compiled)
    {   // Application needs to be cleaned first
        fprintf(fd,"\tApplication must be cleaned before degenerating.\n");
        return -2;
    }

    if(!builderGraphI->generated)
    {   // Nothing generated, nothing to do
        if(!del)
        {
            fprintf(fd,"\tApplication not generated, skipping\n");
            return 0;
        }
    }
    else
    {
        // Remove the application directory.
        //TODO: make this safer. Currently the remove uses an "rm -rf" without any safety.
        std::string taskDir(builderGraphI->outputDir);
        if(system((REMOVEDIR+" "+taskDir).c_str())) // Check that the directory deleted
        {                                  // if it didn't, tell logserver and exit
            //par->Post(817, task_dir, OSFixes::getSysErrorString(errno));
            fprintf(fd,"\tFailed to remove %s\n",taskDir.c_str());
        }

        builderGraphI->generated = 0;
        builderGraphI->provenanceCache = "";
        builderGraphI->supevisorDevIVect.clear();
        builderGraphI->devISuperIdxMap.clear();
        builderGraphI->clearDevTStrsMap();
    }
    
    if(del)
    {
        delete graphISrch->second;
        graphIMap.erase(graphISrch);
    }

    return 0;
}

/******************************************************************************
 * Undo the compilation step by invoking a make clean.
 *****************************************************************************/
int Composer::clean(GraphI_t* graphI)
{   // Tidy up a specific build for a graphI. This invokes make clean
    ComposerGraphI_t*  builderGraphI;
    FILE * fd = graphI->par->par->fd;              // Detail output file

    ComposerGraphIMap_t::iterator srch = graphIMap.find(graphI);
    if (srch == graphIMap.end())
    {   // The Graph Instance has not been seen before, barf.
        return -1;
    }
    builderGraphI = srch->second;

    std::string buildPath(builderGraphI->outputDir);
    buildPath += "/Build";
    if(system(("(cd "+buildPath+";"+COREMAKECLEAN+")").c_str()))
    {
        //TODO: barf?
    }

    // Remove the bin directory.
    //TODO: make this safer. Currently the remove uses an "rm -rf" without any safety.
    std::string taskDir(builderGraphI->outputDir);
    if(system((REMOVEDIR+" "+taskDir+"/bin").c_str())) // Check that the directory deleted
    {                                  // if it didn't, tell logserver and exit
        //par->Post(817, task_dir, OSFixes::getSysErrorString(errno));
        fprintf(fd,"\tFailed to remove %s\n",taskDir.c_str());
    }

    // Cleanup the core Binary paths.
    WALKSET(P_core*,(*(builderGraphI->cores)),coreNode)
    {
        P_core* pCore = (*coreNode);
        pCore->instructionBinary = "";
    }

    // Cleanup Supervisor binary path.
    builderGraphI->graphI->pSupI->binPath = "";
    
    builderGraphI->compiled = false;
    builderGraphI->graphI->built = false;
    return 0;
}


/******************************************************************************
 * Bypass the generate and compile steps, making use of existing binaries.
 *****************************************************************************/
int Composer::bypass(GraphI_t* graphI)
{
    ComposerGraphI_t*  builderGraphI;
    FILE * fd = graphI->par->par->fd;              // Detail output file
    fprintf(fd,"\nComposer bypassing %s (making use of existing binaries)...\n",
                graphI->par->Name().c_str());
    
    
    ComposerGraphIMap_t::iterator srch = graphIMap.find(graphI);
    if (srch == graphIMap.end())
    {   // The Graph Instance has not been seen before, map it.
        builderGraphI = new ComposerGraphI_t(graphI, outputPath);

        // Insert the GraphI
        std::pair<ComposerGraphIMap_t::iterator, bool> insertedGraphI;
        insertedGraphI = graphIMap.insert(ComposerGraphIMap_t::value_type
                                                (graphI, builderGraphI));

    } else {
        builderGraphI = srch->second;
    }
    
    // Get the list of cores
    if(placer == PNULL)
    {   // No placer set, we cannot continue.
        fprintf(fd,"\t***No placer set. Please gently shout at an Orchestrator developer.***\n");
        return -3;
    }
    std::map<GraphI_t*, std::set<P_core*> >::iterator giToCoresFinder;
    giToCoresFinder = placer->giToCores.find(graphI);
    if (giToCoresFinder == placer->giToCores.end())
    {   // Something has gone horribly wrong
        fprintf(fd,"\t***FAILED TO FIND ANY CORES***\n");
        return -2;
    }
    builderGraphI->cores = &(giToCoresFinder->second);
    
    
    if(builderGraphI->generated && builderGraphI->compiled) 
    {   // Already generated and compiled, nothing to do.
        fprintf(fd,"\tApplication already generated and compiled, skipping\n");
        return 0;
    }
    if(builderGraphI->generated || builderGraphI->compiled)
    {   // Already generated or compiled, cannot proceed in this state.
        fprintf(fd,"\tApplication already generatedor compiled, cannot bypass\n");
        return -1;
    }
    
    
    //TODO: check that a hash of the XML and Orchestrator settings match.
    
    // Mark that the graph is generated.
    builderGraphI->generated = true;
    
    // Check that the binaries for all of the cores exist
    if(checkBinaries(builderGraphI) != 0 ) return -1;
    
    // Mark that we have compiled
    builderGraphI->compiled = true;
    builderGraphI->graphI->built = true;
    
    return 0;
}


/******************************************************************************
 * Check that binaries exist for each core.
 *****************************************************************************/
int Composer::checkBinaries(ComposerGraphI_t* builderGraphI)
{
    GraphI_t* graphI = builderGraphI->graphI;
    FILE * fd = graphI->par->par->fd;              // Detail output file
    
    std::string taskDir(builderGraphI->outputDir);
    std::string elfPath(taskDir + "/bin");
    
    
    // Check that the "dummy" binaries for HW idle were generated.
    FILE* dummyBinary;
    
    // Check Dummy Instruction binary and add to GraphI
    std::string dummyPath = elfPath + "/dummy_code.v";
    dummyBinary = fopen(dummyPath.c_str(), "r");
    if(dummyBinary == PNULL)
    { // Failed to open binary
        fprintf(fd,"\tFailed to open dummy instruction binary %s after compilation\n",
                    dummyPath.c_str());
        return -1;
    }
    fclose(dummyBinary);
    builderGraphI->idleInstructionBinary = dummyPath;
    
    
    // Check Dummy Data binary and add to GraphI
    dummyPath = elfPath + "/threadCtxInit_data.v";
    dummyBinary = fopen(dummyPath.c_str(), "r");
    if(dummyBinary == PNULL)
    { // Failed to open binary
        fprintf(fd,"\tFailed to open dummy data binary %s after compilation\n",
                    dummyPath.c_str());
        return -1;
    }
    fclose(dummyBinary);
    builderGraphI->idleDataBinary = dummyPath;
    
    
    
    // Check that the core binaries were made and link to each core.
    WALKSET(P_core*,(*(builderGraphI->cores)),coreNode)
    {
        P_core* pCore = (*coreNode);
        uint32_t coreAddr = pCore->get_hardware_address()->as_uint();

        std::string elfName = elfPath + "/softswitch_";
        elfName += TO_STRING(coreAddr);
        elfName += ".elf";

        // Open the Elf to check existence
        FILE* elfBinary = fopen(elfName.c_str(),"r");
        if(elfBinary != PNULL)
        {   // We have the Elf, check that we have the individual binaries
            FILE* binary;

            // Check Instruction binary and add to pCore.
            const std::string instrName = "softswitch_code_" +
                TO_STRING(coreAddr) + ".v";
            std::string instrPath = elfPath + "/" + instrName;

            binary = fopen(instrPath.c_str(), "r");
            if(binary == PNULL)
            { // Failed to open binary
                fprintf(fd,"\tFailed to open instruction binary %s after compilation\n",
                            instrPath.c_str());
                fclose(elfBinary);
                return -1;
            }
            fclose(binary);

            pCore->instructionBinary = instrName;



            // Check Data binary and add to pCore.
            const std::string dataName = "softswitch_data_" +
                TO_STRING(coreAddr) + ".v";
            std::string dataPath = elfPath + "/" + dataName;

            binary = fopen(dataPath.c_str(), "r");
            if(binary == PNULL)
            { // Failed to open binary
                fprintf(fd,"\tFailed to open data binary %s after compilation\n",
                            dataPath.c_str());
                fclose(elfBinary);
                return -1;
            }
            fclose(binary);

            pCore->dataBinary = dataName;
        }
        else
        { // Failed to open the Elf
            fprintf(fd,"\tFailed to open elf %s after compilation\n",
                            elfName.c_str());
            return -1;
        }

        // Close the binary
        fclose(elfBinary);
    }

    // Check that the Supervisor Shared Object was made
    FILE* superBinary;
    const std::string superName = "libSupervisor.so";
    std::string superPath = elfPath + "/" + superName;
    superBinary = fopen(superPath.c_str(),"r");
    if(superBinary == PNULL)
    { // Failed to open the Shared Object
        fprintf(fd,"\tFailed to open Supervisor binary %s after compilation\n",
                            superPath.c_str());
        return -1;
    }
    fclose(superBinary);

    // Add Supervidor filename to Graph Instance
    graphI->pSupI->binPath = superName;
    
    return 0;
}

/******************************************************************************
 * Form a cache of the common file provenance blurb.
 *****************************************************************************/
void Composer::formFileProvenance(ComposerGraphI_t* builderGraphI)
{
    GraphI_t* graphI = builderGraphI->graphI;
    std::stringstream provStr;

    //TODO: this should be cached
    provStr << " * Graph Instance XML:\t\t" << graphI->par->filename << "\n";
    provStr << " * Graph:\t\t\t\t\t" << graphI->FullName() << "\n";
    provStr << " * Graph Type:\t\t\t\t";
    provStr << (graphI->tyId2.empty() ? graphI->tyId : graphI->tyId2) << "\n";
    provStr << " * Platform Definition:\t" << "\n";
    provStr << " * Placement Method:\t\t";

    std::map<GraphI_t*, Algorithm*>::iterator pGIter;
    pGIter = placer->placedGraphs.find(graphI);
    if (pGIter == placer->placedGraphs.end())
    {   // Something has gone horribly horribly wrong
        provStr << "***UNPLACED***\n";
    }
    else
    {
        provStr << pGIter->second->result.method <<"\n";
    }
    
    
    provStr << " * Softswitch control:\n";
    provStr << " *   Buffering mode:\t\t";
    provStr << (builderGraphI->bufferingSoftswitch ? "true" : "false") << "\n";
    
    provStr << " *   Instrumentation:\t\t";
    provStr << (builderGraphI->softswitchInstrumentation ? "true" : "false");
    provStr << "\n";
    
    provStr << " *   Log handler:\t\t\t";
    switch(builderGraphI->softswitchLogHandler)
    {   // Control the log handler 
        case trivial:   provStr << "trivial\n";                         break;
        default:        provStr << "none\n";                            break;
    }
    
    provStr << " *   Log level:\t\t\t\t";
    provStr << builderGraphI->softswitchLogLevel << "\n";
    
    provStr << " *   Loop mode:\t\t\t\t";
    switch(builderGraphI->softswitchLoopMode)
    {   // Control the main loop order
        case priInstr:  provStr << "prioritise instrumentation\n";      break;
        default:        provStr << "default\n";                         break;
    }
    
    builderGraphI->provenanceCache = provStr.str();
}

/******************************************************************************
 * Write the file provenance blurb to the file.
 *****************************************************************************/
void Composer::writeFileProvenance(std::string& fName,
                                    ComposerGraphI_t* builderGraphI,
                                    std::ofstream& f)
{
    time_t timeNtv;
    time(&timeNtv);
    char timeBuf[sizeof "YYYY-MM-DDTHH:MM:SS"];
    strftime(timeBuf,sizeof timeBuf,"%Y_%m_%dT%H_%M_%S",localtime(&timeNtv));

    f << "/* " << fName << "\n";
    f << " * Generated at " << timeBuf << "\n";

    // Add the cached string
    f << builderGraphI->provenanceCache;

    f << " */\n\n";
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
    std::string taskDir(builderGraphI->outputDir);
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
    cpCmd << outputPath << "Supervisor* ";                      // Sources
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
    GraphI_t* graphI = builderGraphI->graphI;

    //Create the graph instance-specific ones
    std::ofstream supervisor_cpp, supervisor_h, supervisor_bin;


    // Create the Supervisor header
    std::stringstream supervisor_hFName;
    supervisor_hFName << builderGraphI->outputDir << "/" << GENERATED_PATH;
    supervisor_hFName << "/supervisor_generated.h";

    std::string supervisor_hFNameStr = supervisor_hFName.str();

    supervisor_h.open(supervisor_hFNameStr.c_str());
    if(supervisor_h.fail()) // Check that the file opened
    {                 // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, vars_hFName.str(), OSFixes::getSysErrorString(errno));
        return -1;
    }


    // Create the Supervisor source
    std::stringstream supervisor_cppFName;
    supervisor_cppFName << builderGraphI->outputDir << "/" << GENERATED_PATH;
    supervisor_cppFName << "/supervisor_generated.cpp";

    std::string supervisor_cppFNameStr = supervisor_cppFName.str();

    supervisor_cpp.open(supervisor_cppFNameStr.c_str());
    if(supervisor_cpp.fail()) // Check that the file opened
    {                 // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, vars_hFName.str(), OSFixes::getSysErrorString(errno));
        return -1;
    }
    
    // Write the file provenance headers
    writeFileProvenance(supervisor_hFNameStr, builderGraphI, supervisor_h);
    supervisor_h << "#ifndef __SupervisorGeneratedH__H\n";
    supervisor_h << "#define __SupervisorGeneratedH__H\n\n";

    writeFileProvenance(supervisor_cppFNameStr, builderGraphI, supervisor_cpp);
    supervisor_cpp << "#include \"supervisor_generated.h\"\n";
    supervisor_cpp << "#include \"Supervisor.h\"\n\n";

    // Write static member initialisors for Supervisor::init & Supervisor::api
    supervisor_cpp << "bool Supervisor::__SupervisorInit = false;\n";
    supervisor_cpp << "SupervisorApi Supervisor::__api;\n";

    // As part of this, we need to generate an edge index for each device on
    // this supervisor. For now, that is all devices so we (ab)use Digraph.
    // TODO: change for multi supervisor.
    int devIdx = 0; // Faster than using std::distance
    builderGraphI->supevisorDevIVect.clear();       // Sanity clear
    builderGraphI->devISuperIdxMap.clear();         // sanity clear
    
    
      
    // Create the Supervisor's DeviceVector binary blob
    std::stringstream supervisor_binFName;
    supervisor_binFName << builderGraphI->outputDir << "/" << GENERATED_PATH;
    supervisor_binFName << "/supervisor.bin";
    std::string supervisor_binFNameStr = supervisor_binFName.str();

    supervisor_bin.open(supervisor_binFNameStr.c_str());
    if(supervisor_bin.fail()) // Check that the file opened
    {                 // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, vars_hFName.str(), OSFixes::getSysErrorString(errno));
        return -1;
    }
    
    WALKPDIGRAPHNODES(unsigned,DevI_t *,unsigned,EdgeI_t *,unsigned,PinI_t *,graphI->G,i)
    {
        DevI_t* devI = graphI->G.NodeData(i);

        if(devI->pT->devTyp != 'D') continue;

        builderGraphI->supevisorDevIVect.push_back(devI);
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
        
        
        // To keep compilation times reasonable, we create a binary 
        // representation of the Supervisor's DeviceVector in a file. This file
        // is turned into a linkable object as part of the compilation process.
        // When done as an initialiser or initialiser method, the compilation 
        // times for the Supervisor shared object were unreasonable.
        SupervisorDeviceInstance_t tmpSDevI = {
                        thread->get_hardware_address()->as_uint(),
                        devI->addr.as_uint(), ""};
        strcpy(tmpSDevI.Name, devI->Name().c_str());
        supervisor_bin.write((char*)(&tmpSDevI), sizeof(SupervisorDeviceInstance_t));
        
        devIdx++;
    }
    
    supervisor_bin.close(); // We are done with the binary for now.
    
    devIdx++;       // this now becomes a device count rather than index.
    
    
    supervisor_cpp << "#pragma GCC push_options\n";     // Speed up compiles by NOT optimising
    supervisor_cpp << "#pragma GCC optimize (\"O0\")\n";  // devicevector initialisation.
    
    
    // Write the static initialisor for the Device Vector
    supervisor_cpp << "extern SupervisorDeviceInstance_t _binary_supervisor_bin_start[];\n";
    supervisor_cpp << "extern SupervisorDeviceInstance_t _binary_supervisor_bin_end[];\n";
    supervisor_cpp << "const std::vector<SupervisorDeviceInstance_t> ";
    supervisor_cpp << "Supervisor::DeviceVector(_binary_supervisor_bin_start,";
    supervisor_cpp << "_binary_supervisor_bin_end);\n\n";
    

    // Fill a vector of thread hardware addresses that this supervisor is responsible for
    int threadIdx = 0; // Faster than using std::distance

    supervisor_cpp << "const std::vector<uint32_t> ";
    supervisor_cpp << "Supervisor::ThreadVector = { ";
    WALKSET(P_core*,(*(builderGraphI->cores)),coreNode)
    {
        P_core* pCore = (*coreNode);
        WALKMAP(AddressComponent,P_thread*,pCore->P_threadm,threadIterator)
        {   // Iterate over the threads on this core
            P_thread* pThread = threadIterator->second;
            if (placer->threadToDevices[pThread].size())
            {   // if there are devices placed on the thread
                // Add some line splitting
                if(threadIdx%100 == 0) supervisor_cpp << "\n\t";
            
                supervisor_cpp << pThread->get_hardware_address()->as_uint();
                supervisor_cpp << ",";
                
                threadIdx++;
            }
        }
    }
    supervisor_cpp.seekp(-1,ios_base::cur); // Rewind one place to remove the stray ","
    supervisor_cpp << "\n};\n";               // properly terminate the initialiser
    supervisor_cpp << "#pragma GCC pop_options\n\n";


    // Add references for static class members
    supervisor_cpp << "SupervisorProperties_t* ";
    supervisor_cpp << "Supervisor::__SupervisorProperties;\n";
    supervisor_cpp << "SupervisorState_t* Supervisor::__SupervisorState;\n\n";

    // Make a global pointer for Supervisot properties and State
    supervisor_cpp << "SupervisorProperties_t* supervisorProperties;\n";

    supervisor_cpp << "SupervisorState_t* supervisorState;\n\n";


    // Default Supervisor handler strings
    std::string supervisorOnInitHandler = "";
    std::string supervisorOnStopHandler = "";
    std::string supervisorOnImplicitHandler = "return -1;";
    std::string supervisorOnPktHandler = "return -1;";
    std::string supervisorOnCtlHandler = "return 0;";
    std::string supervisorOnIdleHandler = "return 0;";
    std::string supervisorOnRTCLHandler = "return 0;";


    // Default Properties and state content
    std::string supervisorPropertiesBody = "\tbool dummy;";
    std::string supervisorStateBody = "\tbool dummy;";
    
    // Default message struct refs
    std::string inPktFmt = "pkt___default_pyld_t*";
    std::string outPktFmt = "pkt___default_pyld_t*";
    
    // Include the generated global props and messages for all supervisors. 
    supervisor_h << "#include \"GlobalProperties.h\"\n";
    supervisor_h << "#include \"MessageFormats.h\"\n\n";
    supervisor_h << "#include \"DeviceStructs.h\"\n\n";
    
    if(graphI->pT->pSup)    // If we have a non-default Supervisor, build it.
    {
        SupT_t* supType = graphI->pT->pSup;

        supervisor_h << "#define _APPLICATION_SUPERVISOR_ 1\n\n";
        
        supervisor_h << "#include \"Supervisor.h\"\n\n";

        // Global properties initialiser
        writeGlobalPropsI(graphI, supervisor_cpp);
        if(graphI->pT->pPropsD)
        {
            supervisor_cpp << "const global_props_t* graphProperties ";
            supervisor_cpp << "= &GraphProperties;\n\n";

            supervisor_h << "extern const global_props_t* graphProperties;\n\n";
        }

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
            supervisor_h << "// =================================== Code ";
            supervisor_h << "====================================\n";
            supervisor_h << supType->pShCd->C_src();
            supervisor_h << "\n\n";
        }

        supervisor_cpp << "// ================================= Handlers ";
        supervisor_cpp << "==================================\n";

        // Properties
        if(supType->pPropsD)
        {
            supervisorPropertiesBody = supType->pPropsD->C_src();
        }

        // State
        if(supType->pStateD)
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
            supervisorOnImplicitHandler = supType->pPinTSI->pHandl->C_src();
            
            // Input packet format string.
            inPktFmt = "pkt_";
            inPktFmt += supType->pPinTSI->pMsg->Name();
            inPktFmt += "_pyld_t*";
            
            // Output packet format string
            // set in case there is no implicit output pin for replies, etc.
            outPktFmt = "pkt_";
            outPktFmt += supType->pPinTSI->pMsg->Name();
            outPktFmt += "_pyld_t*";
        }
        
        // Implicit send handler
        if(supType->pPinTSO)
        {    
            // Ignore the handler for now, but extract the packet format
            
            outPktFmt = "pkt_";
            outPktFmt += supType->pPinTSO->pMsg->Name();
            outPktFmt += "_pyld_t*";
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

    // Init Handler
    supervisor_cpp << "int Supervisor::OnInit()\n{\n";
    supervisor_cpp << "\tif(__SupervisorInit) return -1;\n";
    supervisor_cpp << "\t__SupervisorProperties = new SupervisorProperties_t;\n";
    supervisor_cpp << "\t__SupervisorState = new SupervisorState_t;\n\n";
    supervisor_cpp << "\tsupervisorProperties = __SupervisorProperties;\n";
    supervisor_cpp << "\tsupervisorState = __SupervisorState;\n\n";
    supervisor_cpp << "\t__SupervisorInit = true;\n\n";
    
    supervisor_cpp << supervisorOnInitHandler;
    supervisor_cpp << "\n\n\treturn 0;\n";
    supervisor_cpp << "}\n";
    supervisor_cpp << "\n";

    // Stop handler
    supervisor_cpp << "int Supervisor::OnStop()\n{\n";
    supervisor_cpp << "\tif(!__SupervisorInit) return -1;\n";
    supervisor_cpp << supervisorOnStopHandler;
    supervisor_cpp << "\n\n\tdelete __SupervisorProperties;\n";
    supervisor_cpp << "\tdelete __SupervisorState;\n";
    supervisor_cpp << "\tsupervisorProperties = 0;\n";
    supervisor_cpp << "\tsupervisorState = 0;\n\n";
    supervisor_cpp << "\t__SupervisorInit = false;\n\n";
    supervisor_cpp << "\treturn 0;\n";
    supervisor_cpp << "}\n\n";

    // OnImplicit
    supervisor_cpp << "int Supervisor::OnImplicit(P_Pkt_t* __inPkt, ";
    supervisor_cpp << "std::vector<P_Addr_Pkt_t>& __outPkt)\n{\n";
    
    supervisor_cpp << "\tconst " << inPktFmt << " message";
    supervisor_cpp << " OS_ATTRIBUTE_UNUSED= ";
    supervisor_cpp << "static_cast<const " << inPktFmt << ">";
    supervisor_cpp << "(static_cast<const void*>(__inPkt->payload));\n";
    supervisor_cpp << "\tOS_PRAGMA_UNUSED(message)\n\n";
    
    supervisor_cpp << "\tP_Pkt_t __reply OS_ATTRIBUTE_UNUSED;\n";
    supervisor_cpp << "\t" << outPktFmt << " reply OS_ATTRIBUTE_UNUSED= ";
    supervisor_cpp << "static_cast<" << outPktFmt << ">";
    supervisor_cpp << "(static_cast<void*>(&(__reply.payload)));\n";
    supervisor_cpp << "\tOS_PRAGMA_UNUSED(reply)\n\n";
    
    supervisor_cpp << "\tP_Pkt_t __bcast OS_ATTRIBUTE_UNUSED;\n";
    supervisor_cpp << "\t" << outPktFmt << " broadcast OS_ATTRIBUTE_UNUSED= ";
    supervisor_cpp << "static_cast<" << outPktFmt << ">";
    supervisor_cpp << "(static_cast<void*>(&(__bcast.payload)));\n";
    supervisor_cpp << "\tOS_PRAGMA_UNUSED(broadcast)\n\n";
    
    supervisor_cpp << "\tbool __rtsBcast = false;\n";
    supervisor_cpp << "\tbool __rtsReply = false;\n\n";
    
    supervisor_cpp << supervisorOnImplicitHandler << "\n\n";
    
        // Reply packet logic
    supervisor_cpp << "\tif(__rtsReply)\n\t{\n";
    supervisor_cpp << "\t\tP_Addr_Pkt_t __oPkt;\n";
    supervisor_cpp << "\t\tconst SupervisorDeviceInstance_t* tgt = ";
    supervisor_cpp << "&DeviceVector[__inPkt->header.pinAddr];\n";
    supervisor_cpp << "\t\t__reply.header.swAddr = tgt->SwAddr;\n";
    supervisor_cpp << "\t\t__reply.header.swAddr |= ";
    supervisor_cpp << "(P_CNC_IMPL << P_SW_OPCODE_SHIFT) & P_SW_OPCODE_MASK;\n";
    supervisor_cpp << "\t\t__oPkt.hwAddr = tgt->HwAddr;\n";
    supervisor_cpp << "\t\t__oPkt.packet = __reply;\n";
    supervisor_cpp << "\t\t__outPkt.push_back(__oPkt);\n";
    supervisor_cpp << "\t}\n\n";
    
        // Broadcast packet logic
    supervisor_cpp << "\tif(__rtsBcast)\n\t{\n";
    supervisor_cpp << "\t\t__bcast.header.swAddr = (P_ADDR_BROADCAST << ";
    supervisor_cpp << "P_SW_DEVICE_SHIFT) & P_SW_DEVICE_MASK;\n";
    supervisor_cpp << "\t\t__bcast.header.swAddr |= ";
    supervisor_cpp << "(P_CNC_IMPL << P_SW_OPCODE_SHIFT) & P_SW_OPCODE_MASK;\n";
    supervisor_cpp << "\t\tWALKCVECTOR(uint32_t,ThreadVector,threadTgt)\n\t{\n";
    supervisor_cpp << "\t\t\tP_Addr_Pkt_t __oPkt;\n";
    supervisor_cpp << "\t\t\t__oPkt.hwAddr = (*threadTgt);\n";
    supervisor_cpp << "\t\t\t__oPkt.packet = __bcast;\n";
    supervisor_cpp << "\t\t\t__outPkt.push_back(__oPkt);\n";
    supervisor_cpp << "\t\t}\n";
    
    supervisor_cpp << "\t}\n\n";
    
    
    supervisor_cpp << "\n\treturn 0;\n";
    supervisor_cpp << "}\n\n";


    // OnPkt - essentially a stub for now
    supervisor_cpp << "int Supervisor::OnPkt(P_Pkt_t* __inPkt, ";
    supervisor_cpp << "std::vector<P_Addr_Pkt_t>& __outPkt)\n{\n";

    supervisor_cpp << "\tconst " << inPktFmt << " message";
    supervisor_cpp << " OS_ATTRIBUTE_UNUSED= ";
    supervisor_cpp << "static_cast<const " << inPktFmt << ">(";
    supervisor_cpp << "static_cast<const void*>(__inPkt->payload));\n";
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

    std::string fName = "GlobalProperties.h";

    props_h << "#include <cstdint>\n";

    if(graphT->pPropsD)
    {
        //Definition comes from the Graph Type
        props_h << "typedef struct " << graphT->Name() << "_properties_t \n{\n";
        props_h << graphT->pPropsD->C_src();   // These are the actual properties
        props_h << "\n} global_props_t;\n\n";
        props_h << "extern const global_props_t GraphProperties;\n\n";
    }

    props_h << "#endif /*_GLOBALPROPS_H_*/\n\n";
}


/******************************************************************************
 * Write the Global Properties initialiser
 *****************************************************************************/
void Composer::writeGlobalPropsI(GraphI_t* graphI, std::ofstream& props_cpp)
{
    //There may be an initialiser in the Graph Instance
    if(graphI->pT->pPropsD)
    {
        props_cpp << "const global_props_t GraphProperties ";
        props_cpp << "OS_ATTRIBUTE_UNUSED= ";
        if(graphI->pPropsI)
        {
            props_cpp << graphI->pPropsI->C_src();
        }else{
            props_cpp << "{}";
        }
        props_cpp << ";\n";
        props_cpp << "OS_PRAGMA_UNUSED(GraphProperties)\n";
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
    
    pkt_h <<  "#pragma pack(push,1)\n";
    
    WALKVECTOR(MsgT_t*,graphT->MsgT_v,msg)
    {
        pkt_h << "typedef struct " << graphT->Name() << "_" << (*msg)->Name();
        pkt_h << "_message_t \n{\n";
        pkt_h << (*msg)->pPropsD->C_src();       // The message struct
        pkt_h << "\n} pkt_" << (*msg)->Name() << "_pyld_t;\n\n";
    }
    
    pkt_h <<  "#pragma pack(pop)\n";
    
    pkt_h << "#endif /*_MESSAGETYPES_H_*/\n\n";
}


/******************************************************************************
 * Write the Device structs preamble to a common header
 *****************************************************************************/
void Composer::writeDeviceStructTypesPreamble(std::ofstream& types_h)
{
    types_h << "#ifndef _DEVICESTRUCTS_H_\n";
    types_h << "#define _DEVICESTRUCTS_H_\n\n";

    types_h << "#include <cstdint>\n";
    
    //types_h <<  "#pragma pack(push,1)\n";
}


/******************************************************************************
 * Write the Device structs to a common header
 *****************************************************************************/
void Composer::writeDeviceStructTypes(DevT_t* devT, std::ofstream& types_h)
{
    GraphT_t* graphT = devT->par;
    
    std::string devTName = devT->Name();  // grab a local copy of the name
    std::string graphTName = graphT->Name();    // grab copy of the name
    
    // Write Properties struct declaration
    if (devT->pPropsD)
    {
        types_h << "typedef struct " << graphTName << "_" << devTName;
        types_h << "_properties_t \n{\n" << devT->pPropsD->C_src() << "\n} ";
        types_h << graphTName << "_" << devTName << "_properties_t;\n\n";
    }

    // Write State struct declaration
    if (devT->pStateD)
    {
        types_h << "typedef struct " << graphTName << "_" << devTName;
        types_h << "_state_t \n{\n" << devT->pStateD->C_src() << "\n} ";
        types_h << graphTName << "_" << devTName << "_state_t;\n\n";
    }
    
    // Walk Input pin types
    WALKVECTOR(PinT_t*,devT->PinTI_v,pinI)
    {
        std::string pinIName = (*pinI)->Name();
        
        if ((*pinI)->pPropsD)
        {   // Write the pin's properties struct
            types_h << "typedef struct " << devTName;
            types_h << "_InPin_" << pinIName << "_edgeproperties_t \n{\n";
            types_h << (*pinI)->pPropsD->C_src();
            types_h << "\n} " << graphTName << "_" << devTName << "_";
            types_h << pinIName << "_properties_t;\n\n";
        }
        
        if ((*pinI)->pStateD)
        {   // Write the pin's state struct
            types_h << "typedef struct " << devTName;
            types_h << "_InPin_" << pinIName << "_edgestate_t \n{\n";
            types_h << (*pinI)->pStateD->C_src();
            types_h << "\n} " << graphTName << "_" << devTName << "_";
            types_h << pinIName << "_state_t;\n\n";
        }
    }
}

/******************************************************************************
 * Write the Device structs postamble to a common header
 *****************************************************************************/
void Composer::writeDeviceStructTypesPostamble(std::ofstream& types_h)
{
    //types_h <<  "#pragma pack(pop)\n";
    
    types_h << "#endif /*_DEVICESTRUCTS_H_*/\n\n";
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
        formDevTInputPinHandlers(dTypStrs);
        formDevTOutputPinHandlers(dTypStrs);

        builderGraphI->devTStrsMap.insert(devTStrsMap_t::value_type(devT, dTypStrs));
    }
}

/******************************************************************************
 * Form the common handler preamble (deviceProperties & deviceState)
 *****************************************************************************/
void Composer::formHandlerPreamble(devTypStrings_t* dTypStrs)
{
    DevT_t* devT = dTypStrs->devT;  // grab a local copy of the devtype
    std::string devTName = devT->Name();  // grab a local copy of the name
    
    GraphT_t* graphT = dTypStrs->graphI->pT;    // grab a local copy of the graphtype
    std::string graphTName = graphT->Name();    // grab a local copy of the name

    std::stringstream handlerPreamble_SS("");
    std::stringstream handlerPreambleS_SS("");        // "normal" state
    std::stringstream handlerPreambleCS_SS("");       // const state

    handlerPreamble_SS << "{\n";

    if (devT->par->pPropsD)
    {
        handlerPreamble_SS << "    const global_props_t* graphProperties ";
        handlerPreamble_SS << "OS_ATTRIBUTE_UNUSED= ";
        handlerPreamble_SS << "static_cast<const global_props_t*>";
        handlerPreamble_SS << "(__GraphProps);\n";
        handlerPreamble_SS << "    OS_PRAGMA_UNUSED(graphProperties)\n";
    }
    handlerPreamble_SS << "   PDeviceInstance* deviceInstance ";
    handlerPreamble_SS << "OS_ATTRIBUTE_UNUSED= ";
    handlerPreamble_SS << "static_cast<PDeviceInstance*>(__Device);\n";
    handlerPreamble_SS << "   OS_PRAGMA_UNUSED(deviceInstance)\n";

    // deviceProperties (with unused variable handling)
    if (devT->pPropsD)
    {
        handlerPreamble_SS << "    const " << graphTName << "_" << devTName;
        handlerPreamble_SS << "_properties_t* deviceProperties ";
        handlerPreamble_SS << "OS_ATTRIBUTE_UNUSED= ";
        handlerPreamble_SS << "static_cast<const " << graphTName << "_";
        handlerPreamble_SS << devTName;
        handlerPreamble_SS << "_properties_t*>(deviceInstance->properties);\n";
        handlerPreamble_SS << "    OS_PRAGMA_UNUSED(deviceProperties)\n";
    }

    // deviceState (with unused variable handling)
    if (devT->pStateD)
    {
        // Const-protected state
        handlerPreambleCS_SS << "    const " << graphTName << "_" << devTName;
        handlerPreambleCS_SS << "_state_t* deviceState ";
        handlerPreambleCS_SS << "OS_ATTRIBUTE_UNUSED= ";
        handlerPreambleCS_SS << "static_cast<" << graphTName << "_" << devTName;
        handlerPreambleCS_SS << "_state_t*>(deviceInstance->state);\n";
        handlerPreambleCS_SS << "    OS_PRAGMA_UNUSED(deviceState)\n";

        // "normal" state
        handlerPreambleS_SS << "    " << graphTName << "_" << devTName;
        handlerPreambleS_SS << "_state_t* deviceState ";
        handlerPreambleS_SS << "OS_ATTRIBUTE_UNUSED= ";
        handlerPreambleS_SS << "static_cast<" << graphTName << "_" << devTName;
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
    handlers_h << "_RTS_handler (const void* __GraphProps, ";
    handlers_h << "void* __Device, uint32_t* readyToSend);\n";

    handlers_cpp << "uint32_t devtyp_" << devTName;
    handlers_cpp << "_RTS_handler (const void* __GraphProps, ";
    handlers_cpp << "void* __Device, uint32_t* readyToSend)\n";
    handlers_cpp << dTypStrs->handlerPreamble;
    handlers_cpp << dTypStrs->handlerPreambleCS;
    
    handlers_cpp << "    bool* requestIdle OS_ATTRIBUTE_UNUSED= ";
    handlers_cpp << "&deviceInstance->requestIdle;\n";
    handlers_cpp << "    OS_PRAGMA_UNUSED(requestIdle)\n";
    
    if (devT->pOnRTS != 0) handlers_cpp << devT->pOnRTS->C_src() << "\n";
    // we assume here the return value is intended to be an RTS bitmap.
    handlers_cpp << "    return *readyToSend;\n";
    handlers_cpp << "}\n\n";


    // OnInit
    handlers_h << "uint32_t devtyp_" << devTName;
    handlers_h << "_OnInit_handler (const void* __GraphProps, ";
    handlers_h << "void* __Device);\n";

    handlers_cpp << "uint32_t devtyp_" << devTName;
    handlers_cpp << "_OnInit_handler (const void* __GraphProps, ";
    handlers_cpp << "void* __Device)\n";
    handlers_cpp << dTypStrs->handlerPreamble;
    handlers_cpp << dTypStrs->handlerPreambleS << "\n";

    if (devT->pOnInit) // insert the OnHWIdle handler if there is one
    {
        handlers_cpp << devT->pOnInit->C_src() << "\n";
    }

    handlers_cpp << "    return 1;\n"; // A default Return of 1 to trigger RTS
    handlers_cpp << "}\n\n";


    // OnIdle
    handlers_h << "uint32_t devtyp_" << devTName;
    handlers_h << "_OnIdle_handler (const void* __GraphProps, ";
    handlers_h << "void* __Device);\n";

    handlers_cpp << "uint32_t devtyp_" << devTName;
    handlers_cpp << "_OnIdle_handler (const void* __GraphProps, ";
    handlers_cpp << "void* __Device)\n";
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
    handlers_h << "_OnHWIdle_handler (const void* __GraphProps, ";
    handlers_h << "void* __Device);\n";

    handlers_cpp << "uint32_t devtyp_" << devTName;
    handlers_cpp << "_OnHWIdle_handler (const void* __GraphProps, ";
    handlers_cpp << "void* __Device)\n";
    handlers_cpp << dTypStrs->handlerPreamble;
    handlers_cpp << dTypStrs->handlerPreambleS << "\n";

    if (devT->pOnHWId) // insert the OnHWIdle handler if there is one
    {
        handlers_cpp << devT->pOnHWId->C_src() << "\n";
        handlers_cpp << "    return 1;\n";  // Default return 1
    }
    else handlers_cpp << "    return 0;\n"; // or a stub if not
    handlers_cpp << "}\n\n";
    
    
    // OnImpl
    handlers_h << "uint32_t devtyp_" << devTName;
    handlers_h << "_OnImpl_handler (const void* __GraphProps, ";
    handlers_h << "void* __Device, const void* pkt);\n";
    
    handlers_cpp << "uint32_t devtyp_" << devTName;
    handlers_cpp << "_OnImpl_handler (const void* __GraphProps, ";
    handlers_cpp << "void* __Device, const void* pkt)\n";
    handlers_cpp << dTypStrs->handlerPreamble;
    handlers_cpp << dTypStrs->handlerPreambleS << "\n";
    
    if (devT->pPinTSI) // insert the OnImpl handler if there is one
    {
        // Write the message cast
        handlers_cpp << "   const pkt_" << devT->pPinTSI->pMsg->Name();
        handlers_cpp << "_pyld_t* message";
        handlers_cpp << " OS_ATTRIBUTE_UNUSED= ";
        handlers_cpp << "static_cast<const pkt_" << devT->pPinTSI->pMsg->Name();
        handlers_cpp << "_pyld_t*>(pkt);\n";
        handlers_cpp << "OS_PRAGMA_UNUSED(message)\n";
        
        // Insert the handler
        handlers_cpp << devT->pPinTSI->pHandl->C_src() << "\n";
    }
    
    handlers_cpp << "    return 0;\n"; // or a stub if not
    handlers_cpp << "}\n\n";
    
    

    // OnCtl
    handlers_h << "uint32_t devtyp_" << devTName;
    handlers_h << "_OnCtl_handler (const void* __GraphProps, ";
    handlers_h << "void* __Device, uint8_t __Opcode, const void* pkt);\n";

    handlers_cpp << "uint32_t devtyp_" << devTName;
    handlers_cpp << "_OnCtl_handler (const void* __GraphProps, ";
    handlers_cpp << "void* __Device, uint8_t __Opcode, const void* pkt)\n";
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
 * Form Device Type Input Pin handler strings
 *****************************************************************************/
void Composer::formDevTInputPinHandlers(devTypStrings_t* dTypStrs)
{
    DevT_t* devT = dTypStrs->devT;  // grab a local copy of the devtype
    std::string devTName = devT->Name();  // grab a local copy of the name
    
    GraphT_t* graphT = dTypStrs->graphI->pT;    // grab the graphtype
    std::string graphTName = graphT->Name();    // grab a local copy of the name

    std::stringstream handlers_h("");
    std::stringstream handlers_cpp("");
    std::stringstream types_h("");



    WALKVECTOR(PinT_t*,devT->PinTI_v,pinI)
    {
        std::string pinIName = (*pinI)->Name();

        handlers_h << "uint32_t devtyp_" << devTName;
        handlers_h << "_InPin_" << pinIName;
        handlers_h << "_Recv_handler (const void* __GraphProps, ";
        handlers_h << "void* __Device, void* __Edge, const void* pkt);\n";

        handlers_cpp << "uint32_t devtyp_" << devTName;
        handlers_cpp << "_InPin_" << pinIName;
        handlers_cpp << "_Recv_handler (const void* __GraphProps, ";
        handlers_cpp << "void* __Device, void* __Edge, const void* pkt)\n";
        handlers_cpp << dTypStrs->handlerPreamble;
        handlers_cpp << dTypStrs->handlerPreambleS;
        handlers_cpp << "   inEdge_t* edgeInstance ";
        handlers_cpp << "OS_ATTRIBUTE_UNUSED= ";
        handlers_cpp << "static_cast<inEdge_t*>(__Edge);\n";
        handlers_cpp << "OS_PRAGMA_UNUSED(edgeInstance)\n";

        if ((*pinI)->pPropsD)
        {   // If the pin type has properties, 
            handlers_cpp << "   const " << graphTName << "_" << devTName << "_";
            handlers_cpp << pinIName << "_properties_t* edgeProperties ";
            handlers_cpp << "OS_ATTRIBUTE_UNUSED= ";
            handlers_cpp << "static_cast<const " << graphTName << "_";
            handlers_cpp << devTName << "_" << pinIName;
            handlers_cpp << "_properties_t*>(edgeInstance->properties);\n";
            handlers_cpp << "OS_PRAGMA_UNUSED(edgeProperties)\n";
        }

        if ((*pinI)->pStateD)
        {   // If the pin type has state, 
            handlers_cpp << graphTName << "_" << devTName << "_" << pinIName;
            handlers_cpp << "_state_t* edgeState OS_ATTRIBUTE_UNUSED= ";
            handlers_cpp << "static_cast<" << graphTName << "_";
            handlers_cpp << devTName << "_" << pinIName;
            handlers_cpp << "_state_t*>(edgeInstance->state);\n";
            handlers_cpp << "OS_PRAGMA_UNUSED(edgeState)\n";
        }

        if ((*pinI)->pMsg->pPropsD)
        {   
            handlers_cpp << "   const pkt_" << (*pinI)->pMsg->Name();
            handlers_cpp << "_pyld_t* message";
            handlers_cpp << " OS_ATTRIBUTE_UNUSED= ";
            handlers_cpp << "static_cast<const pkt_" << (*pinI)->pMsg->Name();
            handlers_cpp << "_pyld_t*>(pkt);\n";
            handlers_cpp << "OS_PRAGMA_UNUSED(message)\n";
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
    dTypStrs->typesH += types_h.str();
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
        handlers_h << "_Send_handler (const void* __GraphProps, ";
        handlers_h << "void* __Device, void* pkt);\n";

        handlers_cpp << "uint32_t devtyp_" << devTName;
        handlers_cpp << "_Supervisor_Implicit_OutPin";
        handlers_cpp << "_Send_handler (const void* __GraphProps, ";
        handlers_cpp << "void* __Device, void* pkt)\n";

        handlers_cpp << dTypStrs->handlerPreamble;
        handlers_cpp << dTypStrs->handlerPreambleS;
        handlers_cpp << "\n";

        if (devT->pPinTSO->pMsg->pPropsD)
        {
            handlers_cpp << "   pkt_" << devT->pPinTSO->pMsg->Name();
            handlers_cpp << "_pyld_t* message";
            handlers_cpp << " OS_ATTRIBUTE_UNUSED= ";
            handlers_cpp << "static_cast<pkt_";
            handlers_cpp << devT->pPinTSO->pMsg->Name() << "_pyld_t*>(pkt);\n";
            handlers_cpp << "OS_PRAGMA_UNUSED(message)\n";
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
        handlers_h << "_Send_handler (const void* __GraphProps, ";
        handlers_h << "void* __Device, void* pkt);\n";

        handlers_cpp << "uint32_t devtyp_" << devTName;
        handlers_cpp << "_OutPin_" << pinOName;
        handlers_cpp << "_Send_handler (const void* __GraphProps, ";
        handlers_cpp << "void* __Device, void* pkt)\n";

        handlers_cpp << dTypStrs->handlerPreamble;
        handlers_cpp << dTypStrs->handlerPreambleS;
        handlers_cpp << "\n";

        if ((*pinO)->pMsg->pPropsD)
        {
            handlers_cpp << "   pkt_" << (*pinO)->pMsg->Name();
            handlers_cpp << "_pyld_t* message";
            handlers_cpp << " OS_ATTRIBUTE_UNUSED= ";
            handlers_cpp << "static_cast<pkt_";
            handlers_cpp << (*pinO)->pMsg->Name() << "_pyld_t*>(pkt);\n";
            handlers_cpp << "OS_PRAGMA_UNUSED(message)\n";
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
int Composer::createCoreFiles(P_core* pCore, ComposerGraphI_t* builderGraphI,
                                std::ofstream& vars_h,
                                std::ofstream& vars_cpp,
                                std::ofstream& handlers_h,
                                std::ofstream& handlers_cpp)
{
    uint32_t coreAddr = pCore->get_hardware_address()->as_uint();
    FILE * fd = pCore->parent->parent->parent->parent->parent->fd;  // Microlog
    
    // Create the vars header
    std::stringstream vars_hFName;
    vars_hFName << builderGraphI->outputDir << "/" << GENERATED_H_PATH;
    vars_hFName << "/vars_" << coreAddr << ".h";

    std::string vars_hFNameStr = vars_hFName.str();

    vars_h.open(vars_hFNameStr.c_str());

    if(vars_h.fail()) // Check that the file opened
    {                 // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, vars_hFName.str(), OSFixes::getSysErrorString(errno));
        fprintf(fd,"\t\tFailed to open %s\n",vars_hFName.str().c_str());
        return -1;
    }


    // Create the vars source
    std::stringstream vars_cppFName;
    vars_cppFName << builderGraphI->outputDir << "/" << GENERATED_CPP_PATH;
    vars_cppFName << "/vars_" << coreAddr << ".cpp";

    std::string vars_cppFNameStr = vars_cppFName.str();

    vars_cpp.open(vars_cppFNameStr.c_str());

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
    handlers_hFName << builderGraphI->outputDir << "/" << GENERATED_H_PATH;
    handlers_hFName << "/handlers_" << coreAddr << ".h";

    std::string handlers_hFNameStr = handlers_hFName.str();

    handlers_h.open(handlers_hFNameStr.c_str());

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
    handlers_cppFName << builderGraphI->outputDir << "/" << GENERATED_CPP_PATH;
    handlers_cppFName << "/handlers_" << coreAddr << ".cpp";

    std::string handlers_cppFNameStr = handlers_cppFName.str();

    handlers_cpp.open(handlers_cppFNameStr.c_str());

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


    writeFileProvenance(vars_hFNameStr, builderGraphI, vars_h);
    writeFileProvenance(vars_cppFNameStr, builderGraphI, vars_cpp);

    writeFileProvenance(handlers_hFNameStr, builderGraphI, handlers_h);
    writeFileProvenance(handlers_cppFNameStr, builderGraphI, handlers_cpp);

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
    vars_h << "#include \"DeviceStructs.h\"\n\n";

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
int Composer::createThreadFile(P_thread* pThread,
                                ComposerGraphI_t* builderGraphI,
                                std::ofstream& tvars_cpp)
{
    uint32_t coreAddr = pThread->parent->get_hardware_address()->as_uint();
    uint32_t threadAddr = pThread->get_hardware_address()->get_thread();
    
    // Create the vars source
    std::stringstream tvars_cppFName;
    tvars_cppFName << builderGraphI->outputDir << "/" << GENERATED_CPP_PATH;
    tvars_cppFName << "/vars_" << coreAddr;
    tvars_cppFName << "_" << threadAddr << ".cpp";

    std::string tvars_cppFNameStr = tvars_cppFName.str();

    tvars_cpp.open(tvars_cppFNameStr.c_str());

    if(tvars_cpp.fail()) // Check that the file opened
    {                       // if it didn't, tell logserver and exit
        //TODO: Barf
        //par->Post(816, tvars_cppFName.str(), OSFixes::getSysErrorString(errno));
        return -1;
    }

    writeFileProvenance(tvars_cppFNameStr, builderGraphI, tvars_cpp);

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

    writeThreadContextInitialiser(builderGraphI, thread, devT,
                                    vars_h, thread_vars_cpp);

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
void Composer::writeThreadContextInitialiser(ComposerGraphI_t* builderGraphI,
                                        P_thread* thread, DevT_t* devT,
                                        std::ofstream& vars_h,
                                        std::ofstream& vars_cpp)
{
    
    FILE * fd = builderGraphI->graphI->par->par->fd;       // Detail output file
    
    AddressComponent threadAddr = thread->get_hardware_address()->get_thread();
    std::list<DevI_t*>::size_type numberOfDevices =
        placer->threadToDevices.at(thread).size();  // Get the thread dev count

    size_t outTypCnt = devT->PinTO_v.size();      // Number of output pins
    bool rtsOF = false;                           // Flag to indicate RTS sz OF

    vars_cpp << "ThreadCtxt_t Thread_" << threadAddr << "_Context ";
    vars_cpp << "__attribute__((section (\".thr" << threadAddr << "_base\"))) ";
    vars_cpp << "= {";
    vars_cpp << "1,";                                                 // numDevT
    vars_cpp << "Thread_" << threadAddr << "_DeviceTypes,";           // devTs
    vars_cpp << numberOfDevices <<  ",";                              // numDevI
    vars_cpp << "Thread_" << threadAddr << "_Devices,";               // devIs
    vars_cpp << ((devT->par->pPropsD)?"&GraphProperties,":"PNULL,");  // props

    /* Set the size of the RTSBuffer: depends on the mode of operation. In non-
     * buffering mode, this will relate to the number of output pins hosted by
     * the Softswitch. In Buffering mode, this is set to the maximum size (or
     * the size specified at the command line/in the config).
     */
    uint32_t buffCount = 0;     // RTSBuff size.
    if(builderGraphI->bufferingSoftswitch)
    {
        /* Use the value in the graph instance directly. */
        buffCount = builderGraphI->rtsBuffSizeMax;
    }
    else
    {
       /* Work out the required size for rtsBuffSize: The size of the RTS buffer
        * is dependant on the number of connected output pins hosted on the 
        * Softswitch.
        * The size is set to 1 + <number of connected pins> + <number of 
        * devices> (if supervisor pin connected), as long as this is less than
        * builderGraphI->rtsBuffSizeMax, so that each connected pin can have a 
        * pending send.
        *
        * The additional slot ensures that the crude, simple wrapping mechanism 
        * for the circular buffer does not set rtsEnd to the same as rtsStart
        * when adding to the buffer. If this occurs, softswitch_IsRTSReady will
        * always return false (as it simply checks that rtsStart != rtsEnd) and
        * the softswitch will not send no further application-generated packets
        * (as softswitch_onRTS will only alter rtsEnd if it adds an entry to the
        * buffer, which it wont do as all pins will already be marked as send
        * pending).
        *
        * If the buffer size is constrained, a warning is generated - if this 
        * occurs frequently, more graceful handling of rtsBuf overflowing may be 
        * required.
        */
        buffCount = 1;     // Intentionally 1 to cope with wrapping.

        if(devT->pPinTSO)
        {   // There is a supervisor output, increase output count by dev count.
            buffCount += numberOfDevices;
        }

        if (outTypCnt) // If we have output pins
        {              // Iterate through devices counting connected output pins
            WALKLIST(DevI_t*, placer->threadToDevices.at(thread), dev)
            {
                /* The below relies on the Pmap in the device instance to find 
                 * output pins with reference to the Pin's PinT_t. 
                 * By design, this map is cleared to save memory. This behaviour
                 * has been changed to facilitate the below. An alternative
                 * would be to maintain a vector of pointers to PinI_ts that is
                 * retained after Pmap is cleared HOWEVER, this requires some
                 * thought to ensure that we can discriminate output pins as a
                 * naive implementation ends up allocating buffer space for
                 * input pins as well as output pins.
                 */
                WALKVECTOR(PinT_t*, devT->PinTO_v, pin)
                {
                    // Check that we have a connection
                    std::map<std::string,PinI_t *>::iterator pinSrch;
                    pinSrch = (*dev)->Pmap.find((*pin)->Name());
                    if(pinSrch != (*dev)->Pmap.end())
                    {
                        if(pinSrch->second->Key_v.size())
                        {
                            buffCount++;

                            // If 0, we have overflowed & there is no point cont
                            if (buffCount==0) break;
                        }
                    }
                }
                if (buffCount==0)
                {
                    buffCount = builderGraphI->rtsBuffSizeMax;
                    break;
                }
            }
        }

        if (buffCount > builderGraphI->rtsBuffSizeMax)
        { // If we have too many pins for one buffer entry per pin, set to max 
          // & warn. This may need a check adding to the Softswitch to stop
          // buffer overflow.
            fprintf(fd,"\nRTS Buffer for thread %u truncated from %u to %lu\n",
            threadAddr, buffCount, builderGraphI->rtsBuffSizeMax);
            
            rtsOF = true;
            
            buffCount = builderGraphI->rtsBuffSizeMax;
        }
        else if (buffCount < MIN_RTSBUFFSIZE)
        {
            fprintf(fd,"\nRTS Buffer for thread %u expanded from %u to %u\n",
            threadAddr, buffCount, MIN_RTSBUFFSIZE);
            buffCount = MIN_RTSBUFFSIZE;
        }
    }


    vars_cpp << buffCount << ",";                                     // rtsBuffSize

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
    
    if(rtsOF)
    {
        vars_cpp << "#warning RTS Buffer for Thread " << threadAddr;
        vars_cpp << " truncated to " << builderGraphI->rtsBuffSizeMax << "\n";
    }
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
    GraphT_t* graphT = devT->par;
    
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
    
    if(devT->pOnDeId) vars_cpp <<"&devtyp_"<<devT->Name()<<"_OnIdle_handler,";  // OnIdle_Handler
    else vars_cpp << "0,";
    
    if(devT->pOnHWId) vars_cpp <<"&devtyp_"<<devT->Name()<<"_OnHWIdle_handler,";// OnHWIdle_Handler
    else vars_cpp << "0,";
    
    vars_cpp << "&devtyp_" << devT->Name() << "_OnImpl_handler,";               // OnImpl_Handler
    vars_cpp << "&devtyp_" << devT->Name() << "_OnCtl_handler,";                // OnCtl_Handler

    if(devT->pPropsD)
    {
        vars_cpp << "sizeof(" << graphT->Name() << "_";
        vars_cpp << devT->Name() << "_properties_t),";           // sz_props
    }
    else vars_cpp << "0,";
    if(devT->pStateD)
    {
        vars_cpp << "sizeof(" << graphT->Name() << "_";
        vars_cpp << devT->Name() << "_state_t),";           // sz_state
    }
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
    
    GraphT_t* graphT = devT->par;    // grab a local copy of the graphtype

    vars_h << "//------------------------------ Pin Type Tables ";
    vars_h << "-------------------------------\n";

    if (inTypCnt)
    {
        // Add declaration for the input pins array to relevant vars header
        vars_h << "extern in_pintyp_t Thread_" << threadAddr;
        vars_h << "_DevTyp_0_InputPins[" << inTypCnt << "];\n";
        
        // Build the dev type input pin name string
        std::string dTypInPin = graphT->Name();
        dTypInPin += std::string("_") + devT->Name() + std::string("_");

        initialiser.str("");    // Clear the initialiser

        initialiser << "{";
        WALKVECTOR(PinT_t*, devT->PinTI_v, ipin) // Build pin initialiser
        {
            initialiser << "{";
            initialiser << "&" << "devtyp_" << devT->Name() << "_InPin_";
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
                initialiser << (*ipin)->Name() << "_properties_t),";
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
    std::string devTName = pinI->pT->par->Name();           // grab device name
    std::string graphTName = pinI->pT->par->par->Name();    // grab Graph name
    
    // Format "{graphTypeId}_{deviceTypeId}_{pinName}_properties_t".  
    vars_h << "extern " << graphTName << "_" << devTName << "_";
    vars_h << pinI->pT->Name() << "_properties_t ";
    vars_h << thrDevName << "_Pin_" << pinI->pT->Name();
    vars_h << "_InEdgeProps[" << edgeCount << "];\n\n";
}


/******************************************************************************
 * Add the edge instances states array declaration to vars h
 *****************************************************************************/
void Composer::writePinStateDecl(PinI_t* pinI, std::string& thrDevName,
                                    size_t edgeCount, std::ofstream& vars_h)
{
    std::string devTName = pinI->pT->par->Name();           // grab device name
    std::string graphTName = pinI->pT->par->par->Name();    // grab Graph name
    
    // Format "{graphTypeId}_{deviceTypeId}_{pinName}_state_t".  
    vars_h << "extern " << graphTName << "_" << devTName << "_";
    vars_h << pinI->pT->Name() << "_state_t ";
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
    FILE * fd = graphI->par->par->fd;              // Detail output file
    
    GraphT_t* graphT = graphI->pT;

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
        
        
        // Retrieve the device index from the Supervisor map
        devISuperIdxMap_t::iterator devISrch;
        devISrch = builderGraphI->devISuperIdxMap.find(devI);
        if (devISrch == builderGraphI->devISuperIdxMap.end())
        {   // Something has gone wrong that we are going to ignore.
            fprintf(fd,"**WARNING** Device not found in Supervisor idx map\n");
            devII << "0,";
        }
        else
        {
            devII << devISrch->second << ",";
        }


        // Find all of the arcs that involve this device.
        std::vector<unsigned> arcKeysIn;
        std::vector<unsigned> arcKeysOut;
        graphI->G.FindArcs(devI->Key, arcKeysIn, arcKeysOut);

        if (inTypCnt)
        {   // Input pin array Def/Init and Declaration

            writeDevIInputPinDefs(graphI, devT, threadAddr, thrDevName,
                                    arcKeysIn, vars_h, vars_cpp);
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
        {   // Output pin array Def/Init and declaration

            writeDevIOutputPinDefs(builderGraphI, devI, threadAddr, thrDevName,
                                arcKeysOut, vars_h, vars_cpp);

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

            // Default is {}
            if(devI->pPropsI)
            {
                devPIStrs[devIdx] = devI->pPropsI->C_src() + ",";
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
            devII << devIdx << "],";

            if(devI->pStateI)
            {
                devSIStrs[devIdx] = devI->pStateI->C_src()+",";
            }
        }
        else
        {
            devII << "PNULL,";
        }
        
        // Initialise requestIdle
        devII << "false},";

        // populate the DevIIStrs with the initialiser
        devIIStrs[devIdx] = devII.str();
    }


    // Process the individual initialisers into a coherent string

    // Start the devInst_t initialiser
    devII.str("");
    devII << "devInst_t Thread_" << threadAddr << "_Devices[";
    devII << numberOfDevices << "] = {";


    // Start the {graphTypeId}_{deviceTypeId}_properties_t initialiser
    devPI.str("");
    devPI << graphT->Name() << "_" << devT->Name() << "_properties_t Thread_";
    devPI << threadAddr << "_DeviceProperties[" << numberOfDevices << "] = {";

    // Start the {graphTypeId}_{deviceTypeId}_state_t initialiser
    devSI.str("");
    devSI << graphT->Name() << "_" << devT->Name() << "_state_t Thread_";
    devSI << threadAddr << "_DeviceState[" << numberOfDevices << "] = {";


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

    // Finish off the {graphTypeId}_{deviceTypeId}_properties_t initialiser
    devPI.seekp(-1, ios_base::cur);   // Remove the stray ,
    devPI << "};\n\n";

    // Finish off the {graphTypeId}_{deviceTypeId}_state_t initialiser
    devSI.seekp(-1, ios_base::cur);   // Remove the stray ,
    devSI << "};\n\n";



    // Write the initialisers to the vars.cpp
    vars_cpp << devII.rdbuf();

    if(devT->pPropsD)
    {
        vars_cpp << devPI.rdbuf();

        // Make sure the properties decl is there.
        vars_h << "extern " << graphT->Name() << "_" << devT->Name();
        vars_h << "_properties_t Thread_" << threadAddr ;
        vars_h << "_DeviceProperties[" << numberOfDevices << "];\n";
    }

    if(devT->pStateD)
    {
        vars_cpp << devSI.rdbuf();

        // Make sure the state decl is there.
        vars_h << "extern " << graphT->Name() << "_" << devT->Name();
        vars_h << "_state_t Thread_" << threadAddr ;
        vars_h << "_DeviceState[" << numberOfDevices << "];\n\n";
    }

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
        PinI_t* oPin = PNULL;
        PinI_t* iPin = PNULL;
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
                    inEdgePropsIStrs[edgeI->Idx] = edgeI->pPropsI->C_src() + ",";
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
                    inEdgeStatesIStrs[edgeI->Idx] = edgeI->pStateI->C_src() + ",";
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

        std::string devTName = pinI->pT->par->Name();        // device name
        std::string graphTName = pinI->pT->par->par->Name(); // Graph name
    
        // Start {graphTypeId}_{deviceTypeId}_{pinName}_state_t initialiser".  
        inEdgeStatesI.str("");
        inEdgeStatesI << graphTName << "_" << devTName << "_";
        inEdgeStatesI << pinI->pT->Name() << "_state_t ";
        inEdgeStatesI << thrDevName << "_Pin_" << pinI->pT->Name();
        inEdgeStatesI << "_InEdgeStates[" << edgeCount << "] = {";

        // Start {graphTypeId}_{deviceTypeId}_{pinName}_properties_t initialiser
        inEdgePropsI.str("");
        inEdgePropsI << graphTName << "_" << devTName << "_";
        inEdgePropsI << pinI->pT->Name() << "_properties_t ";
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
    FILE * fd = graphI->par->par->fd;              // Detail output file
    
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
        PinI_t* oPin = PNULL;
        PinI_t* iPin = PNULL;
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
        outEdgeTI << P_DEST_BROADCAST << ",";

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
            fprintf(fd,"**WARNING** Device not found in Supervisor idx map");
            fprintf(fd," when writing implicit Send Pin initialiser.\n");
            outEdgeTI << "0";
        }
        else
        {
            outEdgeTI << devISrch->second;
        }

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
            DevI_t* inDevI;

            unsigned inPinKey, outPinKey;
            PinI_t* inPinI = PNULL;
            PinI_t* outPinI = PNULL;

            P_thread* farThread;

            // Get the Edge instance
            edgeI = *(graphI->G.FindArc((*arcKey)));

            // Get the DevI_t* for the far end
            graphI->G.FindNodes((*arcKey), outNodeKey, inNodeKey);
            inDevI = *(graphI->G.FindNode(inNodeKey));

            // Get the pinI_t of both ends
            graphI->G.FindArcPins((*arcKey), outPinKey, outPinI, inPinKey, inPinI);

            // Get the Thread of the far end
            // This is probably slow, may be faster to have a field in DevI_t?
            std::map<DevI_t*, P_thread*>::iterator threadSrch;
            threadSrch = placer->deviceToThread.find(inDevI);

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
            SoftwareAddress swAddr = inDevI->addr;

            //TODO: set_task, etc. etc.

            // Get the pin and edge indicies for the other side.
            uint32_t pinAddr = 0;

            // Edge Index
            pinAddr |= ((edgeI->Idx << P_HD_DESTEDGEINDEX_SHIFT)
                        & P_HD_DESTEDGEINDEX_MASK);

            // Pin Index
            if(inPinI != PNULL)
            {   // Pointless sanity check?
                pinAddr |= (((inPinI->pT->Idx) << P_HD_TGTPIN_SHIFT)
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
