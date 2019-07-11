/* Defines behaviour for the dialect 3 functionality of the hardware model file
 * parser (see the accompanying header for further information). */

#include "HardwareFileParser.h"

/* Create some cores and threads, and pile them all into a mailbox.
 *
 * Returns true if there were no validation problems, and false
 * otherwise. Arguments:
 *
 * - mailbox: Pointer to the mailbox to populate.
 * - quatity: Number of cores to create (the number of threads will be
         determined from the UIF parse tree. */
bool HardwareFileParser::d3_create_cores_and_threads_for_mailbox(
    P_mailbox* mailbox, unsigned coreQuantity)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */
    std::string sectionName = "core";

    /* Create all of the cores, and add them to the mailbox. Don't validate the
     * address component (but still catch if we go out of bounds). */
    P_core* tmpCore;
    AddressComponent coreIndex;
    for (coreIndex = 0; coreIndex < coreQuantity; coreIndex++)
    {
        tmpCore = new P_core(dformat("Core %u", coreIndex));
        try
        {
            mailbox->contain(coreIndex, tmpCore);
        }
        catch (OrchestratorException &e)
        {
            d3_errors.append("%s\n", e.message.c_str());
            anyErrors = true;
        }
    }

    /* Valid fields for the core section. All fields are mandatory. */
    std::vector<std::string> validFields;
    std::vector<std::string>::iterator fieldIterator;
    validFields.push_back("core_thread_cost");
    validFields.push_back("data_memory");
    validFields.push_back("instruction_memory");
    validFields.push_back("thread_thread_cost");
    validFields.push_back("threads");

    /* Holds fields we've already grabbed (for validation purposes). */
    std::map<std::string, bool> fieldsFound;
    for (fieldIterator=validFields.begin(); fieldIterator!=validFields.end();
         fieldIterator++)
    {
        fieldsFound.insert(std::make_pair(*fieldIterator, false));
    }

    /* Temporary staging vectors for holding value nodes and variable
     * nodes. */
    std::vector<UIF::Node*> valueNodes;
    std::vector<UIF::Node*> variableNodes;

    /* For iterating through cores in a mailbox, and threads, later. */
    std::map<AddressComponent, P_core*>::iterator coreIterator;
    unsigned threadIndex;
    P_thread* tmpThread;

    /* Iterate through all record nodes in this section, and apply properties
     * to all cores within. */
    std::vector<UIF::Node*> recordNodes;
    std::vector<UIF::Node*>::iterator recordIterator;
    std::string variableName;
    bool isRecordValid;
    GetRecd(untypedSections[sectionName], recordNodes);
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        isRecordValid = true;  /* Innocent until proven guilty. */

        /* Get the value and variable nodes. */
        GetVari((*recordIterator), variableNodes);
        GetValu((*recordIterator), valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Complain if (in order):
         *
         * - The record does not begin with a "+", as all fields in this
         *   section must do.
         * - The variable name is not a valid name.
         * - There is more than one variable node.
         * - There is more than one value node. */
        isRecordValid &= complain_if_node_not_plus_prefixed(
            *recordIterator, variableNodes[0], sectionName, &d3_errors);
        isRecordValid &= complain_if_variable_name_invalid(
            *recordIterator, variableNodes[0], &validFields, sectionName,
            &d3_errors);
        isRecordValid &= complain_if_record_is_multivariable(
            *recordIterator, &variableNodes, sectionName, &d3_errors);
        isRecordValid &= complain_if_record_is_multivalue(
            *recordIterator, &valueNodes, variableNodes[0]->str, sectionName,
            &d3_errors);
        if (!isRecordValid)
        {
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        variableName = variableNodes[0]->str;
        if (complain_if_node_variable_true_in_map(
                *recordIterator, variableNodes[0], &fieldsFound, sectionName,
                &d3_errors))
        {
            anyErrors = true;
            continue;
        }
        fieldsFound[variableName] = true;

        /* Specific logic for each variable. */
        if (variableName == "core_thread_cost")
        {
            /* Complain if not a float. */
            isRecordValid &= complain_if_node_value_not_floating(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Bind! */
            for (coreIterator=mailbox->P_corem.begin();
                 coreIterator!=mailbox->P_corem.end(); coreIterator++)
            {
                coreIterator->second->costCoreThread = \
                    str2unsigned(valueNodes[0]->str);
            }
        }

        else if (variableName == "data_memory")
        {
            /* Complain if not natural */
            isRecordValid &= complain_if_node_value_not_natural(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Bind! */
            for (coreIterator=mailbox->P_corem.begin();
                 coreIterator!=mailbox->P_corem.end(); coreIterator++)
            {
                coreIterator->second->dataMemory = \
                    str2unsigned(valueNodes[0]->str);
            }
        }

        else if (variableName == "instruction_memory")
        {
            /* Complain if not natural */
            isRecordValid &= complain_if_node_value_not_natural(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Bind! */
            for (coreIterator=mailbox->P_corem.begin();
                 coreIterator!=mailbox->P_corem.end(); coreIterator++)
            {
                coreIterator->second->instructionMemory = \
                    str2unsigned(valueNodes[0]->str);
            }
        }

        else if (variableName == "thread_thread_cost")
        {
            /* Complain if not a float. */
            isRecordValid &= complain_if_node_value_not_floating(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Bind! */
            for (coreIterator=mailbox->P_corem.begin();
                 coreIterator!=mailbox->P_corem.end(); coreIterator++)
            {
                coreIterator->second->costThreadThread = \
                    str2unsigned(valueNodes[0]->str);
            }
        }

        else if (variableName == "threads")
        {
            /* Complain if not natural */
            isRecordValid &= complain_if_node_value_not_natural(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Create that many threads on each core, without validating the
             * address component (but still catch if we go out of bounds). */
            for (coreIterator=mailbox->P_corem.begin();
                 coreIterator!=mailbox->P_corem.end(); coreIterator++)
            {
                for (threadIndex = 0;
                     threadIndex < str2unsigned(valueNodes[0]->str);
                     threadIndex++)
                {
                    tmpThread = new P_thread(dformat("Thread %u",
                                                     threadIndex));
                    try
                    {
                        coreIterator->second->contain(threadIndex, tmpThread);
                    }
                    catch (OrchestratorException &e)
                    {
                        d3_errors.append("%s\n", e.message.c_str());
                        anyErrors = true;
                        break;
                    }
                }
            }
        }


        /* Shouldn't be able to enter this, because we've already checked the
         * variable names, but why not write some more code. It's not like this
         * file is big enough already. */
        else
        {
            d3_errors.append(dformat(
                "L%u: Variable name '%s' is not valid in the '%s' section.\n",
                (*recordIterator)->pos, variableName.c_str(),
                sectionName.c_str()));
            anyErrors = true;
        }
    }

    /* Ensure mandatory fields have been defined. */
    anyErrors |= !complain_if_mandatory_field_not_defined(
        &validFields, &fieldsFound, sectionName, &d3_errors);

    return !anyErrors;
}

/* Define the fields of a board from a typed section.
 *
 * Returns true if there were no validation problems, and false
 * otherwise. Arguments:
 *
 * - box: Pointer to the box to populate.
 * - sectionNode: The node that defines the properties of the board. */
bool HardwareFileParser::d3_define_board_fields_from_section(
    P_board* board, UIF::Node* sectionNode)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */

    /* Short on time, sorry... */
    std::string sectionName = dformat(
        "%s(%s)", sectionNode->leaf[0]->leaf[0]->str.c_str(),
        sectionNode->leaf[0]->leaf[0]->leaf[0]->str.c_str());

    /* Valid fields for board sections */
    std::vector<std::string> validFields;
    std::vector<std::string>::iterator fieldIterator;
    validFields.push_back("board_mailbox_cost");
    validFields.push_back("dram");
    validFields.push_back("mailbox_mailbox_cost");
    validFields.push_back("supervisor_memory");
    validFields.push_back("type");

    /* Mandatory fields for board sections.. */
    std::vector<std::string> mandatoryFields;
    mandatoryFields.push_back("board_mailbox_cost");
    mandatoryFields.push_back("dram");
    mandatoryFields.push_back("mailbox_mailbox_cost");
    mandatoryFields.push_back("supervisor_memory");

    /* Holds fields we've already grabbed (for validation purposes). */
    std::map<std::string, bool> fieldsFound;
    for (fieldIterator=validFields.begin(); fieldIterator!=validFields.end();
         fieldIterator++)
    {
        fieldsFound.insert(std::make_pair(*fieldIterator, false));
    }

    /* Temporary staging vectors for holding value nodes and variable
     * nodes. */
    std::vector<UIF::Node*> valueNodes;
    std::vector<UIF::Node*> variableNodes;

    /* Iterate through all record nodes in this section. */
    std::vector<UIF::Node*> recordNodes;
    std::vector<UIF::Node*>::iterator recordIterator;
    std::string variableName;
    bool isRecordValid;
    GetRecd(sectionNode, recordNodes);
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        isRecordValid = true;  /* Innocent until proven guilty. */

        /* Get the value and variable nodes. */
        GetVari((*recordIterator), variableNodes);
        GetValu((*recordIterator), valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Is this a variable definition? (does it have variables, values, and
         * a '+' prefix)? If not, ignore it (for now). */
        if (valueNodes.size() == 0 or variableNodes.size() == 0 or
            variableNodes[0]->qop != Lex::Sy_plus){continue;}

        /* Complain if (in order):
         *
         * - The variable name is not a valid name.
         * - There is more than one variable node.
         * - There is more than one value node. */
        isRecordValid &= complain_if_variable_name_invalid(
            *recordIterator, variableNodes[0], &validFields, sectionName,
            &d3_errors);
        isRecordValid &= complain_if_record_is_multivariable(
            *recordIterator, &variableNodes, sectionName, &d3_errors);
        isRecordValid &= complain_if_record_is_multivalue(
            *recordIterator, &valueNodes, variableNodes[0]->str, sectionName,
            &d3_errors);
        if (!isRecordValid)
        {
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        variableName = variableNodes[0]->str;
        if (complain_if_node_variable_true_in_map(
                *recordIterator, variableNodes[0], &fieldsFound, sectionName,
                &d3_errors))
        {
            anyErrors = true;
            continue;
        }
        fieldsFound[variableName] = true;

        /* Specific logic for each variable. */
        if (variableName == "board_mailbox_cost")
        {
            /* Validate */
            isRecordValid &= complain_if_node_value_not_floating(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Bind */
            board->costBoardMailbox = str2float(valueNodes[0]->str);
        }

        else if (variableName == "dram")
        {
            /* Validate */
            isRecordValid &= complain_if_node_value_not_natural(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Bind */
            board->dram = str2unsigned(valueNodes[0]->str);
        }

        else if (variableName == "mailbox_mailbox_cost")
        {
            /* Validate */
            isRecordValid &= complain_if_node_value_not_floating(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Hold for later use */
            defaultMailboxMailboxCost = str2float(valueNodes[0]->str);
            isDefaultMailboxCostDefined = true;
        }

        else if (variableName == "supervisor_memory")
        {
            /* Validate */
            isRecordValid &= complain_if_node_value_not_natural(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Bind */
            board->supervisorMemory = str2unsigned(valueNodes[0]->str);
        }

        /* Types are processed in d3_get_validate_default_types, so we
         * ignore the definition here. */
        else if (variableName == "type");

        /* Shouldn't be able to enter this, because we've already checked the
         * variable names, but why not write some more code. It's not like this
         * file is big enough already. */
        else
        {
            d3_errors.append(dformat(
                "L%u: Variable name '%s' is not valid in the '%s' section.\n",
                (*recordIterator)->pos, variableName.c_str(),
                sectionName.c_str()));
            anyErrors = true;
        }
    }

    /* Ensure mandatory fields have been defined. */
    anyErrors |= !complain_if_mandatory_field_not_defined(
        &mandatoryFields, &fieldsFound, sectionName, &d3_errors);

    return !anyErrors;
}

/* Define the fields of a box from a typed section.
 *
 * Returns true if there were no validation problems, and false
 * otherwise. Arguments:
 *
 * - box: Pointer to the box to populate.
 * - sectionNode: The node that defines the properties of the box. */
bool HardwareFileParser::d3_define_box_fields_from_section(
    P_box* box, UIF::Node* sectionNode)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */

    /* Short on time, sorry...*/
    std::string sectionName = dformat(
        "%s(%s)", sectionNode->leaf[0]->leaf[0]->str.c_str(),
        sectionNode->leaf[0]->leaf[0]->leaf[0]->str.c_str());

    /* Valid fields for the box section (all are mandatory). */
    std::vector<std::string> validFields;
    std::vector<std::string>::iterator fieldIterator;
    validFields.push_back("box_board_cost");
    validFields.push_back("supervisor_memory");

    /* Holds fields we've already grabbed (for validation purposes). */
    std::map<std::string, bool> fieldsFound;
    for (fieldIterator=validFields.begin(); fieldIterator!=validFields.end();
         fieldIterator++)
    {
        fieldsFound.insert(std::make_pair(*fieldIterator, false));
    }

    /* Temporary staging vectors for holding value nodes and variable
     * nodes. */
    std::vector<UIF::Node*> valueNodes;
    std::vector<UIF::Node*> variableNodes;

    /* Iterate through all record nodes in this section. */
    std::vector<UIF::Node*> recordNodes;
    std::vector<UIF::Node*>::iterator recordIterator;
    std::string variableName;
    bool isRecordValid;
    GetRecd(sectionNode, recordNodes);
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        isRecordValid = true;  /* Innocent until proven guilty. */

        /* Get the value and variable nodes. */
        GetVari((*recordIterator), variableNodes);
        GetValu((*recordIterator), valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Complain if (in order):
         *
         * - The record does not begin with a "+", as all fields in this
         *   section must do.
         * - The variable name is not a valid name.
         * - There is more than one variable node.
         * - There is more than one value node. */
        isRecordValid &= complain_if_node_not_plus_prefixed(
            *recordIterator, variableNodes[0], sectionName, &d3_errors);
        isRecordValid &= complain_if_variable_name_invalid(
            *recordIterator, variableNodes[0], &validFields, sectionName,
            &d3_errors);
        isRecordValid &= complain_if_record_is_multivariable(
            *recordIterator, &variableNodes, sectionName, &d3_errors);
        isRecordValid &= complain_if_record_is_multivalue(
            *recordIterator, &valueNodes, variableNodes[0]->str, sectionName,
            &d3_errors);
        if (!isRecordValid)
        {
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        variableName = variableNodes[0]->str;
        if (complain_if_node_variable_true_in_map(
                *recordIterator, variableNodes[0], &fieldsFound, sectionName,
                &d3_errors))
        {
            anyErrors = true;
            continue;
        }
        fieldsFound[variableName] = true;

        /* Specific logic for each variable. */
        if (variableName == "box_board_cost")
        {
            /* Validate */
            isRecordValid &= complain_if_node_value_not_floating(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Bind */
            box->costBoxBoard = str2float(valueNodes[0]->str);
        }

        else if (variableName == "supervisor_memory")
        {
            /* Validate */
            isRecordValid &= complain_if_node_value_not_natural(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Bind */
            box->supervisorMemory = str2unsigned(valueNodes[0]->str);
        }

        /* Shouldn't be able to enter this, because we've already checked the
         * variable names, but why not write some more code. It's not like this
         * file is big enough already. */
        else
        {
            d3_errors.append(dformat(
                "L%u: Variable name '%s' is not valid in the '%s' section.\n",
                (*recordIterator)->pos, variableName.c_str(),
                sectionName.c_str()));
            anyErrors = true;
        }
    }

    /* Ensure mandatory fields have been defined. */
    anyErrors |= !complain_if_mandatory_field_not_defined(
        &validFields, &fieldsFound, sectionName, &d3_errors);

    return !anyErrors;
}

/* Define the fields of a mailbox from a typed section, and add cores and
 * threads.
 *
 * Returns true if there were no validation problems, and false
 * otherwise. Arguments:
 *
 * - mailbox: Pointer to the mailbox to populate
 * - sectionNode: The node that defines the properties of the mailbox. */
bool HardwareFileParser::d3_define_mailbox_fields_from_section(
    P_mailbox* mailbox, UIF::Node* sectionNode)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */

    /* Short on time, sorry... */
    std::string sectionName = dformat(
        "%s(%s)", sectionNode->leaf[0]->leaf[0]->str.c_str(),
        sectionNode->leaf[0]->leaf[0]->leaf[0]->str.c_str());

    /* Valid fields for mailbox sections (all are mandatory). */
    std::vector<std::string> validFields;
    std::vector<std::string>::iterator fieldIterator;
    validFields.push_back("core_core_cost");
    validFields.push_back("mailbox_core_cost");
    validFields.push_back("cores");

    /* Holds fields we've already grabbed (for validation purposes). */
    std::map<std::string, bool> fieldsFound;
    for (fieldIterator=validFields.begin(); fieldIterator!=validFields.end();
         fieldIterator++)
    {
        fieldsFound.insert(std::make_pair(*fieldIterator, false));
    }

    /* Temporary staging vectors for holding value nodes and variable
     * nodes. */
    std::vector<UIF::Node*> valueNodes;
    std::vector<UIF::Node*> variableNodes;

    /* Iterate through all record nodes in this section. */
    std::vector<UIF::Node*> recordNodes;
    std::vector<UIF::Node*>::iterator recordIterator;
    std::string variableName;
    bool isRecordValid;
    GetRecd(sectionNode, recordNodes);
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        isRecordValid = true;  /* Innocent until proven guilty. */

        /* Get the value and variable nodes. */
        GetVari((*recordIterator), variableNodes);
        GetValu((*recordIterator), valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Complain if (in order):
         *
         * - The record does not begin with a "+", as all fields in this
         *   section must do.
         * - The variable name is not a valid name.
         * - There is more than one variable node.
         * - There is more than one value node. */
        isRecordValid &= complain_if_node_not_plus_prefixed(
            *recordIterator, variableNodes[0], sectionName, &d3_errors);
        isRecordValid &= complain_if_variable_name_invalid(
            *recordIterator, variableNodes[0], &validFields, sectionName,
            &d3_errors);
        isRecordValid &= complain_if_record_is_multivariable(
            *recordIterator, &variableNodes, sectionName, &d3_errors);
        isRecordValid &= complain_if_record_is_multivalue(
            *recordIterator, &valueNodes, variableNodes[0]->str, sectionName,
            &d3_errors);
        if (!isRecordValid)
        {
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        variableName = variableNodes[0]->str;
        if (complain_if_node_variable_true_in_map(
                *recordIterator, variableNodes[0], &fieldsFound, sectionName,
                &d3_errors))
        {
            anyErrors = true;
            continue;
        }
        fieldsFound[variableName] = true;

        /* Specific logic for each variable. */
        if (variableName == "core_core_cost")
        {
            /* Validate */
            isRecordValid &= complain_if_node_value_not_floating(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Bind */
            mailbox->costCoreCore = str2float(valueNodes[0]->str);
        }

        else if (variableName == "cores")
        {
            /* Validate */
            isRecordValid &= complain_if_node_value_not_natural(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Create and add that many cores. */
            if (!d3_create_cores_and_threads_for_mailbox(
                    mailbox, str2unsigned(valueNodes[0]->str)))
            {
                anyErrors = true;
                continue;
            }
        }

        else if (variableName == "mailbox_core_cost")
        {
            /* Validate */
            isRecordValid &= complain_if_node_value_not_floating(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Bind */
            mailbox->costMailboxCore = str2float(valueNodes[0]->str);
        }

        /* Shouldn't be able to enter this, because we've already checked the
         * variable names, but why not write some more code. It's not like this
         * file is big enough already. */
        else
        {
            d3_errors.append(dformat(
                "L%u: Variable name '%s' is not valid in the '%s' section.\n",
                (*recordIterator)->pos, variableName.c_str(),
                sectionName.c_str()));
            anyErrors = true;
        }
    }

    /* Ensure mandatory fields have been defined. */
    anyErrors |= !complain_if_mandatory_field_not_defined(
        &validFields, &fieldsFound, sectionName, &d3_errors);

    return !anyErrors;
}

/* Extract the address (addr) from an item definition.
 *
 * Concatenates and drops the address into 'address', and sets it to zero if
 * there was no address defined. Does no validation (which is a mistake, but
 * I'm tight for time). Returns true if an address was found, and false
 * otherwise. Arguments:
 *
 * - itemNode: Variable node to extract the type from.
 * - address: Integer to write to. */
bool HardwareFileParser::d3_get_address_from_item_definition(
    UIF::Node* itemNode, AddressComponent* address)
{
    std::string fullBinary = "";
    *address = 0;

    /* Get the "addr" field from the item, if any. */
    std::vector<UIF::Node*>::iterator leafIterator = itemNode->leaf.begin();
    while (leafIterator!=itemNode->leaf.end())
    {
        if ((*leafIterator)->str == "addr"){break;}
        leafIterator++;
    }

    /* If we didn't find it, return. */
    if (leafIterator == itemNode->leaf.end()){return false;}

    /* Is there actually an address in there? If not, return. */
    if ((*leafIterator)->leaf.size() == 0){return false;}

    /* Grab and concatenate, assuming they're all innocent (no time for
     * validation). */
    std::vector<UIF::Node*>::iterator lLeafIterator;  /* Sorry */
    for (lLeafIterator=(*leafIterator)->leaf.begin();
         lLeafIterator!=(*leafIterator)->leaf.end(); lLeafIterator++)
    {
        fullBinary.append((*lLeafIterator)->str);
    }

    *address = std::stoi(fullBinary.c_str(), 0, 2);
    return true;
}

/* Extract the compound board name from a node, and validate it. Returns true
 * if valid, and false if invalid or if the box is missing (while writing an
 * angry letter to your manager).
 *
 * Board names are compound names, with box and board components. Arguments:
 *
 * - itemNode: The node (variable, or value).
 * - boardName: BoardName to populate. */
bool HardwareFileParser::d3_get_board_name(UIF::Node* itemNode,
                                           BoardName* boardName)
{
    /* Clear the board name. */
    boardName->first = "";
    boardName->second = "";

    /* Get the box component, and verify a box exists with that name
     * already. */
    std::string boxName = itemNode->str;
    std::map<std::string, P_box*>::iterator boxNameFinder;
    boxNameFinder = boxFromName.find(boxName);
    if(boxNameFinder == boxFromName.end())
    {
        d3_errors.append(dformat("L%u: Box component of board name '%s' does "
                                 "not correspond to an existing box.\n",
                                 itemNode->pos, boxName.c_str()));
        return false;
    }

    /* Get the board component, through some dilligent searching... */
    std::vector<UIF::Node*>::iterator leafIterator;
    for(leafIterator=itemNode->leaf.begin();
        leafIterator!=itemNode->leaf.end(); leafIterator++)
    {
        if ((*leafIterator)->str == "board"){break;}
    }

    if (leafIterator == itemNode->leaf.end())
    {
        d3_errors.append(dformat("L%u: Couldn't find a board field in this "
                                 "board definition (so I don't know what the "
                                 "name is).\n", itemNode->pos));
        return false;
    }

    /* Does the board component have zero or multiple values? If so, that's not
     * valid. */
    if ((*leafIterator)->leaf.size() != 1)
    {
        d3_errors.append(dformat("L%u: Multiple board components defined for "
                                 "the name of this board (only one is "
                                 "allowed).\n", itemNode->pos));
        return false;
    }

    /* Is the board-component of the name valid? */
    if (!complain_if_node_variable_not_a_valid_item_name(
            itemNode, (*leafIterator)->leaf[0], &d3_errors))
    {
        return false;
    }

    /* All good, assign and return. */
    boardName->first = boxName;
    boardName->second = (*leafIterator)->leaf[0]->str;
    return true;
}

/* Extract the cost from an edge definition.
 *
 * Drops the value into 'cost', and sets it to -1 if there was no cost defined,
 * or if the cost was invalid. Returns true if a cost was found, and false
 * otherwise. Arguments:
 *
 * - itemNode: Value node to extract the edge cost from.
 * - cost: String to write the type to. */
bool HardwareFileParser::d3_get_explicit_cost_from_edge_definition(
    UIF::Node* itemNode, float* cost)
{
    *cost = -1;

    /* Get the "cost" field from the item, if any. */
    std::vector<UIF::Node*>::iterator leafIterator = itemNode->leaf.begin();
    while (leafIterator!=itemNode->leaf.end())
    {
        if ((*leafIterator)->str == "cost"){break;}
        leafIterator++;
    }

    /* If we didn't find it, return. */
    if (leafIterator == itemNode->leaf.end()){return false;}

    /* Is there actually a cost in there? If not, return. */
    if ((*leafIterator)->leaf.size() == 0){return false;}

    /* Is the first entry a float? Leave with true otherwise (the value was
     * found, it just was not valid). */
    if (!is_node_value_floating((*leafIterator)->leaf[0])){return true;}

    /* Bind the cost with the first entry. */
    *cost = str2float((*leafIterator)->leaf[0]->str);
    return true;
}

/* Extract the type from an item definition.
 *
 * Drops the type into 'type', and clears it if there is no type defined. Does
 * no validation. Returns true if a type was found, and false
 * otherwise. Arguments:
 *
 * - itemNode: Variable node to extract the type from.
 * - type: String to write the type to. */
bool HardwareFileParser::d3_get_explicit_type_from_item_definition(
    UIF::Node* itemNode, std::string* type)
{
    type->clear();

    /* Get the "type" field from the item, if any. */
    std::vector<UIF::Node*>::iterator leafIterator = itemNode->leaf.begin();
    while (leafIterator!=itemNode->leaf.end())
    {
        if ((*leafIterator)->str == "type"){break;}
        leafIterator++;
    }

    /* If we didn't find it, return. */
    if (leafIterator == itemNode->leaf.end()){return false;}

    /* Is there actually a type in there? If not, return. */
    if ((*leafIterator)->leaf.size() == 0){return false;}

    /* Bind the type with the first entry. */
    *type = (*leafIterator)->leaf[0]->str;
    return true;
}

/* Extract the (non-compound) mailbox name from a node, and validate it. Is
 * analogous to d3_get_board_name.
 *
 * Returns true if valid, and false if invalid while writing to the error*
 * string. Arguments:
 *
 * - itemNode: The node (variable, or value).
 * - mailboxName: MailboxName to populate. */
bool HardwareFileParser::d3_get_mailbox_name(UIF::Node* itemNode,
                                             MailboxName* mailboxName)
{
    /* Clear the mailbox name. */
    *mailboxName = "";

    /* Get the mailbox name, and check that it's valid. */
    if (!complain_if_node_variable_not_a_valid_item_name(itemNode, itemNode,
                                                         &d3_errors))
    {
        return false;
    }

    /* All good, assign and return. */
    *mailboxName = itemNode->str;  /* It's really that simple. */
    return true;
}

/* Find the section node corresponding to the type of an item. Requires the
 * typedSections to be populated.
 *
 * Returns true if a section could be found, and false otherwise (while writing
 * to the error string). Arguments:
 *
 * - itemType: What kind of item we are dealing with (box, board, mailbox...).
 * - type: Type string to search for.
 * - sourceSectionName: The name of the section the record lives in (for
     writing error message).
 * - lineNumber: Line number of the record (for writing error message).
 * - sectionNode: Pointer to write the found section to, set to PNULL if no
     node could be found. */
bool HardwareFileParser::d3_get_section_from_type(
    std::string itemType, std::string type, std::string sourceSectionName,
    unsigned lineNumber, UIF::Node** sectionNode)
{
    *sectionNode = PNULL;

    /* Iterator to find typed sections. Note that this only refers to the inner
     * map of typedSections. */
    std::map<std::string, UIF::Node*>::iterator typedSectionIterator;

    /* Go hunting. */
    typedSectionIterator = typedSections[itemType].find(type);
    if (typedSectionIterator == typedSections[itemType].end())
    {
        d3_errors.append(dformat("L%u: Type '%s' defined in the '%s' section "
                                 "does not correspond to a section.\n",
                                 lineNumber, type.c_str(),
                                 sourceSectionName.c_str()));
        return false;
    }

    else
    {
        *sectionNode = typedSectionIterator->second;
        return true;
    }
}

/* Load types from the default types section, and validate them.
 *
 * Returns true if all fields were valid and describe sections, and false
 * otherwise. NB: if the [default_types] section does not exist, returns true
 * and does not touch the input array. This function appends errors to
 * d3_errors as it goes. Arguments:
 *
 * - globalDefaults: Pointer to a 3-length array of UIF::Nodes, which*/
bool HardwareFileParser::d3_get_validate_default_types(
    UIF::Node** globalDefaults)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */
    std::string sectionName = "default_types";

    /* Valid fields for the header section. All are optional. */
    std::vector<std::string> validFields;
    std::vector<std::string>::iterator fieldIterator;
    validFields.push_back("box_type");
    validFields.push_back("board_type");
    validFields.push_back("mailbox_type");

    /* Holds fields we've already grabbed (for validation purposes). */
    std::map<std::string, bool> fieldsFound;
    for (fieldIterator=validFields.begin(); fieldIterator!=validFields.end();
         fieldIterator++)
    {
        fieldsFound.insert(std::make_pair(*fieldIterator, false));
    }

    /* Staging vectors for holding value nodes and variable nodes. */
    std::vector<UIF::Node*> valueNodes;
    std::vector<UIF::Node*> variableNodes;

    /* Staging vector and iterator for records. */
    std::vector<UIF::Node*> recordNodes;
    std::vector<UIF::Node*>::iterator recordIterator;
    std::string variableName;
    std::string value;
    bool isRecordValid;

    /* Firstly, is there a [default_types] section? */
    std::map<std::string, UIF::Node*>::iterator untypedSectionsIterator;
    untypedSectionsIterator = untypedSections.find(sectionName);
    if (untypedSectionsIterator == untypedSections.end())
    {
        /* If not, let's go. */
        return true;
    }

    /* The section exists for control to reach here. Get the default types from
     * the [default_types] section. */
    GetRecd(untypedSectionsIterator->second, recordNodes);
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        isRecordValid = true;  /* Innocent until proven guilty. */

        /* Get the value and variable nodes. */
        GetVari((*recordIterator), variableNodes);
        GetValu((*recordIterator), valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Complain if (in order):
         *
         * - The record does not begin with a "+", as all fields in this
         *   section must do.
         * - The variable name is not a valid name.
         * - There is more than one variable node.
         * - There is more than one value node.
         * - The type is not valid. */
        isRecordValid &= complain_if_node_not_plus_prefixed(
            *recordIterator, variableNodes[0], sectionName, &d3_errors);
        isRecordValid &= complain_if_variable_name_invalid(
            *recordIterator, variableNodes[0], &validFields, sectionName,
            &d3_errors);
        isRecordValid &= complain_if_record_is_multivariable(
            *recordIterator, &variableNodes, sectionName, &d3_errors);
        isRecordValid &= complain_if_record_is_multivalue(
            *recordIterator, &valueNodes, variableNodes[0]->str,
            sectionName, &d3_errors);
        isRecordValid &= complain_if_node_value_not_a_valid_type(
            *recordIterator, valueNodes[0], sectionName, &d3_errors);
        if (!isRecordValid)
        {
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        variableName = variableNodes[0]->str;
        if (complain_if_node_variable_true_in_map(
                *recordIterator, variableNodes[0], &fieldsFound, sectionName,
                &d3_errors))
        {
            anyErrors = true;
            continue;
        }
        fieldsFound[variableName] = true;

        /* Specific logic for each variable. */
        std::string itemType;
        unsigned arrayIndex;
        if (variableName == "box_type")
        {
            itemType = "box";
            arrayIndex = box;
        }
        else if (variableName == "board_type")
        {
            itemType = "board";
            arrayIndex = board;
        }
        else if (variableName == "mailbox_type")
        {
            itemType = "mailbox";
            arrayIndex = mailbox;
        }

        /* Shouldn't be able to enter this, because we've already checked the
         * variable names, but why not write some more code. It's not like this
         * file is big enough already. */
        else
        {
            d3_errors.append(dformat(
                "L%u: Variable name '%s' is not valid in the '%s' section.\n",
                (*recordIterator)->pos, variableName.c_str(),
                sectionName.c_str()));
            anyErrors = true;
            continue;
        }

        /* Figure out if there is a section corresponding to this type. If not,
         * whine loudly. Either way, assign to globalDefaults. */
        anyErrors |= !d3_get_section_from_type(
            itemType, valueNodes[0]->str, sectionName, (*recordIterator)->pos,
            &(globalDefaults[arrayIndex]));
    }

    return !anyErrors;
}

/* Validate that the section occurences in the hardware description input file
 * is semantically-valid in dialect 3.
 *
 * The following sections must be defined exactly once (0):
 *
 * - header
 * - packet_address_format
 * - engine_box
 * - engine_board
 * - core
 *
 * The following sections must be defined once or more (1):
 *
 * - box
 * - board
 * - mailbox
 *
 * The following sections must be defined once or less (2):
 *
 * - default_types
 *
 * The following sections must have arguments (and there must not be any
 * duplicate arguments within a section type) (3):
 *
 * - box
 * - board
 * - mailbox
 *
 * The following sections must not have arguments (4):
 *
 * - packet_address_format
 * - engine_box
 * - engine_board
 * - core
 * - default_types
 *
 * No sections with names that are not covered by the above rules can exist
 * (5).
 *
 * Returns true if the conditions above are all true, and false
 * otherwise. */
bool HardwareFileParser::d3_load_validate_sections()
{
    /* Valid section names */
    std::vector<std::string> validSectionNames;
    std::vector<std::string>::iterator nameIterator;
    validSectionNames.push_back("header");
    validSectionNames.push_back("packet_address_format");
    validSectionNames.push_back("engine_box");
    validSectionNames.push_back("engine_board");
    validSectionNames.push_back("core");
    validSectionNames.push_back("box");
    validSectionNames.push_back("board");
    validSectionNames.push_back("mailbox");
    validSectionNames.push_back("default_types");

    /* Valid untyped section names (will use nameIterator to go through
     * them). */
    std::vector<std::string> validUntypedSectionNames;
    validUntypedSectionNames.push_back("header");
    validUntypedSectionNames.push_back("packet_address_format");
    validUntypedSectionNames.push_back("engine_box");
    validUntypedSectionNames.push_back("engine_board");
    validUntypedSectionNames.push_back("core");
    validUntypedSectionNames.push_back("default_types");

    /* Valid typed section names (will use nameIterator to go through them). */
    std::vector<std::string> validTypedSectionNames;
    validTypedSectionNames.push_back("box");
    validTypedSectionNames.push_back("board");
    validTypedSectionNames.push_back("mailbox");

    /* Section names by rule. */
    std::vector<std::vector<std::string>> rules(5, std::vector<std::string>());
    rules[0].push_back("header");
    rules[0].push_back("packet_address_format");
    rules[0].push_back("engine_box");
    rules[0].push_back("engine_board");
    rules[0].push_back("core");

    rules[1].push_back("box");
    rules[1].push_back("board");
    rules[1].push_back("mailbox");

    rules[2].push_back("default_types");

    rules[3].push_back("box");
    rules[3].push_back("board");
    rules[3].push_back("mailbox");

    rules[4].push_back("packet_address_format");
    rules[4].push_back("engine_box");
    rules[4].push_back("engine_board");
    rules[4].push_back("core");
    rules[4].push_back("default_types");
    unsigned currentRule = 0;

    /* Booleans to help with printing. */
    std::vector<bool> ruleFailure(6, false);

    /* Section name nodes, by name. */
    std::map<std::string, std::vector<UIF::Node*>> sectionsByName;
    std::map<std::string, std::vector<UIF::Node*>>::iterator \
        sectionsByNameIterator;
    for (nameIterator=validSectionNames.begin();
         nameIterator!=validSectionNames.end(); nameIterator++)
    {
        /* Map the string to an empty vector. */
        sectionsByName.insert(std::make_pair(*nameIterator,
                                             std::vector<UIF::Node*>()));
    }

    /* For each of the section names found in the file, if it's a valid section
     * name, add it to sectionsByName. If it's not, whine loudly and fail
     * slow. */
    std::vector<UIF::Node*> allNodes;
    std::vector<UIF::Node*>::iterator nodeIterator;
    GetNames(allNodes);
    for (nodeIterator=allNodes.begin();
         nodeIterator!=allNodes.end(); nodeIterator++)
    {
        /* Does this section node match one of the valid patterns? If so, add
         * it to the appropriate vector in sectionsByName. */
        sectionsByNameIterator = sectionsByName.find((*nodeIterator)->str);
        if (sectionsByNameIterator != sectionsByName.end())
        {
            sectionsByNameIterator->second.push_back(*nodeIterator);
        }

        /* Otherwise, it must be invalid (5). */
        else
        {
            /* On the first pass, add this header to the message. */
            if (!ruleFailure[5])
            {
                ruleFailure[5] = true;
                d3_errors.append("Sections with invalid names found in the "
                                 "input file:\n");
            }

            d3_errors.append(dformat("    %s (L%i)\n",
                                     (*nodeIterator)->str.c_str(),
                                     (*nodeIterator)->pos));
        }
    }

    /* Rule (0): For each of the rules[0] strings... */
    for (nameIterator=rules[currentRule].begin();
         nameIterator!=rules[currentRule].end(); nameIterator++)
    {
        if (sectionsByName[*nameIterator].size() != 1)
        {
            /* On the first pass, add this header to the message. */
            if (!ruleFailure[currentRule])
            {
                ruleFailure[currentRule] = true;
                d3_errors.append("The following sections were not defined "
                                 "exactly once:\n");
            }

            /* I'm going to write the line numbers of each node, or "not
             * defined" if the section is not defined. A little unusual because
             * I want the printing to be pretty. */
            d3_errors.append(dformat("    %s (", nameIterator->c_str()));

            if (sectionsByName[*nameIterator].empty())
            {
                d3_errors.append("not defined");
            }

            else
            {
                nodeIterator=sectionsByName[*nameIterator].begin();
                d3_errors.append(dformat("L%u", (*nodeIterator)->pos));
                for (;
                     nodeIterator!=sectionsByName[*nameIterator].end();
                     nodeIterator++)
                {
                    d3_errors.append(dformat(", L%u", (*nodeIterator)->pos));
                }
            }

            d3_errors.append(")\n");
        }
    }

    /* Rule (1) */
    currentRule++;
    for (nameIterator=rules[currentRule].begin();
         nameIterator!=rules[currentRule].end(); nameIterator++)
    {
        if (sectionsByName[*nameIterator].empty())
        {
            /* On the first pass, add this header to the message. */
            if (!ruleFailure[currentRule])
            {
                ruleFailure[currentRule] = true;
                d3_errors.append("The following sections were not defined "
                                 "when they should have been defined one time "
                                 "or more:\n");
            }
            d3_errors.append(dformat("    %s\n", nameIterator->c_str()));
        }
    }

    /* Rule (2) */
    currentRule++;
    for (nameIterator=rules[currentRule].begin();
         nameIterator!=rules[currentRule].end(); nameIterator++)
    {
        if (sectionsByName[*nameIterator].size() >= 2)
        {
            /* On the first pass, add this header to the message. */
            if (!ruleFailure[currentRule])
            {
                ruleFailure[currentRule] = true;
                d3_errors.append("The following sections were defined more "
                                 "than once, when they should have been "
                                 "defined one or zero times:\n");
            }

            /* I'm going to write the line numbers of each node. */
            d3_errors.append(dformat("    %s (", nameIterator->c_str()));
            nodeIterator=sectionsByName[*nameIterator].begin();
            d3_errors.append(dformat("L%u", (*nodeIterator)->pos));
            for (;
                 nodeIterator!=sectionsByName[*nameIterator].end();
                 nodeIterator++)
            {
                d3_errors.append(dformat(", L%u", (*nodeIterator)->pos));
            }
            d3_errors.append(")\n");
        }
    }

    /* Rule (3) */
    std::vector<std::string> declaredTypes;
    std::vector<UIF::Node*> arguments;
    std::string argument;
    currentRule++;
    for (nameIterator=rules[currentRule].begin();
         nameIterator!=rules[currentRule].end(); nameIterator++)
    {
        declaredTypes.clear();
        for (nodeIterator=sectionsByName[*nameIterator].begin();
             nodeIterator!=sectionsByName[*nameIterator].end(); nodeIterator++)
        {
            /* Get arguments */
            arguments.clear();
            GetSub(*nodeIterator, arguments);

            /* If argument is empty, or there is no argument, or argument
             * doesn't satisfy [0-9A-Za-z]{2,32}, then add to error message. */
            if (arguments.empty())
            {
                ruleFailure[currentRule] = true;
                d3_errors.append(dformat("L%u: Section '%s' has no associated "
                                         "type.\n", (*nodeIterator)->pos,
                                         (*nameIterator).c_str()));
            }

            else if (arguments.size() > 1)
            {
                ruleFailure[currentRule] = true;
                d3_errors.append(dformat("L%u: Section '%s' has more than one "
                                         "type associated with it.\n",
                                         (*nodeIterator)->pos,
                                         (*nameIterator).c_str()));
            }

            else if (!is_type_valid(arguments[0]))
            {
                ruleFailure[currentRule] = true;
                d3_errors.append(dformat(
                    "L%u: Type '%s' in section '%s' is not a valid type (it "
                    "must satisfy %s).\n", (*nodeIterator)->pos,
                    arguments[0]->str.c_str(), (*nameIterator).c_str(),
                    TYPE_REGEX));
            }

            /* If we've already seen this type defined for a section of this
             * sort (e.g. 'board' or 'mailbox'), then add to error message. */
            else if (std::find(declaredTypes.begin(), declaredTypes.end(),
                               arguments[0]->str) != declaredTypes.end())
            {
                ruleFailure[currentRule] = true;
                d3_errors.append(dformat(
                    "L%u: Duplicate definition of section '%s' with type "
                    "'%s'.\n", (*nodeIterator)->pos, (*nameIterator).c_str(),
                    arguments[0]->str));
            }

            else
            {
                declaredTypes.push_back(arguments[0]->str);
            }
        }
    }

    /* Rule (4) */
    currentRule++;
    for (nameIterator=rules[currentRule].begin();
         nameIterator!=rules[currentRule].end(); nameIterator++)
    {
        for (nodeIterator=sectionsByName[*nameIterator].begin();
             nodeIterator!=sectionsByName[*nameIterator].end(); nodeIterator++)
        {
            /* Get arguments */
            arguments.clear();
            GetSub(*nodeIterator, arguments);

            /* If there is an argument, then add to error message. */
            if (!arguments.empty())
            {
                ruleFailure[currentRule] = true;
                d3_errors.append(dformat(
                    "L%u: Section '%s' has a type, when it should not.\n",
                    (*nodeIterator)->pos, *nameIterator));
            }
        }
    }

    /* If none of the rules were broken, populate untypedSections and
     * typedSections to facilitate better lookups later.
     *
     * We've been dealing only with section name nodes so far, but we need to
     * store section nodes, so there's a bit of FndSect action here.
     *
     * We populate these structures after validating because the logic becomes
     * overly complicated if section nodes do not have the correct number of
     * name arguments. Once we're here, we can be confident that each section
     * that has arguments has the appropriate number of them. It's not too
     * expensive to do a second pass over the sections, because there are few
     * of them. */
    UIF::Node* sectionNode;
    if (std::find(ruleFailure.begin(), ruleFailure.end(), true) == \
        ruleFailure.end())
    {
        /* Deal with the untyped sections first. */
        for (nameIterator=validUntypedSectionNames.begin();
             nameIterator!=validUntypedSectionNames.end(); nameIterator++)
        {
            /* Don't worry if there is no section name corresponding to this
             * expected name; untyped sections may exist once or zero times
             * (depending on the name of the section we're dealing with). */
            if (!sectionsByName[*nameIterator].empty())
            {
                /* Get the section node from the section name node. */
                sectionNode = FndSect(sectionsByName[*nameIterator][0]);

                /* Store the section node. */
                untypedSections.insert(std::make_pair(*nameIterator,
                                                      sectionNode));
            }
        }

        /* Typed sections! */
        for (nameIterator=validTypedSectionNames.begin();
             nameIterator!=validTypedSectionNames.end(); nameIterator++)
        {
            /* First time here, so we need to create the submap for this sort
             * of section. */
            typedSections.insert(
                std::make_pair(*nameIterator,
                               std::map<std::string, UIF::Node*>()));

            /* There can be many of these, and we want to store them all. */
            for (nodeIterator=sectionsByName[*nameIterator].begin();
                 nodeIterator!=sectionsByName[*nameIterator].end();
                 nodeIterator++)
            {
                /* Get the section node from the section name node. */
                sectionNode = FndSect(sectionsByName[*nameIterator][0]);

                /* Drop it into the submap, indexed by the type of the
                 * section. */
                typedSections[*nameIterator].insert(
                    std::make_pair((*nodeIterator)->leaf[0]->str,
                                   sectionNode));
            }

        }

        /* None of the rules were broken (decided when we entered this
         * conditional block), so the validation passed. */
        return true;
    }

    /* Otherwise, our validation failed, and we have not wasted time populating
     * our data structures. */
    else {return false;}
}

/* Populates a POETS Engine with information from a previously-loaded hardware
 * description file in dialect 3, after validating it. Arguments:
 *
 * - engine: Pointer to the POETS Engine to populate. Cleared if the input file
 *   is valid.
 *
 * Throws if:
 *
 *  - the input file is semantically invalid. */
void HardwareFileParser::d3_populate_hardware_model(P_engine* engine)
{
    /* During validation and provisioning, errors will be written to this
     * string. */
    d3_errors = dformat("Error(s) were encountered while loading the hardware "
                        "description file '%s'. Since the parser fails "
                        "slowly, consider addressing the top-most errors "
                        "first:\n", loadedFile.c_str());

    /* Check sections are defined correctly. Not much point continuing if they
     * are not defined correctly. */
    if (!d3_load_validate_sections())
    {
        delete engine;
        throw HardwareSemanticException(d3_errors.c_str());
    }

    /* Populate the engine with information from the header section. */
    bool passedValidation = true;
    passedValidation &= d3_populate_validate_header(engine);

    /* Populate the hardware address format owned by the engine. */
    passedValidation &= d3_populate_validate_address_format(engine);

    /* Verify that default types, and type fields in sections, are valid, and
     * map to sections that exist. */
    passedValidation &= d3_validate_types_define_cache();

    /* We can't go any further than this without risking a segfault, because if
     * the address format is not defined properly, item address assignment will
     * fail badly. */
    if (!passedValidation)
    {
        delete engine;
        throw HardwareSemanticException(d3_errors.c_str());
    }

    /* Boxes */
    passedValidation &= d3_populate_validate_engine_box(engine);

    /* Boards (and the items beneath) */
    passedValidation &= d3_populate_validate_engine_board_and_below(engine);

    if (!passedValidation)
    {
        delete engine;
        throw HardwareSemanticException(d3_errors.c_str());
    }

}

/* Validate the contents of the packet_address_format section, and populate the
 * format object in the engine with them.
 *
 * Returns true if all validation checks pass, and false otherwise. Arguments:
 *
 * - engine: Engine to populate */
bool HardwareFileParser::d3_populate_validate_address_format(P_engine* engine)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */
    std::string sectionName = "packet_address_format";

    /* Valid fields for the header section (all are mandatory). */
    std::vector<std::string> validFields;
    std::vector<std::string>::iterator fieldIterator;
    validFields.push_back("box");
    validFields.push_back("board");
    validFields.push_back("mailbox");
    validFields.push_back("core");
    validFields.push_back("thread");

    /* Holds fields we've already grabbed (for validation purposes). */
    std::map<std::string, bool> fieldsFound;
    for (fieldIterator=validFields.begin(); fieldIterator!=validFields.end();
         fieldIterator++)
    {
        fieldsFound.insert(std::make_pair(*fieldIterator, false));
    }

    /* Staging vectors for holding value nodes and variable nodes. */
    std::vector<UIF::Node*> valueNodes;
    std::vector<UIF::Node*> variableNodes;

    /* Staging area for the values of multi-valued records. */
    std::vector<std::string> values;
    std::vector<std::string>::iterator valueIterator;

    /* Staging pointer for accumulated values to go (for legitimate
     * multi-valued records) */
    unsigned* accumulationTarget;

    /* Iterate through all record nodes in this section. */
    std::vector<UIF::Node*> recordNodes;
    std::vector<UIF::Node*>::iterator recordIterator;
    std::string variableName;
    bool isRecordValid;
    GetRecd(untypedSections[sectionName], recordNodes);
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        isRecordValid = true;  /* Innocent until proven guilty. */

        /* Get the value and variable nodes. */
        GetVari((*recordIterator), variableNodes);
        GetValu((*recordIterator), valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Complain if (in order):
         *
         * - The record does not begin with a "+", as all fields in this
         *   section must do.
         * - The variable name is not a valid name.
         * - There is more than one variable node. */
        isRecordValid &= complain_if_node_not_plus_prefixed(
            *recordIterator, variableNodes[0], sectionName, &d3_errors);
        isRecordValid &= complain_if_variable_name_invalid(
            *recordIterator, variableNodes[0], &validFields, sectionName,
            &d3_errors);
        isRecordValid &= complain_if_record_is_multivariable(
            *recordIterator, &variableNodes, sectionName, &d3_errors);
        if (!isRecordValid)
        {
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        variableName = variableNodes[0]->str;
        if (complain_if_node_variable_true_in_map(
                *recordIterator, variableNodes[0], &fieldsFound, sectionName,
                &d3_errors))
        {
            anyErrors = true;
            continue;
        }
        fieldsFound[variableName] = true;

        /* Specific logic for each variable. For box, core, and thread complain
         * if (in order):
         *
         * - The record is multi-valued.
         * - The value is not natural. */
        if (variableName == "box" or variableName == "core" or
            variableName == "thread")
        {
            /* Validate */
            isRecordValid &= complain_if_record_is_multivalue(
                *recordIterator, &valueNodes, variableName, sectionName,
                &d3_errors);
            isRecordValid &= complain_if_node_value_not_natural(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* We bind! */
            if (variableName == "box")
            {
                engine->addressFormat.boxWordLength =
                    str2unsigned(valueNodes[0]->str);
            }
            else if (variableName == "core")
            {
                engine->addressFormat.coreWordLength =
                    str2unsigned(valueNodes[0]->str);
            }
            else /* Must be thread */
            {
                engine->addressFormat.threadWordLength =
                    str2unsigned(valueNodes[0]->str);
            }
        }

        /* For board and mailbox, complain if any of the values are not
         * natural. */
        else if (variableName == "board" or variableName == "mailbox")
        {
            /* Validate, treating the single-variable and multiple-variable
             * cases separately. */
            get_values_as_strings(&values, valueNodes[0]);
            if (values.size() == 1)
            {
                isRecordValid = complain_if_node_value_not_natural(
                    *recordIterator, valueNodes[0], variableName, sectionName,
                    &d3_errors);
            }
            else
            {
                isRecordValid = complain_if_nodes_values_not_natural(
                    *recordIterator, valueNodes[0], variableName, sectionName,
                    &d3_errors);
            }
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* We bind! (is either board or mailbox) */

            /* Determine target */
            accumulationTarget =
                &(variableName == "board" ?
                  engine->addressFormat.boardWordLength :
                  engine->addressFormat.mailboxWordLength);
            *accumulationTarget = 0;  /* Reset target */

            /* Add values to target. The total word length is (simply) the sum
             * of its multidimensional constituents. */
            for (valueIterator=values.begin(); valueIterator!=values.end();
                 valueIterator++)
            {
                *accumulationTarget += str2unsigned(*valueIterator);
            }
        }

        /* Shouldn't be able to enter this, because we've already checked the
         * variable names, but why not write some more code. It's not like this
         * file is big enough already. */
        else
        {
            d3_errors.append(dformat(
                "L%u: Variable name '%s' is not valid in the '%s' section.\n",
                (*recordIterator)->pos, variableName.c_str(),
                sectionName.c_str()));
            anyErrors = true;
        }
    }

    /* Ensure mandatory fields have been defined. */
    anyErrors |= !complain_if_mandatory_field_not_defined(
        &validFields, &fieldsFound, sectionName, &d3_errors);

    return !anyErrors;
}

/* Creates and 'contains' mailboxes for this board, as well as the items
 * beneath.
 *
 * Relies on the default-typing structures being populated, as well as the
 * default mailbox-mailbox cost (if there is one). Returns true if all
 * validation checks pass, and false otherwise. Arguments:
 *
 * - board: Board to populate with mailboxes, which are in turn populated with
 *       cores etc.
 * - sectionNode: The node that defines the mailboxes in boards of this
 *       type. */
bool HardwareFileParser::d3_populate_validate_board_with_mailboxes(
    P_board* board, UIF::Node* sectionNode)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */

    /* Short on time, sorry... */
    std::string sectionName = dformat(
        "%s(%s)", sectionNode->leaf[0]->leaf[0]->str.c_str(),
        sectionNode->leaf[0]->leaf[0]->leaf[0]->str.c_str());

    /* Temporary staging vectors for holding value nodes and variable
     * nodes. */
    std::vector<UIF::Node*> valueNodes;
    std::vector<UIF::Node*> variableNodes;

    /* Holds the type of the current record, and the section it refers to (if
     * any). */
    std::string type;
    UIF::Node* sectionType;

    /* Holds the concatenated address of a mailbox. */
    AddressComponent address;

    /* Holds the mailbox while we're creating it. */
    P_mailbox* mailbox;
    MailboxName mailboxName;

    /* Holds mailboxes while connecting them together. */
    std::vector<P_mailbox*> joinedMailboxes;

    /* For finding mailbox names in mailboxInfoFromName. */
    std::map<MailboxName, MailboxInfo>::iterator mailboxNameFinder;

    /* Holds an edge-specific mailbox-mailbox cost, if found. */
    float thisEdgeCost;
    bool isThisEdgeCostDefined;

    /* Holds the name of the mailbox on the other end of an edge. */
    MailboxName edgeMailboxName;

    /* For finding a reverse edge, and for iterating through the edge
     * container. */
    std::map<std::pair<MailboxName, MailboxName>, EdgeInfo>::iterator \
        edgeFinder;

    /* Iterate through all record nodes in this section that define
     * mailboxes. */
    std::vector<UIF::Node*> recordNodes;
    std::vector<UIF::Node*>::iterator recordIterator;
    std::vector<UIF::Node*>::iterator edgeIterator;
    std::string variableName;
    GetRecd(sectionNode, recordNodes);
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        sectionType = PNULL;

        /* Get the value and variable nodes. */
        GetVari((*recordIterator), variableNodes);
        GetValu((*recordIterator), valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Is this a variable definition? (does it have variables, values, and
         * a '+' prefix)? If so, ignore it (it's already been dealt with in
         * d3_define_board_fields_from_section. */
        if (valueNodes.size() != 0 and variableNodes.size() != 0 and
            variableNodes[0]->qop == Lex::Sy_plus){continue;}

        /* Validate and get the name of the mailbox. */
        if (!d3_get_mailbox_name(variableNodes[0], &mailboxName))
        {
            anyErrors = true;
            continue;
        }

        /* Whine if a mailbox with this name already exists in this board. */
        mailboxNameFinder = mailboxInfoFromName.find(mailboxName);
        if (mailboxNameFinder != mailboxInfoFromName.end())
        {
            d3_errors.append(dformat(
                "L%u: Mailbox on this line has already been defined on line "
                "%u. Not making it.\n", (*recordIterator)->pos,
                mailboxNameFinder->second.lineNumber));
            anyErrors = true;
            continue;
        }

        /* Is a type explicitly defined in this record? */
        if (d3_get_explicit_type_from_item_definition(variableNodes[0],
                                                      &type))
        {
            /* If there's a matching section for this type, we're all good (it
             * gets written to sectionType). Otherwise, we fall back to
             * defaults. */
            anyErrors |= !d3_get_section_from_type(
                "mailbox", type, sectionName, (*recordIterator)->pos,
                &sectionType);
        }

        /* If the type it was not defined explicitly, or it was and no section
         * matched with it, get the section from the defaults. */
        if (sectionType == PNULL)
        {
            sectionType = defaultTypes[sectionNode];

            /* If it's still zero, then the type hasn't been defined anywhere
             * "validly". That's an error, folks. */
            if (sectionType == PNULL)
            {
                d3_errors.append(dformat("L%u: No section found to define the "
                                         "mailbox on this line. Not making "
                                         "it.\n", (*recordIterator)->pos));
                anyErrors = true;
                continue;  /* Can't do anything without a type definition... */
            }
        }

        /* Get the address without validating it (boss' orders). */
        d3_get_address_from_item_definition(variableNodes[0], &address);

        /* Create the mailbox (the name argument is the name of the mailbox in
         * the record). */
        mailbox = new P_mailbox(mailboxName);

        /* Into the board with ye! */
        board->contain(address, mailbox);

        /* Track the mailbox by name. */
        mailboxInfoFromName[mailboxName] =
            MailboxInfo{(*recordIterator)->pos, mailbox};

        /* Stage the edges from this record. Values (i.e. LHS of the '=' token)
         * each represent the name of a mailbox, optionally with a cost, which
         * defines an edge */
        for(edgeIterator=valueNodes.begin(); edgeIterator!=valueNodes.end();
            edgeIterator++)
        {
            /* Get the name of the mailbox on the other side of this edge,
             * skipping if invalid. */
            if(!d3_get_mailbox_name(*edgeIterator, &edgeMailboxName))
            {
                anyErrors = true;
                continue;  /* Skip this edge - can't identify either end. */
            }

            /* If the edge explicitly describes a cost, use that. Otherwise,
             * use one we found earlier. Complain if:
             *
             * - neither are defined
             * - the explicit cost is invalid (we checked the default cost
             *   earlier) */
            isThisEdgeCostDefined = d3_get_explicit_cost_from_edge_definition(
                *edgeIterator, &thisEdgeCost);
            if (isThisEdgeCostDefined)
            {
                /* Check for -1, meaning the cost was invalid. */
                if (thisEdgeCost == -1)
                {
                    d3_errors.append(dformat(
                        "L%u: Invalid cost on edge connecting mailbox %s to "
                        "mailbox %s (it must be a float).\n",
                        (*recordIterator)->pos,
                        mailboxName.c_str(), edgeMailboxName.c_str()));
                    anyErrors = true;
                    continue;  /* Skip this edge - meaningless without a
                                * comprehensible cost. */
                }
            }
            else if (isDefaultMailboxCostDefined)
            {
                thisEdgeCost = defaultMailboxMailboxCost;
            }
            else
            {
                d3_errors.append(dformat(
                    "L%u: No cost found for edge connecting mailbox %s to "
                    "mailbox %s (it must be a float).\n",
                    (*recordIterator)->pos, mailboxName.c_str(),
                    edgeMailboxName.c_str()));
                anyErrors = true;
                continue;  /* Skip this edge - meaningless without a cost. */
            }

            /* If the reverse edge is in mailboxEdges... */
            /* Bears reiterating - it's the reverse! */
            edgeFinder = mailboxEdges.find(std::make_pair(edgeMailboxName,
                                                          mailboxName));
            if (edgeFinder != mailboxEdges.end())
            {
                /* Complain if:
                 *
                 * - The reverse-cost is different.
                 * - Reverse is already defined. */
                if (edgeFinder->second.weight != thisEdgeCost)
                {
                    d3_errors.append(dformat(
                        "L%u: The cost of the edge connecting mailbox %s to "
                        "mailbox %s (%f) is different from the cost of its "
                        "reverse (%f), defined at L%i.\n",
                        (*recordIterator)->pos,
                        mailboxName.c_str(),
                        edgeMailboxName.c_str(),
                        thisEdgeCost,
                        edgeFinder->second.weight,
                        edgeFinder->second.lineNumber));
                    anyErrors = true;
                    continue;  /* Skip this edge, avoid clobbering. */
                }

                if (edgeFinder->second.isReverseDefined)
                {
                    d3_errors.append(dformat(
                        "L%u: The other end of the edge connecting mailbox "
                        "%s to mailbox %s has already been defined. The first "
                        "definition was at L%i.\n",
                        (*recordIterator)->pos,
                        mailboxName.c_str(),
                        edgeMailboxName.c_str(),
                        edgeFinder->second.lineNumber));
                    anyErrors = true;
                    continue;  /* Skip this edge, avoid clobbering. */
                }

                /* We're all good, mark the reverse as found (we're on it!). */
                edgeFinder->second.isReverseDefined = true;
            }

            /* Otherwise, track this edge in mailboxEdges. */
            else
            {
                /* But complain if it's already there (means we've defined it
                 * twice on this line, probably). */

                /* NB: Not reverse! */
                edgeFinder = mailboxEdges.find(std::make_pair(
                                                   mailboxName,
                                                   edgeMailboxName));
                if (edgeFinder != mailboxEdges.end())
                {
                    d3_errors.append(dformat(
                        "L%u: Duplicate edge definition connecting mailbox "
                        "%s to mailbox %s.\n",
                        (*recordIterator)->pos,
                        mailboxName.c_str(),
                        edgeMailboxName.c_str()));
                    anyErrors = true;
                    continue;  /* Skip this edge, avoid clobbering. */
                }

                /* Okay, actually add it now. */
                mailboxEdges[std::make_pair(mailboxName, edgeMailboxName)] = \
                    EdgeInfo{thisEdgeCost, false, (*recordIterator)->pos};
            }
        }  /* That's all the edges. */

        /* Define properties of this mailbox, and add cores. */
        anyErrors |= !d3_define_mailbox_fields_from_section(mailbox,
                                                            sectionType);
    }

    /* Connect mailboxes together */
    for (edgeFinder = mailboxEdges.begin(); edgeFinder != mailboxEdges.end();
         edgeFinder++)
    {
        /* Get the mailboxes. Complain if one of the mailbox names does not
         * exist. For each name in the pair, check it matches a record in
         * mailboxInfoFromName. */
        joinedMailboxes.clear();
        MailboxName thisName = edgeFinder->first.first;
        while (true)  /* Loop over both names in the map key (LM1). A
                       * concession to the fact that one can't iterate over a
                       * std::pair. */
        {
            mailboxNameFinder = mailboxInfoFromName.find(thisName);
            if (mailboxNameFinder == mailboxInfoFromName.end())
            {
                d3_errors.append(dformat(
                    "L%u: Could not find a definition for mailbox %s defined "
                    "by an edge in this record.\n",
                    edgeFinder->second.lineNumber, thisName.c_str()));
                {
                    anyErrors = true;
                }
            }

            else
            {
                joinedMailboxes.push_back(
                    mailboxNameFinder->second.memoryAddress);
            }

            /* Increment iteration, or leave (LM1). */
            if (thisName != edgeFinder->first.second)
            {
                thisName = edgeFinder->first.second;
            }
            else{break;}
        }

        /* Complain if the reverse of an edge has not been defined. */
        if (!(edgeFinder->second.isReverseDefined))
        {
            d3_errors.append(dformat(
                "L%u: Could not find the reverse edge definition connecting "
                "mailboxes %s and %s\n.", edgeFinder->second.lineNumber,
                edgeFinder->first.first, edgeFinder->first.second));
            anyErrors = true;
            continue;
        }

        /* Hook them up. */
        board->connect(
            joinedMailboxes[0]->get_hardware_address()->get_mailbox(),
            joinedMailboxes[1]->get_hardware_address()->get_mailbox(),
            edgeFinder->second.weight);
    }

    /* Clear mailbox metadata for this board. */
    mailboxEdges.clear();
    mailboxInfoFromName.clear();

    return !anyErrors;
}

/* Validate the contents of the engine_board section, and create boards and
 * items beneath. Relies on engine_box having been read.
 *
 * Returns true if all validation checks pass, and false otherwise. Arguments:
 *
 * - engine: Engine to populate with boards, and other items. */
bool HardwareFileParser::d3_populate_validate_engine_board_and_below(
    P_engine* engine)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */
    std::string sectionName = "engine_board";

    /* Valid fields for this section (none are mandatory). */
    std::vector<std::string> validFields;
    std::vector<std::string>::iterator fieldIterator;
    validFields.push_back("board_board_cost");
    validFields.push_back("type");

    /* Holds fields we've already grabbed (for validation purposes). */
    std::map<std::string, bool> fieldsFound;
    for (fieldIterator=validFields.begin(); fieldIterator!=validFields.end();
         fieldIterator++)
    {
        fieldsFound.insert(std::make_pair(*fieldIterator, false));
    }

    /* Temporary staging vectors for holding value nodes and variable
     * nodes. */
    std::vector<UIF::Node*> valueNodes;
    std::vector<UIF::Node*> variableNodes;

    /* Holds the type of the current record, and the section it refers to (if
     * any). */
    std::string type;
    UIF::Node* sectionType;

    /* Holds the concatenated address of a board. */
    AddressComponent address;

    /* Holds the board while we're creating it. */
    P_board* board;
    BoardName boardName;

    /* Holds boards while connecting them together. */
    std::vector<P_board*> joinedBoards;

    /* For finding board names in boardInfoFromName. */
    std::map<BoardName, BoardInfo>::iterator boardNameFinder;

    /* Holds any default board-board cost, if found. */
    float defaultCost = 0;
    bool isDefaultCostDefined = false;

    /* Holds an edge-specific board-board cost, if found. */
    float thisEdgeCost;
    bool isThisEdgeCostDefined;

    /* Holds the name of the board on the other end of an edge. */
    BoardName edgeBoardName;

    /* For finding a reverse edge, and for iterating through the edge
     * container. */
    std::map<std::pair<BoardName, BoardName>, EdgeInfo>::iterator   \
        edgeFinder;

    /* Iterate through all record nodes in this section, in order to:
     *
     * - Get the default board-board cost.
     * - Complain if there are duplicate entries.
     *
     * Ignore records that are not variable definitions. */
    std::vector<UIF::Node*> recordNodes;
    std::vector<UIF::Node*>::iterator recordIterator;
    std::vector<UIF::Node*>::iterator edgeIterator;
    std::string variableName;
    bool isRecordValid;
    GetRecd(untypedSections[sectionName], recordNodes);
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        isRecordValid = true;  /* Innocent until proven guilty. */

        /* Get the value and variable nodes. */
        GetVari((*recordIterator), variableNodes);
        GetValu((*recordIterator), valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Only proceed if this record is a variable definition (it must have a
         * '+' prefix) */
        if (variableNodes[0]->qop != Lex::Sy_plus){continue;}

        /* Complain if (in order):
         *
         * - The variable name is not a valid name.
         * - There is more than one variable node.
         * - There is more than one value node. */
        isRecordValid &= complain_if_variable_name_invalid(
            *recordIterator, variableNodes[0], &validFields, sectionName,
            &d3_errors);
        isRecordValid &= complain_if_record_is_multivariable(
            *recordIterator, &variableNodes, sectionName, &d3_errors);
        isRecordValid &= complain_if_record_is_multivalue(
            *recordIterator, &valueNodes, variableNodes[0]->str,
            sectionName, &d3_errors);
        if (!isRecordValid)
        {
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        variableName = variableNodes[0]->str;
        if (complain_if_node_variable_true_in_map(
                *recordIterator, variableNodes[0], &fieldsFound,
                sectionName,
                &d3_errors))
        {
            anyErrors = true;
            continue;
        }
        fieldsFound[variableName] = true;

        /* Specific logic for each variable. */
        if (variableName == "board_board_cost")
        {
            /* Complain if not a float. */
            isRecordValid &= complain_if_node_value_not_floating(
                *recordIterator, valueNodes[0], variableName, sectionName,
                &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Hold him! */
            defaultCost = str2unsigned(valueNodes[0]->str);
            isDefaultCostDefined = true;
        }

        /* Types are processed in d3_get_validate_default_types, so we
         * ignore the definition here. */
        else if (variableName == "type");

        /* Shouldn't be able to enter this, because we've already checked
         * the variable names, but why not write some more code. It's not
         * like this file is big enough already. */
        else
        {
            d3_errors.append(dformat(
                "L%u: Variable name '%s' is not valid in the '%s' section.\n",
                (*recordIterator)->pos, variableName.c_str(),
                sectionName.c_str()));
            anyErrors = true;
        }
    }

    /* Iterate through all record nodes in this section, in order to define
     * boards. We ignore variable definitions this time. Yeah we're iterating
     * twice... */
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        isRecordValid = true;  /* Innocent until proven guilty. */
        sectionType = PNULL;  /* Given that this is a board record, we have to
                               * find a section that defines the properties of
                               * this board. */

        /* Get the value and variable nodes. */
        GetVari((*recordIterator), variableNodes);
        GetValu((*recordIterator), valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Only proceed if this record is NOT a variable definition (it must
         * NOT have a '+' prefix) */
        if (variableNodes[0]->qop == Lex::Sy_plus){continue;}

        /* Validate and get the name of the board. */
        if (!d3_get_board_name(variableNodes[0], &boardName))
        {
            anyErrors = true;
            continue;
        }

        /* Whine if a board with this name already exists. */
        std::map<BoardName, BoardInfo>::iterator boardNameFinder;
        boardNameFinder = boardInfoFromName.find(boardName);
        if (boardNameFinder != boardInfoFromName.end())
        {
            d3_errors.append(dformat("L%u: Board name on this line has "
                                     "already been defined on line %u. Not "
                                     "making it.\n",
                                     (*recordIterator)->pos,
                                     boardNameFinder->second.lineNumber));
            anyErrors = true;
            continue;
        }

        /* Is a type explicitly defined in this record? */
        if (d3_get_explicit_type_from_item_definition(variableNodes[0],
                                                      &type))
        {
            /* If there's a matching section for this type, we're all good (it
             * gets written to sectionType). Otherwise, we fall back to
             * defaults. */
            anyErrors |= !d3_get_section_from_type(
                "board", type, sectionName, (*recordIterator)->pos,
                &sectionType);
        }

        /* If the type it was not defined explicitly, or it was and no section
         * matched with it, get the section from the defaults. */
        if (sectionType == PNULL)
        {
            sectionType = defaultTypes[untypedSections[sectionName]];

            /* If it's still zero, then the type hasn't been defined anywhere
             * "validly". That's an error, folks. */
            if (sectionType == PNULL)
            {
                d3_errors.append(dformat("L%u: No section found to define the "
                                         "board on this line. Not making "
                                         "it.\n", (*recordIterator)->pos));
                anyErrors = true;
                continue;  /* Can't do anything without a type definition... */
            }
        }

        /* Get the address without validating it (boss' orders). */
        d3_get_address_from_item_definition(variableNodes[0], &address);

        /* Remove the board from undefinedBoards (it was put there during box
         * declaration, but now we know it exists). */
        undefinedBoards.remove(boardName);

        /* Create the board (the name argument is the name of the board in the
         * record). */
        board = new P_board(boardName.second);

        /* Into the engine with ye! */
        boxFromName[boardName.first]->contain(address, board);
        engine->contain(address, board);

        /* Track the board by name. */
        boardInfoFromName[boardName] =
            BoardInfo{(*recordIterator)->pos, board};

        /* Stage the edges from this record. Values (i.e. LHS of the '=' token)
         * each represent the name of a board, optionally with a cost, which
         * defines an edge */
        for(edgeIterator=valueNodes.begin(); edgeIterator!=valueNodes.end();
            edgeIterator++)
        {
            /* Get the name of the board on the other side of this edge,
             * skipping if invalid. */
            if(!d3_get_board_name(*edgeIterator, &edgeBoardName))
            {
                anyErrors = true;
                continue;  /* Skip this edge - can't identify either end. */
            }

            /* If the edge explicitly describes a cost, use that. Otherwise,
             * use one we found earlier. Complain if:
             *
             * - neither are defined
             * - the explicit cost is invalid (we checked the default cost
             *   earlier) */
            isThisEdgeCostDefined = d3_get_explicit_cost_from_edge_definition(
                *edgeIterator, &thisEdgeCost);
            if (isThisEdgeCostDefined)
            {
                /* Check for -1, meaning the cost was invalid. */
                if (thisEdgeCost == -1)
                {
                    d3_errors.append(dformat(
                        "L%u: Invalid cost on edge connecting board %s-%s to "
                        "board %s-%s (it must be a float).\n",
                        (*recordIterator)->pos,
                        boardName.first.c_str(),
                        boardName.second.c_str(),
                        edgeBoardName.first.c_str(),
                        edgeBoardName.second.c_str()));
                    anyErrors = true;
                    continue;  /* Skip this edge - meaningless without a
                                * comprehensible cost. */
                }
            }
            else if (isDefaultCostDefined)
            {
                thisEdgeCost = defaultCost;
            }
            else
            {
                d3_errors.append(dformat(
                    "L%u: No cost found for edge connecting board %s-%s to "
                    "board %s-%s (it must be a float).\n",
                    (*recordIterator)->pos,
                    boardName.first.c_str(),
                    boardName.second.c_str(),
                    edgeBoardName.first.c_str(),
                    edgeBoardName.second.c_str()));
                anyErrors = true;
                continue;  /* Skip this edge - meaningless without a cost. */
            }

            /* If the reverse edge is in boardEdges... */
            /* Bears reiterating - it's the reverse! */
            edgeFinder = boardEdges.find(std::make_pair(edgeBoardName,
                                                        boardName));
            if (edgeFinder != boardEdges.end())
            {
                /* Complain if:
                 *
                 * - The reverse-cost is different.
                 * - Reverse is already defined. */
                if (edgeFinder->second.weight != thisEdgeCost)
                {
                    d3_errors.append(dformat(
                        "L%u: The cost of the edge connecting board %s-%s to "
                        "board %s-%s (%f) is different from the cost of its "
                        "reverse (%f), defined at L%i.\n",
                        (*recordIterator)->pos,
                        boardName.first.c_str(),
                        boardName.second.c_str(),
                        edgeBoardName.first.c_str(),
                        edgeBoardName.second.c_str(),
                        thisEdgeCost,
                        edgeFinder->second.weight,
                        edgeFinder->second.lineNumber));
                    anyErrors = true;
                    continue;  /* Skip this edge, avoid clobbering. */
                }

                if (edgeFinder->second.isReverseDefined)
                {
                    d3_errors.append(dformat(
                        "L%u: The other end of the edge connecting board "
                        "%s-%s to board %s-%s has already been defined. The "
                        "first definition was at L%i.\n",
                        (*recordIterator)->pos,
                        boardName.first.c_str(),
                        boardName.second.c_str(),
                        edgeBoardName.first.c_str(),
                        edgeBoardName.second.c_str(),
                        edgeFinder->second.lineNumber));
                    anyErrors = true;
                    continue;  /* Skip this edge, avoid clobbering. */
                }

                /* We're all good, mark the reverse as found (we're on it!). */
                edgeFinder->second.isReverseDefined = true;
            }

            /* Otherwise, track this edge in boardEdges. */
            else
            {
                /* But complain if it's already there (means we've defined it
                 * twice on this line, probably). */

                /* NB: Not reverse! */
                edgeFinder = boardEdges.find(std::make_pair(boardName,
                                                            edgeBoardName));
                if (edgeFinder != boardEdges.end())
                {
                    d3_errors.append(dformat(
                        "L%u: Duplicate edge definition connecting board "
                        "%s-%s to board %s-%s.\n",
                        (*recordIterator)->pos,
                        boardName.first.c_str(),
                        boardName.second.c_str(),
                        edgeBoardName.first.c_str(),
                        edgeBoardName.second.c_str()));
                    anyErrors = true;
                    continue;  /* Skip this edge, avoid clobbering. */
                }

                /* Okay, actually add it now. */
                boardEdges[std::make_pair(boardName, edgeBoardName)] = \
                    EdgeInfo{thisEdgeCost, false, (*recordIterator)->pos};
            }
        }  /* That's all the edges. */

        /* Define the properties of this board. If any are missing or broken,
         * continue (we fail slowly). */
        anyErrors |= !d3_define_board_fields_from_section(board, sectionType);

        /* Populate the board with mailboxes. */
        anyErrors |= !d3_populate_validate_board_with_mailboxes(board,
                                                                sectionType);
    }

    /* Connect boards together. */
    for (edgeFinder = boardEdges.begin(); edgeFinder != boardEdges.end();
         edgeFinder++)
    {
        /* Get the boards. Complain if one of the board names does not
         * exist. For each name in the pair, check it matches a record in
         * boardInfoFromName. */
        joinedBoards.clear();
        BoardName thisName = edgeFinder->first.first;
        while (true)  /* Loop over both names in the map key (LM1). A
                       * concession to the fact that one can't iterate over a
                       * std::pair. */
        {
            boardNameFinder = boardInfoFromName.find(thisName);
            if (boardNameFinder == boardInfoFromName.end())
            {
                d3_errors.append(dformat(
                    "L%u: Could not find a definition for board %s-%s defined "
                    "by an edge in this record.\n",
                    edgeFinder->second.lineNumber,
                    thisName.first.c_str(), thisName.second.c_str()));
                {
                    anyErrors = true;
                }
            }

            else
            {
                joinedBoards.push_back(boardNameFinder->second.memoryAddress);
            }

            /* Increment iteration, or leave (LM1). */
            if (thisName != edgeFinder->first.second)
            {
                thisName = edgeFinder->first.second;
            }
            else{break;}
        }

        /* Complain if the reverse of an edge has not been defined. */
        if (!(edgeFinder->second.isReverseDefined))
        {
            /* This is a bit obtuse, but that's pairs for you. Basically:
             *
             * - The first tier is a std::pair<BoardName, BoardName>
             * - The second tier identifies the board by name, and is a
             *   BoardName, which is a std::pair<std::string, std::string>.
             * - The third tier identifies either the box or board component of
             *   the board name, and is a std::string. */
            d3_errors.append(dformat(
                "L%u: Could not find the reverse edge definition connecting "
                "boards %s-%s and %s-%s\n.",
                edgeFinder->second.lineNumber,
                /* Box component of first board name. */
                edgeFinder->first.first.first.c_str(),
                /* Board component of first board name. */
                edgeFinder->first.first.second.c_str(),
                /* Box component of second board name. */
                edgeFinder->first.second.first.c_str(),
                /* Board component of second board name. */
                edgeFinder->first.second.second.c_str()));
            anyErrors = true;
            continue;
        }

        /* Hook them up. */
        engine->connect(
            joinedBoards[0]->get_hardware_address()->get_board(),
            joinedBoards[1]->get_hardware_address()->get_board(),
            edgeFinder->second.weight);
    }

    /* Check for boards that were defined as part of box declarations that have
     * not beend defined. Generally, catch this problem:
     *
     * [engine_box]
     * myBox(boards(B00,B01))
     * ... // Truncation
     * [engine_board]
     * myBox(board(B00),...)=...
     * ...
     * EOF
     * // But I've not defined myBox(board(B01))!
     *
     * Also, we don't enter the loop if we've done nothing wrong. */
    std::list<BoardName>::iterator badBoardIterator;
    for (badBoardIterator = undefinedBoards.begin();
         badBoardIterator != undefinedBoards.end(); badBoardIterator++)
    {
        d3_errors.append(dformat(
            "Board %s-%s has been declared in the 'engine_box' section, but "
            "not defined in the 'engine_board' section.\n",
            (*badBoardIterator).first, (*badBoardIterator).second));
        anyErrors = true;
    }

    return !anyErrors;
}

/* Validate the contents of the engine_box section, and populate an engine with
 * them. Also creates boxes.
 *
 * Returns true if all validation checks pass, and false otherwise. Arguments:
 *
 * - engine: Engine to populate. */
bool HardwareFileParser::d3_populate_validate_engine_box(P_engine* engine)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */
    std::string sectionName = "engine_box";

    /* Valid fields for this section. */
    std::vector<std::string> validFields;
    std::vector<std::string>::iterator fieldIterator;
    validFields.push_back("external_box_cost");
    validFields.push_back("type");

    /* Holds fields we've already grabbed (for validation purposes). */
    std::map<std::string, bool> fieldsFound;
    for (fieldIterator=validFields.begin(); fieldIterator!=validFields.end();
         fieldIterator++)
    {
        fieldsFound.insert(std::make_pair(*fieldIterator, false));
    }

    /* Temporary staging vectors for holding value nodes and variable
     * nodes. */
    std::vector<UIF::Node*> valueNodes;
    std::vector<UIF::Node*> variableNodes;

    /* Holds the type of the current record, and the section it refers to (if
     * any). */
    std::string type;
    UIF::Node* sectionType;

    /* Holds the concatenated address of a box. */
    AddressComponent address;

    /* Holds the box while we're creating it. */
    P_box* box;
    std::string boxName;

    /* Iterate through all record nodes in this section. */
    std::vector<UIF::Node*> recordNodes;
    std::vector<UIF::Node*>::iterator recordIterator;
    std::string variableName;
    bool isRecordValid;
    GetRecd(untypedSections[sectionName], recordNodes);
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        isRecordValid = true;  /* Innocent until proven guilty. */
        sectionType = PNULL;  /* Given that this is a box record, we have to
                               * find a section that defines the properties of
                               * this box. */

        /* Get the value and variable nodes. */
        GetVari((*recordIterator), variableNodes);
        GetValu((*recordIterator), valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Is this a variable definition? (does it have variables, values, and
         * a '+' prefix)? (otherwise, we assume it is a box definition). */
        if (valueNodes.size() > 0 and variableNodes.size() > 0 and
            variableNodes[0]->qop == Lex::Sy_plus)
        {
            /* Complain if (in order):
             *
             * - The variable name is not a valid name.
             * - There is more than one variable node.
             * - There is more than one value node. */
            isRecordValid &= complain_if_variable_name_invalid(
                *recordIterator, variableNodes[0], &validFields, sectionName,
                &d3_errors);
            isRecordValid &= complain_if_record_is_multivariable(
                *recordIterator, &variableNodes, sectionName, &d3_errors);
            isRecordValid &= complain_if_record_is_multivalue(
                *recordIterator, &valueNodes, variableNodes[0]->str,
                sectionName, &d3_errors);
            if (!isRecordValid)
            {
                anyErrors = true;
                continue;
            }

            /* Complain if duplicate. NB: We know the variable name is valid if
             * control has reached here. */
            variableName = variableNodes[0]->str;
            if (complain_if_node_variable_true_in_map(
                    *recordIterator, variableNodes[0], &fieldsFound,
                    sectionName,
                    &d3_errors))
            {
                anyErrors = true;
                continue;
            }
            fieldsFound[variableName] = true;

            /* Specific logic for each variable. */
            if (variableName == "external_box_cost")
            {
                /* Complain if not a float. */
                isRecordValid &= complain_if_node_value_not_floating(
                    *recordIterator, valueNodes[0], variableName, sectionName,
                    &d3_errors);
                if (!isRecordValid)
                {
                    anyErrors = true;
                    continue;
                }

                /* Bind! */
                engine->costExternalBox = str2unsigned(valueNodes[0]->str);
            }

            /* Types are processed in d3_get_validate_default_types, so we
             * ignore the definition here. */
            else if (variableName == "type");

            /* Shouldn't be able to enter this, because we've already checked
             * the variable names, but why not write some more code. It's not
             * like this file is big enough already. */
            else
            {
                d3_errors.append(dformat(
                    "L%u: Variable name '%s' is not valid in the '%s' "
                    "section.\n", (*recordIterator)->pos, variableName.c_str(),
                    sectionName.c_str()));
                anyErrors = true;
            }

            /* This continue is here to reduce the amount of indentation for
             *  dealing with box records. */
            continue;
        }

        /* Otherwise, this must be a box definition. */

        /* Complain if the name is invalid (but keep going). */
        if (!complain_if_node_variable_not_a_valid_item_name(
                *recordIterator, variableNodes[0], &d3_errors))
        {
            anyErrors = true;
        }

        /* Complain if a box already exists with this name (this is
         * near-fatal). */
        std::map<std::string, P_box*>::iterator boxNameFinder;
        boxName = variableNodes[0]->str;
        boxNameFinder = boxFromName.find(boxName);
        if(boxNameFinder != boxFromName.end())
        {
            d3_errors.append(dformat("L%u: Box name '%s' already defined.\n",
                                     (*recordIterator)->pos, boxName.c_str()));
            anyErrors = true;
            continue;
        }

        /* Is a type explicitly defined in this record? */
        if (d3_get_explicit_type_from_item_definition(variableNodes[0],
                                                      &type))
        {
            /* If there's a matching section for this type, we're all good (it
             * gets written to sectionType). Otherwise, we fall back to
             * defaults. */
            anyErrors |= !d3_get_section_from_type(
                "box", type, sectionName, (*recordIterator)->pos,
                &sectionType);
        }

        /* If the type it was not defined explicitly, or it was and no section
         * matched with it, get the section from the defaults. */
        if (sectionType == PNULL)
        {
            sectionType = defaultTypes[untypedSections[sectionName]];

            /* If it's still zero, then the type hasn't been defined anywhere
             * "validly". That's an error, folks. */
            if (sectionType == PNULL)
            {
                d3_errors.append(dformat("L%u: No section found to define the "
                                         "box on this line. Not making it.\n",
                                         (*recordIterator)->pos));
                anyErrors = true;
                continue;  /* Can't do anything without a type definition... */
            }
        }

        /* Get the address without validating it (boss' orders). */
        d3_get_address_from_item_definition(variableNodes[0], &address);

        /* So we know we're dealing with a box definition, and we've got a
         * type and an address for it. */

        /* Create the box (the name argument is the name of the box in the
         * record). */
        box = new P_box(boxName);

        /* Store conveniently. */
        boxFromName[boxName] = box;

        /* Into the engine with ye! */
        engine->contain(address, box);

        /* For each board in the box definition line, add it to
         * undefinedBoards. */
        std::vector<UIF::Node*>::iterator leafIterator;
        leafIterator = variableNodes[0]->leaf.begin();
        while (leafIterator!=variableNodes[0]->leaf.end())
        {
            if ((*leafIterator)->str == "boards"){break;}
            leafIterator++;
        }

        /* Don't panic if there aren't any boards to add. */
        if (leafIterator != variableNodes[0]->leaf.end())
        {
            /* For each board... */
            std::vector<UIF::Node*>::iterator lLeafIterator;  /* Sorry */
            for (lLeafIterator=(*leafIterator)->leaf.begin();
                 lLeafIterator!=(*leafIterator)->leaf.end(); lLeafIterator++)
            {
                /* Add to undefinedBoards. */
                undefinedBoards.push_back(BoardName(boxName,
                                                    (*lLeafIterator)->str));
            }
        }

        /* Populate the fields of the box from its type definition. */
        anyErrors |= !d3_define_box_fields_from_section(box, sectionType);

    } /* End for-each-record */

    return !anyErrors;
}

/* Validate the contents of the header section, and populate an engine with
 * them.
 *
 * Returns true if all validation checks pass, and false otherwise. Arguments:
 *
 * - engine: Engine to populate */
bool HardwareFileParser::d3_populate_validate_header(P_engine* engine)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */
    std::string sectionName = "header";

    /* Valid fields for the header section. */
    std::vector<std::string> validFields;
    std::vector<std::string>::iterator fieldIterator;
    validFields.push_back("author");
    validFields.push_back("datetime");
    validFields.push_back("dialect");
    validFields.push_back("file");
    validFields.push_back("hardware");
    validFields.push_back("version");

    /* Mandatory fields for the header section. Could be indeces that refer to
     * validFields, but this is (probably) easier to read/maintain. */
    std::vector<std::string> mandatoryFields;
    mandatoryFields.push_back("datetime");
    mandatoryFields.push_back("dialect");
    mandatoryFields.push_back("version");

    /* Holds fields we've already grabbed (for validation purposes). */
    std::map<std::string, bool> fieldsFound;
    for (fieldIterator=validFields.begin(); fieldIterator!=validFields.end();
         fieldIterator++)
    {
        fieldsFound.insert(std::make_pair(*fieldIterator, false));
    }

    /* Temporary staging vectors for holding value nodes and variable
     * nodes. */
    std::vector<UIF::Node*> valueNodes;
    std::vector<UIF::Node*> variableNodes;

    /* Iterate through all record nodes in this section. */
    std::vector<UIF::Node*> recordNodes;
    std::vector<UIF::Node*>::iterator recordIterator;
    std::string variableName;
    bool isRecordValid;
    GetRecd(untypedSections[sectionName], recordNodes);
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        isRecordValid = true;  /* Innocent until proven guilty. */

        /* Get the value and variable nodes. */
        GetVari((*recordIterator), variableNodes);
        GetValu((*recordIterator), valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Complain if (in order):
         *
         * - The record does not begin with a "+", as all fields in this
         *   section must do.
         * - The variable name is not a valid name.
         * - There is more than one variable node.
         * - There is more than one value node. */
        isRecordValid &= complain_if_node_not_plus_prefixed(
            *recordIterator, variableNodes[0], sectionName, &d3_errors);
        isRecordValid &= complain_if_variable_name_invalid(
            *recordIterator, variableNodes[0], &validFields, sectionName,
            &d3_errors);
        isRecordValid &= complain_if_record_is_multivariable(
            *recordIterator, &variableNodes, sectionName, &d3_errors);
        isRecordValid &= complain_if_record_is_multivalue(
            *recordIterator, &valueNodes, variableNodes[0]->str, sectionName,
            &d3_errors);
        if (!isRecordValid)
        {
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        variableName = variableNodes[0]->str;
        if (complain_if_node_variable_true_in_map(
                *recordIterator, variableNodes[0], &fieldsFound, sectionName,
                &d3_errors))
        {
            anyErrors = true;
            continue;
        }
        fieldsFound[variableName] = true;

        /* Specific logic for each variable. */
        if (variableName == "author")
        {
            engine->author = valueNodes[0]->str;
        }

        else if (variableName == "datetime")
        {
            /* Special validation for this one. */
            if (valueNodes[0]->qop != Lex::Sy_ISTR)
            {
                d3_errors.append(dformat(
                    "L%u: Variable '%s' in the '%s' section has value '%s', "
                    "which is not a datetime in the form YYYYMMDDhhmmss.\n",
                    (*recordIterator)->pos, variableName.c_str(),
                    valueNodes[0]->str.c_str(), sectionName.c_str()));
                anyErrors = true;
            }

            else
            {
                engine->datetime = str2long(valueNodes[0]->str);
            }

        }

        /* This has already been read and validated, so we don't care. */
        else if (variableName == "dialect"){}

        else if (variableName == "file")
        {
            engine->fileOrigin = valueNodes[0]->str;
        }

        /* Ignore this one for now. */
        else if (variableName == "hardware"){}

        else if (variableName == "version")
        {
            engine->version = valueNodes[0]->str;
        }

        /* Shouldn't be able to enter this, because we've already checked the
         * variable names, but why not write some more code. It's not like this
         * file is big enough already. */
        else
        {
            d3_errors.append(dformat(
                "L%u: Variable name '%s' is not valid in the '%s' section.\n",
                (*recordIterator)->pos, variableName.c_str(),
                sectionName.c_str()));
            anyErrors = true;
        }
    }

    /* Ensure mandatory fields have been defined. */
    anyErrors |= !complain_if_mandatory_field_not_defined(
        &mandatoryFields, &fieldsFound, sectionName, &d3_errors);

    return !anyErrors;
}

/* Validate type defaults, and populate the defaultTypes map with section
 * references for each section that can create items.
 *
 * Returns true if all validation checks pass, and false otherwise */
bool HardwareFileParser::d3_validate_types_define_cache()
{
    bool anyErrors;

    /* Container to hold default sections for boxes, boards, and mailboxes. */
    UIF::Node* globalDefaults[ITEM_ENUM_LENGTH] = {PNULL, PNULL, PNULL};

    /* Populate our new friend. */
    anyErrors = !d3_get_validate_default_types(globalDefaults);

    /* Staging vector and iterator for records. */
    std::vector<UIF::Node*> recordNodes;
    std::vector<UIF::Node*>::iterator recordIterator;
    bool isRecordValid;

    /* Staging vectors for holding value nodes and variable nodes. */
    std::vector<UIF::Node*> valueNodes;
    std::vector<UIF::Node*> variableNodes;
    std::string type;
    unsigned typeLine;

    /* Get all sections that create items, and thus may define +type fields
     * (i.e. engine_box, engine_board, and any board(X) section), and store
     * pointers to them in creativeSections. */
    std::vector<UIF::Node*> creativeSections;
    std::vector<UIF::Node*>::iterator sectionIterator;
    std::string sectionName;
    creativeSections.push_back(untypedSections["engine_box"]);
    creativeSections.push_back(untypedSections["engine_board"]);

    std::map<std::string, UIF::Node*>::iterator typedSectionsIterator;
    for (typedSectionsIterator=typedSections["board"].begin();
         typedSectionsIterator!=typedSections["board"].end();
         typedSectionsIterator++)
    {
        creativeSections.push_back(typedSectionsIterator->second);
    }

    /* For each of these sections, find the section that defines properties of
     * items created within it, and insert this information into
     * defaultTypes. */
    bool typeFieldFound;
    for (sectionIterator=creativeSections.begin();
         sectionIterator!=creativeSections.end(); sectionIterator++)
    {
        typeFieldFound = false;
        sectionName = (*sectionIterator)->leaf[0]->leaf[0]->str;

        /* Has it got a type-defining record? If so, grab the type by iterating
         * through each record in this section. If there's more than one type
         * record, it's an error, but just keep soldiering on (this is okay;
         * the calling method should be checking the return value of this
         * method). */
        GetRecd(*sectionIterator, recordNodes);
        for (recordIterator=recordNodes.begin();
             recordIterator!=recordNodes.end(); recordIterator++)
        {
            isRecordValid = true;  /* Innocent until proven guilty. */

            /* Get the value and variable nodes. */
            GetVari((*recordIterator), variableNodes);
            GetValu((*recordIterator), valueNodes);

            /* Ignore this record if the record has not got a variable/value
             * pair (i.e. if the line is empty, or is just a comment). */
            if (variableNodes.size() == 0 and
                valueNodes.size() == 0){continue;}

            /* Is the variable name "type"? */
            if (variableNodes[0]->str == "type")
            {
                /* Complain if (in order):
                 *
                 * - The record does not begin with a "+".
                 * - There is more than one variable node.
                 * - There is more than one value node.
                 * - The value is not a valid type.
                 *
                 * If so, ignore it. */
                isRecordValid &= complain_if_node_not_plus_prefixed(
                    *recordIterator, variableNodes[0], sectionName,
                    &d3_errors);
                isRecordValid &= complain_if_record_is_multivariable(
                    *recordIterator, &variableNodes, sectionName, &d3_errors);
                isRecordValid &= complain_if_record_is_multivalue(
                    *recordIterator, &valueNodes, variableNodes[0]->str,
                    sectionName, &d3_errors);
                isRecordValid &= complain_if_node_value_not_a_valid_type(
                    *recordIterator, valueNodes[0], sectionName, &d3_errors);
                if (!isRecordValid)
                {
                    anyErrors = true;
                    continue;  /* Skip to the next record in this section. */
                }

                /* We got it! But complain if we've already found a type field
                 * in this way. */
                if (typeFieldFound)
                {
                    d3_errors.append(dformat(
                        "L%u: Duplicate definition of field 'type' in the "
                        "'%s' section (previously defined at L%u.\n",
                        (*recordIterator)->pos, sectionName.c_str(),
                        typeLine));
                    anyErrors = true;
                    continue;  /* Skip to the next record in this section. */
                }
                typeFieldFound = true;
                type = valueNodes[0]->str;
                typeLine = (*recordIterator)->pos;
            }

            /* The variable of this node wasn't named "type", so we ignore it
             * for our current machinations */
            else {continue;}
        }

        /* Figure out what sort of object (i.e. box, board...) is being created
         * in this section from the section name. */
        std::string itemType;
        unsigned arrayIndex;
        if (sectionName == "engine_box")
        {
            itemType = "box";
            arrayIndex = box;
        }
        else if (sectionName == "engine_board")
        {
            itemType = "board";
            arrayIndex = board;
        }
        else /* Must me a [board(something)] section. */
        {
            itemType = "mailbox";
            arrayIndex = mailbox;
        }

        /* Earlier, did we find a +type record in this section? */
        UIF::Node* sectionTarget = 0;
        if (typeFieldFound)  /* Yes! */
        {
            /* Get the section that matches this type. If there isn't one,
             * complain, but keep going (we can use globalDefaults). */
            typedSectionsIterator = typedSections[itemType].find(type);
            if (typedSectionsIterator != typedSections[itemType].end())
            {
                sectionTarget = typedSectionsIterator->second;
            }
            else
            {
                d3_errors.append(dformat("L%u: Type '%s' defined in the '%s' "
                                         "section does not correspond to a "
                                         "section.\n", typeLine, type.c_str(),
                                         sectionName.c_str()));
                anyErrors = true;
            }
        }

        /* If there was no +type record in this section, or if the +type was
         * invalid, use the one from globalDefaults (even if it is PNULL). */
        if (sectionTarget == 0)
        {
            sectionTarget = globalDefaults[arrayIndex];
        }

        /* Apply the section found (even if it is PNULL) to defaultTypes. */
        defaultTypes[*sectionIterator] = sectionTarget;
    }

    return !anyErrors;
}
