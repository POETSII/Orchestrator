//------------------------------------------------------------------------------

#include "flat.h"
#include <climits>

//------------------------------------------------------------------------------

int cmp_noc(const string &s1,const string &s2)
// I can't believe I have to write this myself.....
// Actually, I didn't, I pinched it from Stroustrup p591.
{
string::const_iterator p1=s1.begin();
string::const_iterator p2=s2.begin();

while(p1!=s1.end()&&p2!=s2.end()) {
  if(toupper(*p1)!=toupper(*p2)) return (toupper(*p1)<toupper(*p2)) ? -1 : 1;
  ++p1;
  ++p2;
}
return (s2.size()==s1.size()) ? 0 : (s1.size()<s2.size()) ? -1 : 1;

}

//------------------------------------------------------------------------------

int cmp_noc(const string &s1, const char * s2)
{
return cmp_noc(s1,string(s2));
}

//------------------------------------------------------------------------------
 
uint32 GET4(uchar * p1)
// The input is a pointer to some position in a byte array. (Which is assumed to
// exist at least three bytes into the future). This byte and the subsequent
// three are turned into a uint32
{
union {
  uint32 out;
  uchar  in[4];
} U;

for(int i=0;i<4;i++) U.in[i] = *p1++;

return U.out;
}

//------------------------------------------------------------------------------

void PUT4(uchar * pout,uint32 in)
// Converse of GET4. The input is a uint32, which is mapped onto the four bytes
// starting at pout. These four bytes are assumed to exist.
{
union {
  uint32 in;
  uchar  out[4];
} U;

U.in=in;
for(int i=0;i<4;i++) pout[i] = U.out[i];
}

//------------------------------------------------------------------------------

// I couldn't face the source reorganisation necessary to make these functions
// into templates. There are only two of each, anyway. Been there, drunk the
// beer. Templates are over-rated.

// BORLAND BUG:
// And then the nightmare of bitset<>. I *CANNOT* make y[i-fr] = x[i] compile
// with this joke compiler. There's some stuff in the HELP that burbles about a
// proxy class to assist in things like x = b[i], but I can't make it work.
// Below has - I think - the same semantics, and life's too short......

// U$OFT BUG:
// They've got round this by simply not including the only really useful
// bitset<> constructor: bitset<....> x(unsigned), so I have to write it all
// from scratch.

// I really don't know why I bother.

uchar EX8(uchar in,int fr,int to)
{
const int BITS = 8 * sizeof(uchar);
bitset<BITS> x(in);
bitset<BITS> y;
y.reset();
for(int i=fr;i<=to;i++) y.set(i-fr,x.test(i));
return (uchar)(y.to_ulong());
}

//------------------------------------------------------------------------------

uint32 EX32(uint32 in,int fr,int to)
{
//const int BITS = 8 * sizeof(uint32);
//bitset<BITS> x(in);
//bitset<BITS> y;
//y.reset();
//for(int i=fr;i<=to;i++) y.set(i-fr,x.test(i));
//return (uint32)(y.to_ulong());
// And all because u$soft don't do STL::bitset<>.....

unsigned ans = 0;
unsigned m_fr = 1<<fr;
unsigned m_to = 1;
for(unsigned i=fr;i<=(unsigned)to;i++) {
  ans |= (m_to & ((m_fr&in)==0 ? 0 : 0xffffffff));
  m_to <<= 1;
  m_fr <<= 1;
}
return ans;
}

//------------------------------------------------------------------------------

void INS8(uchar * tgt,uchar x,int fr,int to)
{
if (tgt==0) return;
const int BITS = 8 * sizeof(uchar);
uchar uc = *tgt;
bitset<BITS> ans(uc);
bitset<BITS> mask;
mask.set();
for (int i=fr;i<=to;i++) mask.reset(i);
ans &= mask;
ans |= (x << fr);                      // U$oft doesn't like this:
// C6297: Arithmetic overflow:  32-bit value is shifted, then cast to 64-bit value.
// Results might not be an expected value.
*tgt = (uchar)(ans.to_ulong());
}

//------------------------------------------------------------------------------

