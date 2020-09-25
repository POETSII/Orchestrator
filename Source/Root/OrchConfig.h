#ifndef __OrchConfigH__H
#define __OrchConfigH__H

#include "jnj.h"
#include <stdio.h>
#include <vector>
#include <string>
using namespace std;

//==============================================================================

class OrchConfig
{
public:
                         OrchConfig();
virtual ~                OrchConfig(void);

void                     Dump(FILE * fp = stdout) { Show(fp); }
unsigned                 ErrCnt()                 { return ecnt;  }
vector<pair<int,int> > & ExposeErr()              { return err_v; }
string                   File()                   { return file;  }
void                     Force2Linux();
void                     IncErr(UIF::Node *,int);
void                     Init();
void                     Show(FILE * = stdout);

string                   Apps()        { return default_paths.apps;            }
string                   Author()      { return Orchestrator_header.author;    }
string                   Batch()       { return default_paths.batch;           }
string                   Binaries()    { return default_paths.binaries;        }
string                   Build()       { return flags.build;                   }
string                   Date()        { return Orchestrator_header.date;      }
string                   Engine()      { return default_paths.engine;          }
string                   Grammar()     { return setup_files.grammar;           }
string                   Hardware()    { return setup_files.hardware;          }
string                   Log()         { return default_paths.log;             }
string                   Messages()    { return setup_files.messages;          }
string                   Name()        { return Orchestrator_header.name;      }
string                   Place()       { return default_paths.place;           }
string                   Placement()   { return setup_files.placement;         }
string                   RemoteMshp()  { return default_paths.remote_mship;    }
string                   Trace()       { return default_paths.trace;           }
string                   Stage()       { return default_paths.stage;           }
string                   Supervisors() { return default_paths.supervisors;     }
string                   Ulog()        { return default_paths.ulog;            }
string                   Version()     { return Orchestrator_header.version;   }
static string            Where()       { return WhereAmI;                      }

unsigned                 ecnt;         // File parse error count
string                   file;         // Name of defining file
vector<pair<int,int> >   err_v;
static const string      WhereAmI;     // Hardwired location of config file

private:
struct Orchestrator_header_t {         // Header stuff
  string name;
  string author;
  string date;
  string version;
} Orchestrator_header;

struct default_paths_t {               // Default file paths
  string apps;
  string engine;
  string place;
  string log;
  string ulog;
  string trace;
  string binaries;
  string stage;
  string batch;
  string supervisors;
  string remote_mship;
} default_paths;

struct setup_files_t {
  string messages;                     // LogServer elaboration strings
  string grammar;                      // Application XML grammar definition
  string hardware;                     // Hardware description file
  string placement;                    // Default placement control
} setup_files;

struct flags_t {
  string build;                        // Default x-compiler flags
} flags;

};

//==============================================================================

#endif
