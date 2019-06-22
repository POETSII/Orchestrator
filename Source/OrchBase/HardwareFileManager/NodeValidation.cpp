/* Defines node validation mechanism (see the accompanying
 * header for further information). */

#include "NodeValidation.h"

/* Returns whether the value at a node is a positive floating-point number (or
 * an integer), and appends an error message to a string if it is
 * not. Arguments:
 *
 * - recordNode: Record parent node (for writing error message).
 * - valueNode: The UIF node that holds the value.
 * - variable: Variable name string (for writing error message).
 * - value: Value string (for writing error message).
 * - sectionName: The name of the section the record lives in (for writing
     error message).
 * - errorMessage: String to append the error message to, if any. */
bool complain_if_node_value_not_floating(
    UIF::Node* recordNode, UIF::Node* valueNode, std::string variable,
    std::string value, std::string sectionName, std::string* errorMessage)
{
    if (!is_node_value_floating(valueNode))
    {
        errorMessage->append(dformat(
            "L%u: Variable '%s' in section '%s' has value '%s', which is not "
            "a positive floating-point number or a positive integer.\n",
            recordNode->pos, variable.c_str(), sectionName.c_str(),
            value.c_str()));
        return false;
    }
    return true;
}

/* Returns whether the value at a node is a natural number, and appends an
 * error message to a string if it is not. Arguments:
 *
 * - recordNode: Record parent node (for writing error message).
 * - valueNode: The UIF node that holds the value.
 * - variable: Variable name string (for writing error message).
 * - value: Value string (for writing error message).
 * - sectionName: The name of the section the record lives in (for writing
     error message).
 * - errorMessage: String to append the error message to, if any. */
bool complain_if_node_value_not_natural(
    UIF::Node* recordNode, UIF::Node* valueNode, std::string variable,
    std::string value, std::string sectionName, std::string* errorMessage)
{
    if (!is_node_value_natural(valueNode))
    {
        errorMessage->append(dformat(
            "L%u: Variable '%s' in section '%s' has value '%s', which is not "
            "a natural number.\n",
            recordNode->pos, variable.c_str(), sectionName.c_str(),
            value.c_str()));
        return false;
    }
    return true;
}

/* Returns whether the variable held by a node has a plux prefix, and appends
 * an error message to a string if it is not. Arguments:
 *
 * - recordNode: Record parent node (for writing error message).
 * - variableNode: The UIF node that holds the variable.
 * - sectionName: The name of the section the record lives in (for writing
     error message).
 * - errorMessage: String to append the error message to, if any. */
bool complain_if_node_not_plus_prefixed(
    UIF::Node* recordNode, UIF::Node* variableNode, std::string sectionName,
    std::string* errorMessage)
{
    if (!does_node_variable_have_plus_prefix(variableNode))
    {
        errorMessage->append(dformat(
            "L%u: Variable in record in section '%s' is not prefixed by a '+' "
            "character.\n", recordNode->pos, sectionName.c_str()));
        return false;
    }
    return true;
}


/* Returns whether the value at a node, and all of its children, are natural
 * numbers, and appends an error message to a string if they are
 * not. Arguments:
 *
 * - recordNode: Record parent node (for writing error message).
 * - valueNode: The UIF node that holds the value.
 * - variable: Variable name string (for writing error message).
 * - value: Value string (for writing error message).
 * - sectionName: The name of the section the record lives in (for writing
     error message).
 * - errorMessage: String to append the error message to, if any. */
bool complain_if_nodes_values_not_natural(
    UIF::Node* recordNode, UIF::Node* valueNode, std::string variable,
    std::string value, std::string sectionName, std::string* errorMessage)
{
    std::vector<UIF::Node*>::iterator valueNodeIterator;
    bool valid = true;
    for (valueNodeIterator=valueNode->leaf.begin();
         valueNodeIterator!=valueNode->leaf.end(); valueNodeIterator++)
    {
        valid &= complain_if_node_value_not_natural(
            recordNode, (*valueNodeIterator),
            variable, (*valueNodeIterator)->str.c_str(),
            sectionName.c_str(), errorMessage);
    }
    return valid;
}

/* Returns whether the variable at a node is prefixed with a "+"
 * token. Arguments:
 *
 * - varaibleNode: The UIF node that holds the variable. */
bool does_node_variable_have_plus_prefix(UIF::Node* variableNode)
{
    return (variableNode->qop == Lex::Sy_plus);
}

/* Returns whether the value at a node is a positive floating-point
 * number. Arguments:
 *
 * - valueNode: The UIF node that holds the value. */
bool is_node_value_floating(UIF::Node* valueNode)
{
    return (valueNode->qop == Lex::Sy_FSTR or valueNode->qop == Lex::Sy_ISTR);
}

/* Returns whether the value at a node is a natural number. Arguments:
 *
 * - valueNode: The UIF node that holds the value. */
bool is_node_value_natural(UIF::Node* valueNode)
{
    return (valueNode->qop == Lex::Sy_ISTR);
}

/* Returns whether the name at a node is a valid type name (i.e. must satisfy
 * [0-9A-Za-z]{2,32}. Arguments:
 *
 * - nameNode: The UIF node that holds the name. */
bool is_type_valid(UIF::Node* nameNode)
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
