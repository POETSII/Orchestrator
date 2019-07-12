/* Defines node validation mechanism (see the accompanying
 * header for further information). */

#include "Validator.h"

/* Returns whether all of the mandatory strings (elements of mandatoryFields)
 * map to true in the fieldsFound map, and false otherwise (even if the string
 * is not in the map's keys). Appends an error message to a string for each
 * field not found.
 *
 * But really, just conveniently checks that all fields have been defined in a
 * section, to reduce copypasta.
 *
 * Arguments:
 *
 * - mandatoryFields: Strings in this vector denote the fields that must be
 *   defined in this section.
 * - fieldsFound: Whether or not the fields in mandatoryFields have been
 *   defined. May contain strings that are not in mandatoryFields. */
bool Validator::complain_if_mandatory_field_not_defined(
    std::vector<std::string>* mandatoryFields,
    std::map<std::string, bool>* fieldsFound)
{
    bool returnValue = true;  /* Innocent until proven guilty. */
    std::vector<std::string>::iterator fieldIterator;
    std::map<std::string, bool>::iterator mapRecord;

    bool isThisFieldTrue;
    for (fieldIterator=mandatoryFields->begin();
         fieldIterator!=mandatoryFields->end(); fieldIterator++)
    {
        /* Mark as a failure if either the record in the map was false, or we
         * couldn't find the record. */
        isThisFieldTrue = true;  /* Also innocent until proven guilty. */

        /* Is the field a key in the map? */
        mapRecord = fieldsFound->find(*fieldIterator);
        if (mapRecord != fieldsFound->end())  /* We got him, boss. */
        {
            if (mapRecord->second == false)
            {
                /* The fields is false in the map. */
                isThisFieldTrue = false;
            }
        }
        else /* Key wasn't in the map. */
        {
            isThisFieldTrue = false;
        }

        /* Armageddon is here. https://www.youtube.com/watch?v=lcB58I7gSyA */
        if (!isThisFieldTrue)
        {
            errors.push_back(dformat("Variable '%s' not defined in the '%s' "
                                     "section.",
                                     (*fieldIterator).c_str(),
                                     sectionName.c_str()));
            returnValue = false;
        }
    }

    return returnValue;
}

/* Returns whether the variable held by a node has a plux prefix, and appends
 * an error message to a string if it is not. Arguments:
 *
 * - variableNode: The UIF node that holds the variable. */
bool Validator::complain_if_variable_not_plus_prefixed(UIF::Node* variableNode)
{
    if (!does_node_variable_have_plus_prefix(variableNode))
    {
        errors.push_back(dformat(
            "L%u: Variable in record in the '%s' section is not prefixed by a "
            "'+' character.", record->pos, sectionName.c_str()));
        return false;
    }
    return true;
}

/* Returns whether the value at a node is a valid type, and appends an error
 * message to a string if it is not. Arguments:
 *
 * - valueNode: The UIF node that holds the value. */
bool Validator::complain_if_value_not_a_valid_type(UIF::Node* valueNode)
{
    if (!is_type_valid(valueNode))
    {
        errors.push_back(dformat(
            "L%u: Value in record '%s' in the '%s' section is not a valid "
            "type (it must satisfy %s).", record->pos,
            valueNode->str.c_str(), sectionName.c_str(), TYPE_REGEX));
        return false;
    }
    return true;
}

/* Returns whether the value at a node is a positive floating-point number (or
 * an integer), and appends an error message to a string if it is
 * not. Arguments:
 *
 * - valueNode: The UIF node that holds the value. */
bool Validator::complain_if_value_not_floating(UIF::Node* valueNode)
{
    if (!is_node_value_floating(valueNode))
    {
        errors.push_back(dformat(
            "L%u: Variable '%s' in the '%s' section has value '%s', which is "
            "not a positive floating-point number or a positive integer.",
            record->pos, variable.c_str(), sectionName.c_str(),
            valueNode->str.c_str()));
        return false;
    }
    return true;
}