void INS32(uint32 * tgt,uint32 x,int fr,int to)
{
if (tgt==0) return;
if (fr>to) return;
unsigned m_fr = 1;
unsigned m_to = 1<<fr;
for (int i=0;i<=to-fr;i++) {
  if ((x&m_fr)==0) (*tgt) &= (~m_to);
  else             (*tgt) |=   m_to;
  m_to <<= 1;
  m_fr <<= 1;
}
}

//------------------------------------------------------------------------------

string to_binary(uchar in)
{
const int BITS = 8 * sizeof(uchar);
bitset<BITS> y(in);
//return y.template to_string<char,char_traits<char>,allocator<char> >();
return y.to_string<char,char_traits<char>,allocator<char> >();
}

//------------------------------------------------------------------------------

string to_binary(uint in)
// The syntax here is unbelievably foul and I can't believe it hasn't changed
// in the last ten years. Below is lifted direct from
// BORLAND STL help|bitset|new members
{/*
const int BITS = 8 * sizeof(uint);
bitset<BITS> y(in);
// But it doesn't get through g++:
//return y.template to_string<char,char_traits<char>,allocator<char> >();
// .. so I asked www.cplusplus.com:
return y.to_string<char,char_traits<char>,allocator<char> >();
*/
// And none of it goes through u$oft, so lets try again:
const int BITSPERUNSIGNED = 32;
unsigned mask = 1;
string s_out;
for(int i=0;i<BITSPERUNSIGNED;i++) {
  s_out = (((in&mask)!=0) ? "1" : "0") + s_out;;
  mask <<= 1;
}
return s_out;
}

//------------------------------------------------------------------------------

string toupper(const string & s_in)
// Returns a copy of s with the contents promoted (where applicable) to upper
// case
{
string s_out;
string::const_iterator p=s_in.begin();
while(p!=s_in.end()) s_out += toupper(*p++);
return s_out;
}

//------------------------------------------------------------------------------

bool debug_trace(int i)
{
bool D[8] = {
  false,        // 0
  false,        // 1  Parser routine calls
  true ,        // 2  Lexer token trace
  true ,        // 3  Token trace in GetBehavOp
  true ,        // 4  Invocation tree skeleton build
  true ,        // 5  Blob clone constructor hierarchy
  true ,        // 6  Module invariant scanner
  false};       // 7
return D[i];
}

//------------------------------------------------------------------------------

bool file_exists(const char *filename)
// Pretty obvious what it does; trouble is, I don't know if it's Borland or ANSI
// the documentation - ho ho ho - is opaque on this. It's almost certainly UNIX,
// but then, most things are.
// I found some documentation! See notebook 14/10/10.
// Mode:
// R_OK - test for read permission
// W_OK - test for write permission
// X_OK - test for execute or search permission
// F_OK - test whether the directories leading to the file can be searched and
// the file exists.
// 0    - ?
// BUT it doesn't seem to do what it says on the packet, so we bodge...
{
//int mode = 0;

//return (access(filename,mode)==0);
FILE * fp = fopen(filename,"r");
if (fp != 0) {
  fclose(fp);
  return true;
}
return false;
}

//------------------------------------------------------------------------------

bool file_readable(const char *filename)
// Pretty obvious what it does; trouble is, I don't know if it's Borland or ANSI
// the documentation - ho ho ho - is opaque on this. It's almost certainly UNIX,
// but then, most things are.
// I found some documentation! See notebook 14/10/10.
// Mode:
// R_OK - test for read permission
// W_OK - test for write permission
// X_OK - test for execute or search permission
// F_OK - test whether the directories leading to the file can be searched and
// the file exists.
// 0    - ?
{
//int mode = 0x04;
//return (access(filename,mode)==0);
FILE * fp = fopen(filename,"r");
if (fp != 0) {
  fclose(fp);
  return true;
}
return false;
}

