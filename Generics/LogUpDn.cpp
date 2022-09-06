#include "LogUpDn.h"
#include <math.h>
#include <stdio.h>

/* A logarithmic interval generator: it has a Value. When you Click() it, the
Value goes up or down: 1,2,5 and powers of 10. (1,2,5,10,20,50 ...)
If you try to go beyond the limits, it just saturates.
Note there is no sign; it's a log generator.
The internal state is stored as a {fraction,exponent} pair, not the actual value
*/

//==============================================================================

LogUpDn::LogUpDn()
{
minVal = 0.001;                        // Smallest
maxVal = 1000.0;                       // Biggest
fra    = 1;                            // Internal state data
exp    = 1.0;                          // Value = fra * exp
}

//------------------------------------------------------------------------------

LogUpDn::~LogUpDn()
{
}

//------------------------------------------------------------------------------

void LogUpDn::Click(bool dir)
// fra: 1..2..5..1..2..5 and so on; exp: 1..10..100.. and so on
{
if (dir) {                             // Going up
  switch (fra) {
    case 1 : if ((2.0 * exp) <= maxVal) fra = 2;  return;
    case 2 : if ((5.0 * exp) <= maxVal) fra = 5;  return;
    case 5 : if ((exp * 10.0) <= maxVal) {
               fra = 1;
               exp *= 10.0;
             }                                    return;
  }
}

switch (fra) {                         // Going down
  case 1 : if ((5.0 * exp/10.0) >= minVal) {
             exp /= 10.0;
             fra = 5;
           }                                      return;
  case 2 : if (exp >= minVal) fra = 1;            return;
  case 5 : if ((exp * 2.0) >= minVal) fra = 2;    return;
}
}

//------------------------------------------------------------------------------

double LogUpDn::dValue()
// Return the value as a double
{
return double(fra) * exp;
}

//------------------------------------------------------------------------------

double LogUpDn::dValue(double nVal)
// Set the value from a double
{
if (nVal >= maxVal) nVal = maxVal;     // Clip the request
if (nVal <= minVal) nVal = minVal;
double oVal      = dValue();           // Save original value
                                       // Break up the new value
double logVal    = log10(nVal);        // Log
unsigned charVal = floor(logVal);      // Characteristic
double mantVal   = logVal - charVal;   // Mantissa
                                       // Rebuild the internal state variables
exp = pow(10,charVal);                 // Power of 10 is easy....
fra = 2;                               // fra is 2.....
const double LOG2 = 0.30103;
if (mantVal < LOG2) fra = 1;           // Unless it's 1
const double LOG5 = 0.69897;
if (mantVal > LOG5) fra = 5;           // Or it's 5
return oVal;                           // Return previous value
}

//------------------------------------------------------------------------------

pair<double,double> LogUpDn::Limits()
// Return the current limit pair
{
return pair<double,double>(minVal,maxVal);
}

//------------------------------------------------------------------------------

pair<double,double> LogUpDn::Limits(double nMin,double nMax)
// Set new saturating limits
{
double oMin = minVal;                  // Save old minimum
double oMax = maxVal;                  // Guess
minVal = nMin;                         // Store the new values
maxVal = nMax;
return pair<double,double>(oMin,oMax); // Return the old values
}

//------------------------------------------------------------------------------

pair<unsigned,double>LogUpDn::pValue()
// Return the current state variables
{
return pair<unsigned,double>(fra,exp);
}

//------------------------------------------------------------------------------

pair<unsigned,double>LogUpDn::pValue(pair<unsigned,double> nVal)
// Change the current internal state
{
unsigned oFra = fra;                   // Previous internal state
double oExp = exp;
fra = nVal.first;                      // New internal state
exp = nVal.second;
if (dValue()>maxVal) dValue(maxVal);   // Clamp to limits
if (dValue()<minVal) dValue(minVal);

return pair<unsigned,double>(oFra,oExp); // Return old value
}

//------------------------------------------------------------------------------

string LogUpDn::sValue()
// Turn it into a string so you can write it into GUI controls
{
char buf[256];
sprintf(buf,"%1.0e",exp*double(fra));
return string(buf);
}

//------------------------------------------------------------------------------

