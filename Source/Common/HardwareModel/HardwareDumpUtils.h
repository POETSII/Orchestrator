#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWARE_MODEL_HARDWAREDUMPUTILS_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWARE_MODEL_HARDWAREDUMPUTILS_H

/* Simple file to define some free functions to help with dumping the hardware
 * model. */

#include <algorithm>
#include <stdio.h>
#include <string>

#include "dfprintf.h"

#define MAXIMUM_BREAKER_LENGTH 80
#define GRAPH_CALLBACK static void

namespace HardwareDumpUtils {
    void breaker(FILE* outPlace, const std::string& prefix,
                 unsigned prefixLength, char breakerSymbol);

    /* Convenience aliases. */
    void close_breaker(FILE* outPlace, const std::string& prefix);
    void open_breaker(FILE* outPlace, const std::string& prefix);
}

#endif
