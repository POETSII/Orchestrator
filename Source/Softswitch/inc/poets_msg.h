#ifndef _POETS_MSG_H_
#define _POETS_MSG_H_

#include <cstddef>
#include <cstdint>
#include "config.h"           // Tinsel config

#include "poets_hardware.h"
#include "SoftwareAddressDefs.h"


//------------------------------------------------------------------------------
// Definitions for instrumentation timing
//------------------------------------------------------------------------------
#ifdef TinselClockFreq 
 #define P_DEFAULT_INSTRUMENTATION_INTERVAL     (TinselClockFreq * 1000000)
#else
 #define P_DEFAULT_INSTRUMENTATION_INTERVAL     240000000    //~1s at 240 MHz
 #warning "TinselClockFreq not defined: Assuming 240MHz clock"
#endif

#ifndef P_INSTR_INTERVAL
#define P_INSTR_INTERVAL P_DEFAULT_INSTRUMENTATION_INTERVAL
#endif
//------------------------------------------------------------------------------


#define DEST_BROADCAST 0xFFFFFFFF       // still used in P_builder to id supervisor pins
#define P_SUP_PIN_INIT 0                // very temporary bodge for __init__ pins

#define LOG_DEVICES_PER_THREAD 10       // Seems to be used in P_builder. May conflict with build_derfs definition

//------------------------------------------------------------------------------
// Used in GetHWAddr
//------------------------------------------------------------------------------
#define P_THREAD_HWOS 0
#define P_CORE_HWOS (LOG_THREADS_PER_CORE+P_THREAD_HWOS)
#define P_MAILBOX_HWOS (TinselLogCoresPerMailbox+P_CORE_HWOS)
#define P_BOARD_HWOS (LOG_CORES_PER_BOARD+P_CORE_HWOS)
#define P_BOX_HWOS (LOG_BOARDS_PER_BOX+P_BOARD_HWOS)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Message fragmentation parameters
//------------------------------------------------------------------------------
#define P_MAX_LOGMSG_FRAG       4
#define P_LOG_MAX_LOGMSG_FRAG   2
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Header bitmasks and bitshifts
//------------------------------------------------------------------------------
#define P_HW_HWADDR_MASK            0xFFFFFFFF
#define P_HW_HWADDR_SHIFT           0
    
#define P_SW_MOTHERSHIP_MASK        ISMOTHERSHIP_BIT_MASK       
#define P_SW_MOTHERSHIP_SHIFT       ISMOTHERSHIP_SHIFT
#define P_SW_CNC_MASK               ISCNC_BIT_MASK
#define P_SW_CNC_SHIFT              ISCNC_SHIFT
#define P_SW_TASK_MASK              TASK_BIT_MASK
#define P_SW_TASK_SHIFT             TASK_SHIFT
#define P_SW_OPCODE_MASK            OPCODE_BIT_MASK
#define P_SW_OPCODE_SHIFT           OPCODE_SHIFT
#define P_SW_DEVICE_MASK            DEVICE_BIT_MASK
#define P_SW_DEVICE_SHIFT           DEVICE_SHIFT


#define P_HD_TGTPIN_MASK            0xFF
#define P_HD_TGTPIN_SHIFT           0
#define P_HD_DESTEDGEINDEX_MASK     0xFFFFFF00
#define P_HD_DESTEDGEINDEX_SHIFT    8
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Magic Addresses
//------------------------------------------------------------------------------
#define P_ADDR_BROADCAST            0xFFFF


#define P_CNC_MAX_USER              0xEF

#define P_CNC_INSTR                 0xFA
#define P_CNC_LOG                   0xFB
#define P_CNC_BARRIER               0xFC
#define P_CNC_INIT                  0xFD
#define P_CNC_STOP                  0xFE
#define P_CNC_KILL                  0xFF

//------------------------------------------------------------------------------

#pragma pack(push,1)
typedef struct poets_message_header
{
    // Begin flit 0
    // Header
    //uint32_t hwAddr;
    uint32_t swAddr;
    uint32_t pinAddr;
    // End Header.
    // 64-bits ~~~32-bits~~~ left in the first flit.
    
    /*
    uint32_t destDeviceAddr;   // Opaque destination device address
    uint32_t destEdgeIndex;    // Input edge index at the destination
    uint8_t  destPin;          // Input pin index at the destination
    uint8_t  messageLenBytes;  // Total message length including header
    uint16_t messageTag;       // Type of message
    */
} P_Msg_Hdr_t;


typedef struct poets_message
{
    P_Msg_Hdr_t header; // a message always has a header      
    uint8_t payload[P_MSG_MAX_SIZE-sizeof(P_Msg_Hdr_t)]; // and it might have some more data
    // In the default tinsel configuration the first flit has 4 bytes of payload
    // Further flits contain data.
} P_Msg_t;

typedef struct poets_log_message_payload
{
    uint8_t seq;
    uint8_t payload[P_MSG_MAX_SIZE-(sizeof(P_Msg_Hdr_t)+sizeof(uint8_t))];
} P_Log_Msg_Pyld_t;

typedef struct poets_instr_message_payload
{
    // First Flit
    uint32_t cIDX;
    uint32_t cycles;
    
    // Second Flit
    uint32_t rxCnt;
    uint32_t rxHanCnt;
    uint32_t txCnt;
    uint32_t supCnt;
    
    // Third Flit
    uint32_t txHanCnt;
    uint32_t idleCnt;
    uint32_t idleHanCnt;
#if TinselEnablePerfCount == true   
    uint32_t missCount;         // Cache miss count
    
    // Fourth Flit
    uint32_t hitCount;          // Cache hit count
    uint32_t writebackCount;    // Cache writeback count
    uint32_t CPUIdleCount;      // CPU idle-cycle count (lower 32 bits)
    
    // 32-bits free
#endif 
    
} P_Instr_Msg_Pyld_t;


typedef struct poets_supervisor_message
{
    uint32_t hwAddr;    // The destination HW address
    uint32_t len;       // Number of bytes for the P_Msg_t
    P_Msg_t msg;        // The POETs fabric message
} P_Super_Msg_t;


const unsigned int p_msg_pyld_size = sizeof(P_Msg_t)-sizeof(P_Msg_Hdr_t);
const unsigned int p_logmsg_pyld_size = sizeof(P_Msg_t)
                                         -(sizeof(P_Msg_Hdr_t)+sizeof(uint8_t));
const unsigned int p_logmsg_hdr_size = sizeof(P_Msg_Hdr_t)+sizeof(uint8_t);

const unsigned int p_instrmsg_pyld_size = sizeof(P_Instr_Msg_Pyld_t);

inline size_t p_super_msg_size() {return sizeof(P_Super_Msg_t);}
inline size_t p_msg_size() {return sizeof(P_Msg_t);}
inline size_t p_hdr_size() {return sizeof(P_Msg_Hdr_t);}

// message buffers (last argument) in the message setters must be volatile because we might wish
// to write directly to a hardware resource containing the buffer, which in general may be volatile.
unsigned set_msg_hdr(uint8_t, uint8_t, uint8_t, uint8_t, uint32_t, uint8_t, 
                     uint32_t, uint8_t, P_Msg_Hdr_t*);

unsigned pack_msg(uint8_t, uint8_t, uint8_t, uint8_t, uint32_t, uint8_t, 
                   uint32_t, uint8_t, void*, P_Msg_Hdr_t*);


#pragma pack(pop)
#endif
