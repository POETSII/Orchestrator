#include "MonServerSpy.h"

MonServerSpy::MonServerSpy()
{
    dumpMap = PNULL;
    std::string dumpMapDate = "";
    dumpMapIndex = 0;
    error = "No stored error (yet).";
    isSpyDirNew = true;
    isSpyEnabled = false;
    spyDir = "";
}


MonServerSpy::~MonServerSpy()
{
    CheckCloseDumpMap();
}

/* Safely closes the file at `dumpMap`, if it's open. */
void MonServerSpy::CheckCloseDumpMap()
{
    if (dumpMap != PNULL)
    {
        fclose(dumpMap);
        dumpMap = PNULL;
    }
}

/* And here are a family of dumping methods, which each write some
 * context-sensitive stuff to the file at `out`. */
void MonServerSpy::DumpCommonBits(FILE* out, PMsg_p* message)
{
    int count;

    /* Timestamps */
    fprintf(out, "[timestamps]\n");
    int timeStampIndexes[] = {-1, -2, -3, -4, -10, -20, -30, -40};
    int timeStampIndexLen = 8;
    for (int index = 0; index < timeStampIndexLen; index++)
    {
        double* doubleData = message->Get<double>(
            timeStampIndexes[index], count);
        fprintf(out, "%d=%s\n", timeStampIndexes[index],
                count == 0 ? "\"undefined\"" : dbl2str(*doubleData).c_str());
    }

    /* Addresses (housekeeping fields) */
    fprintf(out, "\n[housekeeping]\n");
    for (int index = 98; index < 100; index++)
    {
        void** housekeepingData = message->Get<void*>(index, count);
        if (count == 0) fprintf(out, "%d=\"undefined\"\n", index);
        else fprintf(out, "%d=%p\n", index, *housekeepingData);
    }

    /* Request chasing */
    int intData = *message->Get<int>(-2, count);
    fprintf(out, "\n[request]\nuuid=\"%s\"\n",
            count == 0 ? "undefined" : int2str(intData).c_str());

    /* Cleanup */
    fprintf(out, "\n");
}

void MonServerSpy::DumpDataBits(FILE* out, PMsg_p* message)
{
    /* Some data pointers we may or may not use. */
    unsigned* uintData;
    int* intData;
    double* doubleData;
    float* floatData;
    bool* boolData;
    int count;

    /* The pattern of data is defined from the signature, so we get that
     * first... */
    fprintf(out, "[data]\n");
    std::string signature;
    message->Get(0, signature);
    fprintf(out, "signature=\"%s\"\n",
            signature == "" ? "\"undefined\"" : signature.c_str());

    /* Check for the leading zero. */
    fprintf(out, "leading_zero=\"%s\"\n",
            signature[0] == '0' ? "present" : "not present");

    /* Now we iterate through the signature, dumping one line per character. */
    for (unsigned index = 1; index < signature.length(); index++)
    {
        std::string caption;
        std::string dataString;

        /* Grab the caption (there may not be one) */
        message->Get(index, caption);

        /* Grab the data as best as we can. */
        switch (signature[index])
        {
        case 'u':
            uintData = message->Get<unsigned>(index, count);
            if (count == 0) dataString = "\"(unsigned) no data found\"";
            else dataString = uint2str(*uintData);
            break;
        case 'i':
            intData = message->Get<int>(index, count);
            if (count == 0) dataString = "\"(integer) no data found\"";
            else dataString = int2str(*intData);
            break;
        case 'd':
            doubleData = message->Get<double>(index, count);
            if (count == 0) dataString = "\"(double) no data found\"";
            else dataString = dbl2str(*doubleData);
            break;
        case 'f':
            floatData = message->Get<float>(index, count);
            if (count == 0) dataString = "\"(float) no data found\"";
            else dataString = dbl2str(*floatData);  /* yeah yeah */
            break;
        case 'b':
            boolData = message->Get<bool>(index, count);
            if (count == 0) dataString = "\"(bool) no data found\"";
            else dataString = bool2str(*boolData);
            break;
        }

        /* Write one line for this entry, and one for its caption if it has
         * one. */
        if (caption.length()) fprintf(out, "caption_%u=%s\n",
                                      index, caption.c_str());
        fprintf(out, "element_%u=%s\n", index, dataString.c_str());
    }
}

void MonServerSpy::DumpMoniDeviAck(FILE* out, PMsg_p* message)
{
    int count;
    bool* boolData;
    int* intData;
    unsigned* uintData;

    fprintf(out, "// Dump of MONI|DEVI|ACK message.\n\n");
    DumpCommonBits(out, message);
    fprintf(out, "[misc]\n");
    fprintf(out, "app=\"%s\"\n", message->Zname(0).c_str());
    fprintf(out, "device=\"%s\"\n", message->Zname(1).c_str());

    boolData = message->Get<bool>(6, count);
    fprintf(out, "error=\"%s\"\n",
            count == 0 ? "undefined" : *boolData == true ? "yes" : "no");

    fprintf(out, "\n[exfiltration]\n");
    uintData = message->Get<unsigned>(4, count);
    if (count == 0) fprintf(out, "source=\"undefined\"\n");
    else fprintf(out, "source=%s\n",
                 *uintData == 1 ? "\"softswitch\"" : "\"mothership\"");

    boolData = message->Get<bool>(0, count);
    fprintf(out, "exfiltration_idempotence=\"%s\"\n",
            count == 0 ? "undefined" :
            *boolData == true ? "start" : "stop");

    boolData = message->Get<bool>(1, count);
    fprintf(out, "device_found=\"%s\"\n",
            count == 0 ? "undefined" : *boolData == true ? "yes" : "no");

    fprintf(out, "\n[device_id]\n");
    for (int index = 0; index < 6; index++)
    {
        std::string component = "";
        switch (index)
        {
        case 0:
            component = "box";
            break;
        case 1:
            component = "board";
            break;
        case 2:
            component = "mailbox";
            break;
        case 3:
            component = "core";
            break;
        case 4:
            component = "thread";
            break;
        case 5:
            component = "device";
            break;
        }
        intData = message->Get<int>(index, count);
        fprintf(out, "%s=%s\n", component.c_str(),
                count == 0 ? "\"undefined\"" : int2str(*intData).c_str());
    }
}

