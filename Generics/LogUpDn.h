#ifndef __LogUpDownH__H
#define __LogUpDownH__H

#include <string>
using namespace std;

//==============================================================================

class LogUpDn
{
public:
                        LogUpDn();
virtual ~               LogUpDn();

void                    Click(bool=true);
double                  dValue();
double                  dValue(double);
pair<double,double>     Limits();
pair<double,double>     Limits(double,double);
pair<unsigned,double>   pValue();
pair<unsigned,double>   pValue(pair<unsigned,double>);
string                  sValue();

double                  minVal;
double                  maxVal;
unsigned                fra;
double                  exp;

};

//==============================================================================



#endif
