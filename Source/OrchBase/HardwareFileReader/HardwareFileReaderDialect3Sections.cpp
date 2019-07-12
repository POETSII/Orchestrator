/* Defines behaviour for the dialect 3 functionality of methods that preloads
 * information about sections and types. */

#include "HardwareFileReader.h"

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
bool HardwareFileReader::d3_load_validate_sections()
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

    /* Holding error messages constructed in stages. */
    std::string ruleErrors;

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
    GetNames(allNodes);

    ruleErrors.clear();

    std::vector<UIF::Node*>::iterator nodeIterator;
    for (nodeIterator=allNodes.begin(); nodeIterator!=allNodes.end();
         nodeIterator++)
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
                ruleErrors = "Sections with invalid names found:";
            }

            else
            {
                ruleErrors.append(",");
            }

            ruleErrors.append(dformat(" %s (L%i)",
                                     (*nodeIterator)->str.c_str(),
                                     (*nodeIterator)->pos));
        }
    }

    /* Add any errors found for this rule to the error container. */
    if (ruleFailure[5])
    {
        errors.push_back(ruleErrors.c_str());
    }

    /* Rule (0): For each of the rules[0] strings... */
    ruleErrors.clear();
    for (nameIterator=rules[currentRule].begin();
         nameIterator!=rules[currentRule].end(); nameIterator++)
    {
        if (sectionsByName[*nameIterator].size() != 1)
        {
            /* On the first pass, add this header to the message. */
            if (!ruleFailure[currentRule])
            {
                ruleFailure[currentRule] = true;
                ruleErrors = "The following sections were not defined exactly "
                             "once:";
            }

            else
            {
                ruleErrors.append(",");
            }

            /* I'm going to write the line numbers of each node, or "not
             * defined" if the section is not defined. A little unusual because
             * I want the printing to be pretty. */
            ruleErrors.append(dformat(" %s (", nameIterator->c_str()));

            if (sectionsByName[*nameIterator].empty())
            {
                errors.push_back("not defined");
            }

            else
            {
                nodeIterator=sectionsByName[*nameIterator].begin();
                errors.push_back(dformat("L%u", (*nodeIterator)->pos));
                for (;
                     nodeIterator!=sectionsByName[*nameIterator].end();
                     nodeIterator++)
                {
                    errors.push_back(dformat(", L%u", (*nodeIterator)->pos));
                }
            }

            errors.push_back(")");
        }
    }

    /* Add any errors found for this rule to the error container. */
    if (ruleFailure[currentRule])
    {
        errors.push_back(ruleErrors.c_str());
    }

    /* Rule (1) */
    currentRule++;
    ruleErrors.clear();
    for (nameIterator=rules[currentRule].begin();
         nameIterator!=rules[currentRule].end(); nameIterator++)
    {
        if (sectionsByName[*nameIterator].empty())
        {
            /* On the first pass, add this header to the message. */
            if (!ruleFailure[currentRule])
            {
                ruleFailure[currentRule] = true;
                ruleErrors = "The following sections were not defined when "
                    "they should have been defined one time or more:";
            }

            else
            {
                ruleErrors.append(",");
            }

            ruleErrors.append(dformat(" %s", nameIterator->c_str()));
        }
    }

    /* Add any errors found for this rule to the error container. */
    if (ruleFailure[currentRule])
    {
        errors.push_back(ruleErrors.c_str());
    }

    /* Rule (2) */
    currentRule++;
    ruleErrors.clear();
    for (nameIterator=rules[currentRule].begin();
         nameIterator!=rules[currentRule].end(); nameIterator++)
    {
        if (sectionsByName[*nameIterator].size() >= 2)
        {
            /* On the first pass, add this header to the message. */
            if (!ruleFailure[currentRule])
            {
                ruleFailure[currentRule] = true;
                ruleErrors = "The following sections were defined more than "
                             "once, when they should have been defined one or "
                             "zero times:";
            }

            else
            {
                ruleErrors.append(",");
            }

            /* I'm going to write the line numbers of each node. */
            errors.push_back(dformat(" %s (", nameIterator->c_str()));
            nodeIterator=sectionsByName[*nameIterator].begin();
            errors.push_back(dformat("L%u", (*nodeIterator)->pos));
            for (;
                 nodeIterator!=sectionsByName[*nameIterator].end();
                 nodeIterator++)
            {
                errors.push_back(dformat(", L%u", (*nodeIterator)->pos));
            }
            errors.push_back(")\n");
        }
    }

    /* Add any errors found for this rule to the error container. */
    if (ruleFailure[currentRule])
    {
        errors.push_back(ruleErrors.c_str());
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
                errors.push_back(dformat(
                    "L%u: Section '%s' has no associated type.",
                    (*nodeIterator)->pos, (*nameIterator).c_str()));
            }

            else if (arguments.size() > 1)
            {
                ruleFailure[currentRule] = true;
                errors.push_back(dformat(
                    "L%u: Section '%s' has more than one type associated with "
                    "it.", (*nodeIterator)->pos, (*nameIterator).c_str()));
            }

            else if (!is_type_valid(arguments[0]))
            {
                ruleFailure[currentRule] = true;
                errors.push_back(dformat(
                    "L%u: Type '%s' in section '%s' is not a valid type (it "
                    "must satisfy %s).", (*nodeIterator)->pos,
                    arguments[0]->str.c_str(), (*nameIterator).c_str(),
                    TYPE_REGEX));
            }

            /* If we've already seen this type defined for a section of this
             * sort (e.g. 'board' or 'mailbox'), then add to error message. */
            else if (std::find(declaredTypes.begin(), declaredTypes.end(),
                               arguments[0]->str) != declaredTypes.end())
            {
                ruleFailure[currentRule] = true;
                errors.push_back(dformat(
                    "L%u: Duplicate definition of section '%s' with type "
                    "'%s'.", (*nodeIterator)->pos, (*nameIterator).c_str(),
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
                errors.push_back(dformat(
                    "L%u: Section '%s' has a type, when it should not.",
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

/* Validate type defaults, and populate the defaultTypes map with section
 * references for each section that can create items.
 *
 * Returns true if all validation checks pass, and false otherwise */
bool HardwareFileReader::d3_validate_types_define_cache()
{
    bool anyErrors;

    /* Container to hold default sections for boxes, boards, and mailboxes. */
    UIF::Node* globalDefaults[ITEM_ENUM_LENGTH] = {PNULL, PNULL, PNULL};

    /* Populate our new friend. */
    anyErrors = !d3_get_validate_default_types(globalDefaults);

    /* Staging vector and iterator for records. */
    std::vector<UIF::Node*> recordNodes;
    std::vector<UIF::Node*>::iterator recordIterator;

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
            record = *recordIterator;

            /* Get the value and variable nodes. */
            GetVari(record, variableNodes);
            GetValu(record, valueNodes);

            /* Ignore this record if the record has not got a variable/value
             * pair (i.e. if the line is empty, or is just a comment). */
            if (variableNodes.size() == 0 and
                valueNodes.size() == 0){continue;}

            /* Is the variable name "type"? */
            if (variableNodes[0]->str == "type")
            {
                /* Complaints */
                if (!(complain_if_variable_not_plus_prefixed(variableNodes[0])
                      and
                      complain_if_record_is_multivariable(&variableNodes) and
                      complain_if_record_is_multivalue(&valueNodes)))
                {
                    anyErrors = true;
                    continue;
                }

                /* We got it! But complain if we've already found a type field
                 * in this way. */
                if (typeFieldFound)
                {
                    errors.push_back(dformat(
                        "L%u: Duplicate definition of field 'type' in the "
                        "'%s' section (previously defined at L%u).",
                        record->pos, sectionName.c_str(), typeLine));
                    anyErrors = true;
                    continue;
                }

                typeFieldFound = true;
                type = valueNodes[0]->str;
                typeLine = record->pos;
            }

            /* The variable of this node wasn't named "type", so we ignore it
             * for our current machinations. */
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
                errors.push_back(dformat(
                    "L%u: Type '%s' defined in the '%s' section does not "
                    "correspond to a section.",
                    typeLine, type.c_str(), sectionName.c_str()));
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
