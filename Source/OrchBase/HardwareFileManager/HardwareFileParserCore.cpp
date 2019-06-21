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

/* Populates a POETS Engine with information from a previously-loaded hardware
 * description file, while performing semantic validation. Arguments:
 *
 * - engine: Pointer to the POETS Engine to populate. Cleared if the input file
 *   is invalid
 *
 * Throws if:
 *
 *  - no input file has been loaded by this parser.
 *  - the input file is semantically invalid. */
void HardwareFileParser::populate_hardware_model(P_engine* engine)
{
    /* Throw if we have not loaded a file yet. */
    if (!isFileLoaded)
    {
        throw HardwareFileNotLoadedException();
    }

    /* Only dialect 1 and 3 for now (get_dialect errors if the dialect is not 1
     * or 3). */
    if (get_dialect() == 1)
    {
        d1_populate_hardware_model(engine);
    }
    else if (get_dialect() == 3)
    {
        d3_populate_hardware_model(engine);
    }
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

/* Returns the dialect of a loaded file. Throws if no input file has been
 * loaded by this parser, or if the dialect is not defined correctly in the
 * input file. */
unsigned HardwareFileParser::get_dialect()
{
    /* Throw if we have not loaded a file yet. */
    if (!isFileLoaded)
    {
        throw HardwareFileNotLoadedException();
    }

    /* Get the header section node, and complain if there is not exactly one of
     * them. */
    std::vector<UIF::Node*> headerSections;
    FndSect("header", headerSections);
    if (headerSections.empty())
    {
        throw HardwareSemanticException("[ERROR] No header section defined in "
                                        "input file.\n");
    }

    else if (headerSections.size() > 1)
    {
        throw HardwareSemanticException(
            dformat("[ERROR] %u header sections defined in input file (only "
                    "one is permitted).\n", headerSections.size()));
    }

    /* Get the dialect entry, and complain if the dialect is not defined
     * exactly once. */
    std::vector<UIF::Node*> recordNodes;
    FndRecdVari(headerSections[0], "dialect", recordNodes);
    if (recordNodes.empty())
    {
        throw HardwareSemanticException("[ERROR] No dialect defined in the "
                                        "header section of the input file.");
    }

    else if (recordNodes.size() > 1)
    {
        throw HardwareSemanticException(
            dformat("[ERROR] The dialect field in the header section has been "
                    "defined %u times (only one is permitted).\n",
                    recordNodes.size()));
    }

    /* Whine if the dialect field has no value. */
    std::vector<UIF::Node*> valueNodes;
    GetValu(recordNodes[0], valueNodes);

    if (valueNodes.empty())
    {
        throw HardwareSemanticException(
            dformat("[ERROR] Dialect definition at L%u has no value.",
                    recordNodes[0]->pos));
    }

    /* Whine if the dialect field has more than one value. If there is only one
     * value, the first value node contains the value. If there are multiple
     * (N) value nodes, the first value node contains N value nodes, each with
     * an entry. */
    if (valueNodes[0]->leaf.size() != 0)
    {
        throw HardwareSemanticException(
            dformat("[ERROR] Dialect definition at L%u has %u values, when "
                    "it should only have one.",
                    recordNodes[0]->pos, valueNodes[0]->leaf.size()));
    }

    /* Whine if the dialect is not a natural number. */
    if(!is_node_value_natural(valueNodes[0]))
    {
        throw HardwareSemanticException(
            dformat("[ERROR] Dialect definition at L%u has value '%s', which "
                    "is not 1 or 3.",
                    recordNodes[0]->pos, valueNodes[0]->str));
    }

    /* Whine if the dialect is not one or three. */
    int dialect = str2unsigned(valueNodes[0]->str);
    if (dialect != 1 and dialect != 3)
    {
        throw HardwareSemanticException(
            dformat("[ERROR] Only dialects 1 and 3 are currently supported "
                    "(the dialect definition at L%u has value '%s'.",
                    recordNodes[0]->pos, valueNodes[0]->str));
    }

    /* Whine if the dialect field does not have a "+" character preceeding the
     * variable name. */
    std::vector<UIF::Node*> variableNodes;
    GetVari(recordNodes[0], variableNodes);
    if (variableNodes[0]->qop != Lex::Sy_plus )
    {
        throw HardwareSemanticException(
            dformat("[ERROR] Dialect definition at L%u is missing the "
                    "preceeding '+' character.", recordNodes[0]->pos));
    }

    /* End of the gauntlet. */
    return dialect;
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

/* Registers a callback function for error handling purposes. */
void HardwareFileParser::set_uif_error_callback()
{
    SetECB(this, on_syntax_error);  /* From UIF. */
}
