#ifndef __BUILD_DEFS_H__
#define __BUILD_DEFS_H__

#include <string>
#include "tinsel-config.h"

extern const std::string SYS_COPY;
extern const std::string RECURSIVE_CPY;
extern const std::string PERMISSION_CPY;
extern const std::string MAKEDIR;
extern const std::string REMOVEDIR;

extern const unsigned int MAX_RTSBUFFSIZE;
extern const unsigned int MAX_HANDLERS;
extern const unsigned int BYTES_PER_THREAD;
extern const unsigned int MAX_DEVICES_PER_THREAD;
extern const unsigned int LOG_DEVICES_PER_THREAD;
extern const unsigned int THREADS_PER_CORE;
extern const unsigned int CORES_PER_BOARD;
extern const unsigned int BOARDS_PER_BOX;
extern const unsigned long long MEM_PER_BOARD;
extern const unsigned int PIN_POS;
extern const bool         SHARED_INSTR_MEM;
extern const std::string STATIC_SRC_PATH;
extern const std::string COMMON_PATH;
extern const std::string TINSEL_PATH;
extern const std::string ORCH_PATH;
extern const std::string GENERATED_PATH;
extern const std::string GENERATED_H_PATH;
extern const std::string GENERATED_CPP_PATH;
extern const std::string COMMON_SRC_PATH;
extern const std::string TINSEL_SRC_PATH;
extern const std::string ORCH_SRC_PATH;
extern const std::string BIN_PATH;
extern const std::string BUILD_PATH;
extern const std::string COREMAKE_BASE;
extern const std::string COREBIN_BASE;
extern const void* DRAM_BASE; // temporary: this should be set to the actual DRAM bottom address.

#endif
