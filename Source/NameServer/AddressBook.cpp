//==============================================================================
#include "AddressBook.hpp"

#include <iostream>

//==============================================================================
namespace AddressBookNS
{
#ifdef EXCEPTS
#define ERETURN(X, Y) throw(X)
#else
#define ERETURN(X, Y) return Y
#endif

// static constant definitions
const unsigned AddressBook::SUCCESS;
const unsigned AddressBook::ERR_INVALID_TASK;
const unsigned AddressBook::ERR_INVALID_DEVTYPE;
const unsigned AddressBook::ERR_INVALID_DEVICE;
const unsigned AddressBook::ERR_INVALID_MESSAGE_TYPE;
const unsigned AddressBook::ERR_INVALID_ATTRIBUTE;
const unsigned AddressBook::ERR_DEVICE_DATA_MISMATCH;
const unsigned AddressBook::ERR_TASKNAME_USED;
const unsigned AddressBook::ERR_DEVICENAME_USED;
const unsigned AddressBook::ERR_DEVICE_ADDR_USED;
const unsigned AddressBook::ERR_TASK_NOT_FOUND;
const unsigned AddressBook::ERR_DEVICE_NOT_FOUND;
const unsigned AddressBook::ERR_INVALID_MAP;
const unsigned AddressBook::ERR_INVALID_SUPERVISOR;

//Constructors
AddressBook::AddressBook(std::string d)
{
   ABderived = d;
   TaskCount = 0;
}

AddressBook::~AddressBook()
{
    // Teardown the TaskMap. Everything else should be handled properly.
    for(TaskMap_t::iterator T=TaskMap.begin();T!=TaskMap.end();T++)
    {
        TaskRecord_t* TRec = &(*(T->second));
        delete TRec;

    }
}


/*==============================================================================
 * GetTaskCount:    Get the number of currently loaded tasks
 *============================================================================*/
unsigned AddressBook::GetTaskCount(void)
{
    return TaskMap.size();
}

//==============================================================================
//Task Manipulation
unsigned AddressBook::AddTask(const std::string &TaskName, TaskData_t &Data)
{
    // Check that the task does not already exist.
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec != PNULL)
    {
        //throw("Task Name Already Used");
        return ERR_TASKNAME_USED;
    }

    // validate here and exit if the device type data is invalid. These will
    // actually be checked again when device types are added below; a duplication,
    // but the number of device types and number of message types is going to
    // be relatively small, so this isn't going to be too costly
    unsigned Ret = ValidateTask(Data);
    if(Ret) return Ret;

    // Build a TaskRecord
    TaskRecord_t * Task  = new TaskRecord_t;

    Task->Name = TaskName;
    Task->Path = Data.Path;
    Task->XML = Data.XML;
    Task->ExecPath = Data.ExecutablePath;

    Task->DevCntMax =  Data.DeviceCount;
    Task->ExtCntMax = Data.ExternalCount;

    // Pre-reserve the Vectors of Records for consistent pointers.
    Task->Devices.reserve(Data.DeviceCount + 1);
    Task->Externals.reserve(Data.ExternalCount + 1);
    Task->Supervisors.reserve(MAXSUPERVISORS + 1);

    // String Data - copy it over, initialise vectors and create index maps.
    // MessageType strings.
    Task->MsgTypes.resize(Data.MessageTypes.size());   //pre-alloc vector
    std::vector<std::string> *MTypes = &(Data.MessageTypes);
    for(std::vector<std::string>::const_iterator M=MTypes->begin();
            M!=MTypes->end(); M++)
    {
        Task->MsgTypes[static_cast<unsigned>(M - MTypes->begin())].Name = *M;
        Task->MsgTypeMap.insert(IdxMap_t::value_type(*M, (M-MTypes->begin())));
    }

    // AttributeType strings.
    Task->AttrTypes.resize(Data.AttributeTypes.size());
    std::vector<std::string> *ATypes = &(Data.AttributeTypes);
    for(std::vector<std::string>::const_iterator A=ATypes->begin();
            A!=ATypes->end(); A++)
    {
        AttrTypePair APair(*A,RecordVect_t());
        Task->AttrTypes[static_cast<unsigned>(A - ATypes->begin())] = APair;
        Task->AttrTypeMap.insert(IdxMap_t::value_type(*A, (A-ATypes->begin())));
    }

    // Add to the Task Map.
    TaskMap.insert(TaskMap_t::value_type(Task->Name, Task));
    ++TaskCount; // Pedantic: avoid assignment to temporary 5 July 2019 ADR

    /* Insert DeviceTypes. Changed order and call subfunction 5 July 2019 ADR
       Original had pre-allocation below, but it's hard to see how this
       really gains anything because a DevTypeRecord_t contains 2 internal
       vectors (which would be zero-initialised) and a resize is thus inevitable
       anyway when actual device types are inserted.
    */
    // Task->DevTypes.resize(Data.DeviceTypes.size());   //pre-alloc vector
    std::vector<DevTypeRecord_t> *DTypes = &(Data.DeviceTypes);
    for(std::vector<DevTypeRecord_t>::iterator D=DTypes->begin();
            D!=DTypes->end(); D++)       
        AddDeviceType(Task->Name, *D); // add is bound to succeed because we have
                                       // pre-validated.
    return SUCCESS;
}

/*==============================================================================
 * AddDeviceType:   Append another device type to the task.
 *============================================================================*/