/* Returns whether the value at a node is a natural number, and appends an
 * error message to a string if it is not. Arguments:
 *
 * - valueNode: The UIF node that holds the value. */
bool Validator::complain_if_value_not_natural(UIF::Node* valueNode)
{
    if (!is_node_value_natural(valueNode))
    {
        errors.push_back(dformat(
            "L%u: Variable '%s' in the '%s' section has value '%s', which is "
            "not a natural number.",
            record->pos, variable.c_str(), sectionName.c_str(),
            valueNode->str.c_str()));
        return false;
    }
    return true;
}

/* Returns whether the value at a node, and all of its children, are natural
 * numbers, and appends an error message to a string if they are
 * not. Arguments:
 *
 * - valueNode: The UIF node that holds the value. */
bool Validator::complain_if_values_and_children_not_natural(
    UIF::Node* valueNode)
{
    std::vector<UIF::Node*>::iterator valueNodeIterator;
    bool valid = true;
    for (valueNodeIterator=valueNode->leaf.begin();
         valueNodeIterator!=valueNode->leaf.end(); valueNodeIterator++)
    {
        valid &= complain_if_value_not_natural(*valueNodeIterator);
    }
    return valid;
}

/* Returns whether the variable at a node is a valid item name, and appends an
 * error message to a string if it is not. Arguments:
 *
 * - variableNode: The UIF node that holds the variable. */
bool Validator::complain_if_variable_not_a_valid_item_name(
    UIF::Node* variableNode)
{
    if (!is_type_valid(variableNode))  /* It's the same rule! */
    {
        errors.push_back(dformat(
            "L%u: Item name '%s' is not a valid item name (it must satisfy "
            "%s).", record->pos, variableNode->str.c_str(), TYPE_REGEX));
        return false;
    }
    return true;
}

/* Returns whether the variable at a node maps to true, and appends an error
 * message if it does so. Arguments:
 *
 * - variableNode: The UIF node that holds the variable.
 * - mapToSearch: The map, mapping names of variables, to a boolean. In most
 *   cases where this is used, this boolean represents whether or not the name
 *   has already been defined in this pass of a section before. */
bool Validator::complain_if_variable_true_in_map(
    UIF::Node* variableNode, std::map<std::string, bool>* mapToSearch)
{
    if(is_node_variable_true_in_map(mapToSearch, variableNode))
    {
        errors.push_back(dformat(
            "L%u: Duplicate definition of variable '%s' in the '%s' section.",
            record->pos, sectionName.c_str()));
        return true;
    }
    return false;
}

/* As with complain_if_record_is_multivariable, but for values and GetValu
 * (which are handled slightly dirrerently). Arguments:
 *
 * - valueNodes: Pointer to a vector holding all value nodes (probably
 *   obtained using JNJ::GetValu). */
bool Validator::complain_if_record_is_multivalue(
    std::vector<UIF::Node*>* valueNodes)
{
    if (is_multivalue_record(valueNodes))
    {
        errors.push_back(dformat(
            "L%u: Variable '%s' in the '%s' section is invalid because it "
            "defines zero, or multiple, values.",
            record->pos, variable.c_str(), sectionName.c_str()));
        return false;
    }
    return true;
}

/* Returns true if the variable vector obtained from a record (using GetVari)
 * is single-variable, or false if it is multi-variable or does not define a
 * variable, and appends an error message to a string if false. Arguments:
 *
 * - variableNodes: Pointer to a vector holding all variable nodes (probably
 *   obtained using JNJ::GetVari). */
bool Validator::complain_if_record_is_multivariable(
    std::vector<UIF::Node*>* variableNodes)
{
    if (is_multivariable_record(variableNodes))
    {
        errors.push_back(dformat(
            "L%u: Record in the '%s' section is invalid because it defines "
            "zero, or multiple, variables.",
            record->pos, sectionName.c_str()));
        return false;
    }
    return true;
}

/* Returns whether the variable at a node is in a given vector of valid
 * variable names, and appends an error message to a string if it is
 * not. Arguments:
 *
 * - variableNode: The UIF node that holds the variable.
 * - validFields: Strings denoting valid field names. */
