#include "DumpUtils.h"

/* Prints something like:
 *
 * prefix XXXXXXXX\n
 *
 * Where X is breakerSymbol, and the whole thing is MAXIMUM_BREAKER_LENGTH
 * characters long. If prefix is too long, just prints the prefix instead with
 * a breakerSymbol at the end. Arguments:
 *
 * - outPlace: Where to write the breaker to.
 *
 * - prefix: Prefix for the breaker.
 *
 * - prefixLength: Length of the prefix.
 *
 * - breakerSymbol: Symbol to use to construct the tail of the breaker.
 */

namespace DumpUtils {

    void breaker(FILE* outPlace, const std::string& prefix, char breakerSymbol)
    {
        std::string breakerTail;
        if (prefix.size() >= MAXIMUM_BREAKER_LENGTH)
        {
            breakerTail.assign(1, breakerSymbol);
        }
        else
        {
            breakerTail.assign(MAXIMUM_BREAKER_LENGTH - prefix.size() - 2,
                               breakerSymbol);
        }

        fprintf(outPlace, "%s %s\n", prefix.c_str(), breakerTail.c_str());
    }

    /* Prints a breaker with -s. */
    void close_breaker(FILE* outPlace, const std::string& prefix)
    {
        breaker(outPlace, prefix, '-');
    }

    /* Prints a breaker with +s. */
    void open_breaker(FILE* outPlace, const std::string& prefix)
    {
        breaker(outPlace, prefix, '+');
    }
}
