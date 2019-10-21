#include "Pglobals.h"

const byte Q::N000;
const byte Q::EXIT;
const byte Q::CMND;
const byte Q::DEBUG;
const byte Q::LOG;
const byte Q::KEYB;
const byte Q::DUMM;
const byte Q::TEST;
const byte Q::PMAP;
const byte Q::SYST;
const byte Q::RTCL;
const byte Q::INJCT;
const byte Q::NAME;
const byte Q::SUPR;
const byte Q::TINS;
const byte Q::CANDC;
// Level 1 subkeys
const byte Q::PING;
const byte Q::POST;
const byte Q::FULL;
const byte Q::FLOO;
const byte Q::FLAG;
const byte Q::HARD;
const byte Q::KILL;
const byte Q::CONN;
const byte Q::RUN;
const byte Q::LOAD;
const byte Q::STOP;
const byte Q::TOPO;
const byte Q::SHOW;
const byte Q::ACPT;
// Nameserver additions ----------------------------------------------------
const byte Q::SEND;
const byte Q::QRY;
const byte Q::RPLY;
const byte Q::DATA;
const byte Q::CFG;
const byte Q::CMDC;
const byte Q::DUMP;
// temporary use: for MPI testing ------------------------------------------
const byte Q::M0;
const byte Q::M1;
const byte Q::MN;
//--------------------------------------------------------------------------
// Level 2 subkeys
const byte Q::REQ;
const byte Q::ACK;
const byte Q::FWD;
// Nameserver additions ---------------------------------------------------
const byte Q::TASK;
const byte Q::DEVT;
const byte Q::DEVI;
const byte Q::SUPV;
const byte Q::EXTN;
const byte Q::LIST;
const byte Q::BLD;
const byte Q::INTG;
const byte Q::STATE;
// moved from L1 to L2 for Nameserver --------------------------------------
const byte Q::DIST;
const byte Q::RECL;
const byte Q::TDIR;
const byte Q::DEL;
const byte Q::MONI;
const byte Q::LOGN;
const byte Q::DEVE;
// Level 3 subkeys
const byte Q::FALSE;
const byte Q::TRUE;
const byte Q::OFF;
const byte Q::ON;
const byte Q::NF;
const byte Q::TNF;
const byte Q::NM;
const byte Q::ID;
const byte Q::ALL;
const byte Q::NGRP;
const byte Q::IGRP;
const byte Q::NSUP;
const byte Q::ISUP;
const byte Q::IN;
const byte Q::OUT;
const byte Q::ATR;

// Not a value
const byte Q::NAV;
const int  Q::NAP;

const byte Q::ROOT;
