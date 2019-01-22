/* Defines behaviour for the hardware model file parser (see the accompanying
 * header for further information). */

#include "HardwareFileParser.h"

/* Constructs the file parser. */
HardwareFileParser::HardwareFileParser():JNJ(),isFileLoaded(false)
{
    /* Registers callback functions (for the UIF base) for error handling
     * purposes. */
    SetECB(this, on_syntax_error);
}

/* Constructs the file parser, parses a file, and dumps it in an engine. */
HardwareFileParser::HardwareFileParser(const char* filePath, P_engine* engine)
    :JNJ(),isFileLoaded(false)
{
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
    Addx(strdup(filePath));
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

/* Populates a POETS Engine with information from the hardware
 * model, after validation. Arguments:
 *
 * - engine: Pointer to the POETS Engine to populate. Must be empty.
 *
 * Throws if:
 *
 *  - no input file has been loaded by this parser.
 *  - if the input file is semantically invalid.
 *  - if engine is not empty. */
void HardwareFileParser::populate_hardware_model(P_engine* engine)
{
    /* Throw if we have not loaded a file yet. */
    if (!isFileLoaded)
    {
        throw HardwareFileNotLoadedException();
    }

    /* Throw if the engine is not empty. */
    if (!(engine->is_empty()))
    {
        throw InvalidEngineException();
    }

    /* Call the various semantic validation methods. */
    std::string errorMessage;
    bool failedValidation = false;
    failedValidation |= !validate_sections(&errorMessage);

    /* Throw if any of the validation methods failed. */
    if (failedValidation)
    {
        throw HardwareSemanticException(errorMessage.c_str());
    }

    /* Now we actually do the population. */
    populate(engine);
}

/* Validate that all sections have correct contents, given that all sections
 * exist. The list of variables too great to enumerate here; look at the
 * documentation or examples for the definitive list.
 *
 * Arguments:
 *
 * - errorMessage: string to write error message to, always appended to.
 *
 * Returns true if all mandatory fields are defined exactly once and if no
 * incorrect variables are defined, and false otherwise. */
bool HardwareFileParser::validate_section_contents(std::string* errorMessage)
{
    /* Define what is valid. */
    std::map<std::string, unsigned> mandatoryHeaderFields;
    std::vector<std::string> optionalHeaderFields;
    mandatoryHeaderFields.insert(std::pair<std::string, unsigned>("datetime", 0));
    mandatoryHeaderFields.insert(std::pair<std::string, unsigned>("dialect", 0));
    mandatoryHeaderFields.insert(std::pair<std::string, unsigned>("version", 0));
    optionalHeaderFields.push_back("author");
    optionalHeaderFields.push_back("hardware");
    optionalHeaderFields.push_back("file");

    /* <!> */
    return true;
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
 * otherwise. Should only be called after a file has been loaded. */
bool HardwareFileParser::validate_sections(std::string* errorMessage)
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

/* Actually populates the hardware model, does not validate, should not
 * throw (hah). Arguments:
 *
 * - engine: Pointer to the POETS Engine to populate. Must be empty. */
void HardwareFileParser::populate(P_engine* engine)
{
    Dialect1Deployer deployer;
    provision_deployer(&deployer);
    deployer.deploy(engine);
}

/* Provisions a dialect 1 deployer with the configuration in this
 * parser. Is pretty naive. Arguments:
 *
 * - deployer: Deployer to provision. */
void HardwareFileParser::provision_deployer(Dialect1Deployer* deployer)
{
    /* Grab sections. */
    std::vector<UIF::Node*> sectionNodes;
    std::vector<UIF::Node*>::iterator sectionIterator;
    GetSect(sectionNodes);

    /* Temporary staging vectors for holding value nodes and variable
     * nodes. NB: Here, each record holds at most one variable node, but can
     * have multiple values. */
    std::vector<UIF::Node*> valueNodes;
    std::vector<UIF::Node*>::iterator valueNodeIterator;
    std::vector<UIF::Node*> variableNodes;
    std::string variable;

    /* For each section... */
    std::vector<UIF::Node*> recordNodes;
    std::vector<UIF::Node*>::iterator recordIterator;
    std::string sectionName;
    for (sectionIterator=sectionNodes.begin();
         sectionIterator!=sectionNodes.end(); sectionIterator++)
    {
        sectionName = (*sectionIterator)->str;

        /* Iterate through all records in this section. NB: Clears the
         * recordNodes vector. */
        GetRecd((*sectionIterator), recordNodes);

        for (recordIterator=recordNodes.begin();
             recordIterator!=recordNodes.end(); recordIterator++)
        {
            /* Get the value and variable nodes. Only the first variable node
             * will be used. */
            GetVari((*recordIterator), variableNodes);
            GetValu((*recordIterator), valueNodes);
            variable = variableNodes[0]->str;

            /* A gigantic if/else for each section the record could be in,
             * followed by a slightly less gigantic if/else for each variable
             * the record corresponds to. Could probably be an
             * enumerated-type-powered-switch-case monstrosity instead, but
             * ehh... */
            if (sectionName == "header")
            {
                if (variable == "author")
                {
                    deployer->author = valueNodes[0]->str;
                }
                else if (variable == "datetime")
                {
                    deployer->datetime = str2long(valueNodes[0]->str);
                }
                else if (variable == "file")
                {
                    deployer->fileOrigin = valueNodes[0]->str;
                }
                else if (variable == "version")
                {
                    deployer->version = valueNodes[0]->str;
                }
                else if (variable == "dialect" || variable == "hardware");
                else {printf("Whoops (header)\n");} // <!>
            }

            else if (sectionName == "packet_address_format")
            {
                if (variable == "board")  /* It's a vector. */
                {
                    for (valueNodeIterator=valueNodes.begin();
                         valueNodeIterator!=valueNodes.end();
                         valueNodeIterator++)
                    {
                        deployer->boardWordLengths.push_back(
                            str2uint((*valueNodeIterator)->str));
                    }
                }
                else if (variable == "box")
                {
                    deployer->boxWordLength = str2uint(valueNodes[0]->str);
                }
                else if (variable == "core")
                {
                    deployer->coreWordLength = str2uint(valueNodes[0]->str);
                }
                else if (variable == "mailbox")  /* It's a vector. */
                {
                    for (valueNodeIterator=valueNodes.begin();
                         valueNodeIterator!=valueNodes.end();
                         valueNodeIterator++)
                    {
                        deployer->mailboxWordLengths.push_back(
                            str2uint((*valueNodeIterator)->str));
                    }
                }
                else if (variable == "thread")
                {
                    deployer->threadWordLength = str2uint(valueNodes[0]->str);
                }
                else {printf("Whoops (packet_address_format)\n");} // <!>
            }

            else if (sectionName == "engine")
            {
                if (variable == "boxes")
                {
                    deployer->boxesInEngine = str2uint(valueNodes[0]->str);
                }
                else if (variable == "boards")
                {
                    /* Hypercube? */
                }
                else if (variable == "external_box_cost")
                {
                    deployer->externalBoxCost = str2float(valueNodes[0]->str);
                }
                else if (variable == "board_board_cost")
                {
                    deployer->boardBoardCost = str2float(valueNodes[0]->str);
                }
                else {printf("Whoops (engine)\n");} // <!>
            }

            else if (sectionName == "box")
            {
                // switch(variable.c_str())
                // {
                // case "box_board_cost":;
                // case "supervisor_memory":;
                // default: printf("Whoops (box)\n"); // <!>
                // }
            }

            else if (sectionName == "board")
            {
                // switch(variable.c_str())
                // {
                // case "mailboxes":;
                // case "board_mailbox_cost":;
                // case "supervisor_memory":;
                // case "mailbox_mailbox_cost":;
                // case "dram":;
                // default: printf("Whoops (board)\n"); // <!>
                // }
            }

            else if (sectionName == "mailbox")
            {
                // switch(variable.c_str())
                // {
                // case "cores":;
                // case "mailbox_core_cost":;
                // case "core_core_cost":;
                // default: printf("Whoops (mailbox)\n"); // <!>
                // }
            }

            else if (sectionName == "core")
            {
                // switch(variable.c_str())
                // {
                // case "threads":;
                // case "instruction_memory":;
                // case "data_memory":;
                // case "thread_thread_cost":;
                // case "core_thread_cost":;
                // default: printf("Whoops (core)\n"); // <!>
                // }
            }

            else {printf("Whoops (section)\n");}
        }
    }
}
