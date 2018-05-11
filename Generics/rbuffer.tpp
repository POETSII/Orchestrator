//------------------------------------------------------------------------------

#include <typeinfo>
#include <stdio.h>
#include <iostream.h>
#include "macros.h"
#include "flat.h"
#include "e.h"

//==============================================================================

RBuffer::RBuffer()
// Build the whole thing from nothing
{
}

//------------------------------------------------------------------------------

RBuffer::RBuffer(string s)
// Restore constructor
{
name = s;                              // Source file name
FILE * fp = fopen(s.c_str(),"rb");     // Open the binary file
if (fp==0) throw(E(__FILE__,__LINE__));// It wasn't there!
Header H(fp);                          // Pull in the header
char Rtype;
fread(&Rtype,1,1,fp);                  // Record type
if (Rtype!=0x74) throw(E(__FILE__,__LINE__));    // Not for me
unsigned bits;                         // Ensure the 32/64 environments match
fread(&bits,SUINT,1,fp);
if (bits!=sizeof(void *)*8) throw(E(__FILE__,__LINE__));
unsigned _suint;                       // Ensure the unsigned sizes match
fread(&_suint,SUINT,1,fp);
if (SUINT!=_suint) throw(E(__FILE__,__LINE__));
freadstr(name,fp);                     // Object name as written
unsigned nbuf;
fread(&nbuf,SUINT,1,fp);               // Buffer count
for(unsigned i=0;i<nbuf;i++) {         // Loop to pull in each buffer
  string sBuf;
  freadstr(sBuf,fp);                   // Buffer name
  m_R[sBuf] = new _R(fp);              // Whack it directly into the map
}
unsigned banksiz;                      // Size of string bank (can be 0)
fread(&banksiz,SUINT,1,fp);            // Stringbank contents
for (unsigned i=0;i<banksiz;i++) {
  string s;
  freadstr(s,fp);
  strbank.push_back(s);
}
fread(&Rtype,1,1,fp);                  // Closing record
if (Rtype!=0x7d) throw(E(__FILE__,__LINE__));
fclose(fp);                            // And go, happy bunnies all round
}

//------------------------------------------------------------------------------

RBuffer::~RBuffer(void)
{
                                       // Walk top-level map; kill each buffer
WALKMAP(string,_R *,m_R,i) delete (*i).second;
}

//------------------------------------------------------------------------------

void RBuffer::Dump(string s)
{
printf("\n++------------------\n");
printf("\nResults Buffer object %s\n",name.c_str());
WALKMAP(string,_R *,m_R,i) {
  void * hBuff = GetBufferHandle((*i).first);
  printf("Buffer: %s, handle: %#0x\n",(*i).first.c_str(),hBuff);
  WALKMAP(string,_B *,(*i).second->m_B,j) {
    printf("  Line: %s, handle: %#0x",
            (*j).first.c_str(),GetLineHandle(hBuff,(*j).first));
    printf("  %u bytes, %u items, size %u, \n        type %s\n",
            (*j).second->_data.size(),(*j).second->_size(),(*j).second->_sizeof,
            (*j).second->_type.c_str());
    HexDump(stdout,&((*j).second->_data[0]),(*j).second->_data.size());
  }
}
printf("String bank (%u entries)\n",strbank.size());
unsigned cnt = 0;
WALKVECTOR(string,strbank,i) printf("%4u : ||%s||\n",cnt++,(*i).c_str());
printf("--------------------\n");
}

//------------------------------------------------------------------------------

string RBuffer::Get(void * _,unsigned i)
// Pull out a single data item from the line handle "_", offset "i"
{
_B * p_B = static_cast<_B *>(_);       // Decode the handle
if (p_B==0) throw(E(__FILE__,__LINE__));
i *= p_B->_sizeof;                     // Get byte address
unsigned buff;                         // Somewhere to put it
memcpy(&buff,&(p_B->_data[i]),p_B->_sizeof);
return strbank[buff];                  // And get the data
}

//------------------------------------------------------------------------------

template<typename T> T RBuffer::Get(void * _,unsigned i)
// Pull out a single data item from the line handle "_", offset "i"
{
_B * p_B = static_cast<_B *>(_);       // Decode the handle
if (p_B==0) throw(E(__FILE__,__LINE__));
i *= p_B->_sizeof;                     // Get byte address
T buff;                                // Somewhere to put it
memcpy(&buff,&(p_B->_data[i]),p_B->_sizeof);
return buff;                           // And get the data
}

//------------------------------------------------------------------------------