void MonServerSpy::DumpMoniInjeAck(FILE* out, PMsg_p* message)
{
    fprintf(out, "// Dump of MONI|INJE|ACK message.\n\n");
    DumpCommonBits(out, message);
    fprintf(out, "[misc]\n");
    fprintf(out, "acknowledgement_string=\"%s\"\n",
            message->Zname(3).c_str());
}

void MonServerSpy::DumpMoniMothData(FILE* out, PMsg_p* message)
{
    fprintf(out, "// Dump of MONI|MOTH|DATA message.\n\n");
    DumpCommonBits(out, message);
    DumpDataBits(out, message);
}

void MonServerSpy::DumpMoniSoftData(FILE* out, PMsg_p* message)
{
    fprintf(out, "// Dump of MONI|SOFT|DATA message.\n\n");
    DumpCommonBits(out, message);
    DumpDataBits(out, message);
}


/* Sets spyDir, and returns True if the value has been changed. */
bool MonServerSpy::SetSpyDir(std::string spyDirIn)
{
    if (spyDir == spyDirIn) return false;
    else
    {
        spyDir = spyDirIn;
        isSpyDirNew = true;
        return true;
    }
}

/* Writes Spy output (if the spy is enabled), as per the description in the
 * header file. Returns non-zero if an error occured, and writes to `error`. */
int MonServerSpy::Spy(PMsg_p* message)
{
    if (!isSpyEnabled) return 0;

    /* If this is the first time we're sending a message since the spy
     * directory has been set, we need to do a bit of preparation. */
    if (isSpyDirNew)
    {
        isSpyDirNew = false;  /* Grow old gracefully */
        CheckCloseDumpMap();
        dumpMapIndex = 0;

        /* Create a new dumpMap file, since we've changed directory. Note the
         * clobber here - we don't expect files with the same timestamp in the
         * same directory. This could happen of course, but life is short... */
        dumpMapDate = GetISO8601Seconds();
        std::string dumpMapName = "dumpmap_" + dumpMapDate + ".csv";
        dumpMap = fopen((spyDir + dumpMapName).c_str(), "w");

        /* Errors! Panic! */
        if (dumpMap == PNULL)
        {
            error = OSFixes::getSysErrorString(errno);
            return 1;
        }

        /* Write a sensible header. */
        fprintf(dumpMap, "index,keystring,mode\n");
    }

    /* Create a new file to mini-dump this message into. */
    FILE* miniDumpFile = fopen((spyDir + dumpMapDate +
                                "_index_" + uint2str(dumpMapIndex) +
                                ".uif").c_str(),
                               "w");
    /* Errors! Panic! */
    if (miniDumpFile == PNULL)
    {
        error = OSFixes::getSysErrorString(errno);
        return 1;
    }

    /* Filter by key type, complaining if not familiar. Note that, if we want
     * to track new key types going to the MonServer in future, we'll need to
     * add to this logic.
     *
     * While we're filtering by key, create a mini-dump of this message (it's
     * convenient). */
    unsigned key = message->Key();
    std::string humanKey = "";
    if (key == PMsg_p::KEY(Q::MONI, Q::DEVI, Q::ACK))
    {
        humanKey = "MONI|DEVI|ACK";
        DumpMoniDeviAck(miniDumpFile, message);
    }
    else if (key == PMsg_p::KEY(Q::MONI, Q::INJE, Q::ACK))
    {
        humanKey = "MONI|INJE|ACK";
        DumpMoniInjeAck(miniDumpFile, message);
    }
    else if (key == PMsg_p::KEY(Q::MONI, Q::MOTH, Q::DATA))
    {
        humanKey = "MONI|MOTH|DATA";
        DumpMoniMothData(miniDumpFile, message);
    }
    else if (key == PMsg_p::KEY(Q::MONI, Q::SOFT, Q::DATA))
    {
        humanKey = "MONI|SOFT|DATA";
        DumpMoniSoftData(miniDumpFile, message);
    }
    else
    {
        error = "Message key " + uint2str(key) + " not recognised.";
        return 1;
    }

    /* Store an entry to this message in the dumpMap. */
    fprintf(dumpMap, (uint2str(dumpMapIndex) + "," +
                      humanKey + "," +
                      int2str(message->Mode()) + "\n").c_str());

    /* What is the purpose of endless cycles if not to be part of a greater
     * endless cycle, ultimately forming endless cycles of endless cycles,
     * which themselves form... */
    dumpMapIndex++;

    /* ...are limit cycles really peaceful? */
    return 0;
}
