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
    std::string* dataPath, uint32_t* coreAddr, unsigned* numThreads)
{
    codePath->clear();
    dataPath->clear();
    *coreAddr = 0;
    *numThreads = 0;
    if(!decode_string_message(message, appName)) return false;
    if(!decode_string_message(message, codePath, 1)) return false;
    if(!decode_string_message(message, dataPath, 2)) return false;
    if(!decode_unsigned_message(message, coreAddr, 3)) return false;
    if(!decode_unsigned_message(message, numThreads, 4)) return false;
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
                                         uint32_t* distCount)
{
    *distCount = 0;
    if(!decode_string_message(message, appName)) return false;
    if(!decode_unsigned_message(message, distCount, 1)) return false;
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
        Post(404, uint2str(message->Key()), uint2str(index));
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
        Post(405, uint2str(message->Key()), uint2str(index));
        return false;
    }
    *result = *resultBuffer;
    return true;
}
