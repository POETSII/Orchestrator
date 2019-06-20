#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_NODEVALIDATION_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_NODEVALIDATION_H

/* Logic for defining validation methods for nodes (which write error
 * messages), and simple functions to convert strings to intelligible
 * values. */

#include "dfprintf.h"
#include "flat.h"
#include "uif.h"

/* Validators */
bool is_value_at_node_natural(UIF::Node* recordNode, UIF::Node* valueNode,
                              std::string variable, std::string value,
                              std::string sectionName,
                              std::string* errorMessage);

bool is_value_at_node_floating(UIF::Node* recordNode, UIF::Node* valueNode,
                               std::string variable, std::string value,
                               std::string sectionName,
                               std::string* errorMessage);

bool are_values_at_node_natural(UIF::Node* recordNode,
                                UIF::Node* valueNode, std::string variable,
                                std::string value, std::string sectionName,
                                std::string* errorMessage);

/* Converters */
float str2float(std::string floatLike);

unsigned str2unsigned(std::string unsignedLike);

#endif
