/* Defines behaviour for the dialect 1 functionality of the hardware model file
 * parser (see the accompanying header for further information). */

#include "HardwareFileParser.h"

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
    d3_errors.clear();

    /* Check sections are defined correctly. Not much point continuing if they
     * are not defined correctly. */
    if (!d3_load_validate_sections())
    {
        throw HardwareSemanticException(d3_errors.c_str());
    }

    /* Populate the engine with information from the header section. */
    bool failedValidation = false;
    failedValidation |= !d3_populate_validate_from_header_section(engine);

    /* Populate the hardware address format owned by the engine. */
    failedValidation != !d3_populate_validate_address_format(engine)
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
                                         *nameIterator));
            }

            else if (arguments.size() > 1)
            {
                ruleFailure[currentRule] = true;
                d3_errors.append(dformat("L%u: Section '%s' has more than one "
                                         "type associated with it.\n",
                                         (*nodeIterator)->pos));
            }

            else if (!is_type_valid(arguments[0]))
            {
                ruleFailure[currentRule] = true;
                d3_errors.append(dformat(
                    "L%u: Section '%s' has invalid type '%s'. It must satisfy "
                    "[0-9A-Za-z]{2,32}\n.",
                    (*nodeIterator)->pos, arguments[0]->str));
            }

            /* If we've already seen this type defined for a section of this
             * sort (e.g. 'board' or 'mailbox'), then add to error message. */
            else if (std::find(declaredTypes.begin(), declaredTypes.end(),
                               arguments[0]->str) != declaredTypes.end())
            {
                ruleFailure[currentRule] = true;
                d3_errors.append(dformat(
                    "L%u: Duplicate definition of section '%s' with type "
                    "'%s'.\n", (*nodeIterator)->pos, arguments[0]->str));
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

/* Validate the contents of the packet_address_format section, and populate the
 * format object in the engine with them.
 *
 * Returns true if all validation checks pass, and false otherwise. Arguments:
 *
 * - engine: Engine to populate */
bool HardwareFileParser::d3_populate_validate_address_format(P_engine* engine)
{
    bool anyErrors = false;
    std::string sectionName = "packet_address_format"

    /* Valid fields for the header section (all are mandatory). */
    std::vector<std::string> validFields;
    std::vector<std::string>::iterator fieldIterator;
    validFields.push_back("box");
    validFields.push_back("board");
    validFields.push_back("mailbox");
    validFields.push_back("core");
    validFields.push_back("thread");

    /* Iterate through all record nodes in the packet_address_format
     * section. */
    std::vector<UIF::Node*> recordNodes;
    std::vector<UIF::Node*>::iterator recordIterator;
    GetRecd(untypedSections[sectionName], recordNodes);
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        /* Get the value and variable nodes. */
        GetVari((*recordIterator), variableNodes);
        GetValu((*recordIterator), valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 || valueNodes.size() == 0){continue;}

        /* Complain if the record does not begin with a "+", as all fields in
         * this section must do. */
        if (!complain_if_node_not_plus_prefixed(
                *recordIterator, variableNodes[0], sectionName, &d3_errors))
        {
            anyErrors = true;
            continue;
        }
    }

    return anyErrors;

}

/* Validate the contents of the header section, and populate an engine with
 * them.
 *
 * Returns true if all validation checks pass, and false otherwise. Arguments:
 *
 * - engine: Engine to populate */
