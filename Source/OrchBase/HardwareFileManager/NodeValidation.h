#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_NODEVALIDATION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_NODEVALIDATION_H

/* Logic for defining validation methods for nodes (which write error
 * messages), and simple functions to convert strings to intelligible
 * values. */

#include "dfprintf.h"
#include "flat.h"
#include "uif.h"

/* Validators that write error messages */
bool complain_if_node_value_not_floating(
    UIF::Node* recordNode, UIF::Node* valueNode, std::string variable,
    std::string sectionName, std::string* errorMessage);

bool complain_if_node_value_not_natural(
    UIF::Node* recordNode, UIF::Node* valueNode, std::string variable,
    std::string sectionName, std::string* errorMessage);

bool complain_if_node_variable_true_in_map(
    UIF::Node* recordNode, UIF::Node* variableNode,
    std::map<std::string, bool>* mapToSearch, std::string sectionName,
    std::string* errorMessage);

bool complain_if_node_not_plus_prefixed(
    UIF::Node* recordNode, UIF::Node* variableNode, std::string sectionName,
    std::string* errorMessage);

bool complain_if_nodes_values_not_natural(
    UIF::Node* recordNode, UIF::Node* valueNode, std::string variable,
    std::string sectionName, std::string* errorMessage);

bool complain_if_record_is_multivalue(
    UIF::Node* recordNode, std::vector<UIF::Node*>* variableNodes,
    std::string sectionName, std::string* errorMessage); /* Synonym */

bool complain_if_record_is_multivariable(
    UIF::Node* recordNode, std::vector<UIF::Node*>* variableNodes,
    std::string sectionName, std::string* errorMessage);

bool complain_if_variable_name_invalid(
    UIF::Node* recordNode, UIF::Node* variableNode,
    std::vector<std::string>* validFields, std::string sectionName,
    std::string* errorMessage);

/* Validators */
bool does_node_variable_have_plus_prefix(UIF::Node* variableNode);
bool is_node_value_floating(UIF::Node* valueNode);
bool is_node_value_natural(UIF::Node* valueNode);
bool is_type_valid(UIF::Node* nameNode);
bool is_node_variable_true_in_map(std::map<std::string, bool>* mapToSearch,
                                  UIF::Node* variableNode);
bool is_variable_name_valid(std::vector<std::string>* validFields,
                            UIF::Node* variableNode);
bool is_multivariable_record(std::vector<UIF::Node*>* variableNodes);

/* Converters */
float str2float(std::string floatLike);
unsigned str2unsigned(std::string unsignedLike);

#endif
