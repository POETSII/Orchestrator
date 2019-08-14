#include "build_defs.h"

using namespace std;

#if (defined __BORLANDC__ || defined _MSC_VER)
const string SYS_COPY = "copy";
const string MAKEDIR = "md";
const string RECURSIVE_CPY = "";
const string REMOVEDIR = "rd /S /Q";
#elif (defined __GNUC__)
const string SYS_COPY = "cp";
const string RECURSIVE_CPY = "-r";
const string PERMISSION_CPY = "-p";
const string MAKEDIR = "mkdir";
const string REMOVEDIR = "rm --force --recursive";
#endif

const unsigned int MAX_HANDLERS = 1024;
const unsigned int BYTES_PER_THREAD = 0x1 << (TinselLogBytesPerDRAM - TinselLogThreadsPerDRAM);
const unsigned int MAX_DEVICES_PER_THREAD = 1024;
const unsigned int LOG_DEVICES_PER_THREAD = 10;
const unsigned int THREADS_PER_CORE = 0x1 << TinselLogThreadsPerCore;
const unsigned int CORES_PER_BOARD = 0x1 << TinselLogCoresPerBoard;
const unsigned int BOARDS_PER_BOX = TinselMeshXLenWithinBox*TinselMeshYLenWithinBox;
const unsigned long long MEM_PER_BOARD = ((long long) 0x1 << TinselLogBytesPerDRAM) * TinselDRAMsPerBoard;
const unsigned int PIN_POS = 24;
const bool         SHARED_INSTR_MEM = TinselSharedInstrMem;
const string STATIC_SRC_PATH = "../";
const string COMMON_PATH = "Softswitch";
const string TINSEL_PATH = "Tinsel";
const string ORCH_PATH = "Orchestrator";
const string GENERATED_PATH = "Generated";
const string GENERATED_H_PATH = GENERATED_PATH+"/inc";
const string GENERATED_CPP_PATH = GENERATED_PATH+"/src";
const string COMMON_SRC_PATH = STATIC_SRC_PATH+COMMON_PATH;
const string TINSEL_SRC_PATH = STATIC_SRC_PATH+TINSEL_PATH;
const string ORCH_SRC_PATH = STATIC_SRC_PATH+ORCH_PATH;
const string BIN_PATH = "bin";
const string BUILD_PATH = "Build";
const string COREMAKE_BASE = "make all 2>&1 >> make_errs.txt";
const string COREBIN_BASE = "softswitch_";
const void* DRAM_BASE = 0; // temporary: this should be set to the actual DRAM bottom address.