void * RBuffer::GetBufferHandle(string name)
// Given an existing buffer name, find the handle - if any
{
                                       // Not there?
if (m_R.find(name)==m_R.end()) return (void *)0;
return static_cast<void *>(m_R[name]); // Found it
}

//------------------------------------------------------------------------------

void * RBuffer::GetLineHandle(void * hBuff,string name)
// Given an existing buffer handle and line name, find the handle - if any
{
if (hBuff==0) return (void *)0;        // No buffer, then
_R * pBuff = static_cast<_R *>(hBuff);
if (pBuff->m_B.find(name)!=pBuff->m_B.end()) // Found it!
  return static_cast<void *>(pBuff->m_B[name]);
return (void *)0;                      // Not there
}

//------------------------------------------------------------------------------

unsigned RBuffer::Lines(void * _)
// Return the number of lines in a given buffer
{
if (_==0) return 0;                    // No buffer
_R * p_R = static_cast<_R *>(_);       // Get buffer handle
return p_R->m_B.size();
}

//------------------------------------------------------------------------------

void * RBuffer::MakeBuffer(string n)
// Create a new (named) buffer and return the handle
{
_R * p_R = new _R;                     // Create it
m_R[n] = p_R;                          // Load the map
return (void *) p_R;                   // Return the handle
}

//------------------------------------------------------------------------------

void * RBuffer::MakeLine(void * _,string n)
// Create a new data line (called "n") in the buffer with handle "_", return
// the line handle
{
if (_==0) throw(E(__FILE__,__LINE__));
_R * p_R = static_cast<_R *>(_);       // Find the buffer
_B * p_B = new _B();                   // Build a new line
p_R->m_B[n] = p_B;                     // Add line to buffer
return (void *) p_B;                   // Return line handle
}

//------------------------------------------------------------------------------

string RBuffer::Name()
// Return the object name
{
return name;
}

//------------------------------------------------------------------------------

void RBuffer::Name(string n)
// Set the object name
{
name = n;
}

//------------------------------------------------------------------------------
 
unsigned RBuffer::Put(void * _,string v)
// Insert a data item (v) into line handle "_". This is a special case of the
// template below, because we have to pull the string data on the heap (the
// actual string) out separately. The string (for all lines) goes into the
// object-wide stringbank, and the offset in the bank is stored as the line data
{
if(_==0) throw(E(__FILE__,__LINE__));  // No line?
_B * p_B = static_cast<_B *>(_);       // Get line handle
if (p_B->_type.empty()) {              // We don't know the type name yet
  p_B->_type = typeid(string).name();  // So set it
  p_B->_sizeof = SUINT;                // Type size (offset in stringbank)
}
                                       // We do know it, and it's inconsistent
if (typeid(string).name()!=p_B->_type) throw(E(__FILE__,__LINE__));
byte buff[SUINT];
strbank.push_back(v);                  // Store the string data itself
unsigned adr = strbank.size()-1;       // Address in stringbank
memcpy(&buff[0],&adr,SUINT);           // Stringbank offset for main buffer
for (unsigned i=0;i<SUINT;i++) p_B->_data.push_back(buff[i]);
return p_B->_size()-1;                 // Return the storage offset
}
//------------------------------------------------------------------------------

template<typename T> unsigned RBuffer::Put(void * _,T v)
// Insert a data item (v) into line handle "_"
// I have no idea why I have to copy from the incoming to a temporary, and from
// that to the store. I get access violations if I try to go direct.
{
if(_==0) throw(E(__FILE__,__LINE__));  // No line?
_B * p_B = static_cast<_B *>(_);       // Get line handle
if (p_B->_type.empty()) {              // We don't know the type name yet
  p_B->_type = typeid(T).name();       // So set it
  p_B->_sizeof = sizeof(T);            // And the type size
}
                                       // We do know it, and it's inconsistent
if (typeid(T).name()!=p_B->_type) throw(E(__FILE__,__LINE__));
const unsigned BUF = 1024;
byte buff[BUF];                        // Stack buffer
byte * pbuff = &buff[0];               // Start of....
                                       // Use heap instead?
if (p_B->_sizeof>=BUF) pbuff = (byte *) new byte[p_B->_sizeof];
memcpy(&buff[0],&v,p_B->_sizeof);      // Copy incoming data to buffer
                                       // And copy this to the store
for (unsigned i=0;i<p_B->_sizeof;i++) p_B->_data.push_back(buff[i]);
if (pbuff!=&buff[0]) delete [] pbuff;  // Kill the heap buffer (if any)
return p_B->_size()-1;                 // Return the storage offset
}
//------------------------------------------------------------------------------

