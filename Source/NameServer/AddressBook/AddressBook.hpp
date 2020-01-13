/*==============================================================================
 *  AddressBook: A back-end implementation for the POETS sbase & Nameserver
 *
 *  Tasks can be *maped* and *linked*:
 *      Mapped: Device Records connected to Device (and Message) and Attribute
 *              types. Device Records with External Connections also listed.
 *      Linked: Device Records connected to Addresses in Address Map and listed
 *              in the relevant Supervisor vector.
 *
 *      Linkage is invalidated when/if a Device Record is updated after load.
 *      Mapping is invalidated if more devices than expected are loaded.
 *
 *  Struct sizing with no Pragma Packing:
 *                      Win 64-Bit  Win 32-Bit  Mac 64-Bit
 *      TaskRecord_t           560         324         464
 *      Record_t                72          64          52
 *      TaskData_t             280         184         216
 *      DeviceData_t            56          48          40
 *
 *      Memory Footprint*  221.7MB     153.2MB       194MB
 *           *1 task with 1 million devices, 1 supervisor and 1 external.
 *
 *
 *
 *============================================================================*/

#ifndef AddressBook_H
#define AddressBook_H

#include "OSFixes.hpp"

#include "AddressBook_Defs.hpp"
#include "AddressBook_Task.hpp"

#include <stdio.h>
#include <cstddef>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <map>



#define MAXSUPERVISORS 16

//==============================================================================
namespace AddressBookNS // changed name to avoid conflicts with the class itself
                        // 14 July 2019 ADR
{
typedef std::map<std::string, TaskRecord_t*> TaskMap_t;

enum ReturnCode_t { ERR_BAD_COUNT = -1, 
                    SUCCESS = 0,
                    ERR_NONFATAL,
                    ERR_INVALID_TASK,
                    ERR_INVALID_DEVTYPE,
                    ERR_INVALID_DEVICE,
                    ERR_INVALID_MESSAGE_TYPE,
                    ERR_INVALID_ATTRIBUTE,
                    ERR_DEVICE_DATA_MISMATCH,
                    ERR_TASKNAME_USED,
                    ERR_DEVICENAME_USED,
                    ERR_DEVICE_ADDR_USED,
                    ERR_TASK_NOT_FOUND,
                    ERR_DEVICE_NOT_FOUND,
                    ERR_INVALID_MAP,
                    ERR_INVALID_SUPERVISOR,
                    ERR_INVALID_STATE
                  };

class AddressBook
{
public:
    AddressBook(std::string d);
    ~AddressBook();

    unsigned GetTaskCount(void);

    // Task Manipulation
    unsigned AddTask(const std::string &TaskName, TaskData_t &Data);
    unsigned ListTask(std::vector<std::string> &Tasks);
    unsigned GetTask(const std::string &TaskName, TaskData_t &Task);
    unsigned DelTask(const std::string &TaskName);

    unsigned AddDeviceType(const std::string &TaskName, const DevTypeRecord_t &DeviceType); // add a new DeviceType
    unsigned ClearTask(const std::string &TaskName); // deletes all the devices for a task without removing the task.

    TaskState_t TaskState(std::string &TaskName);                   // Get the task state
    unsigned TaskState(std::string &TaskName, TaskState_t State);   // Set the task state

    std::string TaskExecPath(std::string &TaskName);                     // Get the path of a task
    unsigned TaskExecPath(std::string &TaskName, std::string &NewPath);  // Set the path of a task

    bool TaskValid(std::string &TaskName);
    bool TaskMapValid(std::string &TaskName);
    bool TaskLinkValid(std::string &TaskName);

    unsigned RebuildTask(std::string &TaskName);
    unsigned BuildMaps(std::string &TaskName);
    unsigned BuildLink(std::string &TaskName);

    // Device Manipulation
    unsigned AddDevice(std::string &TaskName, Record_t &Device, bool Validate = true);
    unsigned UpdateDevice(std::string &TaskName, DeviceData_t &Data);

    // Find a device
    unsigned FindDevice(std::string &TaskName, std::string &Name, const Record_t* &DRec);
    unsigned FindDevice(std::string &TaskName, SymAddr_t Address, const Record_t* &DRec);

    // Find all devices supervised by X
    unsigned FindBySuper(std::string &TaskName, SymAddr_t Supervisor, const RecordVect_t* &Records);

    unsigned FindByType(std::string &TaskName, std::string Type, const RecordVect_t* &Records);
    unsigned FindByAttribute(std::string &TaskName, std::string Attribute, const RecordVect_t* &Records);

    unsigned FindByInMsg(std::string &TaskName, std::string Msg, const RecordVect_t* &Records);            //TODO
    unsigned FindByOuMsg(std::string &TaskName, std::string Msg, const RecordVect_t* &Records);            //TODO

    // device getters. Note that a vector<Record_t> is different from a RecordVect_t,
    // which is a vector<const Record_t*>
    unsigned GetDevices(std::string &TaskName, const std::vector<Record_t>* &Records);
    unsigned GetExternals(std::string &TaskName, const std::vector<Record_t>* &Records);
    unsigned GetSupervisors(std::string &TaskName, const std::vector<Record_t>* &Records);
    unsigned GetExtCon(std::string &TaskName, const RecordVect_t* &Records);

    // Device Count getters
    long GetDeviceCount(std::string &TaskName);
    long GetLoadedDeviceCount(std::string &TaskName);
    long GetExternalCount(std::string &TaskName);
    long GetLoadedExternalCount(std::string &TaskName);
    int GetSupervisorCount(std::string &TaskName);
    int GetMappedSupervisorCount(std::string &TaskName);

    // Integrity
    unsigned IntegCheck(void);                  // Integrity check of ALL tasks
    unsigned IntegCheck(std::vector<unsigned> &TaskIntegs);
    unsigned IntegTask(std::string &TaskName, bool Verbose = false, FILE * = stdout);  // Ingerity Check a Task by name
  //unsigned IntegTask(const TaskRecord_t *TRec, bool Verbose = false, FILE * = stdout);     // Ingerity Check a Task by pointer

    // Debugging
    void Dump(std::string &Name);
    void Dump(FILE * = stdout, std::string Name = "");

private:
    TaskMap_t TaskMap;
    int TaskCount;
    std::string ABderived; // changed to allow multiple inheritance (from CommonBase) 14 July 2019 ADR

    TaskRecord_t * FindTask(const std::string &TaskName);

    unsigned ValidateDevice(TaskRecord_t *TRec, Record_t &DevRec);
    unsigned ValidateDeviceType(const DevTypeRecord_t& DevTypRec, unsigned MaxIdx);
    unsigned ValidateTask(TaskData_t &Data);

    unsigned CheckVector(TaskRecord_t *TRec, std::vector<Record_t> &Vect);

    unsigned LinkDevice(TaskRecord_t *TRec, Record_t *DRec); // Add Device to name map & supervisor list.
    unsigned MapDevice(TaskRecord_t *TRec, Record_t *DRec);  // Add Device to other maps & lists.

    unsigned ClearMap(TaskRecord_t *TRec);
    unsigned ClearLink(TaskRecord_t *TRec);

};
} /* namespace AddressBookNS */

#endif /* AddressBook_H */
