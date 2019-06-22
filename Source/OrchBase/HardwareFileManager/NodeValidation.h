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
    std::string value, std::string sectionName, std::string* errorMessage);

bool complain_if_node_value_not_natural(
    UIF::Node* recordNode, UIF::Node* valueNode, std::string variable,
    std::string value, std::string sectionName, std::string* errorMessage);

bool complain_if_node_not_plus_prefixed(
    UIF::Node* recordNode, UIF::Node* variableNode, std::string sectionName,
    std::string* errorMessage);

bool complain_if_nodes_values_not_natural(
    UIF::Node* recordNode, UIF::Node* valueNode, std::string variable,
    std::string value, std::string sectionName, std::string* errorMessage);

/* Validators */
bool does_node_variable_have_plus_prefix(UIF::Node* variableNode);
bool is_node_value_floating(UIF::Node* valueNode);
bool is_node_value_natural(UIF::Node* valueNode);
bool is_type_valid(UIF::Node* nameNode);

/* Converters */
float str2float(std::string floatLike);
unsigned str2unsigned(std::string unsignedLike);

#endif