unsigned AddressBook::AddDeviceType(const std::string &TaskName, const DevTypeRecord_t &DeviceType)
{
    // Check that the task exists.
    TaskRecord_t *Task = FindTask(TaskName);
    if(Task == PNULL)
    {
        return ERR_TASK_NOT_FOUND;
    }
    // and that the new device type has valid message indices
    if (unsigned retVal = ValidateDeviceType(DeviceType, Task->MsgTypes.size()))
        return retVal;
    DevTypePair DPair(DeviceType,RecordVect_t());
    Task->DevTypes.push_back(DPair);
    unsigned devTypIdx = Task->DevTypes.size() - 1;
    Task->DevTypeMap.insert(IdxMap_t::value_type(DeviceType.Name, devTypIdx));

    // Cross-ref to Message Types.
    for(std::vector<MsgIdx>::const_iterator I=DeviceType.InMsgs.begin();
        I!=DeviceType.InMsgs.end(); I++)        // Input Messages
    {
        Task->MsgTypes[*I].Inputs.push_back(devTypIdx);
    }
    for(std::vector<MsgIdx>::const_iterator O=DeviceType.OuMsgs.begin();
        O!=DeviceType.OuMsgs.end(); O++)        // Output Messages
    {
        Task->MsgTypes[*O].Outputs.push_back(devTypIdx);
    }
    return SUCCESS;
}

/*==============================================================================
 * ClearTask:    Removes all the devices from a task
 *============================================================================*/
unsigned AddressBook::ClearTask(const std::string &TaskName)
{
    // The easiest way to do this is simply to copy the data to be saved into
    // a TaskData_t object, remove the task, then re-add it with all the device
    // data removed.
    TaskData_t cTask;
    unsigned err = SUCCESS;
    if ((err = GetTask(TaskName, cTask)) != SUCCESS) return err;
    cTask.DeviceCountLd = 0;
    cTask.ExternalCountLd = 0;
    cTask.SupervisorCount = 0;
    if ((err = DelTask(TaskName)) != SUCCESS) return err;
    return AddTask(TaskName, cTask);
}  
    
/*==============================================================================
 * ListTask:    Populate a vector with all of the names of loaded tasks
 *============================================================================*/
unsigned AddressBook::ListTask(std::vector<std::string> &Tasks)
{
    // Enumerate the Task List.
    for(TaskMap_t::iterator T=TaskMap.begin();T!=TaskMap.end();T++)
    {
        Tasks.push_back(T->second->Name);
    }
    return SUCCESS;
}

/*==============================================================================
 * GetTask:     Get a task by name
 *============================================================================*/
unsigned AddressBook::GetTask(const std::string &TaskName, TaskData_t &Task)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        //throw("Task Does Not Exist");
        return ERR_TASK_NOT_FOUND;
    }

    Task.Name = TRec->Name;
    Task.Path = TRec->Path;
    Task.XML = TRec->XML;
    Task.ExecutablePath = TRec->ExecPath;
    Task.State = TRec->State;
    Task.DeviceCount = TRec->DevCntMax;
    Task.DeviceCountLd = TRec->DevCnt;
    Task.ExternalCount = TRec->ExtCntMax;
    Task.ExternalCountLd = TRec->ExtCnt;
    Task.SupervisorCount = TRec->SupCnt;

    // Copy MessageType strings
    for(std::vector<MsgTypeRecord_t>::const_iterator M=TRec->MsgTypes.begin();
            M!=TRec->MsgTypes.end(); M++)
    {
        Task.MessageTypes.push_back(M->Name);
    }

    // Copy DeviceType strings
    for(std::vector<DevTypePair>::const_iterator D=TRec->DevTypes.begin();
            D!=TRec->DevTypes.end(); D++)
    {
        Task.DeviceTypes.push_back(D->first);
    }

    // Copy AttributeType strings
    for(std::vector<AttrTypePair>::const_iterator A=TRec->AttrTypes.begin();
            A!=TRec->AttrTypes.end(); A++)
    {
        Task.AttributeTypes.push_back(A->first);
    }

    return SUCCESS;
}

/*==============================================================================
 * DelTask:     Delete the named task
 *============================================================================*/
unsigned AddressBook::DelTask(const std::string &TaskName)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        //throw("Task Does Not Exist");
        return ERR_TASK_NOT_FOUND;
    }

    // Erase it
    delete TRec;
    TaskMap.erase(TaskName);

    TaskCount--;

    return SUCCESS;
}

/*==============================================================================
 * TaskState:   Get the state of the task
 *============================================================================*/
TaskState_t AddressBook::TaskState(std::string &TaskName)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        //throw("Task Does Not Exist");
        return static_cast<TaskState_t>(Unknown);
    }

    return TRec->State;
}

/*==============================================================================
 * TaskState:   Set the state of the task
 *============================================================================*/
unsigned AddressBook::TaskState(std::string &TaskName, TaskState_t State)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        //throw("Task Does Not Exist");
        return ERR_TASK_NOT_FOUND;
    }

    TRec->State = State;

    return SUCCESS;
}

/*==============================================================================
 * TaskExecPath:    Get the path to the executables for the task
 *============================================================================*/
std::string AddressBook::TaskExecPath(std::string &TaskName)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        //throw("Task Does Not Exist");
        return "";
    }

    return TRec->ExecPath;
}

/*==============================================================================
 * TaskExecPath:    Set the path to the executables for the task
 *============================================================================*/
unsigned AddressBook::TaskExecPath(std::string &TaskName, std::string &NewPath)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    TRec->ExecPath = NewPath;
    return SUCCESS;
}

/*==============================================================================
 * TaskValid:   Indicates whether the Task's data is valid
 *============================================================================*/
bool AddressBook::TaskValid(std::string &TaskName)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", false);
    }

    return TRec->TaskValid;
}

