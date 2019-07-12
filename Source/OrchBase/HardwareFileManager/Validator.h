#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_VALIDATOR_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_VALIDATOR_H

/* Logic for defining validation methods for nodes and other data structures
 * (which write error messages), and simple functions to convert strings to
 * intelligible values. */

#include "dfprintf.h"
#include "flat.h"
#include "uif.h"

/* Used for error messages. */
#define TYPE_REGEX "[0-9A-Za-z]{2,32}"

class Validator
{
public:
    /* Holds errors. */
    std::vector<std::string> errors;

    /* Helper members that must be set by the caller in order to have helpful
     * error messages. This is largely a convenience to stop repeated lookups
     * in the tree. */
    UIF::Node* record;
    std::string sectionName;
    std::string variable;  /* The name */

    /* Validators that write error messages. */
    bool complain_if_mandatory_field_not_defined(
        std::vector<std::string>* mandatoryFields,
        std::map<std::string, bool>* fieldsFound);
    bool complain_if_variable_not_plus_prefixed(UIF::Node* variableNode);
    bool complain_if_value_not_a_valid_type(UIF::Node* valueNode);
    bool complain_if_value_not_floating(UIF::Node* valueNode);
    bool complain_if_value_not_natural(UIF::Node* valueNode);
    bool complain_if_values_and_children_not_natural(UIF::Node* valueNode);
    bool complain_if_variable_not_a_valid_item_name(UIF::Node* variableNode);
    bool complain_if_variable_true_in_map(
        UIF::Node* variableNode,
        std::map<std::string, bool>* mapToSearch);
    bool complain_if_record_is_multivalue(std::vector<UIF::Node*>* valueNodes);
    bool complain_if_record_is_multivariable(
        std::vector<UIF::Node*>* variableNodes);
    bool complain_if_variable_name_invalid(
        UIF::Node* variableNode,
        std::vector<std::string>* validVariables);

    /* Validators */
    bool does_node_variable_have_plus_prefix(UIF::Node* variableNode);
    bool is_multivalue_record(std::vector<UIF::Node*>* valueNodes);
    bool is_multivariable_record(std::vector<UIF::Node*>* variableNodes);

    bool is_node_value_floating(UIF::Node* valueNode);
    bool is_node_value_natural(UIF::Node* valueNode);
    bool is_node_variable_true_in_map(std::map<std::string, bool>* mapToSearch,
                                      UIF::Node* variableNode);
    bool is_type_valid(UIF::Node* nameNode);
    bool is_variable_name_valid(std::vector<std::string>* validFields,
                                UIF::Node* variableNode);

};

/* Converters */
float str2float(std::string floatLike);
unsigned str2unsigned(std::string unsignedLike);

#endif
