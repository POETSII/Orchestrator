/* Defines behaviour for the dialect 1 functionality of the hardware model file
 * parser (see the accompanying header for further information). */

#include "HardwareFileParser.h"

/* Populates a POETS Engine with information from a previously-loaded hardware
 * description file in dialect 1, after validating it. Arguments:
 *
 * - engine: Pointer to the POETS Engine to populate. Cleared if the input file
 *   is valid.
 *
 * Throws if:
 *
 *  - the input file is semantically invalid. */
void HardwareFileParser::d1_populate_hardware_model(P_engine* engine)
{
    /* Call the various semantic validation methods. */
    std::string errorMessage;
    bool failedValidation = false;
    failedValidation |= !d1_validate_sections(&errorMessage);

    /* Create a deployer (and do a little incidental validation...) */
    Dialect1Deployer deployer;
    failedValidation |= !d1_provision_deployer(&deployer, &errorMessage);

    /* Throw if any of the validation methods failed. */
    if (failedValidation)
    {
        throw HardwareSemanticException(errorMessage.c_str());
    }

    /* Otherwise, it's go time. */
    deployer.deploy(engine);
}

/* Provisions a dialect 1 deployer with the configuration in this
 * parser. Performs validation on types of values in assignments, as well as
 * conveniently-easy-to-catch semantic mistakes (i.e. missing '+' signs before
 * a binding). Returns false if the validation found any problems, and true
 * otherwise.
 *
 * Arguments:
 *
 * - deployer: Deployer object to provision.
 * - errorMessage: String to append error messages to. */