//------------------------------------------------------------------------------
/*
char * GetDate()
// Writes the current date into buf
{
static char buf[64];
sprintf(buf,"** Can't identify environment **");
#ifdef BORLAND
struct date D;
getdate(&D);
sprintf(buf,"%d/%d/%d",D.da_day,D.da_mon,D.da_year);
return &buf[0];
#endif
#ifdef MICROSOFT
//_strdate(buf);
sprintf(buf,"Micro$oft environment");
return &buf[0];
#endif
}

//.GetDate......................................................................

char * GetTime()
// Writes the current time into buf
{
static char buf[64];
sprintf(buf,"** Can't identify environment **");
#ifdef BORLAND
struct time T;
gettime(&T);
sprintf(buf,"%2d:%02d:%02d.%02d",T.ti_hour,T.ti_min,T.ti_sec,T.ti_hund);
return &buf[0];
#endif
#ifdef MICROSOFT
//_strtime(buf);
sprintf(buf,"Micro$oft environment");
return &buf[0];
#endif
}
*/
//.GetDate......................................................................
              
long Time2long(const string & str)
// Bit of a bodge - it turns a time string into a long integer of 10ms's's's.
// It gets unhappy if the string is in the wrong format.
// This REALLY needs doing properly. Somewhen.
{
if (str.size()!=11) return 0;
int hours = str2int(str.substr(0,1));
int mins = str2int(str.substr(3,4));
int secs = str2int(str.substr(6,7));
int secs10 = str2int(str.substr(9,10));
long ans = long(secs10) + long(100*secs) + long(6000*mins) + long(360000*hours);
return ans;
}

//.Time2long....................................................................

long Timer(long offset)
// Simple timer; works in SECONDS since the Dawn Of Time, i.e. 1970. Wraps
// at *68 years*.
{
long time_now = long(time(0));
if (time_now == -1) return long(0);
return time_now - offset;
}

//.Timer........................................................................

long mTimer(long offset)
// Simple timer; works in MILLISECONDS since the program started; wraps at
// around 2147482 ms, which is damn close to (2^31)/1000, (why?) or *35 minutes*
{
long time_now = long(clock());
if (time_now == -1) return long(0);
return (long(1000) * (time_now-offset))/CLOCKS_PER_SEC;
}

//.mTimer.......................................................................

void mSleep(long idoze)
// Simple millisecond pause - just a spinner
{
long Otime = mTimer();
while(mTimer(Otime)<idoze);
}

//------------------------------------------------------------------------------

int nint(double x)
// FORTRAN nearest integer function
{
if (x>0.0) return int(x+0.5);
else return int(x-0.5);
}

//..............................................................................

void sign(double a,double & rb)
// Ah, FORTRAN....
{
if (a>0.0) rb = fabs(rb);
else rb = -fabs(rb);
}

//..............................................................................

void IError(int e)
// Internal error notifying routine. "Internal" means further execution is
// pointless, so the program stops.
{
printf("\n\n*********************************************\n\n"
       " INTERNAL ERROR %d : Execution stopped.\n"
       " Probably a 'To do' section you've forgotten about. \n\n"
       "*********************************************\n",e);
printf("Type any character to be gone.....");
char cc;
scanf("%c",&cc);
exit(0);
}

//.IError.......................................................................

char * Ostr(int len)
// Routine to hand out a transient string, length len, containing spaces
{
const int BUFLEN=256;
static char buf[BUFLEN];
len = min(len,BUFLEN);
buf[len-1] = '\0';
for(--len;len>=0;len--)buf[len]=' ';
return &buf[0];
}

//.Ostr.........................................................................

string UniS(const string & s,int len,bool reset)
// Routine to return a unique string, starting with "s", appended with a unique
// integer. If the "len" argument is 0, the length is whatever it takes; if it's
// != 0, the length component is padded with leading zeroes as necessary
//
{
static long int i = 0L;
if (reset) i = 0L;
const int BUFLEN = 32;
char buf[BUFLEN];
string fmt = string("%d");
if (len!=0) {
  sprintf(buf,"%%0%dd",len);
  fmt = string(buf);
}
buf[sprintf(buf,fmt.c_str(),i++)]=0;
return s + string(buf);
}

//.UniS.........................................................................

unsigned UniU(int domain)
// Routine to generate unique integers. We have an arbitrary number of domains,
// and each domain maintains a unique unsigned, which is incremented every time
// a value is requested. Domains may be reset to 0 by supplying a negative
// argument. (It follows that domain 0 can never be reset...)
{
static map<int,unsigned> U;
if (domain >= 0) {                     // Give me a number
                                       // Not there? create it
  if (U.find(domain)==U.end()) U[domain]=0;
  else U[domain]++;                    // Domain exists - increment value
} else {                               // Reset domain
  domain = -domain;
  U[domain]=0;                         // May or may not create a new one
}
return U[domain];                      // Either way...

//static long unsigned int i = 0L;
//if (reset) i = 0L;
//long unsigned int a = i++;
//return a;
}
  