bool HardwareFileParser::d3_populate_validate_from_header_section(
    P_engine* engine)
{
    bool anyErrors = false;
    std::string sectionName = "header"

    /* Valid fields for the header section. */
    std::vector<std::string> validFields;
    std::vector<std::string>::iterator fieldIterator;
    validFields.push_back("author");
    validFields.push_back("datetime");
    validFields.push_back("dialect");
    validFields.push_back("file");
    validFields.push_back("hardware");
    validFields.push_back("version");

    /* Mandatory fields for the header section. */
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

    /* Iterate through all record nodes in the header section. */
    std::vector<UIF::Node*> recordNodes;
    std::vector<UIF::Node*>::iterator recordIterator;
    GetRecd(untypedSections[sectionName], recordNodes);
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        /* Get the value and variable nodes. */
        GetVari((*recordIterator), variableNodes);
        GetValu((*recordIterator), valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 || valueNodes.size() == 0){continue;}

        /* Complain if the record does not begin with a "+", as all fields in
         * the header section must do. */
        if (!complain_if_node_not_plus_prefixed(
                *recordIterator, variableNodes[0], sectionName, &d3_errors))
        {
            anyErrors = true;
            continue;
        }

        /* Complain if the variable name is not a valid name. */
        if (!complain_if_variable_name_invalid(
                *recordIterator, variableNodes[0], &validFields, sectionName,
                &d3_errors))
        {
            anyErrors = true;
            continue;
        }

        /* Complain if there is more than one variable node. */
        if (variableNodes.size() > 1)
        {
            d3_errors.append(dformat("L%u: Invalid multi-component variable "
                                     "name in the '%s' section.\n",
                                     (*recordIterator)->pos,
                                     sectionName.c_str()));
            anyErrors = true;
            continue;
        }

        /* Complain if there is more than one value node. */
        if (valueNodes.size() > 1)
        {
            d3_errors.append(dformat(
                "L%u: Only one value can be bound to variable '%s' in the "
                "'%s' section.\n",
                (*recordIterator)->pos, variableNodes[0]->str.c_str(),
                sectionName.c_str));
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        if (fieldsFound[variableNodes[0]->str])
        {
            d3_errors.append(dformat(
                "L%u: Duplicate definition of variable '%s' in the '%s' "
                "section.\n",
                (*recordIterator)->pos, variableNodes[0]->str.c_str(),
                sectionName.c_str()));

            anyErrors = true;
            continue;
        }
        fieldsFound[variableNodes[0]->str] = true;

        /* Specific logic for each variable. */
        if (variableNodes[0]->str == "author")
        {
            engine->author = valueNodes[0]->str;
        }

        else if (variableNodes[0]->str == "datetime")
        {
            /* Special validation for this one. */
            if (valueNodes[0]->qop != Lex::Sy_ISTR)
            {
                d3_errors.append(dformat(
                    "L%u: Variable '%s' in the '%s' section has value '%s', "
                    "which is not a datetime in the form YYYYMMDDhhmmss.\n",
                    (*recordIterator)->pos, variableNodes[0]->str.c_str(),
                    valueNodes[0]->str.c_str(), sectionName.c_str()));
                anyErrors = true;
            }

            else
            {
                engine->datetime = str2long(valueNodes[0]->str);
            }

        }

        /* This has already been read and validated, so we don't care. */
        else if (variableNodes[0]->str == "dialect"){}

        else if (variableNodes[0]->str == "file")
        {
            engine->fileOrigin = valueNodes[0]->str;
        }

        /* Ignore this one for now. */
        else if (variableNodes[0]->str == "hardware"){}

        else if (variableNodes[0]->str == "version")
        {
            engine->version = valueNodes[0]->str;
        }

        /* Shouldn't be able to enter this, because we've already checked the
         * variable names, but why not write some more code. It's not like this
         * file is big enough already. */
        else
        {
            d3_errors.append(dformat("L%u: Variable name '%s' is not valid in "
                                     "the '%s' section (E2).\n",
                                     (*recordIterator)->pos,
                                     variableNodes[0]->str,
                                     sectionName.c_str()));
            anyErrors = true;
        }
    }

    /* Ensure mandatory fields have been defined, inefficiently. */
    for (fieldIterator=mandatoryFields.begin();
         fieldIterator!=mandatoryFields.end(); fieldIterator++)
    {
        if(!fieldsFound[*fieldIterator])
        {
            d3_errors.append(dformat("[ERROR] Variable '%s' not defined in "
                                     "the '%s' section.", *fieldIterator,
                                     sectionName.c_str()));
            anyErrors = true;
        }
    }

    return anyErrors;
}