/*==============================================================================
 * TaskMapValid:    Indicates whether the Task's mappings are valid
 *============================================================================*/
bool AddressBook::TaskMapValid(std::string &TaskName)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", false);
    }

    return TRec->MapValid;
}

/*==============================================================================
 * TaskLinkValid:   Indicates whether the Task's linkage is valid
 *============================================================================*/
bool AddressBook::TaskLinkValid(std::string &TaskName)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", false);
    }

    return TRec->LinkValid;
}


/*==============================================================================
 * RebuildTask: Rebuild the Task's MessageType/DeviceType maps
 *============================================================================*/
unsigned AddressBook::RebuildTask(std::string &TaskName)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    //TODO: test this

    // Rebuild MessageType map
    TRec->MsgTypeMap.clear();
    for(std::vector<MsgTypeRecord_t>::iterator M=TRec->MsgTypes.begin();
            M!=TRec->MsgTypes.end(); M++)
    {
        M->Inputs.clear();              // Clear the Input vector data.
        M->Outputs.clear();             // Clear the Output vector data.
        TRec->MsgTypeMap.insert(IdxMap_t::value_type(M->Name, (M-TRec->MsgTypes.begin())));
    }

    //Rebuild DeviceType Map and links to MessageType - leave the contents of the Device Vector alone!
    TRec->DevTypeMap.clear();
    for(std::vector<DevTypePair>::iterator D=TRec->DevTypes.begin();
            D!=TRec->DevTypes.end(); D++)
    {
        TRec->DevTypeMap.insert(IdxMap_t::value_type(D->first.Name, (D-TRec->DevTypes.begin())));

        // Cross-ref to Message Types.
        for(std::vector<MsgIdx>::iterator I=D->first.InMsgs.begin();
            I!=D->first.InMsgs.end(); I++)        // Input Messages
        {
            TRec->MsgTypes[*I].Inputs.push_back(static_cast<unsigned>(D-TRec->DevTypes.begin()));
        }
        for(std::vector<MsgIdx>::iterator O=D->first.OuMsgs.begin();
            O!=D->first.OuMsgs.end(); O++)        // Output Messages
        {
            TRec->MsgTypes[*O].Outputs.push_back(static_cast<unsigned>(D-TRec->DevTypes.begin()));
        }
    }

    //Rebuild AttributeType map - leave the contents of the Device Vector alone!
    TRec->AttrTypeMap.clear();
    for(std::vector<AttrTypePair>::iterator A=TRec->AttrTypes.begin();
            A!=TRec->AttrTypes.end(); A++)
    {
        TRec->AttrTypeMap.insert(IdxMap_t::value_type(A->first, (A-TRec->AttrTypes.begin())));
    }

    return SUCCESS;
}

/*==============================================================================
 * BuildMaps:   Build Task Maps
 *============================================================================*/
unsigned AddressBook::BuildMaps(std::string &TaskName)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    if(TRec->MapValid || TRec->NameMap.size() > 0
        || TRec->ExtCon.size() > 0) ClearMap(TRec);   // Existing map, clear it


    // Map all of the Devices
    for(std::vector<Record_t>::iterator DRec=TRec->Devices.begin();
            DRec!=TRec->Devices.end(); DRec++)
    {
        MapDevice(TRec, &(*DRec));
    }

    // Map all of the Externals
    for(std::vector<Record_t>::iterator ERec=TRec->Externals.begin();
            ERec!=TRec->Externals.end(); ERec++)
    {
        MapDevice(TRec, &(*ERec));
    }

    // Map all of the Supervisors
    for(std::vector<Record_t>::iterator SRec=TRec->Supervisors.begin();
            SRec!=TRec->Supervisors.end(); SRec++)
    {
        // Add the supervisor to the correct DeviceType list.
        TRec->DevTypes[SRec->DeviceType].second.push_back(&(*SRec));
    }

    TRec->MapValid = true; // added 18 July 2019 ADR update the map status
    
    return SUCCESS;
}

/*==============================================================================
 * BuildLink:   Build Task linkage
 *============================================================================*/
unsigned AddressBook::BuildLink(std::string &TaskName)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    if(TRec->LinkValid || TRec->AddrMap.size() > 0) ClearLink(TRec);   // Existing link, clear it

    // Link all of the Devices
    for(std::vector<Record_t>::iterator DRec=TRec->Devices.begin();
            DRec!=TRec->Devices.end(); DRec++)
    {
        LinkDevice(TRec, &(*DRec));
    }

    // Link all of the Externals
    for(std::vector<Record_t>::iterator ERec=TRec->Externals.begin();
            ERec!=TRec->Externals.end(); ERec++)
    {
        TRec->AddrMap.insert(AddrMap_t::value_type(ERec->Address, &(*ERec)));
    }

    // Link all of the Supervisors
    for(std::vector<Record_t>::iterator SRec=TRec->Supervisors.begin();
            SRec!=TRec->Supervisors.end(); SRec++)
    {
        TRec->AddrMap.insert(AddrMap_t::value_type(SRec->Address, &(*SRec)));
    }

    TRec->LinkValid = true; // added 18 July 2019 ADR update the link status

    return SUCCESS;
}


