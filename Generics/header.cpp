//------------------------------------------------------------------------------

#include <stdio.h>
#include "header.h"
#include <time.h>

//==============================================================================

                                       // Binary record type
const unsigned char Header::Rtype    = 0x7f;
                                       // Unsecure signature
const unsigned int  Header::UnSecSig = 0xff5703b6;
#define DO(i,k) for(int i=0;i<k;i++)

//==============================================================================

Header::Header()
// Constructor from the ground up
{
DO(i,4) ver[i] = 0;                    // Initialise ....version
ver[3]         = 1;
err            = 0;                    // ...internal error flag
ftyp           = 0x00;                 // ...parent file type
tstamp.Set();                          // ...creation timestamp
}

//------------------------------------------------------------------------------

Header::Header(string fname)
// Constructor from a closed, named binary file
{
err=0;                                 // Clear error flag
FILE * fb = fopen(fname.c_str(),"rb"); // Try and open it
err = Init(fb);
if (err==0) fclose(fb);                // Orderly closedown ?
return;
}

//------------------------------------------------------------------------------

Header::Header(FILE * fb)
// Constructor from an open binary file stream
{
err = Init(fb);
if (err!=0) fclose(fb);                // Disorderly closedown ?
return;
}

//------------------------------------------------------------------------------

Header::~Header()
{
//printf("Killing Header\n");
}

//------------------------------------------------------------------------------

void Header::Dump()
{
printf("Header dump+++++++++++++++++++++++++++++++++++\n");
printf("Header name     : %s\n",hname.c_str());
printf("Type            : %u(%x)\n",ftyp,ftyp);
printf("Version         : %03u.%03d.%03d.%03d\n",ver[0],ver[1],ver[2],ver[3]);
printf("Timestamp (str) : %s\n",Tstamp().c_str());
printf(" ... (int)      : %02d/%02d/%04d(%02d:%02d:%02d.%03d)\n",
       tstamp.day,tstamp.mnth,tstamp.year,tstamp.hour,
       tstamp.mins,tstamp.secs,tstamp.msecs);
printf("Name            : %s\n",name.c_str());
printf("Author          : %s\n",author.c_str());
printf("Error state     : %d\n",err);
printf("Header dump-----------------------------------\n");
}

//------------------------------------------------------------------------------

int Header::Init(FILE * fb)
{
if (fb==0) return 1;                   // File not accessible?
unsigned char rtype;                   // Binary record type...
fread(&rtype,1,1,fb);
if (rtype!=Rtype) return 2;            // ...OK?
unsigned int unsecsig;                 // Unsecure signature word...
fread(&unsecsig,4,1,fb);
if (unsecsig!=UnSecSig) return 3;      // ...OK?
fread(&ver,1,4,fb);                    // Version quadruple
ftyp = 0;                              // File type (need to initialise, 'cos
fread(&ftyp,2,1,fb);                   // only two bytes read)
tstamp.GetTS(fb);                      // Timestamp
hname = GetStr(fb);                    // Header argument
name = GetStr(fb);                     // File name
author = GetStr(fb);                   // Author
string e = GetStr(fb);                 // Future proofing
return 0;
}

//------------------------------------------------------------------------------

void Header::SaveA(FILE * fp)
// Save to open ASCII file stream
{
fprintf(fp,"[Header(\"%s\")]\n",hname.c_str());
fprintf(fp,"type          = $%02x\n",ftyp);
fprintf(fp,"version       = %u,%u,%u,%u\n",ver[0],ver[1],ver[2],ver[3]);
fprintf(fp,"create        = \"%s\"\n",tstamp.Tstamp().c_str());
fprintf(fp,"name(saved)   = \"%s\"\n",Name().c_str());
fprintf(fp,"author        = \"%s\"\n\n",author.c_str());
}

//------------------------------------------------------------------------------

void Header::SaveA(string str)
// Save to named ASCII file
{
FILE * fp = fopen(str.c_str(),"w");
err=5;
if (fp==0) return;
err=0;
SaveA(fp);
fclose(fp);
}

//------------------------------------------------------------------------------

void Header::SaveB(FILE * fp)
// Save to open binary stream
{
fwrite(&Rtype,1,1,fp);
fwrite(&UnSecSig,4,1,fp);
fwrite(ver,1,4,fp);
fwrite(&ftyp,2,1,fp);
tstamp.PutTS(fp);
PutStr(fp,hname);
PutStr(fp,name);
PutStr(fp,author);
PutStr(fp,"");
}

//------------------------------------------------------------------------------

void Header::SaveB(string str)
// Save to named binary file
{
FILE * fp = fopen(str.c_str(),"wb");
err=4;
if (fp==0) return;
err=0;
SaveB(fp);
fclose(fp);
}

//------------------------------------------------------------------------------

void Header::Ver(int v0,int v1,int v2,int v3)
// Set the version quadruple
{
ver[0]=v0;
ver[1]=v1;
ver[2]=v2;
ver[3]=v3;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

string Header::GetStr(FILE * fp)
// Pull in a string from the binary file. If the name is too long for the buffer
// it is at least truncated
{
const unsigned int LEN = 512;          // UNCOOL UNCOOL UNCOOL but probably OK
char buf[LEN];
unsigned int len = 0;
fread(&len,2,1,fp);                    // Length written as 2 bytes
fread(buf,sizeof(char),min(len,LEN)+1,fp);
return string(buf);
}

//------------------------------------------------------------------------------

void Header::PutStr(FILE * fp,string str)
{
unsigned int len = str.size();
fwrite(&len,2,1,fp);
fwrite(str.c_str(),sizeof(char),len+1,fp);
}

//------------------------------------------------------------------------------

void Header::wEOFB(FILE * fb)
// Write the EOF record. The code has to go somewhere....
{
unsigned int eof = 0x7d;
fwrite(&eof,1,1,fb);
}

//==============================================================================

void Header::timestamp::GetTS(FILE * fp)
{
fread(&day,1,1,fp);
fread(&mnth,1,1,fp);
fread(&year,2,1,fp);
fread(&hour,1,1,fp);
fread(&mins,1,1,fp);
fread(&msecs,2,1,fp);                  // Milliseconds and seconds are stored as
secs = msecs/1000;                     // one millisecond field
msecs -= secs*1000;
}

//------------------------------------------------------------------------------

void Header::timestamp::PutTS(FILE * fp)
{
fwrite(&day,1,1,fp);
fwrite(&mnth,1,1,fp);
fwrite(&year,2,1,fp);
fwrite(&hour,1,1,fp);
fwrite(&mins,1,1,fp);
int x = msecs + (secs*1000);
fwrite(&x,2,1,fp);
}

//------------------------------------------------------------------------------

void Header::timestamp::Set()
// Set the timestamp structure from the RTL real-time clock.
// Note the resolution - seconds? Ug. I can't find a way of getting real time
// to a resolution of milliseconds.
{
time_t T = time(0);
tm * pT  = gmtime(&T);
day      = pT->tm_mday;
mnth     = pT->tm_mon+1;
year     = pT->tm_year+1900;
hour     = pT->tm_hour;
mins     = pT->tm_min;
secs     = pT->tm_sec;
msecs    = 0;
}

//------------------------------------------------------------------------------

string Header::timestamp::Tstamp()
{
char buf[32];
sprintf(buf,"%02d/%02d/%04d(%02d:%02d:%02d.%03d)",
        day,mnth,year,hour,mins,secs,msecs);
return string(buf);
}

//------------------------------------------------------------------------------

#undef DO 

//==============================================================================























