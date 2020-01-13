#include "AddressBook_Record.hpp"

namespace AddressBookNS
{

/*==============================================================================
 * Record_t::Record_t(): Default initialiser for a Record_t.
 *============================================================================*/
Record_t::Record_t()
{
    Attribute = -1;     // Set to -1 to ensure that we don't go out of range.
}

/*==============================================================================
 * Record_t::Record_t(): Full initialiser for a Record_t
 *============================================================================*/
Record_t::Record_t(std::string Name, SymAddr_t Addr, uint64_t Super,
		   DTypeIdx Type, RecordType_t RT, AttrIdx Attr)
		   : Name(Name)
{
       Address = Addr;
       DeviceType = Type;
       RecordType = RT;
       Attribute = Attr;
       Rank = 0;
       Supervisor = 0;
       if (RecordType == Supervisor) Rank = static_cast<unsigned long>(Super);
       else Supervisor = static_cast<SymAddr_t>(Super); 
}


/*==============================================================================
 * Record_t::size(): Return the size (in bytes) of the address record.
 *============================================================================*/
int Record_t::size() const
{
    int size = 0;
    
    size += sizeof(SymAddr_t) * 2;      // Address and Supervisor/Rank
    size += sizeof(DTypeIdx);           // Device Type Index
    size += sizeof(AttrIdx);            // Attribute Index
    size += sizeof(RecordType_t);       // Record Type
    size += sizeof(char) * Name.size(); // Name
    
    return size;
}

    
} /* namespace AddressBookNS */