//==============================================================================
//Device Addition
unsigned AddressBook::AddDevice(std::string &TaskName, Record_t &DevRec, bool Validate)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    Record_t *DRec;

    if (Validate) // Check that the device does not already exist
    {             // and that the indices are valid.
        unsigned Ret = ValidateDevice(TRec, DevRec);
        if(Ret) return Ret;
    }

    if ((DevRec.RecordType == Device) || (DevRec.RecordType == DeviceExt))
    {
        // Add a Device
        CheckVector(TRec, TRec->Devices);    // Sanity check for reallocation.

        TRec->Devices.push_back(DevRec);     // Add Dev to task.
        TRec->DevCnt++;
        DRec = &TRec->Devices.back();

        LinkDevice(TRec, DRec);         // Add Device to name map & supervisor list.
        MapDevice(TRec, DRec);          // Add dev to other maps & lists.

    } else if (DevRec.RecordType == External) {
        // Add an External
        CheckVector(TRec, TRec->Externals); // Sanity check for reallocation

        TRec->Externals.push_back(DevRec);
        TRec->ExtCnt++;
        DRec = &TRec->Externals.back();

        TRec->AddrMap.insert(AddrMap_t::value_type(DRec->Address, DRec));
        MapDevice(TRec, DRec);          // Add dev to other maps & lists.

    } else if (DevRec.RecordType == Supervisor) {
        // Add a Supervisor
        CheckVector(TRec, TRec->Supervisors);  // Sanity check for reallocation.

        TRec->Supervisors.push_back(DevRec);
        TRec->SupCnt++;
        DRec = &TRec->Supervisors.back();

        TRec->AddrMap.insert(AddrMap_t::value_type(DRec->Address, DRec));

        // Add the supervisor to the correct DeviceType list.
        TRec->DevTypes[DRec->DeviceType].second.push_back(DRec);

        SupMap_t::iterator SSearch = TRec->SupMap.find(DRec->Address);
        if (SSearch == TRec->SupMap.end())
        {   //Supervisor is NOT in the map yet.
            TRec->SupMap.insert(SupMap_t::value_type(DRec->Address,
                                    RecordVect_t()));
        }
    }
    // added 18 July 2019 ADR - if Map and Link were previously invalidated
    // rebuild everything
    unsigned err;
    if (!TRec->MapValid && ((err = BuildMaps(TRec->Name)) != SUCCESS)) return err;   
    if (!TRec->LinkValid && ((err = BuildLink(TRec->Name)) != SUCCESS)) return err;  
    return SUCCESS;
}


/*==============================================================================
 * UpdateDevice:    Update a device with a new address and supervisor.
 *                  This implicitly marks the Linkage as invalid.
 *============================================================================*/
unsigned AddressBook::UpdateDevice(std::string &TaskName, DeviceData_t &Data)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    // Check that the Device Name exists
    NameMap_t::iterator DSearch = TRec->NameMap.find(Data.Name);
    if (DSearch == TRec->NameMap.end()) {
        return ERR_DEVICE_NOT_FOUND;
    }
    Record_t *DRec = DSearch->second;

    TRec->LinkValid = false;              // Mark the Task linkage as dirty
    DRec->Address = Data.Address;         // Update the Device Address
    DRec->Supervisor = Data.Supervisor;   // Update the Device Supervisor
    return SUCCESS;
}

/*==============================================================================
 * GetDeviceCount:  Get the number of devices reserved in the named task
 *============================================================================*/
long AddressBook::GetDeviceCount(std::string &TaskName)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_BAD_COUNT);
    }

    return static_cast<long>(TRec->DevCntMax);
}

/*==============================================================================
 * GetLoadedDeviceCount:    Get the number of devices loaded in the named task
 *============================================================================*/
long AddressBook::GetLoadedDeviceCount(std::string &TaskName)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_BAD_COUNT);
    }

    return static_cast<long>(TRec->DevCnt);
}

/*==============================================================================
 * GetExternalCount:    Get the number of externals reserved in the named task
 *============================================================================*/
long AddressBook::GetExternalCount(std::string &TaskName)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_BAD_COUNT);
    }

    return static_cast<long>(TRec->ExtCntMax);
}

/*==============================================================================
 * GetLoadedExternalCount:  Get the number of externals loaded in the named task
 *============================================================================*/
long AddressBook::GetLoadedExternalCount(std::string &TaskName)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_BAD_COUNT);
    }

    return static_cast<long>(TRec->ExtCnt);
}

/*==============================================================================
 * GetSupervisorCount:  Get the number of supervisors allocated to the task
 *============================================================================*/
int AddressBook::GetSupervisorCount(std::string &TaskName)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_BAD_COUNT);
    }

    return static_cast<int>(TRec->SupCnt);
}


//==============================================================================
//Device Queries

/*==============================================================================
 * FindDevice:  Find a device by Address.
 *              Gives a Pointer to Const Device Record
 *============================================================================*/
unsigned AddressBook::FindDevice(std::string &TaskName, SymAddr_t Address, const Record_t* &DRec)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    if(!(TRec->TaskValid && TRec->MapValid && TRec->LinkValid))        // Return NotFound if dirty
    {
        return ERR_INVALID_MAP;
    }

    AddrMap_t::iterator DSearch = TRec->AddrMap.find(Address);    // Find the Device
    if (DSearch == TRec->AddrMap.end()) {
        ERETURN("Device Not Found", ERR_DEVICE_NOT_FOUND);
    }

    DRec = &(*DSearch->second);
    return SUCCESS;
}


/*==============================================================================
 * FindDevice:  Find a device by Name.
 *              Gives a Pointer to Const Device Record
 *============================================================================*/
unsigned AddressBook::FindDevice(std::string &TaskName, std::string &Name, const Record_t* &DRec)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    if(!(TRec->TaskValid && TRec->MapValid && TRec->LinkValid))        // Return NotFound if dirty
    {
        return ERR_INVALID_MAP;
    }

    NameMap_t::iterator DSearch = TRec->NameMap.find(Name);        // Find the Device
    if (DSearch == TRec->NameMap.end()) {
        ERETURN("Device Not Found", ERR_DEVICE_NOT_FOUND);
    }

    DRec = &(*DSearch->second);
    return SUCCESS;
}