bool HardwareFileParser::d1_provision_deployer(Dialect1Deployer* deployer,
                                               std::string* errorMessage)
{
    bool valid = true;

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
         * "header(Simple)", we care only about "header") by reading only from
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
                valid = false;
                errorMessage->append(dformat(
                    "L%u: Your definition of variable '%s' is missing a '+' "
                    "character.\n", (*recordIterator)->pos, variable.c_str()));
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
            if (sectionName == "header")
            {
                if (variable == "author")
                {
                    deployer->author = values[0];
                }
                else if (variable == "datetime")
                {
                    if (valueNodes[0]->qop != Lex::Sy_ISTR)
                    {
                        valid = false;
                        errorMessage->append(dformat(
                            "L%u: Variable '%s' in section '%s' has value "
                            "'%s', which is not a datetime in the form "
                            "YYYYMMDDhhmmss.\n",
                            (*recordIterator)->pos, variable.c_str(),
                            sectionName.c_str(), values[0].c_str()));
                    }
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
                else if (variable == "dialect")
                {
                    if (values[0] != "1")
                    {
                        valid = false;
                        errorMessage->append(dformat(
                            "L%u: Only dialect 1 is currently supported.\n",
                            (*recordIterator)->pos));
                    }
                }
                else if (variable == "hardware");
                else
                {
                    valid = false;
                    invalid_variable_message(errorMessage, (*recordIterator),
                                             sectionName, variable);
                }
            }

            else if (sectionName == "packet_address_format")
            {
                if (variable == "board")
                {
                    valid &= complain_if_nodes_values_not_natural(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);

                    /* Convert the values from strings to uints, and drop them
                     * in the deployer. */
                    deployer->boardWordLengths.resize(values.size());
                    std::transform(values.begin(), values.end(),
                                   deployer->boardWordLengths.begin(),
                                   str2unsigned);
                }
                else if (variable == "box")
                {
                    valid &= complain_if_node_value_not_natural(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    if (valueNodes[0]->qop != Lex::Sy_ISTR)
                    {
                        valid = false;
                        errorMessage->append(dformat(
                            "L%u: Variable '%s' in section '%s' has value "
                            "'%s', which is not a natural number.\n",
                            (*recordIterator)->pos, variable.c_str(),
                            sectionName.c_str(), values[0].c_str()));
                    }
                    deployer->boxWordLength = str2unsigned(values[0]);
                }
                else if (variable == "core")
                {
                    valid &= complain_if_node_value_not_natural(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->coreWordLength = str2unsigned(values[0]);
                }
                else if (variable == "mailbox")
                {
                    /* Same as with packet_address_format::board. */
                    valid &= complain_if_nodes_values_not_natural(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);

                    deployer->mailboxWordLengths.resize(values.size());
                    std::transform(values.begin(), values.end(),
                                   deployer->mailboxWordLengths.begin(),
                                   str2unsigned);
                }
                else if (variable == "thread")
                {
                    valid &= complain_if_node_value_not_natural(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->threadWordLength = str2unsigned(values[0]);
                }
                else
                {
                    valid = false;
                    invalid_variable_message(errorMessage, (*recordIterator),
                                             sectionName, variable);
                }
            }

            else if (sectionName == "engine")
            {
                if (variable == "boxes")
                {
                    valid &= complain_if_node_value_not_natural(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->boxesInEngine = str2unsigned(values[0]);
                }
                else if (variable == "boards")
                {
                    if (values.size() == 1)  /* Scalar */
                    {
                        valid &= complain_if_node_value_not_natural(
                            (*recordIterator), valueNodes[0], variable,
                            values[0], sectionName, errorMessage);
                        deployer->boardsInEngine.push_back(
                            str2unsigned(values[0]));
                        deployer->boardHypercubePeriodicity.push_back(false);
                        deployer->boardsAsHypercube = false;
                    }
                    else  /* Hypercube */
                    {
                        /* Complain if multidimensional without hypercube. */
                        if (valueNodes[0]->str != "hypercube")
                        {
                            valid = false;
                            errorMessage->append(dformat(
                                "L%u: The value of the 'boards' variable in "
                                "the engine section is multidimensional, and "
                                "so should be a 'hypercube'.\n",
                                (*recordIterator)->pos));
                        }

                        valid &= complain_if_nodes_values_not_natural(
                            (*recordIterator), valueNodes[0], variable,
                            values[0], sectionName, errorMessage);

                        deployer->boardsAsHypercube = true;
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
                                valid = false;
                                errorMessage->append(dformat(
                                    "L%u: Invalid value for boards variable "
                                    "in the engine section.\n",
                                    (*recordIterator)->pos));
                                deployer->boardHypercubePeriodicity.
                                    push_back(false);
                            }
                        }
                    }
                }
                else if (variable == "external_box_cost")
                {
                    valid &= complain_if_node_value_not_floating(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->costExternalBox = str2float(values[0]);
                }
                else if (variable == "board_board_cost")
                {
                    valid &= complain_if_node_value_not_floating(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->costBoardBoard = str2float(values[0]);
                }
                else
                {
                    valid = false;
                    invalid_variable_message(errorMessage, (*recordIterator),
                                             sectionName, variable);
                }

            }

            else if (sectionName == "box")
            {
                if (variable == "box_board_cost")
                {
                    valid &= complain_if_node_value_not_floating(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->costBoxBoard = str2float(values[0]);
                }
                else if (variable == "supervisor_memory")
                {
                    deployer->boxSupervisorMemory = str2unsigned(values[0]);
                }
                else
                {
                    valid = false;
                    invalid_variable_message(errorMessage, (*recordIterator),
                                             sectionName, variable);
                }
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
                        deployer->mailboxesAsHypercube = false;
                    }
                    else  /* Hypercube */
                    {
                        /* Complain if multidimensional without hypercube. */
                        if (valueNodes[0]->str != "hypercube")
                        {
                            valid = false;
                            errorMessage->append(dformat(
                                "L%u: The value of the 'mailboxes' variable "
                                "in the board section is multidimensional, "
                                "and so should be a 'hypercube'.\n",
                                (*recordIterator)->pos));
                        }

                        deployer->mailboxesAsHypercube = true;
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
                                valid = false;
                                errorMessage->append(dformat(
                                    "L%u: Invalid value for mailboxes "
                                    "variable in the board section.\n",
                                    (*recordIterator)->pos));
                                deployer->mailboxHypercubePeriodicity.
                                    push_back(false);
                            }
                        }
                    }
                }
                else if (variable == "board_mailbox_cost")
                {
                    valid &= complain_if_node_value_not_floating(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->costBoardMailbox = str2float(values[0]);
                }
                else if (variable == "mailbox_mailbox_cost")
                {
                    valid &= complain_if_node_value_not_floating(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->costBoardMailbox = str2float(values[0]);
                }
                else if (variable == "supervisor_memory")
                {
                    valid &= complain_if_node_value_not_natural(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->boardSupervisorMemory = str2unsigned(values[0]);
                }
                else if (variable == "dram")
                {
                    valid &= complain_if_node_value_not_natural(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->dram = str2unsigned(values[0]);
                }
                else
                {
                    valid = false;
                    invalid_variable_message(errorMessage, (*recordIterator),
                                             sectionName, variable);
                }
            }

            else if (sectionName == "mailbox")
            {
                if (variable == "cores")
                {
                    valid &= complain_if_node_value_not_natural(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->coresInMailbox = str2unsigned(values[0]);
                }
                else if (variable == "mailbox_core_cost")
                {
                    valid &= complain_if_node_value_not_floating(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->costMailboxCore = str2float(values[0]);
                }
                else if (variable == "core_core_cost")
                {
                    valid &= complain_if_node_value_not_floating(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->costCoreCore = str2float(values[0]);
                }
                else
                {
                    valid = false;
                    invalid_variable_message(errorMessage, (*recordIterator),
                                             sectionName, variable);
                }
            }

            else if (sectionName == "core")
            {
                if (variable == "threads")
                {
                    valid &= complain_if_node_value_not_natural(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->threadsInCore = str2unsigned(values[0]);
                }
                else if (variable == "instruction_memory")
                {
                    valid &= complain_if_node_value_not_natural(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->instructionMemory = str2unsigned(values[0]);
                }
                else if (variable == "data_memory")
                {
                    valid &= complain_if_node_value_not_natural(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->dataMemory = str2unsigned(values[0]);
                }
                else if (variable == "core_thread_cost")
                {
                    valid &= complain_if_node_value_not_floating(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->costCoreThread = str2float(values[0]);
                }
                else if (variable == "thread_thread_cost")
                {
                    valid &= complain_if_node_value_not_floating(
                        (*recordIterator), valueNodes[0], variable, values[0],
                        sectionName, errorMessage);
                    deployer->costThreadThread = str2float(values[0]);
                }
                else
                {
                    valid = false;
                    invalid_variable_message(errorMessage, (*recordIterator),
                                             sectionName, variable);
                }
            }

            else
            {
                valid = false;
                errorMessage->append(dformat(
                    "L%u: Section name '%s' is not recognised by the parser. "
                    "Is it valid?\n",
                    (*sectionIterator)->pos, sectionName.c_str()));
            }
        }
    }

    return valid;
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
bool HardwareFileParser::d1_validate_section_contents(
    std::string* errorMessage)
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
bool HardwareFileParser::d1_validate_sections(std::string* errorMessage)
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
            errorMessage->append(dformat(
                "    %s (L%i)\n",
                (*invalidNamedNodeIterator)->str.c_str(),
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