void RBuffer::Save(string fname)
// Save the entire object to the file "fname"
{
Header H;                              // Housekeeping.....
H.Name(fname);
FILE * fp = fopen(fname.c_str(),"wb"); // Open the save file
H.SaveB(fp);                           // Save the header
char Rtype = 0x74;
fwrite(&Rtype,1,1,fp);                 // Record type
unsigned bits = sizeof(void *) * 8;    // 32/64 bit land?
fwrite(&bits,SUINT,1,fp);
fwrite(&SUINT,SUINT,1,fp);             // Sizeof unsigned
fwritestr(name,fp);                    // Object name
unsigned nbuf = m_R.size();
fwrite(&nbuf,SUINT,1,fp);              // Number of buffers to come
WALKMAP(string,_R *,m_R,i)             // Write each buffer
  (*i).second->SaveB(fp,(*i).first);
unsigned banksiz = strbank.size();     // Size of string bank (can be 0)
fwrite(&banksiz,SUINT,1,fp);           // Stringbank contents
WALKVECTOR(string,strbank,i) fwritestr(*i,fp);
Rtype = 0x7d;                          // EOF
fwrite(&Rtype,1,1,fp);
fclose(fp);
}

//------------------------------------------------------------------------------

unsigned RBuffer::Size(void * _)
// Return number of data items (NOT bytes) in the line "_"
{
if (_==0) return 0;                    // Silly question
_B * p_B = static_cast<_B *>(_);       // Get line handle
if (p_B->_sizeof==0) return 0;         // Type not committed yet
return p_B->_data.size()/p_B->_sizeof; // sum(bytes)/size(item)
}

//------------------------------------------------------------------------------

unsigned RBuffer::Sizeof(void * _)
// Return the size of the data items in a line
{
if (_==0) return 0;                    // Get line handle
_B * p_B = static_cast<_B *>(_);
return p_B->_sizeof;                   // And answer the question
}

//==============================================================================

RBuffer::_R::_R()
// Buffer vanilla constructor
{
}

//------------------------------------------------------------------------------

RBuffer::_R::_R(FILE * fp)
// Reconstruct buffer from binary save
{
unsigned nlines;
fread(&nlines,SUINT,1,fp);             // Number of lines in the buffer
for(unsigned i=0;i<nlines;i++) {
  string name;                         // Line name
  freadstr(name,fp);
  m_B[name]= new _B(fp);               // And the line itself
}
}

//------------------------------------------------------------------------------

RBuffer::_R::~_R()
// Kill all the lines in the buffer
{
WALKMAP(string,_B *,m_B,i) delete (*i).second;
}

//------------------------------------------------------------------------------

void RBuffer::_R::SaveB(FILE * fp,string name)
// Save the line to an open file
{
fwritestr(name,fp);                    // Buffer name
unsigned nline = m_B.size();           // Number of lines in the buffer
fwrite(&nline,SUINT,1,fp);
                                       // And write the lines
WALKMAP(string,_B *,m_B,i) (*i).second->SaveB(fp,(*i).first);
}

//==============================================================================

RBuffer::_B::_B()
// Build an empty line
{
_sizeof = 0;
}

//------------------------------------------------------------------------------

RBuffer::_B::_B(FILE * fp)
// Restore from a binary save
{
freadstr(_type,fp);                    // Line data type name
fread(&_sizeof,SUINT,1,fp);            // Size of each data item
unsigned size;
fread(&size,SUINT,1,fp);               // Number of bytes
byte buff;
for (unsigned i=0;i<size;i++) {        // Pull in the byte stream
  fread(&buff,1,1,fp);
  _data.push_back(buff);
}
}

//------------------------------------------------------------------------------

RBuffer::_B::~_B()
{
}

//------------------------------------------------------------------------------

unsigned RBuffer::_B::_size()
// Number of user-defined items in the data store
{
if (_data.empty()) return 0;           // Nothing there at all
return _data.size()/_sizeof;           // Do the sums
}

//------------------------------------------------------------------------------

void RBuffer::_B::SaveB(FILE * fp,string name)
// Save the line as a byte stream to an open file
{
fwritestr(name,fp);                    // Line name
fwritestr(_type,fp);                   // Line data type name
fwrite(&_sizeof,SUINT,1,fp);           // Size of each data item
unsigned size = _data.size();
fwrite(&size,SUINT,1,fp);              // Number of bytes to come
fwrite(&_data[0],1,size,fp);           // And.... plop.
}

//==============================================================================