//------------------------------------------------------------------------------

string bool2str(bool b)
{
return b ? string("TRUE") : string("FALSE");
}

//------------------------------------------------------------------------------

string dbl2str(double x)
{
char buf[64];
sprintf(buf,"%e",x);
return string(buf);
}

//------------------------------------------------------------------------------
           /*
unsigned int hex2uint(string hex)
{
unsigned int ans = 0;
for (unsigned int i=0;i<hex.size();i++) {
  unsigned int j = 0;
//printf("hex[%d-1] = [%c]\n",i,hex[i-1]);
  switch (hex[i]) {
    case '0' : j = 0; break;
    case '1' : j = 1; break;
    case '2' : j = 2; break;
    case '3' : j = 3; break;
    case '4' : j = 4; break;
    case '5' : j = 5; break;
    case '6' : j = 6; break;
    case '7' : j = 7; break;
    case '8' : j = 8; break;
    case '9' : j = 9; break;
    case 'a' : case 'A' : j = 10; break;
    case 'b' : case 'B' : j = 11; break;
    case 'c' : case 'C' : j = 12; break;
    case 'd' : case 'D' : j = 13; break;
    case 'e' : case 'E' : j = 14; break;
    case 'f' : case 'F' : j = 15; break;
    default  : return 0;
  }
//  (ans<<=4)|=j;
  ans = ans << 4;
  ans = ans + j;
}
return ans;
}
             */
//------------------------------------------------------------------------------
/*
unsigned int hex2int(string h)
{
unsigned int ans = 0;
sscanf(h.c_str(),"%x",&ans);
return ans;
}
  */
//------------------------------------------------------------------------------

string hex2str(unsigned x)
// Yes, I know. Prosaic but useful.
{
char buf[32];                          // "Eek", I hear you cry. But even an 80-
                                       // bit integer is < 1.3e24, so we have -
                                       // by definition - loads of space
sprintf(buf,"%x",x);
return string(buf);
}

//------------------------------------------------------------------------------

string int2str(int x,int w)
// Yes, I know. Prosaic but useful.
{
char buf[32];                          // "Eek", I hear you cry. But even an 80-
                                       // bit integer is < 1.3e24, so we have -
                                       // by definition - loads of space
char fmt[32];                          // Overkill
if (w==0) sprintf(fmt,"%%d%c",'\0');
else sprintf(fmt,"%%%dd%c",w,'\0');
sprintf(buf,fmt,x);
return string(buf);
}

//------------------------------------------------------------------------------

string long2str(long x)
// Yes, I know. Prosaic but useful.
{
char buf[32];                          // "Eek", I hear you cry. But even an 80-
                                       // bit integer is < 1.3e24, so we have -
                                       // by definition - loads of space
sprintf(buf,"%ld",x);
return string(buf);
}

//------------------------------------------------------------------------------

string uint2str(unsigned x,int w)
// Yes, I know. Prosaic but useful.
// "Eek", I hear you cry. But even an 80-bit integer is < 1.3e24, so we have -
// by definition - loads of space
{
char buf[32];
char fmt[32];                          // Overkill
if (w==0) sprintf(fmt,"%%d%c",'\0');
else sprintf(fmt,"%%%du%c",w,'\0');
sprintf(buf,fmt,x);
return string(buf);
//sprintf(buf,"%u",x);
//return string(buf);
}

//------------------------------------------------------------------------------

double str2dble(const string & str,double def)
// I figured it was actually quicker to write this than look one up
{
double ans=def;
sscanf(str.c_str(),"%le",&ans);
return ans;
}

//------------------------------------------------------------------------------

int str2int(const string & str,int def)
// I figured it was actually quicker to write this than look one up
{
int ans=def;
sscanf(str.c_str(),"%d",&ans);
return ans;
}

//------------------------------------------------------------------------------

