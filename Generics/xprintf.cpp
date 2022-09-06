#include "xprintf.h"

// Included not because it's ever actually called -
// it's part of the built-in diagnostics in A_base
// but u$oft get hysterical if it can't link everything
// that's cited anywhere in the source.

int xprintf(const char * fmt,...)
{
va_list ap;                            // Pointer into variable argument list
va_start(ap,fmt);                      // And away we go.....
int ans = vprintf(fmt,ap);
va_end(ap);                            // Tidy the stack
return ans;                            // 'Cos the spec says....
}
