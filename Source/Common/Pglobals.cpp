#include "Pglobals.h"

const byte Q::N000    = 0x00;
const byte Q::EXIT    = 0x01;
const byte Q::CMND    = 0x02;
const byte Q::DEBUG   = 0x03;
const byte Q::LOG     = 0x04;
const byte Q::KEYB    = 0x05;
const byte Q::DUMM    = 0x06;
const byte Q::TEST    = 0x07;
const byte Q::PMAP    = 0x08;
const byte Q::SYST    = 0x09;
const byte Q::RTCL    = 0x0a;
const byte Q::INJCT   = 0x0b;
const byte Q::CANDC   = 0x0c;
const byte Q::APP     = 0x0d;
const byte Q::BEND    = 0x0e;
const byte Q::PKTS    = 0x0f;
const byte Q::DUMP    = 0x10;
const byte Q::MSHP    = 0x11;
const byte Q::PATH    = 0x12;
// Level 1 subkeys
const byte Q::PING    = 0x40;
const byte Q::POST    = 0x41;
const byte Q::FULL    = 0x42;
const byte Q::FLOO    = 0x43;
const byte Q::FLAG    = 0x44;
const byte Q::HARD    = 0x45;
const byte Q::KILL    = 0x46;
const byte Q::CONN    = 0x47;
const byte Q::RUN     = 0x48;
const byte Q::LOAD    = 0x49;
const byte Q::STOP    = 0x4a;
const byte Q::DIST    = 0x4b;
const byte Q::RECL    = 0x4c;
const byte Q::TDIR    = 0x4d;
const byte Q::SHOW    = 0x4e;
const byte Q::ACPT    = 0x4f;
const byte Q::SPEC    = 0x50;
const byte Q::SUPD    = 0x51;
const byte Q::INIT    = 0x52;
const byte Q::CNC     = 0x53;
const byte Q::ACKt     = 0x54;
const byte Q::SUPR    = 0x55;
// temporary use: for MPI testing ------------------------------------------
const byte Q::M0      = 0x60;
const byte Q::M1      = 0x61;
const byte Q::MN      = 0x62;
//--------------------------------------------------------------------------
// Level 2 subkeys
const byte Q::REQ     = 0x80;
const byte Q::ACK     = 0x81;
const byte Q::FWD     = 0x82;
const byte Q::DEFD    = 0x83;
// Level 3 subkeys

const byte Q::NAV     = 0xff;          // Not a value
const int  Q::NAP     = -1;            // Not a process

const byte Q::ROOT    = 0x00;

const string Q::sline = string(120,'-');   // Pretty-print separators
const string Q::dline = string(120,'.');
const string Q::aline = string(120,'*');
const string Q::eline = string(120,'=');
const string Q::pline = string(120,'+');

//==============================================================================
