#include "AddressBook_Task.hpp"

namespace AddressBook
{

/*==============================================================================
 * TaskData - used to communicate task data without maps and pointers.
 *============================================================================*/
TaskData_t::TaskData_t()
{
    Name = "";
    Path = "";
    XML = "";
    ExecutablePath = "";
    State = Loaded;
    DeviceCount = 0;
    DeviceCountLd = 0;
    ExternalCount = 0;
    ExternalCountLd = 0;
    SupervisorCount = 0;

    State = Loaded;
}

/*==============================================================================
 * TaskData_t::TaskData_t(): Full initialiser for a TaskData_t
 *============================================================================*/
TaskData_t::TaskData_t(std::string &N, std::string &P, std::string &X,
                       std::string &E, TaskState_t S, unsigned long DC,
                       unsigned long EC)
{
    Name = N;
    Path = P;
    XML = X;
    ExecutablePath = E;
    State = Loaded;
    DeviceCount = DC;
    DeviceCountLd = 0;
    ExternalCount = EC;
    ExternalCountLd = 0;
    SupervisorCount = 0;

    State = S;
}

/*==============================================================================
 * TaskData_t::size(): Return the size (in bytes) of the address record.
 *============================================================================*/
int TaskData_t::size() const
{
    int size = 0;
    
    size += sizeof(unsigned long)*5;      // Size of the 5 longs
    size += sizeof(char) * Name.size();   // Size of the name
    size += sizeof(char) * Path.size();   // Size of the path
    size += sizeof(char) * XML.size();    // Size of the XML Filename
    size += sizeof(char) * ExecutablePath.size();   // Size of the exec path
    size += sizeof(TaskState_t);
    
    for(std::vector<DevTypeRecord_t>::const_iterator DT = DeviceTypes.begin();
        DT != DeviceTypes.end(); DT++)    // Size of the device types
    {
        size += DT->size();
    }
    
    for(std::vector<std::string>::const_iterator MT = MessageTypes.begin();
        MT != MessageTypes.end(); MT++)   // Size of the Message Types
    {
        size += MT->size();
    }
    
    for(std::vector<std::string>::const_iterator AT = AttributeTypes.begin();
        AT != AttributeTypes.end(); AT++) // Size of the Attribute Types
    {
        size += AT->size();
    }
    
    return size;
}

/*==============================================================================
 * TaskRecord - stores data about the task in an instance of AddressBook
 *============================================================================*/
TaskRecord_t::TaskRecord_t()
{
    Name = "";
    Path = "";
    XML = "";
    ExecPath = "";
    DevCntMax = 0;
    ExtCntMax = 0;
    DevCnt = 0;
    SupCnt = 0;
    ExtCnt = 0;

    State = Loaded;

    TaskValid = true;
    MapValid = true;
    LinkValid = true;
}

/*==============================================================================
 * TaskRecord_t::TaskRecord_t(): Full initialiser for a TaskRecord_t
 *============================================================================*/
TaskRecord_t::TaskRecord_t(std::string &N, std::string &P, std::string &X,
                           std::string &E, TaskState_t S, unsigned long DC, 
                           unsigned long EC)
{
    Name = N;
    Path = P;
    XML = X;
    ExecPath = E;
    DevCntMax = DC;
    ExtCntMax = EC;
    DevCnt = 0;
    SupCnt = 0;
    ExtCnt = 0;

    State = S;

    TaskValid = true;
    MapValid = true;
    LinkValid = true;
}


/*==============================================================================
 * TaskRecord_t::Integrity(): Run an integrity check on the entire task.
 *
 *      Verbose:  Print lots of information to *fp (stdout by default).
 *
 *      Scours the data structure checking that all of the maps and vectors all
 *      point to valid data and that there are no invalid indicies or pointers.
 *
 *      This will mark the underlying data structure as dirty if it finds any
 *      inconsistencies. If RECOVERABLEINTEGRITY is defined, a successful
 *      integrity check marks the data structure as clean.
 *
 *      When AB_THREADING is defined (by default this is when built with C++11 
 *      (or newer) AND (C11 (or newer) OR POSIX)), the integrity checks are
 *      parallelised to reduce the potentially significant runtime of large
 *      applications. The number of threads is set based on the available
 *      concurrency, with at least one thread used for each check type (minimum
 *      4 threads). Where concurrency is high, 2*concurrency threads are used in
 *      the following way:
 *          Devices vector              1*concurrency threads
 *          DeviceType vectors, etc.    0.5*concurrency threads
 *          Supervisors vector          0.25*concurrency threads
 *          Externals vector            0.25*concurrency threads
 *       
 *============================================================================*/
unsigned TaskRecord_t::Integrity(bool Verbose, FILE * fp)
{   
    AB_ULONG ExtConCnt(0);
    AB_UNSIGNED MappedSupervisors(0);

    IntegVals_t retVal;
    retVal.ret = 0;
    retVal.retT = 0;
    retVal.retL = 0;
    retVal.retM = 0;


    if (Verbose) fprintf(fp, "Integrity check of %s:\n", Name.c_str());

    //==========================================================================
    // Task checks

    // Device Counts - failing these means the task is broken?
    if(Devices.size() != DevCnt)
    {
        // Device Count mismatch.
        if (Verbose) fprintf(fp, "Device Count mismatch: %lu!=%lu\n",
                        static_cast<unsigned long>(Devices.size()), DevCnt);
        retVal.ret++;
    }

    if(Externals.size() != ExtCnt)
    {
        // External Count mismatch.
        if (Verbose) fprintf(fp, "External Count mismatch: %lu!=%lu\n",
                        static_cast<unsigned long>(Externals.size()), ExtCnt);
        retVal.ret++;
    }

    if(Supervisors.size() != SupCnt)
    {
        // Supervisor Count mismatch.
        if (Verbose) fprintf(fp, "Supervisor Count mismatch: %lu!=%lu\n",
                        static_cast<unsigned long>(Supervisors.size()), SupCnt);
        retVal.ret++;
    }

    if(SupMap.size() != SupCnt)
    {
        // Supervisor Count-SupMap size mismatch.
        if (Verbose) fprintf(fp, "Supervisor Count<>SupMap size mismatch: %lu!=%lu\n",
                        static_cast<unsigned long>(SupMap.size()), SupCnt);
        retVal.ret++;
    }


#ifdef AB_THREADING
    //==========================================================================
    // Multi-threaded checks of large structures. Number of threads used scales 
    // with the number of CPU cores. The checks are read-only so no locks for
    // data structure access. Atomic types are used for return values.
    unsigned threadCnt = 0, i = 0;

    // Base the scaling off of the available concurrency, with a minimum of 1
    // thread for each check, for a minimum of 4 total threads. Otherwise,
    // maximum concurrency is 2* available concurrency.
    unsigned int concurrency = std::thread::hardware_concurrency();
    unsigned int halfConcurrency = (concurrency < 2) ? 1 : concurrency / 2;
    unsigned int qrtrConcurrency = (concurrency < 4) ? 1 : concurrency / 4;

    // Create a vector of threads of the correct size.
    std::vector<std::thread> threads(concurrency + halfConcurrency
                                    + qrtrConcurrency + qrtrConcurrency);


    //==========================================================================
    // Thread(s) for checking DeviceType vectors, etc.
    unsigned long dtConcurrency = halfConcurrency;
    if (DevTypes.size() < halfConcurrency)      // Limit number of threads if we 
    {                                           // have fewer DeviceTypes than  
        dtConcurrency = DevTypes.size();        // possible threads.
    }
    
    if (dtConcurrency > 0)
    {
        unsigned long devTypeSplit = DevTypes.size() / dtConcurrency;
        for(i = 0; i < dtConcurrency; i++)
        {
            // Handle any remainder in the last thread - integ method checks bounds.
            unsigned endMul = (i==(dtConcurrency-1)) ? 2 : 1;   
            threads[threadCnt+i] = std::thread(&TaskRecord_t::IntegDevTypeMap, this, 
                                                Verbose, fp, (devTypeSplit * i), 
                                                (devTypeSplit * (i+endMul)), 
                                                std::ref(retVal));
        }
        threadCnt += i;
    }

    //==========================================================================
    // Thread(s) for checking Devices vector
    unsigned long dConcurrency = concurrency;
    if (Devices.size() < concurrency)           // Limit number of threads if we 
    {                                           // have fewer Devices than
        dConcurrency = Devices.size();          // possible threads.
    }
    
    if (dConcurrency > 0)   // account for 0 devices, incase we are called before devices loaded
    {
        unsigned long long devSplit = Devices.size() / dConcurrency;
        for(i = 0; i < dConcurrency; i++)
        {
            // Handle any remainder in the last thread - integ method checks bounds.
            unsigned endMul = (i==(dConcurrency-1)) ? 2 : 1;   
            threads[threadCnt+i] = std::thread(&TaskRecord_t::IntegDevices, this, 
                                                Verbose, fp, (devSplit * i), 
                                                (devSplit * (i+endMul)), 
                                                std::ref(retVal), 
                                                std::ref(ExtConCnt));
        }
        threadCnt += i;
    }

    //==========================================================================
    // Thread(s) for checking Externals vector
    unsigned long eConcurrency = qrtrConcurrency;
    if (Externals.size() < qrtrConcurrency)     // Limit number of threads if we 
    {                                           // have fewer Externals than
        eConcurrency = Externals.size();        // possible threads.
    }
    
    if (eConcurrency > 0)   // Account for having no externals
    {
        unsigned long extSplit = Externals.size() / eConcurrency;
        for(i = 0; i < eConcurrency; i++)
        {
            // Handle any remainder in the last thread - integ method checks bounds.
            unsigned endMul = (i==(eConcurrency-1)) ? 2 : 1;
            threads[threadCnt+i] = std::thread(&TaskRecord_t::IntegExternals, this, 
                                                Verbose, fp, (extSplit * i), 
                                                (extSplit * (i+endMul)),
                                                std::ref(retVal));
        }
        threadCnt += i;
    }

    //==========================================================================
    // Thread(s) for checking Supervisors vector. 
    unsigned long sConcurrency = qrtrConcurrency;
    if (Supervisors.size() < qrtrConcurrency)   // Limit number of threads if we
    {                                           // have fewer Supervisors than
        sConcurrency = Supervisors.size();      // possible threads.
    }
    
    if (sConcurrency > 0)   // Account for having no supervisors
    {
        unsigned long superSplit = Supervisors.size() / sConcurrency;
        for(i = 0; 
            i < sConcurrency;
            i++)
        {
            // Handle any remainder in the last thread - integ method checks bounds.
            unsigned endMul = (i==(sConcurrency-1)) ? 2 : 1;
            threads[threadCnt+i] = std::thread(&TaskRecord_t::IntegSupervisors, this, 
                                                Verbose, fp, (superSplit * i), 
                                                (superSplit * (i+endMul)),
                                                std::ref(retVal), 
                                                std::ref(MappedSupervisors));
        }
        threadCnt += i;
    }

#endif

    // Check integrity of MsgType map
    for(std::vector<MsgTypeRecord_t>::iterator M
        =MsgTypes.begin();M!=MsgTypes.end();M++)    // Iterate through message type vector
    {
        unsigned Idx =  static_cast<unsigned>(std::distance(MsgTypes.begin(), M));    // Current Index

        IdxMap_t::const_iterator MSearch = MsgTypeMap.find(M->Name);         // Find the Msg Name in the map
        if (MSearch == MsgTypeMap.end()) {
            // Message Type not mapped
            if (Verbose) fprintf(fp, "T: Message Type not mapped: %s\n",
                                        M->Name.c_str());
            retVal.retT++;
        } else {
            if (MsgTypes[MSearch->second].Name != M->Name){
                // Message Type > Index map is broken.
                if (Verbose) fprintf(fp, "T: Message Type > Index map is broken: %s\n",
                                            M->Name.c_str());
                retVal.retT++;
            }

            if (Idx != MSearch->second) {
                // Index mismatch
                if (Verbose) fprintf(fp, "T: Index mismatch: %s, %u!=%u\n",
                                        M->Name.c_str(), Idx, MSearch->second);
                retVal.retT++;
            }
        }
    } 

    // Check integrity of AttributeType map
    for(std::vector<AttrTypePair>::iterator A
        =AttrTypes.begin();A!=AttrTypes.end();A++)    // Iterate through message type vector
    {
        unsigned Idx =  static_cast<unsigned>(std::distance(AttrTypes.begin(), A));    // Current Index

        IdxMap_t::const_iterator ASearch = AttrTypeMap.find(A->first);         // Find the Msg Name in the map
        if (ASearch == AttrTypeMap.end()) {
            // Attribute Type not mapped
            if (Verbose) fprintf(fp, "T: Attribute Type not mapped: %s\n",
                                    A->first.c_str());
            retVal.retT++;
        } else {
            if (AttrTypes[ASearch->second].first != A->first){
                // Attribute Type > Index map is broken.
                if (Verbose) fprintf(fp, "T: Attribute > Index map broken: %s!=%s\n",
                            A->first.c_str(), AttrTypes[ASearch->second].first.c_str());
                retVal.retT++;
            }

            if (Idx != ASearch->second) {
                // Index mismatch
                if (Verbose) fprintf(fp, "T: Attribute Index mismatch: %u!m%u\n",
                            Idx, ASearch->second);
                retVal.retT++;
            }
        }
    }


    //==========================================================================
    // Map checks

    // Map size integrity checks - Re-mapping should fix.
    unsigned long totalDevCnt = DevCnt + ExtCnt;
    if(NameMap.size() != totalDevCnt)
    {
        // NameMap Size mismatch - Map broken
        if (Verbose) fprintf(fp, "M: NameMap size mismatch: %lu!=%lu\n",
                                static_cast<unsigned long>(NameMap.size()),
                                totalDevCnt);
        retVal.retM++;
    }


    //==========================================================================
    // Link checks

    // Link integrity checks - Re-linking should fix.
    if(AddrMap.size() != (totalDevCnt + SupCnt))
    {
        // AddressMap Size mismatch - Link broken
        if (Verbose) fprintf(fp, "L: AddrMap Size mismatch: %lu!=%lu\n",
                                static_cast<unsigned long>(AddrMap.size()),
                                (DevCnt + SupCnt));
        retVal.retL++;
    }


#ifdef AB_THREADING
    // Wait for all of the threads to finish
    for(i = 0; i < threadCnt; i++)
    {
        threads[i].join();
    }
#else
    // If we are not using threading, run all of the (long) integrity checks single-threaded.

    //==========================================================================
    // Check integrity of DeviceType map
    IntegDevTypeMap(Verbose, fp, 0, DevTypes.size(), retVal);

    //==========================================================================
    // Device checks
    IntegDevices(Verbose, fp, 0, Devices.size(), retVal, ExtConCnt); // Check ALL devices

    //==========================================================================
    // External checks
    IntegExternals(Verbose, fp, 0, Externals.size(), retVal); // Check ALL externals

    //==========================================================================
    // Supervisor checks
    IntegSupervisors(Verbose, fp, 0, Supervisors.size(), retVal, MappedSupervisors); // Check ALL supervisors
#endif


    //==========================================================================
    // Check ExtCon size.
    if(ExtCon.size() != ExtConCnt)
    {
        // External Connection vector size mismatch - Map broken
        if (Verbose) fprintf(fp, "M: External Connection Vector size mismatch: %lu!=%lu\n",
                            static_cast<unsigned long>(ExtCon.size()), 
                            static_cast<unsigned long>(ExtConCnt));
        retVal.retM++;
    }

    //==========================================================================
    // Check Supervisor map size
    if (MappedSupervisors != SupMap.size()) {
        // Supervisor Map size missmatch
        if (Verbose) fprintf(fp, "L: Supervisor Size mismatch: %u!=%lu\n",
                            static_cast<unsigned>(MappedSupervisors), 
                            static_cast<unsigned long>(SupMap.size()));
        retVal.retL++;
    }

    //==========================================================================
    // Set dirty flags if appropriate
    if (Verbose) fprintf(fp, "\nResults:\n");

    if (retVal.ret > 0)
    {
        if (Verbose) fprintf(fp, "\tGeneral Breakage: %d\n", static_cast<int>(retVal.ret));
    }


    if (retVal.retT > 0)   // Task integrity compromised - need to rebuild the task
    {
        TaskValid = false;
        if (Verbose) fprintf(fp, "\tDirty Task: %d\n", static_cast<int>(retVal.retT));
        retVal.ret += retVal.retT;
    }
#ifdef RECOVERABLEINTEGRITY
    else
    {
        TaskValid = true;
    }
#endif

    if (retVal.retM > 0)   // Map integrity compromised - need to rebuild the map
    {
        MapValid = false;
        if (Verbose) fprintf(fp, "\tDirty Map: %d\n", static_cast<int>(retVal.retM));
        retVal.ret += retVal.retM;
    }
#ifdef RECOVERABLEINTEGRITY
    else
    {
        MapValid = true;
    }
#endif

    if (retVal.retL > 0)   // Link integrity compromised - need to rebuild the link
    {
        LinkValid = false;
        if (Verbose) fprintf(fp, "\tDirty Link: %d\n", static_cast<int>(retVal.retL));
        retVal.ret += retVal.retL;
    }
#ifdef RECOVERABLEINTEGRITY
    else
    {
        LinkValid = true;
    }
#endif

    if (Verbose && (retVal.ret == 0)) fprintf(fp, "\tTask Clean\n");

    return retVal.ret;
}


/*==============================================================================
 * TaskRecord_t::IntegDevTypeMap(): Run an integrity check on the device type
 * map in the range of DStart-DEnd. 
 *============================================================================*/
void TaskRecord_t::IntegDevTypeMap(bool Verbose, FILE * fp, unsigned long DTStart, 
                                   unsigned long DTEnd, IntegVals_t &retVal)
{
    for(std::vector<DevTypePair>::iterator D = DevTypes.begin() + DTStart;
        (D != DevTypes.end()) && (D < (DevTypes.begin() + DTEnd));
        D++)    // Iterate through device type vector
    {
        unsigned Idx =  static_cast<unsigned>(std::distance(DevTypes.begin(), D));    // Current Index

        IdxMap_t::const_iterator DTSearch = DevTypeMap.find(D->first.Name);    // Find the DevType name in the map
        if (DTSearch == DevTypeMap.end()) {
            // DevType not mapped
            if (Verbose) fprintf(fp, "T: DevType not mapped: %s\n",
                                    D->first.Name.c_str());
            retVal.retT++;

        } else {
            if (DevTypes[DTSearch->second].first.Name != D->first.Name){
                // DevType > Index map is broken.
                if (Verbose) fprintf(fp, "T: DevTypeMap Broken: %s!=%s\n",
                            D->first.Name.c_str(),
                            DevTypes[DTSearch->second].first.Name.c_str());
                retVal.retT++;
            }

            if (Idx != DTSearch->second) {
                // Index mismatch
                if (Verbose) fprintf(fp, "T: DevTypeMap Index mismatch: %s, %u!=%u\n",
                            D->first.Name.c_str(), Idx, DTSearch->second);
                retVal.retT++;
            }
        }

        //Check the Input Msg Indexes
        for(std::vector<MsgIdx>::const_iterator I
            =D->first.InMsgs.begin();I!=D->first.InMsgs.end();I++)
        {
            const std::vector<MsgIdx> *InMsgs = &MsgTypes[*I].Inputs;
            if (std::find(InMsgs->begin(), InMsgs->end(), Idx) == InMsgs->end())
            {
                // MsgType->DevType (Input Message) Vector broken.
                if (Verbose) fprintf(fp, "T: MsgType->DevType(InMsg) Vector broken: \
                            DevType %u (%s) not found in MsgType InMsg Vector %u\n",
                            Idx, D->first.Name.c_str(), *I);                   //DevTypes[Idx].first.Name
                retVal.retT++;
            }
        }

        //Check the Output Msg Indexes
        for(std::vector<MsgIdx>::const_iterator O
            =D->first.OuMsgs.begin();O!=D->first.OuMsgs.end();O++)
        {
            const std::vector<MsgIdx> *OuMsgs = &MsgTypes[*O].Outputs;
            if (std::find(OuMsgs->begin(), OuMsgs->end(), Idx) == OuMsgs->end())
            {
                // MsgType->DevType (Output Message) Vector broken.
                if (Verbose) fprintf(fp, "T: MsgType->DevType(OuMsg) Vector broken: \
                            DevType %u (%s) not found in MsgType OuMsg Vector %u\n",
                            Idx, D->first.Name.c_str(), *O);
                retVal.retT++;
            }
        }
    }
}

/*==============================================================================
 * TaskRecord_t::IntegDevices(): Run an integrity check on the devices in the
 * range of DStart-DEnd. 
 *============================================================================*/
void TaskRecord_t::IntegDevices(bool Verbose, FILE * fp, unsigned long DStart, 
                                unsigned long DEnd, IntegVals_t &retVal, 
                                AB_ULONG &ExtConCnt)
{
    for(std::vector<Record_t>::const_iterator D = Devices.begin() + DStart;
        (D != Devices.end()) && (D < (Devices.begin() + DEnd));
        D++)    // Iterate through section of Device vector
    {
        // Check record type
        if((D->RecordType != Device) && (D->RecordType != DeviceExt))
        {
            // Non-Device in the Device vector. Something, somewhere has gone very VERY wrong
            if (Verbose) fprintf(fp, "Non-Device in Device Vector: %s (%" SYMA_FMT "), %u\n",
                        D->Name.c_str(), D->Address, D->RecordType);
            retVal.ret++;
        } else {
            // Search Addr Map
            AddrMap_t::const_iterator DASearch = AddrMap.find(D->Address);    // Find the dev Addr in the map
            if (DASearch == AddrMap.end()) {
                // DevAddr not mapped - Link broken
                if (Verbose) fprintf(fp, "L: DevAddr not mapped: %s, %" SYMA_FMT "\n",
                            D->Name.c_str(), D->Address);
                retVal.retL++;
            } else if (DASearch->second != &(*D)) {
                // Address does not map to THIS device - Link broken
                if (Verbose) fprintf(fp, "L: AddrMap to wrong device: %s, %" PTR_FMT "!=%" PTR_FMT "\n",
                            D->Name.c_str(), reinterpret_cast<uint64_t>(DASearch->second),
                            reinterpret_cast<uint64_t>(&(*D)));
                retVal.retL++;
            }

            // Search Name Map
            NameMap_t::const_iterator DNSearch = NameMap.find(D->Name);    // Find the dev Name in the map
            if (DNSearch == NameMap.end()) {
                // DevName not mapped
                if (Verbose) fprintf(fp, "M: Device Name not mapped: %s\n",
                            D->Name.c_str());
                retVal.retM++;
            } else if (DNSearch->second != &(*D)) {
                // Name does not map to THIS device  - Map Broken
                if (Verbose) fprintf(fp, "M: Namemap to wrong device: %s, %" PTR_FMT "!=%" PTR_FMT "\n",
                            D->Name.c_str(), reinterpret_cast<uint64_t>(DNSearch->second),
                            reinterpret_cast<uint64_t>(&(*D)));
                retVal.retM++;
            }

            // Check Device type Index
            if (D->DeviceType >= DevTypes.size()) {
                // DeviceType Index out of range
                if (Verbose) fprintf(fp, "DeviceType Index out of range: %s (%" SYMA_FMT "), DType: %d>%d\n",
                            D->Name.c_str(), D->Address, D->DeviceType,
                            static_cast<int>(DevTypes.size()));
                retVal.ret++;
            } else {
                // Find the pointer to the device record in the Device Type vector
                const RecordVect_t *DTypeV = &DevTypes[D->DeviceType].second;
                if (std::find(DTypeV->begin(), DTypeV->end(), &(*D)) == DTypeV->end())
                {
                    // Device not in Device Type vector - Map Broken
                    if (Verbose) fprintf(fp, "M: Device %s (%" SYMA_FMT ") not in DevicetypeVector %u (%s)\n",
                                D->Name.c_str(), D->Address, D->DeviceType,
                                DevTypes[D->DeviceType].first.Name.c_str());
                    retVal.retM++;
                }
            }

            // Search Supervisor
            SupMap_t::const_iterator DSSearch = SupMap.find(D->Supervisor);    // Find the dev Addr in the map
            if (DSSearch == SupMap.end()) {
                // Device's Supervisor not found - Link broken
                if (Verbose) fprintf(fp, "L: Dev's Supervisor not found in AddrMap: %s, %" SYMA_FMT "\n",
                            D->Name.c_str(), D->Supervisor);
                retVal.retL++;
            } else {
                // Search Supervsor Vector
                const RecordVect_t *SupV = &DSSearch->second;
                if (std::find(SupV->begin(), SupV->end(), &(*D)) == SupV->end())
                {
                    // Pointer to Device Record not found in Supervisor Vector.
                    if (Verbose) fprintf(fp, "L: Ptr to Device not in Supervisor \
                                vector: %s, Supervisor: %" SYMA_FMT "\n",
                                D->Name.c_str(), D->Supervisor);
                    retVal.retL++;
                }
            }

            // Check if the Attribute Index is out of range
            if(D->Attribute >= static_cast<int>(AttrTypes.size()))
            {
                // Attribute Index out of range.
                if (Verbose) fprintf(fp, "Attribute Index out of range: %s (%" SYMA_FMT "), Attr: %d>%d\n",
                            D->Name.c_str(), D->Address, D->Attribute,
                            static_cast<int>(AttrTypes.size()));
                retVal.ret++;
            } else if (D->Attribute > -1) {
                // Find the pointer to the device record in the Attribute Type vector
                const RecordVect_t *ATypeV = &AttrTypes[static_cast<unsigned>(D->Attribute)].second;
                if (std::find(ATypeV->begin(), ATypeV->end(), &(*D)) == ATypeV->end())
                {
                    // Device not in Attribute Type vector - Map Broken
                    if (Verbose) fprintf(fp, "L: Dev not in Attribute vector: %s\
                                , Attribute: %d (%s)\n", D->Name.c_str(), D->Attribute,
                                AttrTypes[static_cast<unsigned>(D->Attribute)].first.c_str());
                    retVal.retM++;
                }
            }

            // If ExtCon, search ExtCon Vector, ExtConCnt++
            if(D->RecordType == DeviceExt)
            {
                if (std::find(ExtCon.begin(), ExtCon.end(), &(*D)) == ExtCon.end())
                {
                    // Device with External Connection not found in ExtCon - Map Broken
                    if (Verbose) fprintf(fp, "M: Device with External connection not in\
                                ExtCon: %s (%" SYMA_FMT ")\n", D->Name.c_str(), D->Address);
                    retVal.retM++;
                }
                ExtConCnt++;
            }
        }
    }
}

/*==============================================================================
 * TaskRecord_t::IntegExternals(): Run an integrity check on the externals in 
 * the range of DStart-DEnd. 
 *============================================================================*/
void TaskRecord_t::IntegExternals(bool Verbose, FILE * fp, unsigned long EStart, 
                                  unsigned long EEnd, IntegVals_t &retVal)
{
    for(std::vector<Record_t>::const_iterator E = Externals.begin() + EStart;
        (E != Externals.end()) && (E < (Externals.begin() + EEnd));
        E++)    // Iterate through External vector
    {
        // Check record type
        if(E->RecordType != External)
        {
            // Non-External in the External vector. Something, somewhere has gone very VERY wrong
            if (Verbose) fprintf(fp, "Non-External in External Vector: %s, \
                        %" SYMA_FMT ", %d\n", E->Name.c_str(), E->Address, E->RecordType);
            retVal.ret++;
        } else {
            // Search Addr Map
            AddrMap_t::const_iterator EASearch = AddrMap.find(E->Address);    // Find the dev Addr in the map
            if (EASearch == AddrMap.end()) {
                // ExtAddr not mapped - Link broken
                if (Verbose) fprintf(fp, "L: ExtAddr not mapped: %s, %" SYMA_FMT "\n",
                            E->Name.c_str(), E->Address);
                retVal.retL++;
            } else if (EASearch->second != &(*E)) {
                // Address does not map to THIS external - Link broken
                if (Verbose) fprintf(fp, "L: AddrMap to wrong External: %s, %" PTR_FMT "!=%" PTR_FMT "\n",
                            E->Name.c_str(), reinterpret_cast<uint64_t>(EASearch->second),
                            reinterpret_cast<uint64_t>(&(*E)));
                retVal.retL++;
            }

            // Search Name Map
            NameMap_t::const_iterator ENSearch = NameMap.find(E->Name);    // Find the dev Name in the map
            if (ENSearch == NameMap.end()) {
                // ExtName not mapped
                if (Verbose) fprintf(fp, "M: External Name not mapped: %s (%" SYMA_FMT ")\n",
                            E->Name.c_str(), E->Address);
                retVal.retM++;
            } else if (ENSearch->second != &(*E)) {
                // Name does not map to THIS external - Map Broken
                if (Verbose) fprintf(fp, "M: Namemap to wrong external: %s, %" PTR_FMT "!=%" PTR_FMT "\n",
                            E->Name.c_str(), reinterpret_cast<uint64_t>(ENSearch->second),
                            reinterpret_cast<uint64_t>(&(*E)));
                retVal.retM++;
            }

            // Check Device type Index
            if (E->DeviceType >= DevTypes.size()) {
                // DeviceType Index out of range
                if (Verbose) fprintf(fp, "DeviceType Index out of range: %s (%" SYMA_FMT ")\
                            , DType: %u>%d\n", E->Name.c_str(), E->Address, E->DeviceType,
                            static_cast<int>(DevTypes.size()));
                retVal.ret++;
            } else {
                // Find the pointer to the device record in the Device Type vector
                const RecordVect_t *DTypeV = &DevTypes[E->DeviceType].second;
                if (std::find(DTypeV->begin(), DTypeV->end(), &(*E)) == DTypeV->end())
                {
                    // External not in Device Type vector - Map Broken
                    if (Verbose) fprintf(fp, "M: External %s (%" SYMA_FMT ") not in \
                                DeviceType Vector %u (%s)\n",
                                E->Name.c_str(), E->Address, E->DeviceType,
                                DevTypes[E->DeviceType].first.Name.c_str());
                    retVal.retM++;
                }
            }

            // Check if the Attribute Index is out of range
            if(E->Attribute >= static_cast<int>(AttrTypes.size()))
            {
                // Attribute Index out of range.
                if (Verbose) fprintf(fp, "Attribute Index out of range: %s (%" SYMA_FMT "), \
                            Attr: %d>%d\n", E->Name.c_str(), E->Address, E->Attribute,
                            static_cast<int>(AttrTypes.size()));
                retVal.ret++;
            } else if (E->Attribute > -1) {
                // Find the pointer to the external record in the Attribute Type vector
                const RecordVect_t *ATypeV = &AttrTypes[static_cast<unsigned>(E->Attribute)].second;
                if (std::find(ATypeV->begin(), ATypeV->end(), &(*E)) == ATypeV->end())
                {
                    // External not in Attribute Type vector - Map Broken
                    if (Verbose) fprintf(fp, "L: Ext not in Attribute vector: %s, \
                                Attribute: %d (%s)\n", E->Name.c_str(), E->Attribute,
                                AttrTypes[static_cast<unsigned>(E->Attribute)].first.c_str());
                    retVal.retM++;
                }
            }
        }
    }
}

/*==============================================================================
 * TaskRecord_t::IntegSupervisors(): Run an integrity check on the supervisors
 * in the range of DStart-DEnd. 
 *============================================================================*/
void TaskRecord_t::IntegSupervisors(bool Verbose, FILE * fp, unsigned long SStart, 
                                    unsigned long SEnd, IntegVals_t &retVal, 
                                    AB_UNSIGNED &MappedSupervisors)
{
    for(std::vector<Record_t>::const_iterator S = Supervisors.begin() + SStart;
        (S != Supervisors.end()) && (S < (Supervisors.begin() + SEnd));
        S++)    // Iterate through Supervisor vector
    {
        // Check record type
        if(S->RecordType != Supervisor)
        {
            // Non-Supervisor in the Supervisor vector. Something, somewhere has gone very VERY wrong
            if (Verbose) fprintf(fp, "Non-Supervisor in Supervisor Vector: %s, \
                        %" SYMA_FMT ", %d\n", S->Name.c_str(), S->Address, S->RecordType);
            retVal.ret++;
        } else {
            // Check that Supervisor is listed in Addr Map
            AddrMap_t::const_iterator SASearch = AddrMap.find(S->Address);    // Find the Addr in the map
            if (SASearch == AddrMap.end()) {
                // SupAddr not mapped - Link broken
                if (Verbose) fprintf(fp, "L: SupAddr not mapped: %s, %" SYMA_FMT "\n",
                            S->Name.c_str(), S->Address);
                retVal.retL++;
            } else if (SASearch->second != &(*S)) {
                // Address does not map to THIS supervisor - Link broken
                if (Verbose) fprintf(fp, "L: AddrMap to wrong Supervisor: %s, %" PTR_FMT "!=%" PTR_FMT "\n",
                            S->Name.c_str(), reinterpret_cast<uint64_t>(SASearch->second),
                            reinterpret_cast<uint64_t>(&(*S)));
                retVal.retL++;
            }

            // Check that Supervisor is listed in SupMap
            SupMap_t::const_iterator SMSearch = SupMap.find(S->Address);
            if (SMSearch == SupMap.end())
            {
                // SupAddr not mapped - Link broken
                if (Verbose) fprintf(fp, "L: SupAddr not in SupMap: %s, %" SYMA_FMT "\n",
                            S->Name.c_str(), S->Address);
                retVal.retL++;
            } else {
                MappedSupervisors++;

                // Check all devices allocated to supervisor use THIS supervisor
                for(RecordVect_t::const_iterator SMV
                =SMSearch->second.begin();SMV!=SMSearch->second.end();SMV++)    // Iterate through Supervisor's devices
                {
                    if((*SMV)->Supervisor != SMSearch->first)
                    {
                        // Supervisor Address in device record does not match THIS supervisor - Link broken
                        if (Verbose) fprintf(fp, "L: Dev SupAddr mismatch: %s, %"\
                                    SYMA_FMT "!=%" SYMA_FMT "\n", (*SMV)->Name.c_str(),
                                    (*SMV)->Supervisor, SMSearch->first);
                        retVal.retL++;
                    }
                }
            }

            // Check Device type Index
            if (S->DeviceType >= DevTypes.size()) {
                // DeviceType Index out of range
                if (Verbose) fprintf(fp, "DeviceType Index out of range: Supervisor (%" SYMA_FMT \
                            "), DType: %u>%d\n", S->Address, S->DeviceType,
                            static_cast<int>(DevTypes.size()));
                retVal.ret++;
            } else {
                // Find the pointer to the device record in the Device Type vector
                const RecordVect_t *DTypeV = &DevTypes[S->DeviceType].second;
                if (std::find(DTypeV->begin(), DTypeV->end(), &(*S)) == DTypeV->end())
                {
                    // Supervisor not in Device Type vector - Map Broken
                    if (Verbose) fprintf(fp, "M: Supervisor (%" SYMA_FMT \
                                ") not in DeviceType Vector %u (%s)\n",
                                S->Address, S->DeviceType,
                                DevTypes[S->DeviceType].first.Name.c_str());
                    retVal.retM++;
                }
            }
        }
    }
}

} /* namespace AddressBook */
