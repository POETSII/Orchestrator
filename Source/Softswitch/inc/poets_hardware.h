#ifndef _POETS_HARDWARE_H_
#define _POETS_HARDWARE_H_

#include <config.h>

#define P_PKT_MAX_SIZE (0x1 << TinselLogBytesPerMsg)
#define LOG_BOARDS_PER_BOX (TinselMeshXBitsWithinBox+TinselMeshYBitsWithinBox)
#define LOG_CORES_PER_BOARD TinselLogCoresPerBoard
#define LOG_THREADS_PER_CORE TinselLogThreadsPerCore
#define NUM_BOARDS_PER_BOX (TinselMeshXLenWithinBox*TinselMeshYLenWithinBox)

#endif