/*==============================================================================
 * FindBySuper: Find a vector of devices supervised by Supervisor.
 *              Gives a Pointer to a const Vector of Pointers to const Device
 *              Records.
 *============================================================================*/
unsigned AddressBook::FindBySuper(std::string &TaskName, SymAddr_t Supervisor, const RecordVect_t* &Records)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    if(!(TRec->TaskValid && TRec->MapValid && TRec->LinkValid))        // Return NotFound if dirty
    {
        return ERR_INVALID_MAP;
    }

    SupMap_t::iterator SSearch = TRec->SupMap.find(Supervisor);   // Find the Supervisor
    if (SSearch == TRec->SupMap.end()) {
        ERETURN("Device Not Found", ERR_DEVICE_NOT_FOUND);
    }

    Records = &SSearch->second;
    return SUCCESS;
}

unsigned AddressBook::FindByType(std::string &TaskName, std::string Type, const RecordVect_t* &Records)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    if(!(TRec->TaskValid && TRec->MapValid && TRec->LinkValid))        // Return NotFound if dirty
    {
        return ERR_INVALID_MAP;
    }

    IdxMap_t::iterator TSearch = TRec->DevTypeMap.find(Type);  // Find the Index
    if (TSearch == TRec->DevTypeMap.end()) {
        ERETURN("Device Type Not Found", ERR_DEVICE_NOT_FOUND);
    }

    Records = &(TRec->DevTypes[TSearch->second].second);

    return SUCCESS;
}

unsigned AddressBook::FindByAttribute(std::string &TaskName, std::string Attribute, const RecordVect_t* &Records)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    if(!(TRec->TaskValid && TRec->MapValid && TRec->LinkValid))        // Return NotFound if dirty
    {
        return ERR_INVALID_MAP;
    }

    IdxMap_t::iterator ASearch = TRec->AttrTypeMap.find(Attribute);  // Find the Index
    if (ASearch == TRec->AttrTypeMap.end()) {
        ERETURN("Device Type Not Found", ERR_DEVICE_NOT_FOUND);
    }

    Records = &(TRec->AttrTypes[ASearch->second].second);

    return SUCCESS;
}

unsigned AddressBook::FindByInMsg(std::string &TaskName, std::string Msg, const RecordVect_t* &Records)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    if(!(TRec->TaskValid && TRec->MapValid && TRec->LinkValid))        // Return NotFound if dirty
    {
        return ERR_INVALID_MAP;
    }

    return SUCCESS;
}

unsigned AddressBook::FindByOuMsg(std::string &TaskName, std::string Msg, const RecordVect_t* &Records)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    if(!(TRec->TaskValid && TRec->MapValid && TRec->LinkValid))        // Return NotFound if dirty
    {
        return ERR_INVALID_MAP;
    }

    return SUCCESS;
}

unsigned AddressBook::GetDevices(std::string &TaskName, const std::vector<Record_t>* &Records)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    if(!(TRec->TaskValid && TRec->MapValid && TRec->LinkValid))        // Return NotFound if dirty
    {
        return ERR_INVALID_MAP;
    }

    Records = &(TRec->Devices);

    return SUCCESS;
}

unsigned AddressBook::GetExternals(std::string &TaskName, const std::vector<Record_t>* &Records)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    if(!(TRec->TaskValid && TRec->MapValid && TRec->LinkValid))        // Return NotFound if dirty
    {
        return ERR_INVALID_MAP;
    }

    Records = &(TRec->Externals);

    return SUCCESS;
}

unsigned AddressBook::GetSupervisors(std::string &TaskName, const std::vector<Record_t>* &Records)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    if(!(TRec->TaskValid && TRec->MapValid && TRec->LinkValid))        // Return NotFound if dirty
    {
        return ERR_INVALID_MAP;
    }

    Records = &(TRec->Supervisors);

    return SUCCESS;
}

unsigned AddressBook::GetExtCon(std::string &TaskName, const RecordVect_t* &Records)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    if(!(TRec->TaskValid && TRec->MapValid && TRec->LinkValid))        // Return NotFound if dirty
    {
        return ERR_INVALID_MAP;
    }

    Records = &(TRec->ExtCon);

    return SUCCESS;
}




//==============================================================================
//Private Methods

/*==============================================================================
 * FindTask:    Return a pointer to the named task.
 *============================================================================*/
TaskRecord_t * AddressBook::FindTask(const std::string &TaskName)
{
    // Check that the task exists.
    TaskMap_t::iterator TSearch = TaskMap.find(TaskName);
    if (TSearch == TaskMap.end()) {
        ERETURN("Task Does Not Exist", PNULL);
    }

    return TSearch->second;
}


unsigned AddressBook::MapDevice(TaskRecord_t *TRec, Record_t *DRec)
{
    // Add device to name map
    TRec->NameMap.insert(NameMap_t::value_type(DRec->Name, DRec));

    // Add the device to the correct DeviceType list.
    TRec->DevTypes[DRec->DeviceType].second.push_back(DRec);

    if(DRec->RecordType == DeviceExt)  // Device has an external conn
    {
        TRec->ExtCon.push_back(DRec);  // Add it to the list of such devices.
    }

    // Add the device to the correct attribute list
    if(DRec->Attribute > -1)
    {
        TRec->AttrTypes[static_cast<unsigned>(DRec->Attribute)].second.push_back(DRec);
    }

    return SUCCESS;
}