unsigned str2uint(const string & str,unsigned def)
// Deep sigh. There appears to be no way of detecting a translation overflow
// from C. Any departure from what I consider to be reality causes the default
// to be returned.
{
unsigned t=0;
unsigned u0 = unsigned('0');
for(unsigned i=0;i<str.size();i++) {
  unsigned char uc = str[i];
  if (!isdigit(uc)) return def;
  unsigned d = unsigned(uc) - u0;
  if (t<((ULONG_MAX-d)/10)) t = (t*10)+d;
  else return def;
}
return t;
}

//------------------------------------------------------------------------------

long int str2long(const string & str)
// I figured it was actually quicker to write this than look one up
{
long int ans=0L;
sscanf(str.c_str(),"%ld",&ans);
return ans;
}

//------------------------------------------------------------------------------

bool str2bool(const string & str)
{
if (str=="yes")  return true;
if (str=="true") return true;
return false;
}

//------------------------------------------------------------------------------

unsigned str2hex(const string & str)
{
unsigned ans = 0;
sscanf(str.c_str(),"%x",&ans);
return ans;
}

//------------------------------------------------------------------------------

char * GetTime()
// Stroustrup p905; Schildt p176
// There seems to be no way to get absolute time to a precision of ms, even
// though you can get relative time to a precision of CLOCKS_PER_SEC, (which
// appears to be a double).
// So: we use time() and gmtime() to get absolute hours:mins:secs, then clock()
// to get the milliseconds since the program started. This latter number simply
// increases monotonically, so we do some dirt and turn it into the decimal
// absolute seconds. Its' not actually absolutely correct, but it will deliver
// correct timing *intervals*.
// Note we could up the timing accuracy from 10ms to 1 ms.... to do.....
{
const int SIZE = 64;
static char buf[SIZE];
time_t x = time(0);                    // Low-precision absolute time
strftime(buf,SIZE,"%H:%M:%S",localtime(&x));
                                       // Fast clock there?
if (clock()==clock_t(-1)) return &buf[0];
                                       // High-precision relative time
double ds = double(clock())/CLOCKS_PER_SEC;   // ... in seconds
long stuff = long(ds*1000.0)%1000;            // ... in tenths of a second
sprintf(&buf[8],".%02ld",stuff/10);    // Tack it onto the time string
return &buf[0];
}

//------------------------------------------------------------------------------

char * GetDate()
{
const int SIZE = 64;
static char buf[SIZE];
time_t x = time(0);
strftime(buf,SIZE,"%d/%m/%Y",gmtime(&x));
return &buf[0];
}

//------------------------------------------------------------------------------

string GetStr(FILE * fb)
// Pull in a string from the binary file. If the name is too long for the buffer
// it is at least truncated
{
const unsigned LEN = 512+1;            // UNCOOL UNCOOL UNCOOL but probably OK
char buf[LEN];
unsigned len = 0;
if (fb==0) return string();
fread(&len,2,1,fb);                    // Length written as 2 bytes
fread(buf,sizeof(char),min(len,LEN)+1,fb);
return string(buf);
}

//------------------------------------------------------------------------------

void PutStr(FILE * fb,string str)
{
if (fb==0) return;
unsigned len = str.size();
fwrite(&len,2,1,fb);
fwrite(str.c_str(),sizeof(char),len+1,fb);
}

//==============================================================================

int FindFirst(const char * name,struct FindData_t * fdata)
// To unify BORLAND::findfirst and u$OFT::_findfirst...
// Borland    // Success returns 0, otherwise -1
// u$oft      // Success returns file handle, otherwise -1
// Force the result code from u$soft to 0/-1, and we shove the file handle
// into the "reserved" field in the file descriptor structure
{
#ifdef BORLAND
struct ffblk fileblk;
int ans = findfirst(name,&fileblk,0);         // Don't bother with attributes
fdata->FD_reserved = (void *)fileblk.ff_reserved;     // Probably junk
fdata->FD_fsize = fileblk.ff_fsize;           // Copy over size....
fdata->FD_ftime = fileblk.ff_ftime;           // ...and time (probably junk)
int len = min(256,_MAX_PATH);                 // u$soft paths are bigger
strncpy(fdata->FD_name,fileblk.ff_name,len);  // Copy over found filename
return ans;
#endif

#ifdef MICROSOFT
struct _finddata_t fileblk;
intptr_t ans = _findfirst(name,&fileblk);     // u$oft doesn't do attributes
fdata->FD_reserved = (void *)ans;             // Save the returned file handle
fdata->FD_fsize = fileblk.size;               // Copy size....
fdata->FD_ftime = (long)fileblk.time_create;  // And time, but probably junk
strncpy(fdata->FD_name,fileblk.name,_MAX_PATH);
return (ans!= -1) ? 0 : -1;                   // Fix return value to yes/no
#endif

                                              // Some compilers do, some don't..
#ifndef BORLAND                               // Oh, for heavens sake.....
#ifndef MICROSOFT
return -1;                                    // Paranoia.....
#endif
#endif
}

