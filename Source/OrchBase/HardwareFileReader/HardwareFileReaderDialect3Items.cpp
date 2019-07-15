/* Defined behaviour for the dialect 3 functionality of methods that create
 * and/or modify items in the engine. */

#include "HardwareFileReader.h"

/* Create some cores and threads, and pile them all into a mailbox.
 *
 * Returns true if there were no validation problems, and false
 * otherwise. Arguments:
 *
 * - mailbox: Pointer to the mailbox to populate.
 * - quatity: Number of cores to create (the number of threads will be
         determined from the UIF parse tree. */
bool HardwareFileReader::d3_create_cores_and_threads_for_mailbox(
    P_mailbox* mailbox, unsigned coreQuantity)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */

    /* We'll be modifying the 'record', 'sectionName', and 'variable'
     * members. To ensure they are preserved on exit, store the old values
     * here, and reinstate them once finished. */
    UIF::Node* callingRecord = record;
    std::string callingSectionName = sectionName;
    std::string callingVariable = variable;

    sectionName = "core";

    /* Create all of the cores, and add them to the mailbox. Don't validate the
     * address component (but still catch if we go out of bounds). */
    P_core* tmpCore;
    AddressComponent coreId;
    for (coreId = 0; coreId < coreQuantity; coreId++)
    {
        tmpCore = new P_core(dformat(
                             "C%0*u", how_many_digits(coreQuantity), coreId));
        try
        {
            mailbox->contain(coreId, tmpCore);
        }
        catch (OrchestratorException &e)
        {
            errors.push_back(e.message.c_str());
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

    /* For iterating through cores when binding and creating threads. */
    std::map<AddressComponent, P_core*>::iterator coreIterator;

    /* Iterate through all record nodes in this section, and apply properties
     * to all cores within. */
    std::vector<UIF::Node*> recordNodes;
    GetRecd(untypedSections[sectionName], recordNodes);

    for (std::vector<UIF::Node*>::iterator recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        record = *recordIterator;

        /* Get the value and variable nodes. */
        GetVari(record, variableNodes);
        GetValu(record, valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Complaints */
        if (!(complain_if_variable_not_plus_prefixed(variableNodes[0]) and
              complain_if_variable_name_invalid(variableNodes[0],
                                                &validFields) and
              complain_if_record_is_multivariable(&variableNodes) and
              complain_if_record_is_multivalue(&valueNodes)))
        {
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        if (complain_if_variable_true_in_map(variableNodes[0], &fieldsFound))
        {
            anyErrors = true;
            continue;
        }
        variable = variableNodes[0]->str;
        fieldsFound[variable] = true;

        /* Specific logic for each variable. */
        if (variable == "core_thread_cost")
        {
            /* Complain if not a float. */
            if (!complain_if_value_not_floating(valueNodes[0]))
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

        else if (variable == "data_memory")
        {
            /* Complain if not natural */
            if (!complain_if_value_not_natural(valueNodes[0]))
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

        else if (variable == "instruction_memory")
        {
            /* Complain if not natural */
            if (!complain_if_value_not_natural(valueNodes[0]))
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

        else if (variable == "thread_thread_cost")
        {
            /* Complain if not a float. */
            if (!complain_if_value_not_floating(valueNodes[0]))
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

        else if (variable == "threads")
        {
            /* Complain if not natural */
            if (!complain_if_value_not_natural(valueNodes[0]))
            {
                anyErrors = true;
                continue;
            }

            /* Create that many threads on each core, without validating the
             * address component (but still catch if we go out of bounds). */
            std::map<AddressComponent, P_core*>::iterator coreIterator;
            unsigned threadId;
            P_thread* tmpThread;
            unsigned threadQuantity = str2unsigned(valueNodes[0]->str);
            for (coreIterator=mailbox->P_corem.begin();
                 coreIterator!=mailbox->P_corem.end(); coreIterator++)
            {
                for (threadId=0; threadId<threadQuantity; threadId++)
                {
                    tmpThread = new P_thread(dformat(
                        "T%0*u", how_many_digits(threadQuantity), threadId));

                    try
                    {
                        coreIterator->second->contain(threadId, tmpThread);
                    }
                    catch (OrchestratorException &e)
                    {
                        errors.push_back(e.message.c_str());
                        anyErrors = true;
                        break;
                    }
                }
            }
        }
    }

    /* Ensure mandatory fields have been defined. */
    if (!complain_if_mandatory_field_not_defined(&validFields, &fieldsFound))
    {
        anyErrors = true;
    }

    /* Restore the old 'record', 'sectionName', and 'variable' members. */
    record = callingRecord;
    sectionName = callingSectionName;
    variable = callingVariable;

    return !anyErrors;
}

/* Define the fields of a board from a typed section.
 *
 * Returns true if there were no validation problems, and false
 * otherwise. Arguments:
 *
 * - box: Pointer to the box to populate.
 * - sectionNode: The node that defines the properties of the board. */
bool HardwareFileReader::d3_define_board_fields_from_section(
    P_board* board, UIF::Node* sectionNode)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */

    /* We'll be modifying the 'record', 'sectionName', and 'variable'
     * members. To ensure they are preserved on exit, store the old values
     * here, and reinstate them once finished. */
    UIF::Node* callingRecord = record;
    std::string callingSectionName = sectionName;
    std::string callingVariable = variable;

    /* Short on time, sorry... */
    sectionName = dformat(
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
    GetRecd(sectionNode, recordNodes);

    for (std::vector<UIF::Node*>::iterator recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        record = *recordIterator;

        /* Get the value and variable nodes. */
        GetVari(record, variableNodes);
        GetValu(record, valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Is this a variable definition? (does it have variables, values, and
         * a '+' prefix)? If not, ignore it (for now). */
        if (valueNodes.size() == 0 or variableNodes.size() == 0 or
            variableNodes[0]->qop != Lex::Sy_plus){continue;}

        /* Complaints for variable definitions. */
        if (!(complain_if_variable_name_invalid(variableNodes[0],
                                                &validFields) and
              complain_if_record_is_multivariable(&variableNodes) and
              complain_if_record_is_multivalue(&valueNodes)))
        {
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        if (complain_if_variable_true_in_map(variableNodes[0], &fieldsFound))
        {
            anyErrors = true;
            continue;
        }
        variable = variableNodes[0]->str;
        fieldsFound[variable] = true;

        /* Specific logic for each variable. */
        if (variable == "board_mailbox_cost")
        {
            /* Complain if not a float. */
            if (!complain_if_value_not_floating(valueNodes[0]))
            {
                anyErrors = true;
                continue;
            }

            /* Bind */
            board->costBoardMailbox = str2float(valueNodes[0]->str);
        }

        else if (variable == "dram")
        {
            /* Complain if not natural */
            if (!complain_if_value_not_natural(valueNodes[0]))
            {
                anyErrors = true;
                continue;
            }

            /* Bind */
            board->dram = str2unsigned(valueNodes[0]->str);
        }

        else if (variable == "mailbox_mailbox_cost")
        {
            /* Complain if not a float. */
            if (!complain_if_value_not_floating(valueNodes[0]))
            {
                anyErrors = true;
                continue;
            }

            /* Hold for later use */
            defaultMailboxMailboxCost = str2float(valueNodes[0]->str);
            isDefaultMailboxCostDefined = true;
        }

        else if (variable == "supervisor_memory")
        {
            /* Complain if not natural */
            if (!complain_if_value_not_natural(valueNodes[0]))
            {
                anyErrors = true;
                continue;
            }

            /* Bind */
            board->supervisorMemory = str2unsigned(valueNodes[0]->str);
        }

        /* Types are processed in d3_get_validate_default_types, so we
         * ignore the definition here. */
        else if (variable == "type");

    }

    /* Ensure mandatory fields have been defined. */
    if (!complain_if_mandatory_field_not_defined(&mandatoryFields,
                                                 &fieldsFound))
    {
        anyErrors = true;
    }

    /* Restore the old 'record', 'sectionName', and 'variable' members. */
    record = callingRecord;
    sectionName = callingSectionName;
    variable = callingVariable;

    return !anyErrors;
}

/* Define the fields of a box from a typed section.
 *
 * Returns true if there were no validation problems, and false
 * otherwise. Arguments:
 *
 * - box: Pointer to the box to populate.
 * - sectionNode: The node that defines the properties of the box. */
bool HardwareFileReader::d3_define_box_fields_from_section(
    P_box* box, UIF::Node* sectionNode)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */

    /* We'll be modifying the 'record', 'sectionName', and 'variable'
     * members. To ensure they are preserved on exit, store the old values
     * here, and reinstate them once finished. */
    UIF::Node* callingRecord = record;
    std::string callingSectionName = sectionName;
    std::string callingVariable = variable;

    /* Short on time, sorry...*/
    sectionName = dformat(
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
    GetRecd(sectionNode, recordNodes);

    for (std::vector<UIF::Node*>::iterator recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        record = *recordIterator;

        /* Get the value and variable nodes. */
        GetVari(record, variableNodes);
        GetValu(record, valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Complaints */
        if (!(complain_if_variable_not_plus_prefixed(variableNodes[0]) and
              complain_if_variable_name_invalid(variableNodes[0],
                                                &validFields) and
              complain_if_record_is_multivariable(&variableNodes) and
              complain_if_record_is_multivalue(&valueNodes)))
        {
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        if (complain_if_variable_true_in_map(variableNodes[0], &fieldsFound))
        {
            anyErrors = true;
            continue;
        }
        variable = variableNodes[0]->str;
        fieldsFound[variable] = true;

        /* Specific logic for each variable. */
        if (variable == "box_board_cost")
        {
            /* Complain if not a float. */
            if (!complain_if_value_not_floating(valueNodes[0]))
            {
                anyErrors = true;
                continue;
            }

            /* Bind */
            box->costBoxBoard = str2float(valueNodes[0]->str);
        }

        else if (variable == "supervisor_memory")
        {
            /* Complain if not natural */
            if (!complain_if_value_not_natural(valueNodes[0]))
            {
                anyErrors = true;
                continue;
            }

            /* Bind */
            box->supervisorMemory = str2unsigned(valueNodes[0]->str);
        }
    }

    /* Ensure mandatory fields have been defined. */
    if (!complain_if_mandatory_field_not_defined(&validFields, &fieldsFound))
    {
        anyErrors = true;
    }

    /* Restore the old 'record', 'sectionName', and 'variable' members. */
    record = callingRecord;
    sectionName = callingSectionName;
    variable = callingVariable;

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
bool HardwareFileReader::d3_define_mailbox_fields_from_section(
    P_mailbox* mailbox, UIF::Node* sectionNode)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */

    /* We'll be modifying the 'record', 'sectionName', and 'variable'
     * members. To ensure they are preserved on exit, store the old values
     * here, and reinstate them once finished. */
    UIF::Node* callingRecord = record;
    std::string callingSectionName = sectionName;
    std::string callingVariable = variable;

    /* Short on time, sorry... */
    sectionName = dformat(
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
    GetRecd(sectionNode, recordNodes);

    for (std::vector<UIF::Node*>::iterator recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        record = *recordIterator;

        /* Get the value and variable nodes. */
        GetVari(record, variableNodes);
        GetValu(record, valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Complaints */
        if (!(complain_if_variable_not_plus_prefixed(variableNodes[0]) and
              complain_if_variable_name_invalid(variableNodes[0],
                                                &validFields) and
              complain_if_record_is_multivariable(&variableNodes) and
              complain_if_record_is_multivalue(&valueNodes)))
        {
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        if (complain_if_variable_true_in_map(variableNodes[0], &fieldsFound))
        {
            anyErrors = true;
            continue;
        }
        variable = variableNodes[0]->str;
        fieldsFound[variable] = true;

        /* Specific logic for each variable. */
        if (variable == "core_core_cost")
        {
            /* Complain if not a float. */
            if (!complain_if_value_not_floating(valueNodes[0]))
            {
                anyErrors = true;
                continue;
            }

            /* Bind */
            mailbox->costCoreCore = str2float(valueNodes[0]->str);
        }

        else if (variable == "cores")
        {
            /* Complain if not natural */
            if (!complain_if_value_not_natural(valueNodes[0]))
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

        else if (variable == "mailbox_core_cost")
        {
            /* Complain if not a float. */
            if (!complain_if_value_not_floating(valueNodes[0]))
            {
                anyErrors = true;
                continue;
            }

            /* Bind */
            mailbox->costMailboxCore = str2float(valueNodes[0]->str);
        }
    }

    /* Ensure mandatory fields have been defined. */
    if (!complain_if_mandatory_field_not_defined(&validFields, &fieldsFound))
    {
        anyErrors = true;
    }

    /* Restore the old 'record', 'sectionName', and 'variable' members. */
    record = callingRecord;
    sectionName = callingSectionName;
    variable = callingVariable;

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
bool HardwareFileReader::d3_populate_validate_board_with_mailboxes(
    P_board* board, UIF::Node* sectionNode)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */

    /* We'll be modifying the 'record', 'sectionName', and 'variable'
     * members. To ensure they are preserved on exit, store the old values
     * here, and reinstate them once finished. */
    UIF::Node* callingRecord = record;
    std::string callingSectionName = sectionName;
    std::string callingVariable = variable;

    /* Short on time, sorry... */
    sectionName = dformat(
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

    /* Holds information on all mailboxes in the current board. The key is the
     * name of the mailbox in this board, and the value holds the information
     * about that mailbox. */
    std::map<MailboxName, MailboxInfo> mailboxInfoFromName;

    /* For finding mailbox names in mailboxInfoFromName. */
    std::map<MailboxName, MailboxInfo>::iterator mailboxNameFinder;

    /* Holds the edge that connects to mailboxes together in a given board, if
     * any. The two elements of the pair held in the key of the map represent
     * the mailboxes at each end of the edge, and the value of the map holds
     * the mailbox information. */
    std::map<std::pair<MailboxName, MailboxName>, EdgeInfo> mailboxEdges;

    /* For finding a reverse edge, and for iterating through mailboxEdges. */
    std::map<std::pair<MailboxName, MailboxName>, EdgeInfo>::iterator \
        edgeFinder;

    /* Holds an edge-specific mailbox-mailbox cost, if found. */
    float thisEdgeCost;
    bool isThisEdgeCostDefined;

    /* Holds the name of the mailbox on the other end of an edge. */
    MailboxName edgeMailboxName;



    /* Iterate through all record nodes in this section that define
     * mailboxes. */
    std::vector<UIF::Node*> recordNodes;
    GetRecd(sectionNode, recordNodes);

    for (std::vector<UIF::Node*>::iterator recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        record = *recordIterator;
        sectionType = PNULL;

        /* Get the value and variable nodes. */
        GetVari(record, variableNodes);
        GetValu(record, valueNodes);

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
            errors.push_back(dformat(
                "L%u: Mailbox on this line has already been defined on line "
                "%u. Not making it.",
                record->pos, mailboxNameFinder->second.lineNumber));
            anyErrors = true;
            continue;
        }

        /* Is a type explicitly defined in this record? */
        if (d3_get_explicit_type_from_item_definition(variableNodes[0], &type))
        {
            /* If there's a matching section for this type, we're all good (it
             * gets written to sectionType). Otherwise, we fall back to
             * defaults. */
            anyErrors |= !d3_get_section_from_type(
                "mailbox", type, &sectionType);
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
                errors.push_back(dformat(
                    "L%u: No section found to define the mailbox on this "
                    "line. Not making it.", record->pos));
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
            MailboxInfo{record->pos, mailbox};

        /* Stage the edges from this record. Values (i.e. LHS of the '=' token)
         * each represent the name of a mailbox, optionally with a cost, which
         * defines an edge */
        for(std::vector<UIF::Node*>::iterator edgeIterator=valueNodes.begin();
            edgeIterator!=valueNodes.end(); edgeIterator++)
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
                    errors.push_back(dformat(
                        "L%u: Invalid cost on edge connecting mailbox %s to "
                        "mailbox %s (it must be a float).", record->pos,
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
                errors.push_back(dformat(
                    "L%u: No cost found for edge connecting mailbox %s to "
                    "mailbox %s (it must be a float).", record->pos,
                    mailboxName.c_str(), edgeMailboxName.c_str()));
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
                    errors.push_back(dformat(
                        "L%u: The cost of the edge connecting mailbox %s to "
                        "mailbox %s (%f) is different from the cost of its "
                        "reverse (%f), defined at L%i.",
                        record->pos,
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
                    errors.push_back(dformat(
                        "L%u: The other end of the edge connecting mailbox "
                        "%s to mailbox %s has already been defined. The first "
                        "definition was at L%i.",
                        record->pos,
                        mailboxName.c_str(),
                        edgeMailboxName.c_str(),
                        edgeFinder->second.lineNumber));
                    anyErrors = true;
                    continue;  /* Skip this edge, avoid clobbering. */
                }

                /* We're all good, mark the reverse as found. */
                edgeFinder->second.isReverseDefined = true;
            }

            /* Otherwise, track this edge in mailboxEdges. */
            else
            {
                /* But complain if it's already there (means we've defined it
                 * twice on this line, probably). */

                /* NB: Not reverse! */
                edgeFinder = mailboxEdges.find(
                    std::make_pair(mailboxName, edgeMailboxName));
                if (edgeFinder != mailboxEdges.end())
                {
                    errors.push_back(dformat(
                        "L%u: Duplicate edge definition connecting mailbox "
                        "%s to mailbox %s.",
                        record->pos,
                        mailboxName.c_str(),
                        edgeMailboxName.c_str()));
                    anyErrors = true;
                    continue;  /* Skip this edge, avoid clobbering. */
                }

                /* Okay, actually add it now. */
                mailboxEdges[std::make_pair(mailboxName, edgeMailboxName)] = \
                    EdgeInfo{thisEdgeCost, false, record->pos};
            }
        }  /* That's all the edges. */

        /* Define properties of this mailbox, and add cores. */
        if (!d3_define_mailbox_fields_from_section(mailbox, sectionType))
        {
            anyErrors = true;
        }
    }

    /* Connect mailboxes together */
    for (edgeFinder=mailboxEdges.begin(); edgeFinder!=mailboxEdges.end();
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
                errors.push_back(dformat(
                    "L%u: Could not find a definition for mailbox %s defined "
                    "by an edge in this record.",
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
            errors.push_back(dformat(
                "L%u: Could not find the reverse edge definition connecting "
                "mailboxes %s and %s.", edgeFinder->second.lineNumber,
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

    /* Restore the old 'record', 'sectionName', and 'variable' members. */
    record = callingRecord;
    sectionName = callingSectionName;
    variable = callingVariable;

    return !anyErrors;
}

/* Validate the contents of the engine_board section, and create boards and
 * items beneath. Relies on engine_box having been read.
 *
 * Returns true if all validation checks pass, and false otherwise. Arguments:
 *
 * - engine: Engine to populate with boards, and other items. */
bool HardwareFileReader::d3_populate_validate_engine_board_and_below(
    P_engine* engine)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */
    sectionName = "engine_board";

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

    /* Holds information on all boards. The key is the unique name of the
     * board, and the value holds the information about that board. */
    std::map<BoardName, BoardInfo> boardInfoFromName;

    /* For finding board names in boardInfoFromName. */
    std::map<BoardName, BoardInfo>::iterator boardNameFinder;

    /* Holds the edge that connects to boards together, if any. The two
     * elements of the pair held in the key of the map represent the boards at
     * each end of the edge, and the value of the map holds the edge
     * information. */
    std::map<std::pair<BoardName, BoardName>, EdgeInfo> boardEdges;

    /* For finding a reverse edge, and for iterating through the edge
     * container. */
    std::map<std::pair<BoardName, BoardName>, EdgeInfo>::iterator \
        edgeFinder;

    /* Holds any default board-board cost, if found. */
    float defaultCost = 0;
    bool isDefaultCostDefined = false;

    /* Holds an edge-specific board-board cost, if found. */
    float thisEdgeCost;
    bool isThisEdgeCostDefined;

    /* Holds the name of the board on the other end of an edge. */
    BoardName edgeBoardName;

    /* Iterate through all record nodes in this section, in order to:
     *
     * - Get the default board-board cost.
     * - Complain if there are duplicate entries.
     *
     * Ignore records that are not variable definitions. */
    std::vector<UIF::Node*> recordNodes;
    GetRecd(untypedSections[sectionName], recordNodes);

    std::vector<UIF::Node*>::iterator recordIterator;
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        record = *recordIterator;

        /* Get the value and variable nodes. */
        GetVari(record, variableNodes);
        GetValu(record, valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Only proceed if this record is a variable definition (it must have a
         * '+' prefix) */
        if (variableNodes[0]->qop != Lex::Sy_plus){continue;}

        /* Complaints for variable definitions. */
        if (!(complain_if_variable_name_invalid(variableNodes[0],
                                                &validFields) and
              complain_if_record_is_multivariable(&variableNodes) and
              complain_if_record_is_multivalue(&valueNodes)))
        {
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        if (complain_if_variable_true_in_map(variableNodes[0], &fieldsFound))
        {
            anyErrors = true;
            continue;
        }
        variable = variableNodes[0]->str;
        fieldsFound[variable] = true;

        /* Specific logic for each variable. */
        if (variable == "board_board_cost")
        {
            /* Complain if not a float. */
            if (!complain_if_value_not_floating(valueNodes[0]))
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
        else if (variable == "type");
    }

    /* Iterate through all record nodes in this section, in order to define
     * boards. We ignore variable definitions this time. Yeah we're iterating
     * twice... */
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        record = *recordIterator;
        sectionType = PNULL;  /* Given that this is a board record, we have to
                               * find a section that defines the properties of
                               * this board. */

        /* Get the value and variable nodes. */
        GetVari(record, variableNodes);
        GetValu(record, valueNodes);

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
            errors.push_back(dformat(
                "L%u: Board name on this line has already been defined on "
                "line %u. Not making it.",
                record->pos, boardNameFinder->second.lineNumber));
            anyErrors = true;
            continue;
        }

        /* Is a type explicitly defined in this record? */
        if (d3_get_explicit_type_from_item_definition(variableNodes[0], &type))
        {
            /* If there's a matching section for this type, we're all good (it
             * gets written to sectionType). Otherwise, we fall back to
             * defaults. */
            if (!d3_get_section_from_type("board", type, &sectionType))
            {
                anyErrors = true;
            }
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
                errors.push_back(dformat(
                    "L%u: No section found to define the board on this line. "
                    "Not making it.", record->pos));
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
            BoardInfo{record->pos, board};

        /* Stage the edges from this record. Values (i.e. LHS of the '=' token)
         * each represent the name of a board, optionally with a cost, which
         * defines an edge */
        for(std::vector<UIF::Node*>::iterator edgeIterator=valueNodes.begin();
            edgeIterator!=valueNodes.end(); edgeIterator++)
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
                    errors.push_back(dformat(
                        "L%u: Invalid cost on edge connecting board "
                        "%s(board(%s)) to board %s(board(%s)) (it must be a "
                        "float).",
                        record->pos,
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
                errors.push_back(dformat(
                    "L%u: No cost found for edge connecting board "
                    "%s(board(%s)) to board %s(board(%s)) (it must be a "
                    "float).",
                    record->pos,
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
                    errors.push_back(dformat(
                        "L%u: The cost of the edge connecting board "
                        "%s(board(%s)) to board %s(board(%s)) (%f) is "
                        "different from the cost of its reverse (%f), defined "
                        "at L%i.",
                        record->pos,
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
                    errors.push_back(dformat(
                        "L%u: The other end of the edge connecting board "
                        "%s(board(%s)) to board %s(board(%s)) has already "
                        "been defined. The first definition was at L%i.",
                        record->pos,
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
                    errors.push_back(dformat(
                        "L%u: Duplicate edge definition connecting board "
                        "%s(board(%s)) to board %s(board(%s)).",
                        record->pos,
                        boardName.first.c_str(),
                        boardName.second.c_str(),
                        edgeBoardName.first.c_str(),
                        edgeBoardName.second.c_str()));
                    anyErrors = true;
                    continue;  /* Skip this edge, avoid clobbering. */
                }

                /* Okay, actually add it now. */
                boardEdges[std::make_pair(boardName, edgeBoardName)] = \
                    EdgeInfo{thisEdgeCost, false, record->pos};
            }
        }  /* That's all the edges. */

        /* Define the properties of this board. If any are missing or broken,
         * continue (we fail slowly). */
        if (!d3_define_board_fields_from_section(board, sectionType))
        {
            anyErrors = true;
        }

        /* Populate the board with mailboxes. */
        if (!d3_populate_validate_board_with_mailboxes(board, sectionType))
        {
            anyErrors = true;
        }
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
                errors.push_back(dformat(
                    "L%u: Could not find a definition for board %s(board(%s)) "
                    "defined by an edge in this record.",
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
            errors.push_back(dformat(
                "L%u: Could not find the reverse edge definition connecting "
                "boards %s(board(%s)) and %s(board(%s)).",
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
        errors.push_back(dformat(
            "Board %s(board(%s)) has been declared in the 'engine_box' "
            "section, but not defined in the 'engine_board' section.",
            (*badBoardIterator).first.c_str(),
            (*badBoardIterator).second.c_str()));
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
bool HardwareFileReader::d3_populate_validate_engine_box(P_engine* engine)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */
    sectionName = "engine_box";

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
    GetRecd(untypedSections[sectionName], recordNodes);

    for (std::vector<UIF::Node*>::iterator recordIterator =recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        sectionName = "engine_box";  /* Will be changed by
                                      * d3_define_box_fields_from_section, so
                                      * we need to change it back in our
                                      * loop. */
        record = *recordIterator;
        sectionType = PNULL;  /* Given that this is a box record, we have to
                               * find a section that defines the properties of
                               * this box. */

        /* Get the value and variable nodes. */
        GetVari(record, variableNodes);
        GetValu(record, valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Is this a variable definition? (does it have variables, values, and
         * a '+' prefix)? (otherwise, we assume it is a box definition). */
        if (valueNodes.size() > 0 and variableNodes.size() > 0 and
            variableNodes[0]->qop == Lex::Sy_plus)
        {
            /* Complaints for variable definitions. */
            if (!(complain_if_variable_name_invalid(variableNodes[0],
                                                &validFields) and
                  complain_if_record_is_multivariable(&variableNodes) and
                  complain_if_record_is_multivalue(&valueNodes)))
            {
                anyErrors = true;
                continue;
            }


            /* Complain if duplicate. NB: We know the variable name is valid if
             * control has reached here. */
            if (complain_if_variable_true_in_map(variableNodes[0],
                                                 &fieldsFound))
            {
                anyErrors = true;
                continue;
            }
            variable = variableNodes[0]->str;
            fieldsFound[variable] = true;

            /* Specific logic for each variable. */
            if (variable == "external_box_cost")
            {
                /* Complain if not a float. */
                if (!complain_if_value_not_floating(valueNodes[0]))
                {
                    anyErrors = true;
                    continue;
                }

                /* Bind! */
                engine->costExternalBox = str2unsigned(valueNodes[0]->str);
            }

            /* Types are processed in d3_get_validate_default_types, so we
             * ignore the definition here. */
            else if (variable == "type");

            /* This continue is here to reduce the amount of indentation for
             * dealing with box records. */
            continue;
        }

        /* Otherwise, this must be a box definition. */

        /* Complain if the name is invalid (but keep going). */
        if (!complain_if_variable_not_a_valid_item_name(variableNodes[0]))
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
            errors.push_back(dformat("L%u: Box name '%s' already defined.",
                                     record->pos, boxName.c_str()));
            anyErrors = true;
            continue;
        }

        /* Is a type explicitly defined in this record? */
        if (d3_get_explicit_type_from_item_definition(variableNodes[0], &type))
        {
            /* If there's a matching section for this type, we're all good (it
             * gets written to sectionType). Otherwise, we fall back to
             * defaults. */
            if (!d3_get_section_from_type("box", type, &sectionType))
            {
                anyErrors = true;
            }
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
                errors.push_back(dformat(
                    "L%u: No section found to define the box on this line. "
                    "Not making it.", record->pos));
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

/* Returns the number of digits in the argument. */
int how_many_digits(unsigned printable){return (int)log10(printable) + 1;}
