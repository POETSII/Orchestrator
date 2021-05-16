#ifndef __ORCHESTRATOR_SOURCE_COMMON_DUMPUTILS_H
#define __ORCHESTRATOR_SOURCE_COMMON_DUMPUTILS_H

/* Simple file to define some free functions to help with dumping things. */

#include <algorithm>
#include <stdio.h>
#include <string>

#include "dfprintf.h"
#include "DumpChan.h"

#define MAXIMUM_BREAKER_LENGTH 80
#define GRAPH_CALLBACK static void

namespace DumpUtils {
    void breaker(FILE* outPlace, const std::string& prefix,
                 unsigned prefixLength, char breakerSymbol);

    /* Convenience aliases. */
    void close_breaker(FILE* outPlace, const std::string& prefix);
    void open_breaker(FILE* outPlace, const std::string& prefix);
}

#endif
