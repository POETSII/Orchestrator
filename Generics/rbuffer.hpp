#ifndef __RBufferH__H
#define __RBufferH__H

#include "header.h"
#include <string>
#include <utility>
#include <map>
#include <vector>
using namespace std;

//==============================================================================

class RBuffer
{
typedef unsigned char         byte;
public:
                              RBuffer();
                              RBuffer(string);
virtual ~                     RBuffer(void);
void                          Dump(string=string());
string                        Get(void *,unsigned);
template<typename T> T        Get(void *,unsigned);
void *                        GetBufferHandle(string);
void *                        GetLineHandle(void *,string);
unsigned                      Lines(void *);
void *                        MakeBuffer(string);
void *                        MakeLine(void *,string);
string                        Name();
void                          Name(string);
unsigned                      Put(void *,string);
template<typename T> unsigned Put(void *,T);
void                          Save(string);
unsigned                      Size(void *);
unsigned                      Sizeof(void *);

protected:
class                         _R;      // Single buffer forward declare
class                         _B;      // Line base class forward declare
friend                        _R;
friend                        _B;
string                        name;    // Object name
map<string,_R *>              m_R;     // Holder for the buffers
static const unsigned         SUINT = sizeof(unsigned);
vector<string>                strbank; // Special case: string store

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class _R
// A single results buffer, one of an arbitrary number
{
friend                        RBuffer; // I don't like writing code backwards
                              _R();    // Empty constructor
                              _R(FILE *);            // Restore from file
virtual ~                     _R();
void                          SaveB(FILE *,string);  // Write one buffer

map<string,_B *>              m_B;     // Holder for many lines
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class _B
// Base class for a single line
{
                              _B();    //Empty constructor
                              _B(FILE *);            // Restore from file
virtual ~                     _B();
unsigned                      _size(); // Length of vector (in items)
void                          SaveB(FILE *,string);  // Write one line

friend                        RBuffer;
friend                        _R;
string                        _type;   // Data type name (from typeid.name())
unsigned                      _sizeof; // Type size (bytes)
vector<byte>                  _data;   // The actual data!
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

};

//==============================================================================

#include "rbuffer.tpp"
#endif
