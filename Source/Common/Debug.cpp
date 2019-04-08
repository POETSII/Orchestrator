/* See the accompanying header for more information on what this does. */
#include "Debug.h"

/* ORCHESTRATOR_DEBUG should be defined by the compiler invocation if
 * debug-printing is desired. If it's not defined by the compiler, we assume
 * the user does not want it. */
#ifndef ORCHESTRATOR_DEBUG
#define ORCHESTRATOR_DEBUG 0
#endif

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
#else
void DebugPrint(const char*, ...){return;}
#endif