//------------------------------------------------------------------------------

int FindNext(struct FindData_t * fdata)
{
#ifdef BORLAND
struct ffblk fileblk;
fileblk.ff_reserved = (long)fdata->FD_reserved;
fileblk.ff_fsize = fdata->FD_fsize;
fileblk.ff_ftime = fdata->FD_ftime;
int len = min(256,_MAX_PATH);
strncpy(fileblk.ff_name,fdata->FD_name,len);
int ans = findnext(&fileblk);
fdata->FD_reserved = (void *)fileblk.ff_reserved;
fdata->FD_fsize = fileblk.ff_fsize;
fdata->FD_ftime = fileblk.ff_ftime;
strncpy(fdata->FD_name,fileblk.ff_name,len);
return ans;
#endif

#ifdef MICROSOFT
struct _finddata_t fileblk;
fileblk.size = fdata->FD_fsize;
fileblk.time_create = fdata->FD_ftime;
strncpy(fileblk.name,fdata->FD_name,_MAX_PATH);
int ans = _findnext((intptr_t)fdata->FD_reserved,&fileblk);
fdata->FD_fsize = fileblk.size;
fdata->FD_ftime = (long)fileblk.time_create;
strncpy(fdata->FD_name,fileblk.name,_MAX_PATH);
return ans;
#endif

#ifndef BORLAND                               // Oh, for heavens sake.....
#ifndef MICROSOFT
return -1;                                    // Paranoia.....
#endif
#endif
}

//------------------------------------------------------------------------------

int FindClose(struct FindData_t * fdata)
// The difference here is that Borland wants the descriptor structure, whereas
// u$oft wants the file handle (which we snuck into the .reserved field)
// Everything else is the same
{
#ifdef BORLAND
struct ffblk fileblk;
fileblk.ff_reserved = (long)fdata->FD_reserved;
fileblk.ff_fsize = fdata->FD_fsize;
fileblk.ff_attrib = 0;
fileblk.ff_ftime = fdata->FD_ftime;
fileblk.ff_fdate = 0;
int len = min(256,_MAX_PATH);
strncpy(fileblk.ff_name,fdata->FD_name,len);
return findclose(&fileblk);
#endif

#ifdef MICROSOFT
struct _finddata_t fileblk;
fileblk.size = fdata->FD_fsize;
fileblk.time_create = fdata->FD_ftime;
strncpy(fileblk.name,fdata->FD_name,_MAX_PATH);
return _findclose((intptr_t)fdata->FD_reserved);
#endif

#ifndef BORLAND                               // Oh, for heavens sake.....
#ifndef MICROSOFT
return -1;                                    // Paranoia.....
#endif
#endif
}

//-------------------------------------------------------------------------------

void DiskFree(unsigned drive,struct DiskFree_t * DiskData)
{
#ifdef BORLAND
struct dfree diskdata;
getdfree(drive,&diskdata);
DiskData->DF_total_clusters      = diskdata.df_total;
DiskData->DF_avail_clusters      = diskdata.df_avail;
DiskData->DF_sectors_per_cluster = diskdata.df_sclus;
DiskData->DF_bytes_per_sector    = diskdata.df_bsec;
#endif

#ifdef MICROSOFT
struct diskfree_t diskdata;
_getdiskfree(drive,&diskdata);
DiskData->DF_total_clusters      = diskdata.total_clusters;
DiskData->DF_avail_clusters      = diskdata.avail_clusters;
DiskData->DF_sectors_per_cluster = diskdata.sectors_per_cluster;
DiskData->DF_bytes_per_sector    = diskdata.bytes_per_sector;
#endif

return;
}

