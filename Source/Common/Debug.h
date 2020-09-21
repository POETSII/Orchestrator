#ifndef __ORCHESTRATOR_SOURCE_COMMON_DEBUG_H
#define __ORCHESTRATOR_SOURCE_COMMON_DEBUG_H
/* This header-source pair defines a simple print-debug method, which does
 * nothing if the environment variable ORCHESTRATOR_DEBUG is not true, and
 * writes to standard output otherwise. */
void DebugPrint(const char*, ...);
#endif
