#ifndef AddressBook_Task_H
#define AddressBook_Task_H

#include "AddressBook_Defs.hpp"
#include "AddressBook_Record.hpp"

#include <iterator>
#include <algorithm>
#include <map>

#if (__cplusplus >= 201103) && (\
    (defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)) ||\
    defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)))
 //C++11+ and C11+ or POSIX: 
 //     fprintf is thread safe so let's thread stuff!
 #include <thread>
 #include <atomic>
 #define AB_THREADING

 // We need Atomic types if we are using threading.
 #define ABUNSIGNED std::atomic<unsigned> 
 #define ABULONG    std::atomic<unsigned long> 
#else
 #define ABUNSIGNED unsigned
 #define ABULONG    unsigned long

#endif


#define RECOVERABLEINTEGRITY

namespace AddressBookNS
{

typedef std::map<std::string, unsigned> IdxMap_t;
typedef std::vector<const Record_t*> RecordVect_t;
typedef std::map<SymAddr_t, RecordVect_t > SupMap_t;
typedef std::map<std::string, RecordVect_t > RecordMap_t;
typedef std::map<SymAddr_t, const Record_t*> AddrMap_t;
typedef std::map<std::string, Record_t*> NameMap_t;

typedef std::pair<DevTypeRecord_t, RecordVect_t> DevTypePair;
typedef std::pair<std::string, RecordVect_t> AttrTypePair;

struct IntegVals_t {
    ABUNSIGNED ret;
    ABUNSIGNED retT;
    ABUNSIGNED retL;
    ABUNSIGNED retM;
};

struct TaskData_t {
	TaskData_t();
    TaskData_t(std::string &N, std::string &P, std::string &X, std::string &E,
                TaskState_t S, unsigned long DC, unsigned long EC);
                
    int size() const;                   // Get the size (in bytes) of the TaskData

    std::string Name; // = "";
    std::string Path; // = "";
    std::string XML; // = "";
    std::string ExecutablePath; // = "";
    TaskState_t State; // = Loaded;
    unsigned long DeviceCount; // = 0;          // The maximum number of devices in task
    unsigned long DeviceCountLd; // = 0;        // The number of devices currently loaded
    unsigned long ExternalCount; // = 0;        // The maximum number of externals in task
    unsigned long ExternalCountLd; // = 0;      // The number of externals currently loaded
    unsigned long SupervisorCount; // = 0;      // The number of allocated supervisors
    std::vector<DevTypeRecord_t> DeviceTypes;
    std::vector<std::string> MessageTypes;
    std::vector<std::string> AttributeTypes;
};


class TaskRecord_t {
public:
	TaskRecord_t();
    TaskRecord_t(std::string &N, std::string &P, std::string &X, std::string &E,
                TaskState_t S, unsigned long DC, unsigned long EC);
    TaskRecord_t(TaskData_t &Data);

    unsigned Integrity(bool Verbose = false, FILE * = stdout);
    
    int size() const;             // Get the size (in bytes) of the TaskData


    std::string Name; // = "";
    std::string Path; // = "";
    std::string XML; // = "";
    std::string ExecPath; // = "";
    unsigned long DevCntMax; // = 0;             // The expected number of devices
    unsigned long ExtCntMax; // = 0;             // The expected number of external devices
    unsigned long DevCnt; // = 0;                // The loaded number of Devices
    unsigned long SupCnt; // = 0;                // The loaded number of Supervisors
    unsigned long ExtCnt; // = 0;                // The loaded number of External Devices

    TaskState_t State; // = Loaded;           //State of the task

    std::vector<Record_t> Devices;        // List of Devices in the task
    std::vector<Record_t> Externals;      // List of Externals in the task
    std::vector<Record_t> Supervisors;    // List of Supervisors in the task

    std::vector<MsgTypeRecord_t> MsgTypes;  // List of task Msg types from XML
  //std::vector<MsgTypePair> MsgTypes;    // List of task Msg types from XML
    std::vector<DevTypePair> DevTypes;     // List of task Dev Types from XML
    std::vector<AttrTypePair> AttrTypes;   // List of task Attrs from XML

    RecordVect_t ExtCon;        // List of Devices with external conns

    // Task String Data Maps
    IdxMap_t MsgTypeMap;  // Map Msg type string to Idx
    IdxMap_t DevTypeMap;  // Map Dev type Name to Idx
    IdxMap_t AttrTypeMap; // Map Attr type string to Idx

    //Device Maps
    SupMap_t SupMap;       // Supervisor>DeviceAddress map
    AddrMap_t AddrMap;     // Addr>record map basis of DNAME, DGRP, SNAME
    NameMap_t NameMap;     // Name>record map basis of DNAME, DGRP, SNAME

    bool TaskValid; // = true;          // Indicate whether the task string info is valid
    bool MapValid; // = true;           // Indicate whether the task maps are valid
    bool LinkValid; // = true;          // Indicate whether the Device > Address links are valid

private:
    void IntegDevices(bool Verbose, FILE * fp, unsigned long DStart, 
                        unsigned long DEnd, IntegVals_t &retVal, 
                        ABULONG &ExtConCnt);

    void IntegExternals(bool Verbose, FILE * fp, unsigned long EStart, 
                        unsigned long EEnd, IntegVals_t &retVal);

    void IntegSupervisors(bool Verbose, FILE * fp, unsigned long SStart, 
                        unsigned long SEnd, IntegVals_t &retVal, 
                        ABUNSIGNED &MappedSupervisors);

    void IntegDevTypeMap(bool Verbose, FILE * fp, unsigned long DTStart, 
                        unsigned long DTEnd, IntegVals_t &retVal);
};


} /* namespace AddressBookNS */
#endif /* AddressBook_Task_H */