//------------------------------------------------------------------------------

int patMatch(const char * pat,const char * str)
// Case insensitive pattern match.
{
switch (pat[0]) {
  case '*'  : return patMatch(pat+1,str) || (str[0] && patMatch(pat,str+1));
  case '?'  : return str[0] && patMatch(pat+1,str+1);
  case '\0' : return !str[0];
  default   : return (pat[0]==str[0]) && patMatch(pat+1,str+1);
// Or, to make it case insensitive (apart from the obvious way):
//  default : return (toupper(pat[0])==toupper(str[0]))&&patMatch(pat+1,str+1);
}

}

//==============================================================================

void freadstr(string & name,FILE * fp)
// Pull in a string from an (open) binary stream. The format is
// Unsigned worth of string length (in characters)
// The string itself
{
unsigned len;
fread(&len,sizeof(unsigned),1,fp);     // Pull in the string length
//printf("freadstr pulling in %d characters....\n",len);
const unsigned BUF = 1024;             // Buffer on the stack
char buf[BUF];
char * pbuf = &buf[0];                 // If not big enough, replace with heap
if (len>=BUF) pbuf = (char *) new char[len+1];
fread(pbuf,1,len,fp);                  // Pull the characters in
pbuf[len] = '\0';                      // Write the terminator
name = string(buf);                    // Create the string
//printf("freadstr gets ||%s||\n",name.c_str());
if (pbuf!=&buf[0]) delete [] pbuf;     // Kill the heap buffer if it's there
}

//------------------------------------------------------------------------------

void fwritestr(string name,FILE * fp)
// Write a string to an (open) binary stream. The format is
// Unsigned worth of string length (in characters)
// The string itself
{
unsigned len = name.size();            // Number of characters a-coming of.
fwrite(&len,sizeof(unsigned),1,fp);    // Write the string size....
fwrite(name.c_str(),1,len,fp);         // ...and the data itself
}

//------------------------------------------------------------------------------

void HexDump(FILE * fp,unsigned char * base,unsigned len)
// Pretty-print for a hex dump. We assume the stream is open....
{
static const unsigned W = 16;          // Number of bytes/line
unsigned i=0;                          // Seperator
for(unsigned j=0;j<=5*W;j++) fprintf(fp,"%c",j!=5*W?'=':'\n');
if (len==0) return;                    // Just in case...
for(unsigned row=0;;row++) {           // One row at a time...
  fprintf(fp,"%08x ",row*W);           // Index
  unsigned i0 = i;
  for(unsigned col=0;col<W;col++) {    // Data bytes in hex
    fprintf(fp,"%02x ",base[i]);
    if (++i>=len) {                    // Early line exit
      for(unsigned j=col+1;j<W;j++) fprintf(fp,"   ");
      break;
    }
  }
  i=i0;
  for(unsigned col=0;col<W;col++) {    // Next bit in this row: printable?
    fprintf(fp,"%c",isprint(base[i])?base[i]:'.');
    if (++i>=len) break;               // Early line exit
  }
  fprintf(fp,"\n");
  if (i>=len) break;                   // Final line exit
}
fprintf(fp,"\n");
}

//------------------------------------------------------------------------------

string sBank(map<string,unsigned> & rm,unsigned i,string s)
// String bank:
// If the input string is less than i characters, it goes straight out again
// Otherwise it gets aliassed onto a unique unsigned.
// Just helps the pretty-print
{
if (s.size()<i) return s;
rm[s] = UniU(1001);
return "{" + uint2str(rm[s]) + "}";
}

//------------------------------------------------------------------------------

void sBankShow(FILE * fo,map<string,unsigned> & rm)
// And this lets you pretty-print the stringbank alias map
{
if (rm.empty()) return;
WALKMAP(string,unsigned,rm,i)
  fprintf(fo,"{%3u} == %s\n",(*i).second,(*i).first.c_str());
rm.clear();
fprintf(fo,"\n");
}

//------------------------------------------------------------------------------

//==============================================================================
