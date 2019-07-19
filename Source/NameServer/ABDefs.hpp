#ifndef ABDefs_H
#define ABDefs_H

#include "OSFixes.hpp"

#include <stdio.h>
#include <cstddef>
#include <string>
#include <vector>

namespace AddressBookNS
{

//----------------------------------------
// Temporary typedefs
typedef uint64_t    SymAddr_t;     // A 64-bit full-symbolic address

static const SymAddr_t INVALID_ADDRESS = UINT64_MAX; 
//typedef unsigned long    SymAddr_t;     // A 64-bit full-symbolic address

//----------------------------------------

enum TaskState_t { Loaded = 0, Linked, Built, Deployed, Init, Running, Finished, Unknown };

typedef unsigned DTypeIdx;
typedef int AttrIdx;
typedef unsigned MsgIdx;


struct DeviceData_t {
    int size() const;           // Get the size of the DeviceData
    
    SymAddr_t Address;          // Device's full sym address.
    SymAddr_t Supervisor;       // Full sym address of the Device's Supervisor.
    std::string Name;           // Device's cnonical name.
};


struct DevTypeRecord_t {             
    DevTypeRecord_t(std::string N = "");
    int size() const;               // Get the size of the DeviceType Record
    
    std::string Name;               // Device Type cnonical name.
    std::vector<MsgIdx> InMsgs;     // Vector of Input Message indexes
    std::vector<MsgIdx> OuMsgs;     // Vector of Output Message indexes
};



struct MsgTypeRecord_t {
    MsgTypeRecord_t(std::string N = "");
    int size() const;               // Get the size of the MsgType Record
    
    std::string Name;               // Message Type Name
    std::vector<MsgIdx> Inputs;     // Indicies of DeviceTypes that have this MessageType as an input
    std::vector<MsgIdx> Outputs;    // Indicies of DeviceTypes that have this MessageType as an output
};
    
} /* namespace AddressBookNS */
#endif /* ABDefs_H */