unsigned AddressBook::LinkDevice(TaskRecord_t *TRec, Record_t *DRec)
{
    // Add Device to Address map.
    TRec->AddrMap.insert(AddrMap_t::value_type(DRec->Address, DRec));

    // Add the device to the correct Supervisor list.
    // Supervisors may be loaded after devices, may need to build the map.
    SupMap_t::iterator SSearch = TRec->SupMap.find(DRec->Supervisor);
    if (SSearch == TRec->SupMap.end()) { //Supervisor is NOT in the map yet.
        std::pair<SupMap_t::iterator,bool> LastVal;     // Have to do this because C++98 does not like initialiser lists...
        LastVal = TRec->SupMap.insert(SupMap_t::value_type(DRec->Supervisor, RecordVect_t()));
        LastVal.first->second.push_back(DRec);  // RecordVect_t({DRec}) in the insert would be neater and not need the pair.

    } else {                             //Supervisor is already in the map.
        SSearch->second.push_back(DRec);
    }

    return SUCCESS;
}


/*==============================================================================
 * ValidateTask:    Check that the task has a valid vector of DeviceTypes.
 *============================================================================*/
unsigned AddressBook::ValidateTask(TaskData_t &Data)
{
    for(std::vector<DevTypeRecord_t>::const_iterator D=Data.DeviceTypes.begin();
            D!=Data.DeviceTypes.end(); D++)
    {
        if (unsigned retVal = ValidateDeviceType(*D, Data.MessageTypes.size()))
            return retVal;
    }

    return SUCCESS;
}

/*==============================================================================
 * ValidateDeviceType:  Check that DeviceType Message indices are valid and not 
 *                  out of range.
 * Added and separated from ValidateTask 5 July 2019 ADR
 *============================================================================*/
unsigned AddressBook::ValidateDeviceType(const DevTypeRecord_t& DevTypRec, unsigned MaxIdx)
{
    for(std::vector<MsgIdx>::const_iterator I=DevTypRec.InMsgs.begin();
        I!=DevTypRec.InMsgs.end(); I++)
    {
        if(*I >= MaxIdx)
        {
            ERETURN("Invalid MessageType Index", ERR_INVALID_MESSAGE_TYPE);
        }
    }

    // changed to iterate over OuMsgs (was InMsgs) 5 July 2019 ADR
    for(std::vector<MsgIdx>::const_iterator O=DevTypRec.OuMsgs.begin();
        O!=DevTypRec.OuMsgs.end(); O++)
    {
        if(*O >= MaxIdx)
        {
            ERETURN("Invalid MessageType Index", ERR_INVALID_MESSAGE_TYPE);
        }
    }
    return 0;
}


/*==============================================================================
 * ValidateDevice:  Check that the device does not already exist in the name map
 *                  and that indices in the Record are not out of range.
 *============================================================================*/
unsigned AddressBook::ValidateDevice(TaskRecord_t *TRec, Record_t &DevRec)
{
    // Check that the device does not already exist in the maps
    AddrMap_t::const_iterator DASearch = TRec->AddrMap.find(DevRec.Address);
    if (DASearch != TRec->AddrMap.end()) {
        ERETURN("DUPADDR: Device Address already added", ERR_DEVICE_ADDR_USED);
    }

    NameMap_t::const_iterator DNSearch = TRec->NameMap.find(DevRec.Name);
    if (DNSearch != TRec->NameMap.end()) {
        ERETURN("DUPNAME: Device Name already added", ERR_DEVICENAME_USED);
    }

    // Check that the indices are valid BEFORE adding the Device.
    if (DevRec.DeviceType >= TRec->DevTypes.size()) {
        ERETURN("Invalid DeviceType Index", ERR_INVALID_DEVTYPE);
    }

    if (DevRec.Attribute >= static_cast<int>(TRec->AttrTypes.size())) {
        ERETURN("Invalid AttributeType Index", ERR_INVALID_ATTRIBUTE);
    }

    return SUCCESS;   // Validation Successful
}

/*==============================================================================
 * CheckVector: Check that adding a Record to the vector won't cause a
 *              reallocation as this would silently invalidate all of the maps.
 *              If the vector is at capacity, increase it's size, clear the task
 *              linkage & mappings and mark both as dirty.
 *
 *              As the Record vectors are pre-sized based on what should be in
 *              the task plus one element, there should never be a reallocation.
 *============================================================================*/
unsigned AddressBook::CheckVector(TaskRecord_t *TRec, std::vector<Record_t> &Vect)
{
    if(Vect.size() == Vect.capacity())
    {
        Vect.reserve(static_cast<unsigned>(Vect.capacity()*1.25));
        ClearLink(TRec);
        ClearMap(TRec);
        return ERR_INVALID_MAP;
    }
    return SUCCESS;
}


/*==============================================================================
 * ClearMap:    Clear the Device to DeviceType/Attribute/(Message) Type links
 *              and mark the linkage as dirty. Also clears the Name>Record map
 *              and list of records with external connections.
 *============================================================================*/
unsigned AddressBook::ClearMap(TaskRecord_t *TRec)
{
    TRec->MapValid = false;     // Mark the map as dirty.
    TRec->NameMap.clear();      // Clear the Name map
    TRec->ExtCon.clear();       // Clear External Connections List.

    // Clear DevType->DeviceRecord vectors
    for(std::vector<DevTypePair>::iterator D=TRec->DevTypes.begin();
           D!=TRec->DevTypes.end();D++)
    {
        D->second.clear();
    }

    // Clear Attribute->DeviceRecord vectors
    for(std::vector<AttrTypePair>::iterator A=TRec->AttrTypes.begin();
           A!=TRec->AttrTypes.end();A++)
    {
        A->second.clear();
    }

    return SUCCESS;
}


