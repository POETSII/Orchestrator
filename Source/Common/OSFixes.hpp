#ifndef __OSFIXES__H
#define __OSFIXES__H
#include <string>
#include <string.h>

/* =============================================================================
 * =============================================================================
 * OS/compiler-specific oddities to make code as portable as possible.
 *
 *
 * =============================================================================
 * ===========================================================================*/

/* =============================================================================
 * Avoid format warnings on MINGW from it trying to use old MSVc printf
 * ===========================================================================*/
#if defined(__MINGW64__) || defined(__MINGW32__)
 #define __USE_MINGW_ANSI_STDIO  1
#endif


/* =============================================================================
 * Headers for fixed-width ints (i.e. uint64_t) depend on c++ version.
 * These are needed for addresses.
 * ===========================================================================*/
#if __cplusplus >= 201103
 #include <cstdint>
 #include <cinttypes>
#else
 #include <stdint.h>
 #include <inttypes.h>
 #include <sstream>
#endif

/* =============================================================================
 * Pointer/Address formatting depends on C++ & Compiler version.
 *
 * C++98 doesn't necessarily know about long-long and long is not 64-bits on all
 * compiler/platform combinations. To complicate matters, uint64_t (inittpypes.h
 * /cinittypes) is not consistently typed across platforms. e.g. it is long on
 * Ubuntu 16.04 GCC but long long on MinGW 64-bit.
 * Relies on inttypes.h/cinittypes.
 *
 * Example Usage:
 *      fprintf(fp, "Pointer: %" PTR_FMT "\n", static_cast<uint64_t>(&Var);
 * ===========================================================================*/
#define PTR_FMT     "#018" PRIx64
#define SYMA_FMT    "#018" PRIx64
#define HWA_FMT     "#010" PRIx32
#define SWA_FMT     "#010" PRIx32


/* =============================================================================
 * Handle NULL<>nullptr for C++98 and C++11+ compatibility.
 *
 * C++11 complains about using NULL as it compares a pointer to 0, which is NOT
 * portable according to the standard. C++98 does not have nullptr.
 *
 * Usage is anywhere that you would use NULL or nullptr
 * ===========================================================================*/
#if __cplusplus >= 201103   // C++11+ land
 #define PNULL nullptr
#else
 #define PNULL NULL
#endif


/* =============================================================================
 * Offer a to_string method for C++98
 * ===========================================================================*/
#if __cplusplus >= 201103               // C++11+ land
 #define TO_STRING   std::to_string
