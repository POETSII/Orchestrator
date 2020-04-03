/* See the accompanying header for more information on what this does. */
#include "Debug.h"

/* If we're debugging, DebugPrint writes the message to stdout (with
 * formatting). Otherwise, DebugPrint does nothing. */
#if ORCHESTRATOR_DEBUG
#include <stdarg.h>
#include <stdio.h>
void DebugPrint(const char* format, ...)
{
    va_list printingArguments;
    va_start(printingArguments, format);
    vfprintf(stdout, format, printingArguments);
    fflush(stdout);
}
void debug_print(const char* format, ...)
{
    va_list printingArguments;
    va_start(printingArguments, format);
    vfprintf(stdout, format, printingArguments);
    fflush(stdout);
}
#else
void DebugPrint(const char*, ...){return;}
void debug_print(const char*, ...){return;}
#endif