/*==============================================================================
 * ClearLink:   Clear the Address>Record map and the Supervisor>Device links.
 *              Also marks the linkage as dirty.
 *============================================================================*/
unsigned AddressBook::ClearLink(TaskRecord_t *TRec)
{
    TRec->LinkValid = false;    // Mark the linkage as dirty.
    TRec->AddrMap.clear();      // Clear the Addres>DeviceRecord map.

    //Clear the Device Record vectors for each supervisor.
    for(SupMap_t::iterator S=TRec->SupMap.begin();S!=TRec->SupMap.end();S++)
    {
         S->second.clear();
    }

    return SUCCESS;
}


//==============================================================================
//Integrity Checks
unsigned AddressBook::IntegCheck(void)
{
    std::vector<unsigned> Temp;
    return IntegCheck(Temp);
}

unsigned AddressBook::IntegCheck(std::vector<unsigned> &TaskIntegs)
{
    unsigned TInteg, Ret = 0;

    for(TaskMap_t::const_iterator T=TaskMap.begin();
            T!=TaskMap.end();T++)
    {
        TInteg = T->second->Integrity();
        TaskIntegs.push_back(TInteg);
        if(TInteg) Ret++;
    }

    return Ret;
}

unsigned AddressBook::IntegTask(std::string &TaskName, bool Verbose, FILE * fp)
{
    TaskRecord_t *TRec = FindTask(TaskName);
    if(TRec == PNULL)
    {
        ERETURN("Task Does Not Exist", ERR_TASK_NOT_FOUND);
    }

    return TRec->Integrity(Verbose, fp);
}



//==============================================================================
//Dumps

void AddressBook::Dump(std::string &Name)
{
    Dump(stdout, Name);
}

