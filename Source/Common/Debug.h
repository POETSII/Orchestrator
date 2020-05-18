#ifndef __ORCHESTRATOR_SOURCE_COMMON_DEBUG_H
#define __ORCHESTRATOR_SOURCE_COMMON_DEBUG_H
/* This header-source pair defines a simple print-debug method, which does
 * nothing if the preprocessor variable ORCHESTRATOR_DEBUG is not true, and
 * writes to standard output otherwise. ORCHESTRATOR_DEBUG should be defined by
 * the compiler invocation if debug-printing is desired. If it's not defined by
 * the compiler, we assume the user does not want it. */
#ifndef ORCHESTRATOR_DEBUG
#define ORCHESTRATOR_DEBUG 0
#endif
void DebugPrint(const char*, ...);
#endif
