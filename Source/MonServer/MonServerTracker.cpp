#include "flat.h"
#include "FileName.cpp"
#include "MonServerTracker.h"
#include "OSFixes.hpp"

MonServerTracker::MonServerTracker()
{
    error = "No stored error (yet).";
    isTrackerDirNew = true;
    isTrackerEnabled = false;
    trackerDir = "";
}

MonServerTracker::~MonServerTracker()
{
    /* Close all streams that have successfully been opened. */
    for (std::map<int, std::ofstream>::iterator streamIt = streams.begin();
         streamIt != streams.end(); streamIt++)
        if (streamIt->second.is_open()) streamIt->second.close();
}


/* Sets trackerDir, and returns True if the value has been changed. */
bool MonServerTracker::SetTrackerDir(std::string trackerDirIn)
{
    if (trackerDir == trackerDirIn) return false;
    else
    {
        trackerDir = trackerDirIn;
        isTrackerDirNew = true;
        return true;
    }
}

/* Gogogo! */
int MonServerTracker::Track(PMsg_p* message)
{
    if (!isTrackerEnabled) return 0;

    /* Grab the uuid, hollering if it's not there. */
    int count, uuid;
    uuid = *message->Get<int>(-2, count);
    if (count == 0)
    {
        error = "No UUID (int, -2) given in data packet.";
        return 1;
    }

    /* Grab the signature, complaining if it's not there. */
    std::string signature;
    message->Get(0, signature);
    std::string::size_type numElements = signature.length();
    if (numElements == 0)
    {
        error = "No signature found (or it's empty).";
        return 1;
    }

    /* Is this the first message of the stream? If so, we need to create an
     * entry in streams. If not, just use the stream that exists. */
    std::map<int, std::ofstream>::iterator streamFinder = streams.find(uuid);
    if (streamFinder == streams.end())
    {
        /* Open up (complaining if not). */
        std::string logPathReq = message->Zname(3);
        FileName::Force1Linux(logPathReq);
        std::string baseName = FileName(logPathReq).FNBase();
        streams[uuid].open((trackerDir + baseName).c_str(),
                           std::ofstream::out | std::ofstream::trunc);
        if(!streams[uuid].is_open())
        {
            error = std::string("Could not open file: ") +
                OSFixes::getSysErrorString(errno);
            return 1;
        }

        /* Write a header: For each element, write a label. */
        for (int element = 1; element < (int)numElements; element++)
        {
            if (element != 1) streams[uuid] << " ";
            std::string label;
            message->Get(element, label);
            if (label.empty())
            {
                label = "Unknown field with descriptor '" +
                    int2str(signature[element]) + "'.";
            }

            streams[uuid] << "\"" << label << "\"";
        }
        streams[uuid] << "\n";
    }

    /* Now let's write some data! */
    unsigned* uintData;
    int* intData;
    double* doubleData;
    float* floatData;
    bool* boolData;
    for (int element = 1; element < (int)numElements; element++)
    {
        if (element != 1) streams[uuid] << " ";
        switch (signature[element])
        {
        case 'u':
            uintData = message->Get<unsigned>(element, count);
            if (count == 0)
            {
                error = "No unsigned data found at field " +
                    int2str(element) + ".";
                return 1;
            }
            streams[uuid] << *uintData;
            break;
        case 'i':
            intData = message->Get<int>(element, count);
            if (count == 0)
            {
                error = "No integer data found at field " +
                    int2str(element) + ".";
                return 1;
            }
            streams[uuid] << *intData;
            break;
        case 'd':
            doubleData = message->Get<double>(element, count);
            if (count == 0)
            {
                error = "No double data found at field " +
                    int2str(element) + ".";
                return 1;
            }
            streams[uuid] << *doubleData;
            break;
        case 'f':
            floatData = message->Get<float>(element, count);
            if (count == 0)
            {
                error = "No float data found at field " +
                    int2str(element) + ".";
                return 1;
            }
            streams[uuid] << *floatData;
            break;
        case 'b':
            boolData = message->Get<bool>(element, count);
            if (count == 0)
            {
                error = "No boolean data found at field " +
                    int2str(element) + ".";
                return 1;
            }
            streams[uuid] << *boolData;
            break;
        }
    }

    /* Next! */
    streams[uuid] << "\n";
    return 0;
}
