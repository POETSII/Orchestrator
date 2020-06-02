/* See the accompanying header for more information on what this does. */
#include "Debug.h"

/* If we're debugging, DebugPrint writes the message to stdout (with
 * formatting). Otherwise, DebugPrint does nothing. */
#if ORCHESTRATOR_DEBUG
#include <stdarg.h>
#include <stdio.h>
#include "dfprintf.h"
void DebugPrint(const char* format, ...)
{
    va_list printingArguments;
    va_start(printingArguments, format);
    std::string newFormat = dformat("[DEBUG]: %s", format);
    vfprintf(stdout, newFormat.c_str(), printingArguments);
    fflush(stdout);
}
#else
void DebugPrint(const char*, ...){return;}
#endif
