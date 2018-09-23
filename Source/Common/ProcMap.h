#ifndef __ProcMapH__H
#define __ProcMapH__H

#include <stdio.h>
#include "mpi.h"
#include "PMsg_p.hpp"
#include <vector>
#include <string>
using namespace std;
class CommonBase;

//==============================================================================

class ProcMap
{
public:

struct ProcMap_t {
           ProcMap_t();
  int      P_rank;                     // Process rank
  string   P_proc;                     // Machine on which we run
  string   P_user;                     // Username
  string   P_class;                    // Parent object (derived) class name
  unsigned P_BPW;                      // Compiled bits per word
  string   P_OS;                       // Compiled operating system
  string   P_compiler;                 // Compiler
  string   P_source;                   // Derived class main source file
  string   P_binary;                   // Process binary
  string   P_TIME;                     // Compilation time
  string   P_DATE;                     // Compilation date
  int      P_ttype;                    // Perceived MPI thread handling model
  bool     P_tig;                      // Local MPI time_is_global opinion
  double   P_tick;                     // Local MPI timer tick
};
vector<ProcMap_t> vPmap;


                       ProcMap(CommonBase *);
virtual ~              ProcMap();
void                   Dump(FILE * = stdout);
void                   GetProcs(vector<ProcMap_t> &);
string                 Name(int rank) { return M[rank]; }
void                   Register(PMsg_p *);

CommonBase *           par;

struct U_t {                           // Special ranks
              U_t();
  int         Root;
  int         LogServer;
  int         RTCL;
  vector<int> Dummy;
  int         Injector;
  int         NameServer;
  vector<int> Monitor;
  vector<int> Mothership;
} U;
map<int,string> M;                     // Names of special ranks

};

//==============================================================================
   
#endif
