//------------------------------------------------------------------------------

#ifndef __flat__H
#define __flat__H

#include <string>
#include <cstdlib>

#include <bitset>
#include <vector>
#include <map>
#include <stdio.h>
//#include <io.h>
#include <time.h>
#include "macros.h"
#include <math.h>       // fabs
#include <algorithm>    // To pick up max and min....
using namespace std;

// __BORLANDC__ is defined in all Borland compilers and its value indicates the
// compiler version - this one is 0x0560.
// In addition, if the compiler is set to work in the C++ mode (which
// practically means always) then __BCPLUSPLUS__ is also defined and has the
// same value as __BORLANDC__
// Note that I'm using Borland Builder 6.0, but the underlying compiler engine
// is 5.6 (the one where they got STL in properly.....)

#ifdef __BORLANDC__
#define BORLAND BORLAND
#pragma warn -8032  // Temporary created
#pragma warn -8058  // Cannot create pre-compiled header: initialised data
#include <dir.h>
#include <dos.h>
#undef MICROSOFT
#undef GNUC
#endif

#ifdef _MSC_VER
#define MICROSOFT MICROSOFT
#include <dos.h>
#include <io.h>
#undef BORLAND
#undef GNUC
#endif

#ifdef __GNUC__
#define GNUC GNUC
#define _MAX_PATH 256
#undef BORLAND
#undef MICROSOFT
#endif

// Routines for getting the time and date
//#ifdef MICROSOFT                       // We're using MicroSoft
//#include <time.h>
//#else                                  // We're using Borland
//#include <stdio.h>
//#include <dos.h>
//#endif

// Debugging stuff

bool   debug_trace(int);
#define TRACE(i) if(debug_trace(i))

typedef unsigned int  uint;
typedef unsigned int  uint32;
//typedef unsigned int  uint32_t;
typedef unsigned int  uint16;
typedef long int      lint;
typedef unsigned char uchar;
typedef unsigned char byte;

/*inline*/ template<class T> int BYTES(T v=(T)0) {return sizeof(T);}
//inline void * operator new(size_t, void * p) {return p;}
template<class T> pair<bool,T> Get(vector<T> &,unsigned,T);

int          cmp_noc(const string &,const string &);
int          cmp_noc(const string &,const char *);
uint32       GET4(uchar *);
void         PUT4(uchar *,uint);
uchar        EX8(uchar,int,int);
uint32       EX32(uint32,int,int);
void         INS8(uchar *,uchar,int,int);
void         INS32(uint32 *,uint32,int,int);
string       to_binary(uchar);
string       to_binary(uint);
string       toupper(const string &);
bool         file_exists(const char *);
bool         file_readable(const char *);
char *       GetDate();
char *       GetTime();
long         Time2long(const string &);
//int          max(int,int);
//int          min(int,int);
long         Timer(long=long(0));
long         mTimer(long=long(0));
void         mSleep(long=0);
int          nint(double);
void         sign(double,double &);
void         IError(int);
char *       Ostr(int);
string       UniS(const string &,int=0,bool=false);
unsigned     UniU(int=0);
string       bool2str(bool b);
string       dbl2str(double);
//unsigned int hex2uint(string);
//unsigned int hex2int(string);
string       hex2str(unsigned);
string       int2str(int,int=0);
string       long2str(long);
string       uint2str(unsigned,int=0);
double       str2dble(const string &,double=0.0);
int          str2int(const string &,int=0);
unsigned     str2uint(const string &,unsigned=0);
lint         str2long(const string &);
bool         str2bool(const string &);
unsigned     str2hex(const string &);
unsigned     str2bin(string &);
//int          sV2uV(vector<string> &,vector<unsigned> &);
string          GetStr(FILE *);
void            PutStr(FILE *,string);

