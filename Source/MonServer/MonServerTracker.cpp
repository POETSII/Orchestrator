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
    /* Close all files that have successfully been opened. */
    for (std::map<int, FILE*>::iterator fileIt = files.begin();
         fileIt != files.end(); fileIt++)
        if (fileIt->second != PNULL) fclose(fileIt->second);
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

    /* Is this the first message of the sequence? If so, we need to create an
     * entry in files. If not, just use the file that exists. */
    std::map<int, FILE*>::iterator fileFinder = files.find(uuid);
    FILE* out;
    if (fileFinder != files.end()) out = fileFinder->second;
    else
    {
        /* Open up (complaining if not). */
        std::string baseName = FileName(message->Zname(3)).FNBase();
        std::string fullName = trackerDir + baseName;
        out = fopen(fullName.c_str(), "w");
        files[uuid] = out;  /* convenience */
        if(out == PNULL)
        {
            error = "Could not open file at \"" + baseName + "\": " +
                OSFixes::getSysErrorString(errno);
            return 1;
        }

        /* Write a header: For each element, write a label, putting a timestamp
         * label first. */
        fprintf(out, "\"Time leaving Mothership\"");
        for (int element = 1; element < (int)numElements; element++)
        {
            std::string label;
            message->Get(element, label);
            if (label.empty()) label = "Unknown field with descriptor '" +
                                   int2str(signature[element]) + "'.";
            fprintf(out, " \"%s\"", label.c_str());
        }
        fprintf(out, "\n");
    }

    /* Write the timestamp. Blindly assume that it's there. */
    double* doubleData;
    doubleData = message->Get<double>(-40, count);
    fprintf(out, "%f", *doubleData);

    /* Now let's write some data! */
    unsigned* uintData;
    int* intData;
    float* floatData;
    bool* boolData;
    for (int element = 1; element < (int)numElements; element++)
    {
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
            fprintf(out, " %u", *uintData);
            break;
        case 'i':
            intData = message->Get<int>(element, count);
            if (count == 0)
            {
                error = "No integer data found at field " +
                    int2str(element) + ".";
                return 1;
            }
            fprintf(out, " %d", *intData);
            break;
        case 'd':
            doubleData = message->Get<double>(element, count);
            if (count == 0)
            {
                error = "No double data found at field " +
                    int2str(element) + ".";
                return 1;
            }
            fprintf(out, " %f", *doubleData);
            break;
        case 'f':
            floatData = message->Get<float>(element, count);
            if (count == 0)
            {
                error = "No float data found at field " +
                    int2str(element) + ".";
                return 1;
            }
            fprintf(out, " %f", *floatData);
            break;
        case 'b':
            boolData = message->Get<bool>(element, count);
            if (count == 0)
            {
                error = "No boolean data found at field " +
                    int2str(element) + ".";
                return 1;
            }
            fprintf(out, *boolData ? "true" : "false");
            break;
        }
    }

    /* Next record! */
    fprintf(out, "\n");
    return 0;
}
