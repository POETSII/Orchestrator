/* Defines behaviour for the dialect 3 functionality of methods that get
 * attributes from items (either variables or values) from the UIF data
 * structure. */

#include "HardwareFileReader.h"

/* Extract the address (addr) from an item definition.
 *
 * Concatenates and drops the address into 'address', and sets it to zero if
 * there was no address defined. Does no validation (which is a mistake, but
 * I'm tight for time). Returns true if an address was found, and false
 * otherwise. Arguments:
 *
 * - itemNode: Variable node to extract the type from.
 * - address: Integer to write to. */
bool HardwareFileReader::d3_get_address_from_item_definition(
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
bool HardwareFileReader::d3_get_board_name(
    UIF::Node* itemNode, BoardName* boardName)
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
        errors.push_back(dformat(
            "L%u: Box component of board name '%s' at C%u does not correspond "
            "to an existing box.",
            record->pos, boxName.c_str(), itemNode->pos));
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
        errors.push_back(dformat(
            "L%u: Couldn't find a board field in this board definition near "
            "C%u (so I don't know what the name is).",
            record->pos, itemNode->pos));
        return false;
    }

    /* Does the board component have zero or multiple values? If so, that's not
     * valid. */
    if ((*leafIterator)->leaf.size() != 1)
    {
        errors.push_back(dformat("L%u: Multiple board components defined for "
                                 "the name of this board (only one is "
                                 "allowed).", record->pos));
        return false;
    }

    /* Is the board-component of the name valid? */
    if (!complain_if_variable_not_a_valid_item_name((*leafIterator)->leaf[0]))
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
bool HardwareFileReader::d3_get_explicit_cost_from_edge_definition(
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
bool HardwareFileReader::d3_get_explicit_type_from_item_definition(
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
bool HardwareFileReader::d3_get_mailbox_name(UIF::Node* itemNode,
                                             MailboxName* mailboxName)
{
    /* Clear the mailbox name. */
    *mailboxName = "";

    /* Get the mailbox name, and check that it's valid. */
    if (!complain_if_variable_not_a_valid_item_name(itemNode))
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
 * - sectionNode: Pointer to write the found section to, set to PNULL if no
     node could be found. */
bool HardwareFileReader::d3_get_section_from_type(
    std::string itemType, std::string type, UIF::Node** sectionNode)
{
    *sectionNode = PNULL;

    /* Iterator to find typed sections. Note that this only refers to the inner
     * map of typedSections. */
    std::map<std::string, UIF::Node*>::iterator typedSectionIterator;

    /* Go hunting. */
    typedSectionIterator = typedSections[itemType].find(type);
    if (typedSectionIterator == typedSections[itemType].end())
    {
        errors.push_back(dformat("L%u: Type '%s' defined in the '%s' section "
                                 "does not correspond to a section.",
                                 record->pos, type.c_str(),
                                 sectionName.c_str()));
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
bool HardwareFileReader::d3_get_validate_default_types(
    UIF::Node** globalDefaults)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */
    sectionName = "default_types";

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
    std::vector<UIF::Node*> recordNodes;
    GetRecd(untypedSectionsIterator->second, recordNodes);

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
        std::string itemType;
        unsigned arrayIndex;
        if (variable == "box_type")
        {
            itemType = "box";
            arrayIndex = box;
        }
        else if (variable == "board_type")
        {
            itemType = "board";
            arrayIndex = board;
        }
        else if (variable == "mailbox_type")
        {
            itemType = "mailbox";
            arrayIndex = mailbox;
        }

        /* Figure out if there is a section corresponding to this type. If not,
         * whine loudly. Either way, assign to globalDefaults. */
        if (!d3_get_section_from_type(itemType, valueNodes[0]->str,
                                      &(globalDefaults[arrayIndex])))
        {
            anyErrors = true;
        }
    }

    return !anyErrors;
}