//------------------------------------------------------------------------------
/* The following is intended to unify the Borland and u$oft
interpretations of the file-finding stuff.
Save you looking it up all the time:
BORLAND
=======
For a single file:
In <dir.h>
  struct ffblk {
    long           ff_reserved;
    long           ff_fsize;       // file size (bytes)
    unsigned long  ff_attrib;      // attribute found
    unsigned short ff_ftime;       // file time  (complicated compressed format)
    unsigned short ff_fdate;       // file date
    char           ff_name[256];   // found file name
  };

"name" can contain wildcards, "attrib" is the OR of various attribute flags.
ans = findfirst(name,&fileblk,attrib);   // Success returns 0, otherwise -1
ans = findnext(&fileblk);                // Success returns 0, otherwise -1
ans = findnext(&fileblk);
ans = findnext(&fileblk);
.
.
.
ans = findclose(&fileblk);               // Success returns 0, otherwise -1


For a given drive:
drive = 0 - default, 1 - A: ....
  void getdfree(unsigned char drive,struct dfree * pdf)
  struct dfree {
    unsigned df_avail;                // Number of available clusters
    unsigned df_total;                // Total number of clusters
    unsigned df_bsec;                 // Bytes/sector
    unsigned df_sclus;                // Sectors/cluster
  };

u$OFT
=====
For a single file:
  struct _finddata_t {
    unsigned   attrib;        // Attribute found
    char       name[260];     // Found full name
    _fsize_t   size;          // File size (bytes)
    __time64_t time_access;   // Time last accessed - -1 in FAT systems
    __time64_t time_create;   // Time created       - -1 in FAT systems
    __time64_t time_write;    // Time last written to
  };


struct _finddata_t fileblk;
intptr_t rx = _findfirst("*.xpg",&fileblk); // Success returns file handle,
                                            // otherwise -1
int ans;
ans = _findnext(rx,&fileblk);               // Success returns 0, otherwise -1
ans = _findnext(rx,&fileblk);
ans = _findnext(rx,&fileblk);
.
.
.
ans = _findclose(rx);                       // Success returns 0, otherwise -1

For a given drive:
  struct diskfree_t {
    unsigned total_clusters;     // count of all disk clusters
    unsigned avail_clusters;     // free unallocated clusters
    unsigned sectors_per_cluster;
    unsigned bytes_per_sector;
  };



*/

// Finding files in a directory:
/* There is all sorts of dynamic weirdness going on under these routines, which
is opaque to us mortals, but ultimately means you can't nest directory-walking
loops. AS soon as the inner loop terminates, all the outer ones do too. Bum.
*/
//
// A useful (and easy) intersection of the Borland and u$oft structures:
struct FindData_t {
  void *         FD_reserved;          // Has to be void * 'cos of u$oft_64
  long           FD_fsize;             // File size (bytes)
  long           FD_ftime;             // File time
  char           FD_name[_MAX_PATH];   // Found file name
};

int FindFirst(const char *,struct FindData_t *);
int FindNext(struct FindData_t *);
int FindClose(struct FindData_t *);

// Directory stats:
//
// For some reason, the u$oft structure is in the Borland headers, (although
// there doesn't appear to be a Borland function that uses it?). But for the
// sake of uniformity, I copy it to a local struct defined here.
struct DiskFree_t {
  unsigned DF_total_clusters;          // Count of all disk clusters
  unsigned DF_avail_clusters;          // Free unallocated clusters
  unsigned DF_sectors_per_cluster;     // Guess
  unsigned DF_bytes_per_sector;
};

void DiskFree(unsigned,struct DiskFree_t *);

//------------------------------------------------------------------------------

int patMatch(const char *,const char *);
void freadstr(string &,FILE *);
void fwritestr(string,FILE *);
void HexDump(FILE *,unsigned char *,unsigned);
string sBank(map<string,unsigned> &,unsigned,string);
void sBankShow(FILE *,map<string,unsigned> &);

//------------------------------------------------------------------------------

template<class T> pair<bool,T> Get(vector<T> & v,unsigned i,T d)
// C++98 version of C++11 vector.at().
// It allows arbitrary index READ access to a vector without falling over.
// If the index exists, a copy of the value is returned.
// If not, a copy of the default value is returned.
{
if (v.empty())   return pair<bool,T>(false,d);   // Vector empty
if (v.size()<=i) return pair<bool,T>(false,d);   // Index OOR?
return pair<bool,T>(true,v[i]);                  // We're good. Do it.
}

//==============================================================================

#endif
