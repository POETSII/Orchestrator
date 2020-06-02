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
MSHP |ACK  |DEFD |-    | (0:string)Application name
MSHP |ACK  |LOAD |-    | (0:string)Application name
MSHP |ACK  |RUN  |-    | (0:string)Application name
MSHP |ACK  |STOP |-    | (0:string)Application name

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

Mothership
----------
EXIT |-    |-    |-    | (None)
SYST |KILL |-    |-    | (None)
APP  |SPEC |-    |-    | (0:string)Application name
                         (1:uint32_t)Number of expected distribution messages
APP  |DIST |-    |-    | (0:string)Application name
                         (1:string)Code path for this core
                         (2:string)Data path for this core
                         (3:uint32_t)Core hardware address
                         (4:uint8_t)Number of expected threads for this core
APP  |SUPD |-    |-    | (0:string)Application name
                         (1:string)Shared object path for this Supervisor
CMND |RECL |-    |-    | (0:string)Application name
CMND |INIT |-    |-    | (0:string)Application name
CMND |RUN  |-    |-    | (0:string)Application name
CMND |STOP |-    |-    | (0:string)Application name
BEND |CNC  |-    |-    | (0:P_Pkt_t)Packet
BEND |SUPR |-    |-    | (0:P_Pkt_t)Packet
PKTS |-    |-    |-    | (0:vector<pair<uint32_t, P_Pkt_t> >)Packets
DUMP |-    |-    |-    | (0:string)Path to write the dump to

*/


class Q {
// Holder class for all the static constants; or things that behave like static
// constants, anyway. All the names are very terse 'cos this is a very 'thin'
// class that gets invoked a lot, and you know me and typing.

public:
// Top level (0) subkeys
static const byte N000  = 0x00;
static const byte EXIT  = 0x01;
static const byte CMND  = 0x02;
static const byte DEBUG = 0x03;
static const byte LOG   = 0x04;
static const byte KEYB  = 0x05;
static const byte DUMM  = 0x06;
static const byte TEST  = 0x07;
static const byte PMAP  = 0x08;
static const byte SYST  = 0x09;
static const byte RTCL  = 0x0a;
static const byte INJCT = 0x0b;
static const byte CANDC = 0x0c;
static const byte APP   = 0x0d;
static const byte BEND  = 0x0e;
static const byte PKTS  = 0x0f;
static const byte DUMP  = 0x10;
static const byte MSHP  = 0x11;
// Level 1 subkeys
static const byte PING  = 0x40;
static const byte POST  = 0x41;
static const byte FULL  = 0x42;
static const byte FLOO  = 0x43;
static const byte FLAG  = 0x44;
static const byte KILL  = 0x45;
static const byte CONN  = 0x46;
static const byte RUN   = 0x47;
static const byte LOAD  = 0x48;
static const byte STOP  = 0x49;
static const byte DIST  = 0x4a;
static const byte RECL  = 0x4b;
static const byte ACPT  = 0x4c;
static const byte SPEC  = 0x4d;
static const byte SUPD  = 0x4e;
static const byte INIT  = 0x4f;
static const byte CNC   = 0x50;
static const byte ACK   = 0x51;
static const byte SUPR  = 0x52;
// temporary use: for MPI testing ------------------------------------------
static const byte M0    = 0x60;
static const byte M1    = 0x61;
static const byte MN    = 0x62;
//--------------------------------------------------------------------------
// Level 2 subkeys
static const byte REQ   = 0x80;
static const byte FWD   = 0x81;
// Level 3 subkeys
static const byte DEFD  = 0xc0;

// Not a value
static const byte NAV   = 0xff;
static const int  NAP   = -1;

static const byte ROOT  = 0x00;

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

};

//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//==============================================================================
          /*
class Now {
// This class provides the global real time clock, in the sense that now()
// provides the MPI time since the whole thing started. It's not that accurate,
// but it's a start. Perfect is the enemy of good.
// Default constructor and destructor are provided just for the sake of
// completeness. There is a single 'base time' so that the returned current time
// is relative to when the program(s) started. Note it's not accessible to
// anywhere apart from InitMPI, whose job it is to initialise the clock.
// IT DOES NOT FOLLOW (DESPITE WHAT MPI SAYS) THAT THE PROCESS CLOCKS ARE LOCKED

// TO DO: the friend should be the InitMPI constructor, but I don't know how
// to specify this without documentation at the moment

private : Now();
virtual ~ Now();
private : static double baseT;
//friend void InitMPI(int &,int &,int &,char *&);
friend class InitMPI;
public : static double now() { return MPI_Wtime() - baseT; }
};

//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

double Now::baseT = 0.0;               // Keep the linker happy...
            */
//==============================================================================


#endif
