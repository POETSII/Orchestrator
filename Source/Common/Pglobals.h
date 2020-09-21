#ifndef __PGlobalsH__H
#define __PGlobalsH__H

#include "flat.h"

//==============================================================================
/*
Monkey input

Message layouts
===============

CommonBase
----------
EXIT |-    |-    |-    | (None)
PMAP |-    |-    |-    | (1:int)urank
                         (2:string)Process name
                         (3:string)Class
                         (4:unsigned)Bits per word
                         (5:string)Compiler
                         (6:string)OS
                         (7:string)Source
                         (8:string)Binary
                         (9:string)Time
                         (10:string)Date
                         (11:int)Thread_type
SYST |CONN |-    |-    | (0:string)Service name
SYST |ACPT |-    |-    |
SYST |PING |ACK  |-    | (0:double)Request send MPI time
                         (1:double)Request arrival MPI time
                         (1:string)Target (foldback) class
                         (2:string)Request OS date from source
                         (3:string)Request OS time from source
                         (4:string)Acknowledge OS date from source
                         (5:string)Acknowledge OS time from source
                         (0:int)Target (foldback) rank
                         (4:unsigned)Ping attempt
SYST |PING |REQ  |-    | (1:string)Target class
                         (2:string)Request OS date
                         (3:string)Request OS time
                         (4:unsigned)Ping attempt
                         (0:int)Sending rank (Why???)
SYST |RUN  |-    |     | (0:vector<int>)Number of instances of a process to run
                         (1:vector<string>)Target host
                         (2:vector<string>)Process name to run
TEST |FLOO |-    |-    | (1:unsigned)width
                         (2:unsigned)level

Root
----
KEYB |-    |-    |-    | (1:char)...'\0'
TEST |-    |-    |-    | (1:char)...'\0'
LOG  |FULL |-    |-    | (1:int)Message id
                         (2:char)Message type
                         (3:string)Full message
INJCT|REQ  |-    |-    | (1:string)Command string

LogServer
---------
LOG  |POST |-    |-    | (1:int)Message id
                         (1:vector<string>)Argument list

RTCL
----
RTCL |-    |-    |-    | (1:string)Command string
CMND |EXIT |-    |-    | (None)

Injector
--------
INJCT|ACK  |-    |-    | (????)
INJCT|FLAG |-    |-    | (1:string)Command string

Mothercore
----------
CMND |LOAD |-    |-    | (0:string)Task name
CMND |RUN  |-    |-    | (0:string)Task name
CMND |STOP |-    |-    | (0:string)Task name
EXIT |-    |-    |-    | (None)
NAME |DIST |-    |-    | (0:string)Task name
                         (1:vector<pair<uint32_t,P_addr_t>>) Core list for Mothership
NAME |RECL |-    |-    | (0:string)Task name
NAME |TDIR |-    |-    | (0:string)Task name
                         (1:string)File directory
SUPR |-    |-    |-    | (0:vector<P_Sup_Msg_t>)Args
SYST |HARD |-    |-    | (0:vector<string>)Args
SYST |KILL |-    |-    | (None)
SYST |SHOW |-    |-    | (None)
SYST |TOPO |-    |-    | (None)
TINS |-    |-    |-    | (0:vector<P_Msg_t>) Packet(s) to deliver

*/


class Q {
// Holder class for all the static constants; or things that behave like static
// constants, anyway. All the names are very terse 'cos this is a very 'thin'
// class that gets invoked a lot, and you know me and typing.

public:
// Top level (0) subkeys
static const byte N000;
static const byte EXIT;
static const byte CMND;
static const byte DEBUG;
static const byte LOG;
static const byte KEYB;
static const byte DUMM;
static const byte TEST;
static const byte PMAP;
static const byte SYST;
static const byte RTCL;
static const byte INJCT;
static const byte NAME;
static const byte SUPR;
static const byte TINS;
static const byte CANDC;
static const byte QUERY;
static const byte APP;
// Level 1 subkeys
static const byte PING;
static const byte POST;
static const byte FULL;
static const byte FLOO;
static const byte FLAG;
static const byte HARD;
static const byte KILL;
static const byte CONN;
static const byte RUN;
static const byte LOAD;
static const byte STOP;
static const byte TOPO;
static const byte DIST;
static const byte RECL;
static const byte TDIR;
static const byte SHOW;
static const byte ACPT;
static const byte CLEAR;
static const byte DUMP;
static const byte MONI;
// temporary use: for MPI testing ------------------------------------------
static const byte M0;
static const byte M1;
static const byte MN;
//--------------------------------------------------------------------------
// Level 2 subkeys
static const byte REQ;
static const byte ACK;
static const byte FWD;
// Level 3 subkeys

static const byte NAV;                 // Not a value
static const int  NAP;                 // Not a process

static const byte ROOT;

// Process names - defined as Cstrings 'cos you can't initialise string() here
#define csNOTaPROCproc   "Not a process"
#define csROOTproc       "Root:OrchBase:CommonBase"
#define csDUMMYproc      "Dummy:CommonBase"
#define csLOGSERVERproc  "LogServer:CommonBase"
#define csRTCLproc       "RTCL:CommonBase"
#define csINJECTORproc   "Injector:CommonBase"
#define csNAMESERVERproc "NameServer:CommonBase"
#define csMONITORproc    "Monitor:CommonBase"
#define csMOTHERSHIPproc "Mothership:CommonBase"
#define csMPITESTproc    "MPITest:CommonBase"

// tag defined as a directive because MPI libraries are c-based, have no concept
// of const, and thus calls to them would whinge with a const-qualified argument
#define POETS_TAG 0x1234

const static string sline;             // Pretty-print separators
const static string dline;
const static string aline;
const static string eline;
const static string pline;

};

//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//==============================================================================
  
#endif
