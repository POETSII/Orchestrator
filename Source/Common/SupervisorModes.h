#ifndef _SUPERVISOR_MODES_H_
#define _SUPERVISOR_MODES_H_

// Define operation in single-supervisor or multi-supervisor mode, where:
//
//  - Single-supervisor mode is where applications are serviced by one
//    supervisor device on the cluster host machine, regardless of where the
//    application is placed (even if it is placed across multiple boxes).
//
//  - Multi-supervisor mode is where applications are serviced by one
//    supervisor device for each box the application is placed on. Each normal
//    device is serviced by the supervisor device running on the same box as
//    the normal device.
//
// At time of writing (2020-11-18), multi-supervisor mode is not supported.
#ifndef SINGLE_SUPERVISOR_MODE
#define SINGLE_SUPERVISOR_MODE 1
#else
#define SINGLE_SUPERVISOR_MODE 0
#endif

#if SINGLE_SUPERVISOR_MODE
#define SUPER_SEND(PAYLOAD) tinselSend(tinselHostId(), PAYLOAD)
#else
#define SUPER_SEND(PAYLOAD) tinselSend(tinselMyBridgeId(), PAYLOAD)
#endif

#endif
