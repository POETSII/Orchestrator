/* This header file includes a series of header files for the hardware model,
 * and exists as a convenience mechanism for sources that require the entire
 * model.
 *
 * There are no header guards in this header file, because the "lower-level"
 * headers themselves have guards. */

class P_engine;
class P_box;
class P_board;
class P_mailbox;
class P_core;
class P_thread;

#include "P_engine.h"
#include "P_box.h"
#include "P_board.h"
#include "P_mailbox.h"
#include "P_core.h"
#include "P_thread.h"
