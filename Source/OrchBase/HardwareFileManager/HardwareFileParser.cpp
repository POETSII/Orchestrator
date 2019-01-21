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
        throw HardwareFileNotLoadedException();
    }

    /* Call the various semantic validation methods. */
    std::string errorMessage;
    bool failedValidation = false;
    failedValidation |= !validateSections(&errorMessage);

    /* Throw if any of the validation methods failed. */
    if (failedValidation)
    {
        throw HardwareSemanticException(errorMessage.c_str());
    }
}

/* Validate that there are no duplicated sections in the hardware model, and
 * that the following mandatory section names are defined (in any order):
 *
 * - header, with an optional context-sensitive argument.
 * - packet_address_format
 * - engine
 * - box
 * - board
 * - mailbox
 * - core
 *
 * Arguments:
 *
 * - errorMessage: string to write error message to, always appended to.
 *
 * Returns true if all sections exist and are unique, and false
 * otherwise. Should only be called after a file has been loaded.*/
bool HardwareFileParser::validateSections(std::string* errorMessage)
{
    /* Grab section names. */
    std::vector<UIF::Node*> sectionNameNodes;
    GetNames(sectionNameNodes);

    /* Define valid node names, and counters for them (we're failing if any of
     * these are not 1 at the end). */

    std::map<std::string, unsigned> validNodeCounters;
    std::map<std::string, unsigned>::iterator validNodeIterator;
    validNodeCounters.insert(std::pair<std::string, unsigned>("header", 0));
    validNodeCounters.insert(std::pair<std::string, unsigned>
                             ("packet_address_format", 0));
    validNodeCounters.insert(std::pair<std::string, unsigned>("engine", 0));
    validNodeCounters.insert(std::pair<std::string, unsigned>("box", 0));
    validNodeCounters.insert(std::pair<std::string, unsigned>("board", 0));
    validNodeCounters.insert(std::pair<std::string, unsigned>("mailbox", 0));
    validNodeCounters.insert(std::pair<std::string, unsigned>("core", 0));

    /* Store invalid node names for error reporting. */
    std::vector<UIF::Node*> invalidNamedNodes;
    std::vector<UIF::Node*>::iterator invalidNamedNodeIterator;

    /* For each of the sections found in the file, get its name. If it's a
     * valid section name, increment the counter. If it's not, push-back the
     * node to invalidNamedNodes. */
    std::vector<UIF::Node*>::iterator nodeIterator;
    for (nodeIterator=sectionNameNodes.begin();
         nodeIterator!=sectionNameNodes.end(); nodeIterator++)
    {
        validNodeIterator = validNodeCounters.find((*nodeIterator)->str);

        /* We found it! Increment the counter. */
        if (validNodeIterator != validNodeCounters.end())
        {
            validNodeIterator->second++;
        }
        /* We didn't find it! Add it to the list of invalid node names passed
         * in. */
        else
        {
            invalidNamedNodes.push_back((*nodeIterator));
        }
    }

    /* The input was valid if the validNodeCounters are all 1, and if there are
     * no invalid node names. */
    bool returnValue = true;  /* Success/failure state (true is a pass) */

    /* Checking for invalid node names. */
    if (invalidNamedNodes.size() != 0)
    {
        /* We've failed. */
        returnValue = false;

        /* Let's let the user know how mean they are. */
        errorMessage->append("Sections with invalid names found in the input "
                             "file:\n");
        for (invalidNamedNodeIterator=invalidNamedNodes.begin();
             invalidNamedNodeIterator!=invalidNamedNodes.end();
             invalidNamedNodeIterator++)
        {
            errorMessage->append(dformat("    %s (L%i)\n",
                                         (*invalidNamedNodeIterator)->str,
                                         (*invalidNamedNodeIterator)->pos));
        }
    }

    /* Checking all sections are defined exactly once. */
    bool headerPrinted = false;
    for (validNodeIterator=validNodeCounters.begin();
         validNodeIterator!=validNodeCounters.end(); validNodeIterator++)
    {
        /* We didn't find the section. */
        if (validNodeIterator->second != 1)
        {
            /* We've failed. */
            returnValue = false;
            if (!headerPrinted)
            {
                errorMessage->append("The following sections were not defined "
                                     "exactly once:\n");
                headerPrinted = true;
            }

            errorMessage->append(dformat("    %s, defined %u times.\n",
                                         validNodeIterator->first.c_str(),
                                         validNodeIterator->second));
        }
    }

    return returnValue;
}