void AddressBook::Dump(FILE * fp, std::string Name)
{
    fprintf(fp,"AddressBook+++++++++++++++++++++++++++++++++\n");
    fprintf(fp,"ABderived (this derived process) : %s\n",ABderived.c_str());

    if(Name == std::string(""))  // Dump high-level task data.
    {
        fprintf(fp,"Total number of tasks: %d:\n", TaskCount);
        for(TaskMap_t::iterator T=TaskMap.begin();
            T!=TaskMap.end();T++)
        {
            TaskRecord_t *TRec = T->second;
            fprintf(fp,"Task %ld: Key: %s At: %" PTR_FMT "\n",
                        static_cast<long>(std::distance(TaskMap.begin(), T)),
                        T->first.c_str(), reinterpret_cast<uint64_t>(TRec));
            fprintf(fp,"\tName: %s\n", TRec->Name.c_str());
            fprintf(fp,"\tPath: %s\n", TRec->Path.c_str());
            fprintf(fp,"\tXML: %s\n", TRec->XML.c_str());
            fprintf(fp,"\tExecPath: %s\n", TRec->ExecPath.c_str());
            fprintf(fp,"\tExternal Count: %ld\n", TRec->ExtCntMax);
            fprintf(fp,"\tDevice Count: %ld\n", TRec->DevCntMax);
            fprintf(fp,"\tState: %d\n", TRec->State);
            fprintf(fp,"\tAllocated Supervisors: %ld\n", TRec->SupCnt);
            fprintf(fp,"\tLoaded Externals: %ld\n", TRec->ExtCnt);
            fprintf(fp,"\tLoaded Devices: %ld\n", TRec->DevCnt);
        }
    } else {      // Dump task-specific data.
        // Check that the task exists.
        TaskMap_t::const_iterator Search = TaskMap.find(Name);
        if (Search == TaskMap.end()) {
            fprintf(fp,"ERROR: Task %s not found.\n", Name.c_str());
        } else {
            TaskRecord_t *TRec = Search->second;

            //Dump Task Data
            fprintf(fp,"Task: %s at %" PTR_FMT "\n", Search->first.c_str(), reinterpret_cast<uint64_t>(TRec));
            fprintf(fp,"\tPath: %s\n", TRec->Path.c_str());
            fprintf(fp,"\tExecPath: %s\n", TRec->ExecPath.c_str());
            fprintf(fp,"\tExternal Count: %ld\n", TRec->ExtCntMax);
            fprintf(fp,"\tDevice Count: %ld\n", TRec->DevCntMax);
            fprintf(fp,"\tState: %d\tTaskValid: %d\tMap Valid: %d\tLink Valid: %d\n",
                        TRec->State, TRec->TaskValid, TRec->MapValid, TRec->LinkValid);

            //Dump Message Types
            fprintf(fp,"\tMessage Types:\t%lu\n", static_cast<unsigned long>(TRec->MsgTypes.size()));
            fprintf(fp,"\t\tIdx\tName\tMapIdx\tMapName\tDTypeI\tDTypeO\n");
            for(std::vector<MsgTypeRecord_t>::const_iterator M
                =TRec->MsgTypes.begin();M!=TRec->MsgTypes.end();M++)
            {
                fprintf(fp,"\t\t%ld\t%s\t", static_cast<long>(M-TRec->MsgTypes.begin()), M->Name.c_str());

                IdxMap_t::const_iterator MSearch = TRec->MsgTypeMap.find(M->Name);
                if (MSearch == TRec->MsgTypeMap.end()) {
                    fprintf(fp,"Not Mapped\t");
                } else {
                    fprintf(fp,"%d\t%s\t", MSearch->second,
                            TRec->MsgTypes[MSearch->second].Name.c_str());
                }

                for(std::vector<MsgIdx>::const_iterator I=
                    M->Inputs.begin(); I!=M->Inputs.end(); I++)
                {   // Input Messages
                    fprintf(fp,"%d,",*I);
                }
                fprintf(fp,"\t");

                for(std::vector<MsgIdx>::const_iterator O=
                    M->Outputs.begin();O!=M->Outputs.end(); O++)
                {   // Output Messages
                    fprintf(fp,"%d,",*O);
                }
                fprintf(fp,"\n");
            }
            fprintf(fp,"\n");

            //Dump Device Types
            fprintf(fp,"\tDevice Types:\t%lu\n", static_cast<unsigned long>(TRec->DevTypes.size()));
            fprintf(fp,"\t\tIdx\tName\tMapIdx\tMapName\tDevCnt\tInMsgs\tOuMsgs\n");
            for(std::vector<DevTypePair>::const_iterator D
                =TRec->DevTypes.begin();D!=TRec->DevTypes.end();D++)
            {
                fprintf(fp,"\t\t%ld\t%s\t", static_cast<long>(D-TRec->DevTypes.begin()), D->first.Name.c_str());

                //Print the DevTypeMap mapping
                IdxMap_t::const_iterator DSearch = TRec->DevTypeMap.find(D->first.Name);
                if (DSearch == TRec->DevTypeMap.end()) {
                    fprintf(fp,"Not Mapped\t");
                } else {
                    fprintf(fp,"%d\t%s\t", DSearch->second,
                            TRec->DevTypes[DSearch->second].first.Name.c_str());
                }

                //Print the number of devices of this type.
                fprintf(fp,"%lu\t", static_cast<unsigned long>(D->second.size()));

                //Print the Input Msg Indices
                for(std::vector<MsgIdx>::const_iterator I
                    =D->first.InMsgs.begin();I!=D->first.InMsgs.end();I++)
                {
                     fprintf(fp,"%d,", *I);
                }
                fprintf(fp,"\t");

                //Print the Output Msg Indices
                for(std::vector<MsgIdx>::const_iterator O
                    =D->first.OuMsgs.begin();O!=D->first.OuMsgs.end();O++)
                {
                     fprintf(fp,"%d,", *O);
                }
                fprintf(fp,"\n");

            }
            fprintf(fp,"\n");

            //Dump Attribute Types
            fprintf(fp,"\tAttribute Types:\t%lu\n", static_cast<unsigned long>(TRec->AttrTypes.size()));
            if(TRec->AttrTypes.size()>0)
            {
                fprintf(fp,"\t\tIdx\tName\tMapIdx\tMapName\tDevCnt\n");
                for(std::vector<AttrTypePair>::const_iterator A
                    =TRec->AttrTypes.begin();A!=TRec->AttrTypes.end();A++)
                {
                    fprintf(fp,"\t\t%ld\t%s\t", static_cast<long>(A-TRec->AttrTypes.begin()),
                            A->first.c_str());

                    IdxMap_t::const_iterator ASearch = TRec->AttrTypeMap.find(A->first);
                    if (ASearch == TRec->AttrTypeMap.end()) {
                        fprintf(fp,"Not Mapped\t");
                    } else {
                        fprintf(fp,"%d\t%s\t", ASearch->second,
                                TRec->AttrTypes[ASearch->second].first.c_str());
                    }

                    //Print the number of devices with this attribute.
                    fprintf(fp,"%ld\n", static_cast<long>(A->second.size()));
                }
                fprintf(fp,"\n");
            }

            // Dump allocated Supervisors
            fprintf(fp,"\tAllocated Supervisors: %ld\n", TRec->SupCnt);
            if(TRec->SupCnt>0)
            {
                fprintf(fp,"\t\tIdx\tAddress\tRank\tDev Count\n");
                for(std::vector<Record_t>::const_iterator R
                    =TRec->Supervisors.begin();R!=TRec->Supervisors.end();R++)
                {
                    //TODO: Write the map search to get count of devices for
                    // a supervisor.
                    fprintf(fp,"\t\t%ld\t%" SYMA_FMT "\t%lu\t%d\n",
                           static_cast<long>(R-TRec->Supervisors.begin()),
                           R->Address, R->Rank, 0);
                }
                fprintf(fp,"\n");
            }

            // Dump loaded Externals
            fprintf(fp,"\tLoaded Externals: %ld\n", TRec->ExtCnt);
            if(TRec->ExtCnt>0)
            {
                fprintf(fp,"\t\tIdx\tName\tAddress\n");
                for(std::vector<Record_t>::const_iterator R
                    =TRec->Externals.begin();R!=TRec->Externals.end();R++)
                {
                    fprintf(fp,"\t\t%ld\t%s\t%" SYMA_FMT "\n",
                            static_cast<long>(R-TRec->Externals.begin()),
                            R->Name.c_str(), R->Address);
                }
                fprintf(fp,"\n");
            }


            // Dump loaded Devices
            fprintf(fp,"\tLoaded Devices: %ld\n", TRec->DevCnt);
            if(TRec->DevCnt>0)
            {
                fprintf(fp,"\t\tIdx\tName\tAddress\t\tSupervisor\n");
                for(std::vector<Record_t>::const_iterator R
                    =TRec->Devices.begin();R!=TRec->Devices.end();R++)
                {
                    fprintf(fp,"\t\t%ld\t%s\t%" SYMA_FMT "\t%" SYMA_FMT "\n",
                            static_cast<long>(R-TRec->Devices.begin()),
                            R->Name.c_str(), R->Address, R->Supervisor);
                }
            }
        }
    }

    fprintf(fp,"AddressBook---------------------------------\n");
    fflush(fp);
}
} /* namespace AddressBookNS */
