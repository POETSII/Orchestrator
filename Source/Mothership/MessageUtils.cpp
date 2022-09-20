/* This source file defines the private Mothership methods for decoding MPI
 * messages (NB: not CommonBase::Decode). If there is an error decoding the
 * message (i.e. if one of the fields of the message does not decode
 * correctly), the error is Post-ed.
 *
 * Input arguments that aren't the PMsg_p pointer are cleared or set to zero
 * before reading from the message.
 *
 * These methods all return true if there was no error, and false otherwise. */

#include "Mothership.h"

bool Mothership::decode_app_dist_message(
    PMsg_p* message, std::string* appName, std::string* codePath,
    std::string* dataPath, uint32_t* coreAddr,
    std::vector<uint32_t>* threadsExpected)
{
    codePath->clear();
    dataPath->clear();
    *coreAddr = 0;
    threadsExpected->clear();
    if(!decode_string_message(message, appName)) return false;
    if(!decode_string_message(message, codePath, 1)) return false;
    if(!decode_string_message(message, dataPath, 2)) return false;
    if(!decode_unsigned_message(message, coreAddr, 3)) return false;
    if(!decode_addresses_message(message, threadsExpected, 4)) return false;
    return true;
}

bool Mothership::decode_app_supd_message(PMsg_p* message, std::string* appName,
                                         std::string* soPath)
{
    soPath->clear();
    if(!decode_string_message(message, appName)) return false;
    if(!decode_string_message(message, soPath, 1)) return false;
    return true;
}

bool Mothership::decode_app_spec_message(PMsg_p* message, std::string* appName,
                                         uint32_t* distCount,
                                         uint8_t* appNumber)
{
    *distCount = 0;
    if(!decode_string_message(message, appName)) return false;
    if(!decode_unsigned_message(message, distCount, 1)) return false;
    if(!decode_char_message(message, appNumber, 2)) return false;
    return true;
}

bool Mothership::decode_moni_devi_req_message(
    PMsg_p* message, std::string* ackMsg, unsigned* updatePeriod,
    unsigned* dataType, unsigned* source, bool* exfiltrationControl,
    int* hwAddr)
{
    *ackMsg = message->Zname(2);  /* Eh */
    *updatePeriod = 0;
    *dataType = 0;
    *source = 0;
    *exfiltrationControl = false;
    *hwAddr = 0;
    if(!decode_unsigned_message(message, updatePeriod, 0)) return false;
    if(!decode_unsigned_message(message, dataType, 1)) return false;
    if(!decode_unsigned_message(message, source, 4)) return false;
    if(!decode_bool_message(message, exfiltrationControl, 0)) return false;
    if(!decode_int_message(message, hwAddr, 1)) return false;
    return true;
}

bool Mothership::decode_addresses_message(PMsg_p* message,
                                          std::vector<uint32_t>* addresses,
                                          unsigned index)
{
    std::vector<uint32_t> addressesBuffer;
    std::vector<uint32_t>::iterator addressIt;

    addresses->clear();

    message->Get<uint32_t>(index, addressesBuffer);
    if (addressesBuffer.empty())
    {
        Post(522, hex2str(message->Key()), uint2str(index));
        return false;
    }

    /* Copy the addresses from the buffer to the input argument. */
    for (addressIt = addressesBuffer.begin();
         addressIt != addressesBuffer.end(); addressIt++)
        addresses->push_back(*addressIt);

    return true;
}

bool Mothership::decode_addressed_packets_message(PMsg_p* message,
    std::vector<P_Addr_Pkt_t>* packets, unsigned index)
{
    message->Put<P_Addr_Pkt_t>();  // Tell the message its type

    packets->clear();

    message->Get<P_Addr_Pkt_t>(index, *packets);

    if (packets->empty())
    {
        Post(516, hex2str(message->Key()), uint2str(index));
        return false;
    }

    return true;
}

bool Mothership::decode_bool_message(PMsg_p* message, bool* result,
                                     unsigned index)
{
    int countBuffer;
    bool* resultBuffer;

    /* Get and check for errors. */
    resultBuffer = message->Get<bool>(index, countBuffer);
    if (resultBuffer == PNULL)
    {
        *result = 0;
        Post(505, hex2str(message->Key()), uint2str(index));
        return false;
    }
    *result = *resultBuffer;
    return true;
}

bool Mothership::decode_char_message(PMsg_p* message, unsigned char* result,
                                     unsigned index)
{
    int countBuffer;
    unsigned char* resultBuffer;

    /* Get and check for errors. */
    resultBuffer = message->Get<unsigned char>(index, countBuffer);
    if (resultBuffer == PNULL)
    {
        *result = 0;
        Post(519, hex2str(message->Key()), uint2str(index));
        return false;
    }
    *result = *resultBuffer;
    return true;
}

bool Mothership::decode_int_message(PMsg_p* message, int* result,
                                    unsigned index)
{
    int countBuffer;
    int* resultBuffer;

    /* Get and check for errors. */
    resultBuffer = message->Get<int>(index, countBuffer);
    if (resultBuffer == PNULL)
    {
        *result = 0;
        Post(505, hex2str(message->Key()), uint2str(index));
        return false;
    }
    *result = *resultBuffer;
    return true;
}

bool Mothership::decode_packets_message(PMsg_p* message,
                                        std::vector<P_Pkt_t>* packets,
                                        unsigned index)
{
    /* Before interacting with the message, we need to "teach" it to recognise
     * the P_Pkt_t type. See the Msg_p copy constructor for more information -
     * note that the copy constructor is invoked when the message is staged in
     * a queue! */
    message->Put<P_Pkt_t>();

    packets->clear();

    message->Get<P_Pkt_t>(index, *packets);

    // If the packet vector has come back empty, there is an error.
    if (packets->empty())
    {
        Post(506, hex2str(message->Key()), uint2str(index));
        return false;
    }

    return true;
}

bool Mothership::decode_string_message(PMsg_p* message, std::string* result,
                                       unsigned index)
{
    result->clear();

    /* Get and check for errors. */
    message->Get(index, *result);
    if (result->empty())
    {
        Post(504, hex2str(message->Key()), uint2str(index));
        return false;
    }
    return true;
}

bool Mothership::decode_unsigned_message(PMsg_p* message, unsigned* result,
                                         unsigned index)
{
    int countBuffer;
    unsigned* resultBuffer;

    /* Get and check for errors. */
    resultBuffer = message->Get<unsigned>(index, countBuffer);
    if (resultBuffer == PNULL)
    {
        *result = 0;
        Post(505, hex2str(message->Key()), uint2str(index));
        return false;
    }
    *result = *resultBuffer;
    return true;
}
