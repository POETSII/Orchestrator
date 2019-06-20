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

bool complain_if_nodes_values_not_natural(
    UIF::Node* recordNode, UIF::Node* valueNode, std::string variable,
    std::string value, std::string sectionName, std::string* errorMessage);

/* Validators */
bool is_node_value_floating(UIF::Node* valueNode);
bool is_node_value_natural(UIF::Node* valueNode);

/* Converters */
float str2float(std::string floatLike);
unsigned str2unsigned(std::string unsignedLike);

#endif
