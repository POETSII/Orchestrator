/* Defines behaviour for the core functionality (independent of dialect) of the
 * hardware model file parser (see the accompanying header for further
 * information). */

#include "HardwareFileParser.h"

/* Constructs the file parser. */
HardwareFileParser::HardwareFileParser():JNJ(),isFileLoaded(false)
{
    set_uif_error_callback();
}

/* Constructs the file parser, parses a file, and dumps it in an engine. */
HardwareFileParser::HardwareFileParser(const char* filePath, P_engine* engine)
    :JNJ(),isFileLoaded(false)
{
    set_uif_error_callback();
    load_file(filePath);
    populate_hardware_model(engine);
}

/* Returns true if a file exists at the path passed to by argument, and false
 * otherwise.
 *
 * Would be more elegant and faster with POSIX's stat...
 *
 * Arguments:
 *
 * - filePath: Path to look at. */
bool HardwareFileParser::does_file_exist(const char* filePath)
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
void HardwareFileParser::load_file(const char* filePath)
{
    if(!does_file_exist(filePath))
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
    std::string filePathAsString = filePath;
    Add(filePathAsString);
}

/* Registers a callback function for error handling purposes. */
void HardwareFileParser::set_uif_error_callback()
{
    SetECB(this, on_syntax_error);  /* From UIF. */
}

/* Defines behaviour when a syntax error is encountered when reading a file.
 *
 * Basically, this just throws an exception with some debug messaging
 * attached. See the example callback (UIF::DefECB) for more information, since
 * this is lifted from there.
 *
 * Scoping be damned. */
void HardwareFileParser::on_syntax_error(void* parser, void* uifInstance, int)
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

/* Writes an 'invalid variable' error message, and appends it to a
 * string. Arguments:
 *
 * - errorMessage: String to append to.
 * - recordNode: Record node where the invalid variable was found.
 * - sectionName: Name of the section in which the invalid variable was found.
 * - variable: Invalid variable name. */
void HardwareFileParser::invalid_variable_message(
    std::string* errorMessage, UIF::Node* recordNode, std::string sectionName,
    std::string variable)
{
    errorMessage->append(dformat(
        "L%u: Variable '%s' in section '%s' is not recognised by the parser. "
        "Is it valid?\n",
        recordNode->pos, variable.c_str(), sectionName.c_str()));
}
