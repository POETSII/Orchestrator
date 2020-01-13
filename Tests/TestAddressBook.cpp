/* Tests the AddressBook for functionality and integrity. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "OSFixes.hpp"
#include "AddressBook.hpp"
#include "AddressBook_Defs.hpp"
#include "AddressBook_Task.hpp"

#include <stdio.h>
#include <cstddef>
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <numeric>
#include <iterator>
#include <algorithm>
#include <map>


TEST_CASE("AddressBook Small Tests", "[Simple]")
{
    // Create the AddressBook instance
    AddressBookNS::AddressBook AddrBook
        = AddressBookNS::AddressBook(std::string("AddressBookMain"));
    AddressBookNS::TaskData_t taskData;
    
    //Create a Supervisor devicetype record and add some messagetypes
    AddressBookNS::DevTypeRecord_t SuperDTR("Super");
    SuperDTR.InMsgs.push_back(1);
    SuperDTR.OuMsgs.push_back(2);
    
    //Create a Cell devicetype record and add some messagetytpes
    AddressBookNS::DevTypeRecord_t CellDTR("Cell");
    CellDTR.InMsgs.push_back(0);
    CellDTR.InMsgs.push_back(2);
    CellDTR.OuMsgs.push_back(0);
    CellDTR.OuMsgs.push_back(1);
    
    //Create a Fixed node devicetype record and add some messagetytpes
    AddressBookNS::DevTypeRecord_t FixedDTR("Fixed");
    FixedDTR.InMsgs.push_back(2);
    FixedDTR.OuMsgs.push_back(0);
    FixedDTR.OuMsgs.push_back(1);
    
    
    
    //==========================================================================
    // Add a task (and check that it added correctly)
    //==========================================================================
    std::string t1Name = "Task1";
    AddressBookNS::TaskData_t T1Data;
    
    T1Data.DeviceCount = 5;
    T1Data.ExternalCount = 2;
    T1Data.Path = "/local/extra/Orchestrator/application_staging/xml";
    T1Data.XML = "task1.xml";
    T1Data.ExecutablePath = "~/";
    T1Data.MessageTypes.push_back("Update");
    T1Data.MessageTypes.push_back("Fin");
    T1Data.MessageTypes.push_back("Done");
    
    T1Data.DeviceTypes.push_back(SuperDTR);
    T1Data.DeviceTypes.push_back(CellDTR);
    T1Data.DeviceTypes.push_back(FixedDTR);
    T1Data.AttributeTypes.push_back("Test");
    
    // Add the task to the AddressBook
    int t1Ret = AddrBook.AddTask(t1Name, T1Data);  
    REQUIRE(t1Ret == AddressBookNS::SUCCESS);
    
    SECTION("Check that a task added correctly", "[Simple]")
    {
        REQUIRE(AddrBook.GetTaskCount() == 1);      // We should have one task,
        REQUIRE(AddrBook.IntegCheck() == 0);        // that passes integrity.
        
        // Get the task for running tests on
        t1Ret = AddrBook.GetTask(t1Name, taskData);
        REQUIRE(t1Ret == AddressBookNS::SUCCESS);
        
        REQUIRE(taskData.Name == t1Name);                // Check the task name.
        REQUIRE(taskData.Path == T1Data.Path);          // Check the path string.
        REQUIRE(taskData.XML == T1Data.XML);            // Check the XML string.
        REQUIRE(taskData.ExecutablePath == T1Data.ExecutablePath);   // Check the executable path string.
        
        REQUIRE(taskData.State == AddressBookNS::Loaded); // Check the task state.
        
        REQUIRE(taskData.DeviceTypes.size() == T1Data.DeviceTypes.size());
        REQUIRE(taskData.DeviceTypes.size() == 3);      // with 5 device types,
        REQUIRE(taskData.MessageTypes.size() == 3);     // with 3 message types,
        REQUIRE(taskData.AttributeTypes.size() == 1);   // and 1 attribute type.
        
        REQUIRE(taskData.DeviceCount == T1Data.DeviceCount); // Expecting 10000 devices
        REQUIRE(taskData.DeviceCountLd == 0);           // With none loaded.
        
        REQUIRE(taskData.ExternalCount == T1Data.ExternalCount);   // Expecting 1 external device
        REQUIRE(taskData.ExternalCountLd == 0);         // With none loaded.
        
        REQUIRE(taskData.SupervisorCount == 0);     // Should have no supervisors.
    }
    //==========================================================================
    
    
    
    //==========================================================================
    // Add another task
    //==========================================================================
    std::string t2Name = "Task2";
    AddressBookNS::TaskData_t T2Data;
    
    T2Data.DeviceCount = 5;
    T2Data.ExternalCount = 2;
    T2Data.Path = "/local/extra/Orchestrator/application_staging/xml";
    T2Data.XML = "task2.xml";
    T2Data.ExecutablePath = "~/";
    T2Data.MessageTypes.push_back("Update");
    T2Data.MessageTypes.push_back("Fin");
    T2Data.MessageTypes.push_back("Done");
    
    T2Data.DeviceTypes.push_back(SuperDTR);
    T2Data.DeviceTypes.push_back(CellDTR);
    T2Data.DeviceTypes.push_back(FixedDTR);
    
    // Add the task to the AddressBook
    int t2Ret = AddrBook.AddTask(t2Name, T2Data);  
    REQUIRE(t2Ret == AddressBookNS::SUCCESS);
    
    
    SECTION("Check that a second task added correctly", "[Simple]")
    {
        REQUIRE(AddrBook.GetTaskCount() == 2);      // We should have two tasks,
        REQUIRE(AddrBook.IntegCheck() == 0);        // that pass integrity.
        
        // Get the task for running tests on
        t2Ret = AddrBook.GetTask(t2Name, taskData);
        REQUIRE(t2Ret == AddressBookNS::SUCCESS);
        
        REQUIRE(taskData.Name == t2Name);                // Check the task name.
        REQUIRE(taskData.Path == T2Data.Path);          // Check the path string.
        REQUIRE(taskData.XML == T2Data.XML);            // Check the XML string.
        REQUIRE(taskData.ExecutablePath == T2Data.ExecutablePath);   // Check the executable path string.
        
        REQUIRE(taskData.State == AddressBookNS::Loaded); // Check the task state.
        
        REQUIRE(taskData.DeviceTypes.size() == T2Data.DeviceTypes.size());
        REQUIRE(taskData.DeviceTypes.size() == 3);      // with 5 device types,
        REQUIRE(taskData.MessageTypes.size() == 3);     // with 3 message types,
        REQUIRE(taskData.AttributeTypes.size() == 0);   // and no attribute types.
        
        REQUIRE(taskData.DeviceCount == T2Data.DeviceCount); // Expecting 10000 devices
        REQUIRE(taskData.DeviceCountLd == 0);           // With none loaded.
        
        REQUIRE(taskData.ExternalCount == T2Data.ExternalCount);   // Expecting 1 external device
        REQUIRE(taskData.ExternalCountLd == 0);         // With none loaded.
        
        REQUIRE(taskData.SupervisorCount == 0);     // Should have no supervisors.
    }
    //==========================================================================
    
    
    
    //==========================================================================
    // Check that we can't add a duplicate task
    //==========================================================================
    SECTION("Check that a duplicate task is correctly rejected", "[Simple]")
    {
        int t3Ret = AddrBook.AddTask(t2Name, T2Data);
        REQUIRE(t3Ret == AddressBookNS::ERR_TASKNAME_USED);
        REQUIRE(AddrBook.GetTaskCount() == 2);  // We should have two tasks,
        REQUIRE(AddrBook.IntegCheck() == 0);    // that pass integrity.
    }
    //==========================================================================
    
    
    
    //==========================================================================
    // Delete a task
    //==========================================================================
    SECTION("Check that tasks can be deleted", "[Simple]")
    {
        int delRet = AddrBook.DelTask(t2Name);
        REQUIRE(delRet == AddressBookNS::SUCCESS);
        REQUIRE(AddrBook.GetTaskCount() == 1);  // We should have one task,
        REQUIRE(AddrBook.IntegCheck() == 0);    // that passes integrity.
    }
    //==========================================================================
    
    
    
    SECTION("Check device handling", "[Simple]")
    {
        // Get the task
        AddrBook.GetTask(t1Name, taskData);
        
        // Add a supervisor
        AddressBookNS::Record_t SData1;
        SData1.Name = "";
        SData1.Address = 0xFFFF0001;
        SData1.Rank = 5;
        SData1.DeviceType = 0;
        SData1.RecordType = static_cast<AddressBookNS::RecordType_t>(AddressBookNS::Supervisor);
        try {
            unsigned SAdd = AddrBook.AddDevice(t1Name, SData1);
            if (SAdd > 0) std::cerr << std::endl << "ERROR adding SData1: " << SAdd << std::endl;
            REQUIRE(SAdd == 0);
        } catch (const char* msg) {
            std::cerr << std::endl << "ERROR adding SData1: " << msg << std::endl;
            std::cin.get();
        }
        REQUIRE(AddrBook.GetSupervisorCount(t1Name) == 1);
        
        
        // Add another Supervisor
        AddressBookNS::Record_t SData2;
        SData2.Name = "";
        SData2.Address = 0xFFFF0002;
        SData2.Rank = 5;
        SData2.DeviceType = 0;
        SData2.RecordType = static_cast<AddressBookNS::RecordType_t>(AddressBookNS::Supervisor);
        try {
            unsigned SAdd = AddrBook.AddDevice(t1Name, SData2);
            if (SAdd > 0) std::cerr << std::endl << "ERROR adding SData2: " << SAdd << std::endl;
            REQUIRE(SAdd == 0);
        } catch (const char* msg) {
            std::cerr << std::endl << "ERROR adding SData2: " << msg << std::endl;
            std::cin.get();
        }
        REQUIRE(AddrBook.GetSupervisorCount(t1Name) == 2);
        
        
        // Add an External
        AddressBookNS::Record_t EData1;
        EData1.Name = "E_0,0";
        EData1.Address = 0xFFFFF001;
        EData1.DeviceType = 2;
        EData1.RecordType = static_cast<AddressBookNS::RecordType_t>(AddressBookNS::External);
        try {
            unsigned EAdd = AddrBook.AddDevice(t1Name, EData1);
            if (EAdd > 0) std::cerr << std::endl << "ERROR adding EData2: " << EAdd << std::endl;
            REQUIRE(EAdd == 0);
        } catch (const char* msg) {
            std::cerr << std::endl << "ERROR adding EData1: " << msg << std::endl;
            std::cin.get();
        }
        REQUIRE(AddrBook.GetLoadedExternalCount(t1Name) == 1);
        
        
        // Setup common device data
        AddressBookNS::Record_t DData1;
        DData1.Supervisor = 0xFFFF0001;
        DData1.DeviceType = 1;
        DData1.RecordType = static_cast<AddressBookNS::RecordType_t>(AddressBookNS::Device);
        AddressBookNS::SymAddr_t BaseAddr = 0x00000000;
        
        
        // Add a Device
        DData1.Name = "C_0,0";
        DData1.Address = BaseAddr++;
        try {
            unsigned DAdd = AddrBook.AddDevice(t1Name, DData1);
            if (DAdd > 0) std::cerr << std::endl << "ERROR adding Device"
                        << DData1.Name << " ("<< DData1.Address << "): "
                        << DAdd << std::endl;
            REQUIRE(DAdd == 0);
        } catch (const char* msg) {
            std::cerr << std::endl << "ERROR adding Device "
                      << DData1.Name << " (" << DData1.Address << "): "
                      << msg << std::endl;
            std::cin.get();
        }
        REQUIRE(AddrBook.GetLoadedDeviceCount(t1Name) == 1);
        
        
        // Add another Device
        DData1.Name = "C_0,1";
        DData1.Address = BaseAddr++;
        try {
            unsigned DAdd = AddrBook.AddDevice(t1Name, DData1);
            if (DAdd > 0) std::cerr << std::endl << "ERROR adding Device"
                        << DData1.Name << " ("<< DData1.Address << "): "
                        << DAdd << std::endl;
            REQUIRE(DAdd == 0);
        } catch (const char* msg) {
            std::cerr << std::endl << "ERROR adding Device "
                      << DData1.Name << " (" << DData1.Address << "): "
                      << msg << std::endl;
            std::cin.get();
        }
        REQUIRE(AddrBook.GetLoadedDeviceCount(t1Name) == 2);
        
        
        // Add another Device
        DData1.Name = "C_0,2";
        DData1.Address = BaseAddr++;
        try {
            unsigned DAdd = AddrBook.AddDevice(t1Name, DData1);
            if (DAdd > 0) std::cerr << std::endl << "ERROR adding Device"
                        << DData1.Name << " ("<< DData1.Address << "): "
                        << DAdd << std::endl;
            REQUIRE(DAdd == 0);
        } catch (const char* msg) {
            std::cerr << std::endl << "ERROR adding Device "
                      << DData1.Name << " (" << DData1.Address << "): "
                      << msg << std::endl;
            std::cin.get();
        }
        REQUIRE(AddrBook.GetLoadedDeviceCount(t1Name) == 3);
        
        
        // Add another Device
        DData1.Name = "C_0,3";
        DData1.Address = BaseAddr++;
        try {
            unsigned DAdd = AddrBook.AddDevice(t1Name, DData1);
            if (DAdd > 0) std::cerr << std::endl << "ERROR adding Device"
                        << DData1.Name << " ("<< DData1.Address << "): "
                        << DAdd << std::endl;
            REQUIRE(DAdd == 0);
        } catch (const char* msg) {
            std::cerr << std::endl << "ERROR adding Device "
                      << DData1.Name << " (" << DData1.Address << "): "
                      << msg << std::endl;
            std::cin.get();
        }
        REQUIRE(AddrBook.GetLoadedDeviceCount(t1Name) == 4);
        
        
        // Check duplicate device handling
        int DDupeRet = AddrBook.AddDevice(t1Name, DData1);
        REQUIRE(AddrBook.GetLoadedDeviceCount(t1Name) == 4);      // We should have 4 devices
        REQUIRE(AddrBook.IntegTask(t1Name, false) == 0);    // and pass integrity.
        
        
        // Check that invalid Device types are picked up on
        DData1.DeviceType = 5;
        DData1.Name = "C_0,4";
        DData1.Address = BaseAddr++;
        try {
            unsigned DAdd = AddrBook.AddDevice(t1Name, DData1);
            REQUIRE(DAdd == AddressBookNS::ERR_INVALID_DEVTYPE);
        } catch (const char* msg) {
            std::cerr << std::endl << "ERROR adding Device "
                      << DData1.Name << " (" << DData1.Address << "): "
                      << msg << std::endl;
            std::cin.get();
        }
        REQUIRE(AddrBook.GetLoadedDeviceCount(t1Name) == 4);
        
        
        // Check that the task is valid
        REQUIRE(AddrBook.TaskValid(t1Name) == true);
        REQUIRE(AddrBook.TaskMapValid(t1Name) == true);
        REQUIRE(AddrBook.TaskLinkValid(t1Name) == true);
        
        // Check that the rest of the data structure is valid
        REQUIRE(AddrBook.GetTaskCount() == 2);  // We should have two tasks,
        REQUIRE(AddrBook.IntegCheck() == 0);    // that passes integrity.
        
        
        
        // Add a device that forces another Supervisor
        REQUIRE(AddrBook.GetMappedSupervisorCount(t1Name) == 2);
        DData1.Supervisor = 0xFFFF0005;
        DData1.Name = "C_0,4";
        DData1.DeviceType = 1;
        try {
            unsigned DAdd = AddrBook.AddDevice(t1Name, DData1);
            if (DAdd > 0) std::cerr << std::endl << "ERROR adding Device"
                        << DData1.Name << " ("<< DData1.Address << "): "
                        << DAdd << std::endl;
            REQUIRE(DAdd == 0);
        } catch (const char* msg) {
            std::cerr << std::endl << "ERROR adding Device "
                      << DData1.Name << " (" << DData1.Address << "): "
                      << msg << std::endl;
            std::cin.get();
        }
        REQUIRE(AddrBook.GetLoadedDeviceCount(t1Name) == 5);
        REQUIRE(AddrBook.GetSupervisorCount(t1Name) == 2);          // Only two added Supervisors
        REQUIRE(AddrBook.GetMappedSupervisorCount(t1Name) == 3);    // But 3 in the map.
        REQUIRE(AddrBook.IntegCheck() != 0);    // this fails integrity.
        REQUIRE(AddrBook.TaskLinkValid(t1Name) == false);   // and the link is invalid
        
        // Add the missing Supervisor
        AddressBookNS::Record_t SData3;
        SData3.Name = "";
        SData3.Address = 0xFFFF0005;
        SData3.Rank = 5;
        SData3.DeviceType = 0;
        SData3.RecordType = static_cast<AddressBookNS::RecordType_t>(AddressBookNS::Supervisor);
        try {
            unsigned SAdd = AddrBook.AddDevice(t1Name, SData3);
            if (SAdd > 0) std::cerr << std::endl << "ERROR adding SData3: " << SAdd << std::endl;
            REQUIRE(SAdd == 0);
        } catch (const char* msg) {
            std::cerr << std::endl << "ERROR adding SData3: " << msg << std::endl;
            std::cin.get();
        }
        REQUIRE(AddrBook.GetLoadedDeviceCount(t1Name) == 5);
        REQUIRE(AddrBook.GetSupervisorCount(t1Name) == 3);    
        REQUIRE(AddrBook.GetMappedSupervisorCount(t1Name) == 3);
        REQUIRE(AddrBook.IntegCheck() == 0);    // Should now pass integrity.
#ifdef RECOVERABLEINTEGRITY
        // If RECOVERABLEINTEGRITY is defined, the link should be valid.
        REQUIRE(AddrBook.TaskLinkValid(t1Name) == true);
#else   
        // Otherwise it is invalid and we need to relink.
        REQUIRE(AddrBook.TaskLinkValid(t1Name) == false);
        REQUIRE(AddrBook.BuildLink(t1Name) == AddressBookNS::SUCCESS);
        REQUIRE(AddrBook.TaskLinkValid(t1Name) == true);    // Link should now be valid
#endif
        // Check that the rest of the task is valid
        REQUIRE(AddrBook.TaskValid(t1Name) == true);
        REQUIRE(AddrBook.TaskMapValid(t1Name) == true);
        
        
    }
    
    
    //TODO: Add more checks to:
        //  Check that adding too many devices is handled
        //  Check that adding too many externals is handled
        //  Check that adding too many Supervisors is handled
        
        // Update a device
        
        // Find by Supervisor
        // Find by Attribute
        // Find by Messages
        // Clear Task
        // Task States
            
        // Intentionally mess with message links
        
        // Rebuild Map
        // Rebuild Task
        // Integrity check after futzing

}


TEST_CASE("AddressBook Large Plate Test", "[Simple]")
{
    // Create the AddressBook instance
    AddressBookNS::AddressBook AddrBook
        = AddressBookNS::AddressBook(std::string("AddressBookMain"));
    
    std::string tName = "plate_1000x1000";
    AddressBookNS::TaskData_t taskData;
    
    
    //==========================================================================
    // Create a "fake" 1000x1000 heated plate.
    //==========================================================================
    AddressBookNS::TaskData_t TData;

    TData.DeviceCount = 65536;
    TData.ExternalCount = 1;
    TData.Path = "/local/extra/Orchestrator/application_staging/xml";
    TData.XML = "plate_1000x1000.xml";
    TData.ExecutablePath = "~/";
    TData.MessageTypes.push_back("Update");
    TData.MessageTypes.push_back("Fin");
    TData.MessageTypes.push_back("Done");
    
    //Create a Supervisor devicetype record and add some messagetypes
    AddressBookNS::DevTypeRecord_t SuperDTR("Super");
    SuperDTR.InMsgs.push_back(1);
    SuperDTR.OuMsgs.push_back(2);
    
    //Create a Cell devicetype record and add some messagetytpes
    AddressBookNS::DevTypeRecord_t CellDTR("Cell");
    CellDTR.InMsgs.push_back(0);
    CellDTR.InMsgs.push_back(2);
    CellDTR.OuMsgs.push_back(0);
    CellDTR.OuMsgs.push_back(1);
    
    //Create a Fixed node devicetype record and add some messagetytpes
    AddressBookNS::DevTypeRecord_t FixedDTR("Fixed");
    FixedDTR.InMsgs.push_back(2);
    FixedDTR.OuMsgs.push_back(0);
    FixedDTR.OuMsgs.push_back(1);
    
    //Create a Router devicetype record and add some messagetytpes
    AddressBookNS::DevTypeRecord_t RouterDTR("Router");
    RouterDTR.InMsgs.push_back(1);
    RouterDTR.OuMsgs.push_back(1);
    RouterDTR.OuMsgs.push_back(2);
    
    //Create a Extern devicetype record and add some messagetytpes
    AddressBookNS::DevTypeRecord_t ExternDTR("Extern");
    ExternDTR.InMsgs.push_back(2);
    ExternDTR.OuMsgs.push_back(0);

    // Add all of the device types to the Task Data
    TData.DeviceTypes.push_back(SuperDTR);
    TData.DeviceTypes.push_back(CellDTR);
    TData.DeviceTypes.push_back(FixedDTR);
    TData.DeviceTypes.push_back(RouterDTR);
    TData.DeviceTypes.push_back(ExternDTR);
    
    // Add an attribute
    TData.AttributeTypes.push_back("Test");
    
    // Add the task to the AddressBook
    AddrBook.AddTask(tName, TData);
    //==========================================================================
    
    
    // Get the task for running tests on
    AddrBook.GetTask(tName, taskData);
    
    
    
    
    
    //==========================================================================
    //Check that the task was added and read correctly
    //==========================================================================
    SECTION("Check that we have the right number of devices in the task data", "[Simple]")
    {
        REQUIRE(AddrBook.GetTaskCount() == 1);          // We should have one task,
        
        REQUIRE(taskData.Name == tName);                // Check the task name.
        REQUIRE(taskData.Path == TData.Path);           // Check the path string.
        REQUIRE(taskData.XML == TData.XML);             // Check the XML string.
        REQUIRE(taskData.ExecutablePath == TData.ExecutablePath);   // Check the executable path string.
        
        REQUIRE(taskData.State == AddressBookNS::Loaded); // Check the task state.
        
        REQUIRE(taskData.DeviceTypes.size() == TData.DeviceTypes.size());
        REQUIRE(taskData.DeviceTypes.size() == 5);      // with 5 device types,
        REQUIRE(taskData.MessageTypes.size() == 3);     // with 3 message types,
        REQUIRE(taskData.AttributeTypes.size() == 1);   // and 1 attribute type.
        
        REQUIRE(taskData.DeviceCount == TData.DeviceCount); // Expecting 10000 devices
        REQUIRE(taskData.DeviceCountLd == 0);           // With none loaded.
        
        REQUIRE(taskData.ExternalCount == 1);           // Expecting 1 external device
        REQUIRE(taskData.ExternalCountLd == 0);         // With none loaded.
        
        REQUIRE(taskData.SupervisorCount == 0);     // Should have no supervisors.
    }
    //==========================================================================
    
    
    
    //==========================================================================
    // Add a supervisor
    //==========================================================================
    AddressBookNS::Record_t SData1;
    SData1.Name = "";
    SData1.Address = 0xFFFF0001;
    SData1.Rank = 5;
    SData1.DeviceType = 0;  // SuperDTR
    SData1.RecordType = static_cast<AddressBookNS::RecordType_t>(AddressBookNS::Supervisor);
    
    REQUIRE(taskData.DeviceTypes.size() == 5);      // with 5 device types,

    try {
        unsigned SAdd = AddrBook.AddDevice(tName, SData1);
        if (SAdd > 0) std::cerr << std::endl << "ERROR adding SData1: " << SAdd << std::endl;
        REQUIRE(SAdd == 0);
    } catch (const char* msg) {
        std::cerr << std::endl << "ERROR adding SData1: " << msg << std::endl;
        std::cin.get();
    }
    //==========================================================================
    
    
    //==========================================================================
    // Add two fixed nodes
    //==========================================================================
    AddressBookNS::Record_t FData1;
    FData1.Name = "C_0,0";
    FData1.Address = 0xFFE00000;
    FData1.Supervisor = 0xFFFF0001;
    FData1.DeviceType = 2;
    FData1.RecordType = static_cast<AddressBookNS::RecordType_t>(AddressBookNS::DeviceExt);

    try {
        unsigned FAdd = AddrBook.AddDevice(tName, FData1);
        if (FAdd > 0) std::cerr << std::endl << "ERROR adding FData1: " << FAdd << std::endl;
        REQUIRE(FAdd == 0);
    } catch (const char* msg) {
        std::cerr << std::endl << "ERROR adding FData1: " << msg << std::endl;
        std::cin.get();
    }
    
    
    AddressBookNS::Record_t FData2;
    FData2.Name = "C_255,255";
    FData2.Address = 0xFFE00001;
    FData2.Supervisor = 0xFFFF0001;
    FData2.DeviceType = 2;
    FData2.RecordType = static_cast<AddressBookNS::RecordType_t>(AddressBookNS::DeviceExt);

    try {
        unsigned FAdd = AddrBook.AddDevice(tName, FData2);
        if (FAdd > 0) std::cerr << std::endl << "ERROR adding FData2: " << FAdd << std::endl;
        REQUIRE(FAdd == 0);
    } catch (const char* msg) {
        std::cerr << std::endl << "ERROR adding FData2: " << msg << std::endl;
        std::cin.get();
    }
    //==========================================================================
    
    
    //==========================================================================
    // Add an External
    //==========================================================================
    AddressBookNS::Record_t EData1;
    EData1.Name = "E_0,0";
    EData1.Address = 0xFFFFF001;
    EData1.DeviceType = 4;
    EData1.RecordType = static_cast<AddressBookNS::RecordType_t>(AddressBookNS::External);

    try {
        unsigned EAdd = AddrBook.AddDevice(tName, EData1);
        if (EAdd > 0) std::cerr << std::endl << "ERROR adding EData2: " << EAdd << std::endl;
        REQUIRE(EAdd == 0);
    } catch (const char* msg) {
        std::cerr << std::endl << "ERROR adding EData1: " << msg << std::endl;
        std::cin.get();
    }
    //==========================================================================
    
    
    //==========================================================================
    // Add 999998 Cells
    //==========================================================================
    AddressBookNS::SymAddr_t BaseAddr = 0x00000000;

    AddressBookNS::Record_t DData1;
    DData1.Supervisor = 0xFFFF0001;
    DData1.DeviceType = 1;
    DData1.RecordType = static_cast<AddressBookNS::RecordType_t>(AddressBookNS::Device);

    for (long i = 0; i < 256; i++)
    {
        //if(i%100 == 0) std::cout << "\t" << i*1000 << "..." << std::endl;
        for (long j = 0; j < 256; j++)
        {
            if (!(i == 0 && j == 0) && !(i == 255 && j == 255))
            {
                DData1.Name = "C_" + TO_STRING(i) + "," + TO_STRING(j);
                DData1.Address = BaseAddr++;

                try{
                    unsigned DAdd = AddrBook.AddDevice(tName, DData1);
                        if (DAdd > 0) std::cerr << std::endl << "ERROR adding Device"
                                                << DData1.Name << " ("
                                                << DData1.Address << "): "
                                                << DAdd << std::endl;
                } catch (const char* msg) {
                        std::cerr << std::endl << "ERROR adding Device "
                                                << DData1.Name << " ("
                                                << DData1.Address << "): "
                                                << msg << std::endl;
                        std::cin.get();
                }
            }
        }
    }    
    //==========================================================================
    
    
    // Get the task for running tests on
    AddrBook.GetTask(tName, taskData);
    
    //==========================================================================
    //Check that the task hasn't been altered by adding devices
    //==========================================================================
    SECTION("Check that we have the right number of devices in the task data after adding", "[Simple]")
    {
        REQUIRE(AddrBook.GetTaskCount() == 1);          // We should have one task,
    
        REQUIRE(taskData.Name == tName);                // Check the task name.
        REQUIRE(taskData.Path == TData.Path);           // Check the path string.
        REQUIRE(taskData.XML == TData.XML);             // Check the XML string.
        REQUIRE(taskData.ExecutablePath == TData.ExecutablePath);   // Check the executable path string.
    
        REQUIRE(taskData.State == AddressBookNS::Loaded);   // Check the task state.
        
        //REQUIRE(taskData.DeviceTypes.size() == TData.DeviceTypes.size());
        REQUIRE(taskData.DeviceTypes.size() == 5);      // with 5 device types,
        REQUIRE(taskData.MessageTypes.size() == 3);     // with 3 message types,
        REQUIRE(taskData.AttributeTypes.size() == 1);   // and 1 attribute type.
    
        REQUIRE(taskData.DeviceCount == TData.DeviceCount);    // Expecting 10000 devices
        REQUIRE(taskData.DeviceCountLd == TData.DeviceCount);  // With all loaded.
    
        REQUIRE(taskData.ExternalCount == 1);           // Expecting 1 external device
        REQUIRE(taskData.ExternalCountLd == 1);         // With 1 loaded.
    
        REQUIRE(taskData.SupervisorCount == 1);         // Should have one supervisors.
    
    
    }
    //==========================================================================
    
    
    //==========================================================================
    // Search for devices
    //==========================================================================
    SECTION("Check that devices added as expected and can be found by name and address", "[Simple]")
    {
        std::string DName = "C_75,199";                  // Name of a device we are going to search for
        AddressBookNS::SymAddr_t DNameAddr = 0x4BC6;    // and its address
    
        std::string DAddrName = "C_1,119";               // and its name    
        AddressBookNS::SymAddr_t DAddr = 0x0176;        // Address to search for
        
        const AddressBookNS::Record_t * DeviceRecord;     // pointer to a const-qualified Device record
        
        // Find device by name
        int a = AddrBook.FindDevice(tName, DName, DeviceRecord);
        REQUIRE(a == 0);                                // Found the device?
        if(a == 0)
        {
            REQUIRE(DName == DeviceRecord->Name);           // Check the name
            REQUIRE(DNameAddr == DeviceRecord->Address);    // Check the address
            REQUIRE(taskData.DeviceTypes[DeviceRecord->DeviceType].Name == "Cell"); // Check the type
        }
        
        // Find device by address
        int b = AddrBook.FindDevice(tName, DAddr, DeviceRecord);
        REQUIRE(b == 0);                                // Found the device?
        if(b == 0)
        {
            REQUIRE(DAddrName == DeviceRecord->Name);       // Check the name
            REQUIRE(DAddr == DeviceRecord->Address);        // Check the address
            REQUIRE(taskData.DeviceTypes[DeviceRecord->DeviceType].Name == "Cell"); // Check the type
        }
    }
    
    SECTION("Integrity Check - this will take some time", "[Simple]")
    {
        int result = AddrBook.IntegTask(tName, false);
        REQUIRE(result == 0);
    }
}