#else                                   // C++98 land
 #define TO_STRING   POETS::to_string
 namespace POETS
 {
    template<typename T>
    std::string to_string(const T& value)
    {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
 }
#endif


/* =============================================================================
 * Quick and dirty cross-platform method to get error string from error code.
 *
 * When called with "errno", this will return an std::string containing the
 * relevant error text for the last failed system call.
 *
 * The "default" option (using sterror) is NOT thread safe, but is portable.
 * Other options (using sterror_s and friends) need adding for Windows/Borland.
 *
 * Example Usage:
 *  std::cout << getSysErrorString(errno) << std::endl;
 * ===========================================================================*/
namespace POETS
{
    inline std::string getSysErrorString(int errNum)
    {
      #if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && _GNU_SOURCE
        char errMsg[1000];
        return std::string(strerror_r(errNum, errMsg, 1000));
      #elif _POSIX_C_SOURCE >= 200112L
        char errMsg[1000];
        if(strerror_r(errNum, errMsg, 1000)) return std::string("");
        else return std::string(errMsg);
      #else
        return std::string(strerror(errNum));
      #endif
    }
}

/* =============================================================================
 * Macros for indicating that variables may be unused by design.
 *
 * Example use cases include common code (e.g. softswitch) where user-provided
 * code is inserted into pre-defined methods.
 *
 * OS_ATTRIBUTE_UNUSED   Placed immedidiately after potentially unused var.
 * OS_PRAGMA_UNUSED(x)   Placed on a line after a potentially unused var.
 *
 * Example Usage:
 *      unsigned foo OS_ATTRIBUTE_UNUSED = 0;
 *      OS_PRAGMA_UNUSED(foo)				// N.B. note the lack of ";"
 * ===========================================================================*/
#ifdef __GNUC__				/* GCC Specific definitions. */
#define OS_ATTRIBUTE_UNUSED		__attribute__((unused))
#define OS_PRAGMA_UNUSED(x)

#elif __BORLANDC__			/* Borland specific definitions. */
#define OS_ATTRIBUTE_UNUSED
#define OS_PRAGMA_UNUSED(x)		(void) x ;

#elif __clang__				/* Clang Specific definitions. */
#define OS_ATTRIBUTE_UNUSED
#define OS_PRAGMA_UNUSED(x)		#pragma unused(x);

#elif _MSC_VER				/* MS Specific definitions. */
#define OS_ATTRIBUTE_UNUSED
#define OS_PRAGMA_UNUSED(x)		x;

#else						/* Other compilers. */
#warning "Building without Compiler-specific unused variable handling."
#define OS_ATTRIBUTE_UNUSED
#define OS_PRAGMA_UNUSED(x)
#endif

/* =============================================================================
 * Hostname retrieval, as a string.
 * ===========================================================================*/
#ifdef _WIN32
#else /* Unix-like way */
#include <unistd.h>
#define HOSTNAME_LENGTH 128
#endif

namespace POETS
{
    inline std::string get_hostname()
    {
        #ifdef _WIN32
        printf("[POETS::get_hostname] MLV should edit this to use "
               "GetComputerNameExA!\n"); // <!>
        return "";
        #else
        char buffer[HOSTNAME_LENGTH];
        gethostname(buffer, HOSTNAME_LENGTH);
        return buffer;
        #endif
    }
}

/* =============================================================================
 * Filesystem manipulation
 * ===========================================================================*/

#ifdef _WIN32
#include <stdlib.h>
#include <windows.h>
#define MAXIMUM_PATH_LENGTH MAX_PATH /* Actually, MAX_PATH isn't. See
https://stackoverflow.com/questions/833291/is-there-an-equivalent-to-winapis
-max-path-under-linux-unix */
#else /* Unix-like way */
#include <libgen.h>
#include <linux/limits.h>
#include <unistd.h>
#define MAXIMUM_PATH_LENGTH PATH_MAX /* Actually, PATH_MAX isn't. See
http://insanecoding.blogspot.com/2007/11/pathmax-simply-isnt.html */
#endif


namespace POETS
{
    /* Returns the directory component of 'fullPath'. Don't pass in a path to a
     * directory unless you know what you're doing. */
    inline std::string dirname(std::string fullPath)
    {
        #ifdef _WIN32
        printf("[POETS::dirname] MLV should edit this to use "
               "Generics/filename!\n"); // <!>
        return "";
        #else
        /* FYI: The function '::dirname' refers to dirname in the global
         * namespace. Another pitfall: The const_cast is implicitly trusting
         * ::dirname not to modify the path passed in. */
        return ::dirname(const_cast<char*>(fullPath.c_str()));
        #endif
    }

    /* Gets the path to the executable calling this function. Note that this
     * will not get the object. Returns a string containing the path if
     * successful, and returns an empty string otherwise. */
    inline std::string get_executable_path()
    {
        char path[MAXIMUM_PATH_LENGTH] = {};  /* Empty initialiser to placate
                                               * Valgrind. */

        /* Determining the predicate by OS. Value resolution is independent. */
        #ifdef _WIN32
        HMODULE hModule = GetModuleHandle(PNULL);
        if (hModule != PNULL)
        {
            GetModuleFileName(hModule, path, sizeof(path));

        #else /* Unix-like way. If you want to harden this further, look at the
               * man page for readlink, at:
               * http://man7.org/linux/man-pages/man2/readlink.2.html#EXAMPLE */
        if (readlink("/proc/self/exe", path, MAXIMUM_PATH_LENGTH))
        {

        #endif
            return path;
        }

        else
        {
            return "";
        }
    }
}


/* ===========================================================================*/
#endif /* __OSFIXES__H */
