#ifndef ABRecord_H
#define ABRecord_H

#include "ABDefs.hpp"

namespace AddressBookNS
{
   enum RecordType_t { Device = 0, DeviceExt, External, Supervisor };

   // data contents factored out for serialisation 17 July 2019 ADR  
   struct RecordData_t
   {
       public:
     
       SymAddr_t Address;          // Device's full sym address.
       union
       {
           SymAddr_t Supervisor;   // Full sym address of the Device's Supervisor.
           unsigned long Rank;	   // OR Supervisor's MPI Rank.
       };
       DTypeIdx DeviceType;        // Index of the Device's type in task.
       AttrIdx Attribute;          // Index of the Device's attribute in task.
       RecordType_t RecordType;    // Class of device represented by the record.     
   };

   class Record_t : public RecordData_t
   {
        public:
     
	Record_t();
        Record_t(std::string, SymAddr_t, uint64_t, DTypeIdx, RecordType_t = Device, AttrIdx = -1);
    
        int size() const;           // Size of the record    

        std::string Name;           // Device's canonical name. 
   };

} /* namespace AddressBookNS */
#endif /* ABRecord_H */
