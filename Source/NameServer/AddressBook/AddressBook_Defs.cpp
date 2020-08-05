#include "AddressBook_Defs.hpp"

namespace AddressBook
{

// const SymAddr_t INVALID_ADDRESS;
  
int DeviceData_t::size() const
{
    int size = 0;
    
    size += sizeof(SymAddr_t) * 2;
    size += Name.size();
    
    return size;
}


DevTypeRecord_t::DevTypeRecord_t(std::string N)
{
    Name = N;
}

int DevTypeRecord_t::size() const
{
    int size = 0;
    
    size += Name.size();
    size += sizeof(MsgIdx) * InMsgs.size();
    size += sizeof(MsgIdx) * OuMsgs.size();
    
    return size;
}


MsgTypeRecord_t::MsgTypeRecord_t(std::string N)
{
    Name = N;
}

int MsgTypeRecord_t::size() const
{
    int size = 0;
    
    size += Name.size();
    size += sizeof(MsgIdx) * Inputs.size();
    size += sizeof(MsgIdx) * Outputs.size();
    
    return size;
}



} /* namespace AddressBook */