bool Validator::complain_if_variable_name_invalid(
    UIF::Node* variableNode, std::vector<std::string>* validFields)
{
    if (!is_variable_name_valid(validFields, variableNode))
    {
        errors.push_back(dformat(
            "L%u: Variable name '%s' is not valid in the '%s' section.",
            record->pos, variableNode->str.c_str(), sectionName.c_str()));
        return false;
    }
    return true;
}

/* Returns whether the variable at a node is prefixed with a "+"
 * token. Arguments:
 *
 * - varaibleNode: The UIF node that holds the variable. */
bool Validator::does_node_variable_have_plus_prefix(UIF::Node* variableNode)
{
    return (variableNode->qop == Lex::Sy_plus);
}

/* Returns true if the value vector obtained from a record is multi-valued or
 * has no value, and false otherwise. Arguments:
 *
 * - valueNodes: All of the variables associated with a record (probably
 *   obtained using JNJ::GetValu). */
bool Validator::is_multivalue_record(std::vector<UIF::Node*>* valueNodes)
{
    if (valueNodes->size() > 0)  /* Are there any values? */
    {
        /* If there is only one value, the first value node contains the
         * value. If there are multiple (N) value nodes, the first value node
         * contains N value nodes, each with an entry. */
        return ((*valueNodes)[0]->leaf.size() != 0);
    }
    return true;
}

/* Returns true if the variable vector obtained from a record is
 * multi-variable or has no value, and false otherwise. Arguments:
 *
 * - variableNodes: All of the variables associated with a record (probably
 *   obtained using JNJ::GetVari). */
bool Validator::is_multivariable_record(std::vector<UIF::Node*>* variableNodes)
{
    return (variableNodes->size() != 1);
}

/* Returns whether the value at a node is a positive floating-point
 * number. Arguments:
 *
 * - valueNode: The UIF node that holds the value. */
bool Validator::is_node_value_floating(UIF::Node* valueNode)
{
    return (valueNode->qop == Lex::Sy_FSTR or valueNode->qop == Lex::Sy_ISTR);
}

/* Returns whether the value at a node is a natural number. Arguments:
 *
 * - valueNode: The UIF node that holds the value. */
bool Validator::is_node_value_natural(UIF::Node* valueNode)
{
    return (valueNode->qop == Lex::Sy_ISTR);
}

/* Returns whether the variable at a node maps to true. Arguments:
 *
 * - mapToSearch: The map, mapping names of variables, to a boolean. In most
 *   cases where this is used, this boolean represents whether or not the name
 *   has already been defined in this pass of a section before.
 * - variableNode: The UIF node that holds the variable.
 *
 * Will throw if the map does not contain variableNode's string name. */
bool Validator::is_node_variable_true_in_map(
    std::map<std::string, bool>* mapToSearch, UIF::Node* variableNode)
{
    return mapToSearch->at(variableNode->str);
}

/* Returns whether the name at a node is a valid type name (i.e. must satisfy
 * [0-9A-Za-z]{2,32}. Arguments:
 *
 * - nameNode: The UIF node that holds the name. */
bool Validator::is_type_valid(UIF::Node* nameNode)
{
    std::string toValidate = nameNode->str;

    /* Check length. */
    if (toValidate.size() < 2 or toValidate.size() > 32)
    {
        return false;
    }

    /* Check for alphanumeric-ness. */
    for (unsigned zI=0; zI<toValidate.size(); zI++)
    {
        if (!std::isalnum(toValidate[zI]))
        {
            return false;
        }
    }

    return true;
}

/* Returns whether the variable at a node is in a given vector of valid
 * variable names. Arguments:
 *
 * - validFields: Strings denoting valid field names.
 * - variableNode: The UIF node that holds the variable. */
bool Validator::is_variable_name_valid(std::vector<std::string>* validFields,
                                       UIF::Node* variableNode)
{
    return (std::find(validFields->begin(), validFields->end(),
                      variableNode->str) != validFields->end());
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
