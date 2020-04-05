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

bool Mothership::decode_addresses_message(PMsg_p* message,
                                          std::vector<uint32_t>* addresses,
                                          unsigned index)
{
    int countBuffer;
    std::vector<uint32_t>* addressesBuffer;

    addresses->clear();

    addressesBuffer = message->Get<std::vector<uint32_t> >(index, countBuffer);
    if (addressesBuffer == PNULL)
    {
        Post(422, hex2str(message->Key()), uint2str(index));
        return false;
    }

    /* Copy the addresses from the buffer to the input argument. */
    for (std::vector<uint32_t>::iterator packetIt=addressesBuffer->begin();
         packetIt!=addressesBuffer->end(); packetIt++)
    {
        addresses->push_back(*packetIt);
    }

    return true;
}

bool Mothership::decode_addressed_packets_message(PMsg_p* message,
    std::vector<std::pair<uint32_t, P_Pkt_t> >* packets, unsigned index)
{
    int countBuffer;
    std::vector<std::pair<uint32_t, P_Pkt_t> >* packetsBuffer;
    std::vector<std::pair<uint32_t, P_Pkt_t> >::iterator packetIt;

    packets->clear();

    packetsBuffer = message->Get<std::vector<std::pair<uint32_t, P_Pkt_t> > >
        (index, countBuffer);
    if (packetsBuffer == PNULL)
    {
        Post(416, hex2str(message->Key()), uint2str(index));
        return false;
    }

    /* Copy the packets from the buffer to the input argument. */
    for (packetIt = packetsBuffer->begin(); packetIt != packetsBuffer->end();
         packetIt++)
    {
        packets->push_back(*packetIt);
    }

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
        Post(419, hex2str(message->Key()), uint2str(index));
        return false;
    }
    *result = *resultBuffer;
    return true;
}

bool Mothership::decode_packets_message(PMsg_p* message,
                                        std::vector<P_Pkt_t>* packets,
                                        unsigned index)
{
    int countBuffer;
    std::vector<P_Pkt_t>* packetsBuffer;

    packets->clear();

    packetsBuffer = message->Get<std::vector<P_Pkt_t> >(index, countBuffer);
    if (packetsBuffer == PNULL)
    {
        Post(406, hex2str(message->Key()), uint2str(index));
        return false;
    }

    /* Copy the packets from the buffer to the input argument. */
    for (std::vector<P_Pkt_t>::iterator packetIt=packetsBuffer->begin();
         packetIt!=packetsBuffer->end(); packetIt++)
    {
        packets->push_back(*packetIt);
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
        Post(404, hex2str(message->Key()), uint2str(index));
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
        Post(405, hex2str(message->Key()), uint2str(index));
        return false;
    }
    *result = *resultBuffer;
    return true;
}
