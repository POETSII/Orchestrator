/* Defines behaviour for the hardware model file parser (see the accompanying
 * header for further information). */

#include "HardwareFileParser.h"

/* Constructs the file parser. */
HardwareFileParser::HardwareFileParser():JNJ(),isFileLoaded(false)
{
    /* Registers callback functions (for the UIF base) for error handling
     * purposes. */
    SetECB(this, onSyntaxError);
}

/* Constructs the file parser, parses a file, and dumps it in an engine. */
HardwareFileParser::HardwareFileParser(const char* filePath, P_engine* engine)
    :JNJ(),isFileLoaded(false)
{
    loadFile(filePath);
    populateHardwareModel(engine);
}

/* Returns true if a file exists at the path passed to by argument, and false
 * otherwise.
 *
 * Would be more elegant and faster with POSIX's stat...
 *
 * Arguments:
 *
 * - filePath: Path to look at. */
bool HardwareFileParser::doesFileExist(const char* filePath)
{
    FILE* testFile = fopen(filePath, "r");
    if (testFile)
    {
        fclose(testFile);
        return true;
    }
    else{return false;}
}

/* Loads an input file and parses it through the UIF parsing mechanism to
 * generate the tree structure. Arguments:
 *
 * - filePath: Path to the file to load
 *
 * If a file has already been loaded, it is unloaded before the new file is
 * read.
 *
 * Throws if:
 *
 * - the input file does not exist.
 * - the input file is syntactically invalid. */
void HardwareFileParser::loadFile(const char* filePath)
{
    if(!doesFileExist(filePath))
    {
        throw HardwareFileNotFoundException(
            dformat("[ERROR] No input file found at \"%s\".\n", filePath));
    }

    if (isFileLoaded)
    {
        Reset();  /* From UIF. */
    }

    loadedFile.clear();
    loadedFile += filePath;
    isFileLoaded = true;
    Addx(strdup(filePath));
    // <!> Some error handling needed here.
}

/* Defines behaviour when a syntax error is encountered when reading a file.
 *
 * Basically, this just throws an exception with some debug messaging
 * attached. See the example callback (UIF::DefECB) for more information, since
 * this is lifted from there.
 *
 * Scoping be damned. */
void HardwareFileParser::onSyntaxError(void* parser, void* uifInstance, int)
{
    std::string accumulatedMessage;
    std::vector<std::string> errorMessages;
    static_cast<UIF*>(uifInstance)->Lx.Hst.Dump(errorMessages, 20);
    accumulatedMessage = std::accumulate(errorMessages.begin(),
                                         errorMessages.end(), std::string(""));
    throw HardwareSyntaxException(dformat(
        "[ERROR] Error in input hardware file syntax in \"%s\":\n%s",
        static_cast<HardwareFileParser*>(parser)->loadedFile.c_str(),
        accumulatedMessage.c_str()));
}

/* Populates a POETS Engine with information from the hardware
 * model. Arguments:
 *
 * - engine: Pointer to the POETS Engine to populate. Must be empty.
 *
 * Throws if:
 *
 *  - no input file has been loaded by this parser.
 *  - if the input file is semantically invalid.
 *  - if engine is not empty. */
void HardwareFileParser::populateHardwareModel(P_engine* engine)
{
    /* Throw if we have not loaded a file yet. */
    if (!isFileLoaded)
    {
        // <!> Throw!
    }
}
