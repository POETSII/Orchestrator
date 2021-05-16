#ifndef __pathDecodeH__H
#define __pathDecodeH__H

#include <stdio.h>
#include <string>
#include <vector>
using namespace std;

//==============================================================================
/*
This class is a single method functor. The (sole) functionality is to decode the
path string appearing in the POETS XML edge instance element
The string is (should be) of the form

    to_device : to_pin - from_device : from_pin

Either (or both, if you must) device fields may be empty, implying supervisor
connections.

The functor returns a string vector with at least one element:
[0] empty if the functor argument parsed OK, otherwise the STRING "1", "2"
    and so on indicating the position of the error in the string.
    In this situation, this is the sole element of the vector.
[1] to_device (may legitimately be empty)
[2] to_pin (never empty)
[3] from_device (may legitimately be empty)
[4] from_device (never empty)
*/
//==============================================================================

class pathDecode
{
public:
                   pathDecode(){}
virtual ~          pathDecode(void){}

vector<string>     operator()(string); // Functor that does the decode
};

//==============================================================================

#endif
