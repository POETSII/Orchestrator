/* Defines behaviour for the hardware model file parser (see the accompanying
 * header for further information). */
#include <iostream> // <!>
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
    printf("provisioning deployer...\n");  // <!>
    provision_deployer(&deployer);
    deployer.deploy(engine);
}

/* Provisions a dialect 1 deployer with the configuration in this
 * parser. Performs validation on types of values in assignments, as well as
 * conveniently-easy-to-catch semantic mistakes (i.e. missing '+' signs before
 * a binding).
 *
 * This method creates a deployer, then uses that deployer to populate the one
 * passed as argument. This approach is taken so that semantic errors found
 * while populating the proxy deployer do not corrupt the one passed in as an
 * argument.
 *
 * Arguments:
 *
 * - deployerTarget: Self explanatory really. */
void HardwareFileParser::provision_deployer(Dialect1Deployer* deployerTarget)
{
    /* Create a deployer to populate. */
    Dialect1Deployer deployer;
    bool valid = true;
    std::string error;

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
    std::vector<UIF::Node*> sectionNameNodes;
    std::vector<std::string> values;
    for (sectionIterator=sectionNodes.begin();
         sectionIterator!=sectionNodes.end(); sectionIterator++)
    {
        /* Determine the names of this section. */
        GetNames((*sectionIterator), sectionNameNodes);

        /* A comment at the top of the file creates a section node with empty
         * string. This section node has record nodes corresponding to the
         * comment text. We do not care about those for the purposes of
         * populating the deployer object, so we skip this section.
         *
         * On the bright side, this can only happen once, because after the
         * next section, every record is within a section (it's not possible to
         * escape!). */
        if (sectionNameNodes.empty()){continue;}

        /* We only care about the first name (i.e. if the name is
         * "header(Aesop)", we care only about "header") by reading only from
         * the first name-type node. */
        sectionName = sectionNameNodes[0]->str;

        /* Iterate through all records in this section. NB: Clears the
         * recordNodes vector. */
        GetRecd((*sectionIterator), recordNodes);

        for (recordIterator=recordNodes.begin();
             recordIterator!=recordNodes.end(); recordIterator++)
        {
            /* Get the value and variable nodes. */
            GetVari((*recordIterator), variableNodes);
            GetValu((*recordIterator), valueNodes);

            /* Ignore this record if the record has not got a variable/value
               pair (i.e. if the line is empty, or is just a comment). */
            if (variableNodes.size() == 0 || valueNodes.size() == 0){continue;}

            /* Only the first variable node will be used, and it must have a
             * "+" preceeding it. */
            variable = variableNodes[0]->str;
            if (variableNodes[0]->qop != Lex::Sy_plus)
            {
                printf("Your variable '%s' is missing a '+'.\n",
                       variable.c_str()); // <!>
            }

            /* There may be many values. If there is only one value, the first
             * value node contains the value. If there are multiple (N) value
             * nodes, the first value node contains N value nodes, each with an
             * entry. */
            values.clear();
            if (valueNodes[0]->leaf.size() == 0)  /* There's only one value. */
            {
                values.push_back(valueNodes[0]->str);
            }
            else  /* There are many values. */
            {
                for (valueNodeIterator=valueNodes[0]->leaf.begin();
                     valueNodeIterator!=valueNodes[0]->leaf.end();
                     valueNodeIterator++)
                {
                    values.push_back((*valueNodeIterator)->str);
                }
            }

            /* A gigantic if/else for each section the record could be in,
             * followed by a slightly less gigantic if/else for each variable
             * the record corresponds to. Could probably be an
             * enumerated-type-powered-switch-case monstrosity instead, but
             * ehh...
             *
             * We also do some validation on input types in here. */
            printf("section: %s, variable: %s, value(s): ", sectionName.c_str(), variable.c_str());  // <!>
            for (std::vector<std::string>::iterator i=values.begin();
                 i!=values.end(); ++i) std::cout << *i << ' ';
            printf("\n"); // <!>

            if (sectionName == "header")
            {
                if (variable == "author")
                {
                    deployer->author = values[0];
                }
                else if (variable == "datetime")
                {
                    deployer->datetime = str2long(values[0]);
                }
                else if (variable == "file")
                {
                    deployer->fileOrigin = values[0];
                }
                else if (variable == "version")
                {
                    deployer->version = values[0];
                }
                else if (variable == "dialect" || variable == "hardware");
                else {printf("Whoops (header)\n");} // <!>
            }

            else if (sectionName == "packet_address_format")
            {
                if (variable == "board")
                {
                    /* Convert the values from strings to uints, and drop them
                     * in the deployer. */
                    deployer->boardWordLengths.resize(values.size());
                    std::transform(values.begin(), values.end(),
                                   deployer->boardWordLengths.begin(),
                                   str2unsigned);
                }
                else if (variable == "box")
                {
                    deployer->boxWordLength = str2unsigned(values[0]);
                }
                else if (variable == "core")
                {
                    deployer->coreWordLength = str2unsigned(values[0]);
                }
                else if (variable == "mailbox")
                {
                    /* Same as with packet_address_format::board. */
                    deployer->mailboxWordLengths.resize(values.size());
                    std::transform(values.begin(), values.end(),
                                   deployer->mailboxWordLengths.begin(),
                                   str2unsigned);
                }
                else if (variable == "thread")
                {
                    deployer->threadWordLength = str2unsigned(values[0]);
                }
                else {printf("Whoops (packet_address_format)\n");} // <!>
            }

            else if (sectionName == "engine")
            {
                if (variable == "boxes")
                {
                    deployer->boxesInEngine = str2unsigned(values[0]);
                }
                else if (variable == "boards")
                {
                    if (values.size() == 1)  /* Scalar */
                    {
                        deployer->boardsInEngine.push_back(
                            str2unsigned(values[0]));
                        deployer->boardHypercubePeriodicity.push_back(false);
                    }
                    else  /* Hypercube */
                    {
                        /* Complain if multidimensional without hypercube. */
                        if (valueNodes[0]->str != "hypercube")
                        {
                            printf("engine::boards isn't a hypercube when it "
                                   "should be...\n"); // <!>
                        }

                        deployer->boardsInEngine.resize(values.size());
                        std::transform(values.begin(), values.end(),
                                       deployer->boardsInEngine.begin(),
                                       str2unsigned);

                        /* Handle periodicity ('+' sign in values) */
                        for (valueNodeIterator=valueNodes[0]->leaf.begin();
                             valueNodeIterator!=valueNodes[0]->leaf.end();
                             valueNodeIterator++)
                        {
                            if ((*valueNodeIterator)->qop == Lex::Sy_plus)
                            {
                                deployer->boardHypercubePeriodicity.
                                    push_back(true);
                            }
                            else if ((*valueNodeIterator)->qop == Lex::Sy_ISTR)
                            {
                                deployer->boardHypercubePeriodicity.
                                    push_back(false);
                            }
                            else
                            {
                                printf("Invalid value for boards variable in "
                                       "the engine section. Not sure how to "
                                       "react to this.\n"); // <!>
                                deployer->boardHypercubePeriodicity.
                                    push_back(false);
                            }
                        }
                    }
                }
                else if (variable == "external_box_cost")
                {
                    deployer->costExternalBox = str2float(values[0]);
                }
                else if (variable == "board_board_cost")
                {
                    deployer->costBoardBoard = str2float(values[0]);
                }
                else {printf("Whoops (engine)\n");} // <!>
            }

            else if (sectionName == "box")
            {
                if (variable == "box_board_cost")
                {
                    deployer->costBoxBoard = str2float(values[0]);
                }
                else if (variable == "supervisor_memory")
                {
                    deployer->boxSupervisorMemory = str2unsigned(values[0]);
                }
                else {printf("Whoops (box)\n");}  // <!>
            }

            else if (sectionName == "board")
            {
                if (variable == "mailboxes")
                {
                    if (values.size() == 1)  /* Scalar */
                    {
                        deployer->mailboxesInBoard.push_back(
                            str2unsigned(values[0]));
                        deployer->mailboxHypercubePeriodicity.push_back(false);
                    }
                    else  /* Hypercube */
                    {
                        /* Complain if multidimensional without hypercube. */
                        if (valueNodes[0]->str != "hypercube")
                        {
                            printf("board::mailboxes isn't a hypercube when "
                                   "it should be...\n"); // <!>
                        }

                        deployer->mailboxesInBoard.resize(values.size());
                        std::transform(values.begin(), values.end(),
                                       deployer->mailboxesInBoard.begin(),
                                       str2unsigned);

                        /* Handle periodicity ('+' sign in values) */
                        for (valueNodeIterator=valueNodes[0]->leaf.begin();
                             valueNodeIterator!=valueNodes[0]->leaf.end();
                             valueNodeIterator++)
                        {
                            if ((*valueNodeIterator)->qop == Lex::Sy_plus)
                            {
                                deployer->mailboxHypercubePeriodicity.
                                    push_back(true);
                            }
                            else if ((*valueNodeIterator)->qop == Lex::Sy_ISTR)
                            {
                                deployer->mailboxHypercubePeriodicity.
                                    push_back(false);
                            }
                            else
                            {
                                printf("Invalid value for mailboxes variable "
                                       "in the engine section. Not sure how "
                                       "to react to this.\n"); // <!>
                                deployer->mailboxHypercubePeriodicity.
                                    push_back(false);
                            }
                        }
                    }
                }
                else if (variable == "board_mailbox_cost")
                {
                    deployer->costBoardMailbox = str2float(values[0]);
                }
                else if (variable == "mailbox_mailbox_cost")
                {
                    deployer->costBoardMailbox = str2float(values[0]);
                }
                else if (variable == "supervisor_memory")
                {
                    deployer->boardSupervisorMemory = str2unsigned(values[0]);
                }
                else if (variable == "dram")
                {
                    deployer->dram = str2unsigned(values[0]);
                }
                else {printf("Whoops (board)\n");}  // <!>
            }

            else if (sectionName == "mailbox")
            {
                if (variable == "cores")
                {
                    if (valueNodes[0]->qop != Lex::Sy_ISTR)
                    {
                        printf("That's not an integer string...\n"); // <!>
                    }
                    deployer->coresInMailbox = str2unsigned(values[0]);
                }
                else if (variable == "mailbox_core_cost")
                {
                    deployer->costMailboxCore = str2float(values[0]);
                }
                else if (variable == "core_core_cost")
                {
                    deployer->costCoreCore = str2float(values[0]);
                }
                else {printf("Whoops (mailbox)\n");}  // <!>
            }

            else if (sectionName == "core")
            {
                if (variable == "threads")
                {
                    deployer->threadsInCore = str2unsigned(values[0]);
                }
                else if (variable == "instruction_memory")
                {
                    deployer->instructionMemory = str2unsigned(values[0]);
                }
                else if (variable == "data_memory")
                {
                    deployer->dataMemory = str2unsigned(values[0]);
                }
                else if (variable == "core_thread_cost")
                {
                    deployer->costCoreThread = str2float(values[0]);
                }
                else if (variable == "thread_thread_cost")
                {
                    deployer->costThreadThread = str2float(values[0]);
                }
                else {printf("Whoops (core)\n");}  // <!>
            }

            else {printf("Whoops (section)\n");}
        }
    }

    /* If every record was valid, we populate the input deployer with our
     * created one. */

}

/* Converts a float-like input string to an actual float. */
float str2float(std::string floatLike)
{
    float returnValue;
    sscanf(floatLike.c_str(), "%f", &returnValue);
    return returnValue;
}

/* Converts a float-like input string to an actual float. I know something like
 * this exists in flat, but I want something with a single argument so that I
 * can use std::transform to transform a vector of strings into a vector of
 * unsigneds. */
unsigned str2unsigned(std::string uintLike)
{
    unsigned returnValue;
    sscanf(uintLike.c_str(), "%u", &returnValue);
    return returnValue;
}
