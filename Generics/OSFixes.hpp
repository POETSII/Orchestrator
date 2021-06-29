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
  #ifndef __BORLANDC__        // Borland doesn't know about inttypes.h,
   #include <inttypes.h>      // despite knowing about stdint.h...
  #else
    // Add defines for Borland
    #ifndef PRIx64
        #define PRIx32    "lx"
    #endif
    #ifndef PRIx32
        #define PRIx64    "llx"
    #endif
  #endif
 #include <sstream>
#endif

/* =============================================================================
 * Permit alternative operator spellings for Borland
 * ===========================================================================*/
#ifdef __BORLANDC__
#include <iso646.h>
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
#ifndef __BORLANDC__        // Borland doesn't know about inttypes.h,
#define PTR_FMT     "#018" PRIx64
#define SYMA_FMT    "#018" PRIx64
#define HWA_FMT     "#010" PRIx32
#define SWA_FMT     "#010" PRIx32

#else
#define PTR_FMT     "#018llx"
#define SYMA_FMT    "#018llx"
#define HWA_FMT     "#010lx"
#define SWA_FMT     "#010lx"

#endif


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
 #define TO_STRING   OSFixes::to_string
 namespace OSFixes
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
 * Cross-platform sleep (as in suspend-execution, not simply a delay). Due to
 * the precision on your hardware platform, your mileage may vary. Designed for
 * a pre-C++98 world.
 * ===========================================================================*/
#ifdef _WIN32
#include <windows.h>
#elif __unix
#include <time.h>
#endif
namespace OSFixes
{
    inline void sleep(int milliseconds)
    {
        #ifdef _WIN32
        Sleep(milliseconds);
        #elif __unix
        timespec time;
        time.tv_sec = milliseconds / 1000;
        time.tv_nsec = milliseconds % 1000 * 1000000;
        nanosleep(&time, PNULL);
        #endif
    }
}

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
#include <cerrno>

namespace OSFixes
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
#ifndef __riscv    // Not under RISCV
#ifdef _WIN32
#else /* Unix-like way */
#include <unistd.h>
#define HOSTNAME_LENGTH 128
#endif

namespace OSFixes
{
    inline std::string get_hostname()
    {
        #ifdef _WIN32
        printf("[OSFixes::get_hostname] MLV should edit this to use "
               "GetComputerNameExA!\n"); // <!>
        return "";
        #else
        char buffer[HOSTNAME_LENGTH];
        gethostname(buffer, HOSTNAME_LENGTH);
        return buffer;
        #endif
    }
}
#endif

/* =============================================================================
 * Filesystem manipulation - not under RISCV though.
 * ===========================================================================*/
#ifndef __riscv
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


/* =============================================================================
 * File operation consts for use with System- not under RISCV though.
 * TODO: make these into comprehensive methods for manipulation
 * ===========================================================================*/
#ifndef __riscv
#if (defined __BORLANDC__ || defined _MSC_VER)
const std::string SYS_COPY = "copy";
const std::string MAKEDIR = "md";
const std::string RECURSIVE_CPY = "";
const std::string REMOVEDIR = "rd /S /Q";
#elif (defined __GNUC__)
const std::string SYS_COPY = "cp";
const std::string RECURSIVE_CPY = "-r";
const std::string PERMISSION_CPY = "-p";
const std::string MAKEDIR = "mkdir";
const std::string REMOVEDIR = "rm --force --recursive";
#endif
#endif


namespace OSFixes
{
    /* Returns the directory component of 'fullPath'. Don't pass in a path to a
     * directory unless you know what you're doing. */
    inline std::string dirname(std::string fullPath)
    {
        #ifdef _WIN32
        printf("[OSFixes::dirname] MLV should edit this to use "
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
  #ifdef __BORLANDC__
        char path[MAXIMUM_PATH_LENGTH];        /* No initialiser to placate
                                                * Borland/C++98. */
  #else
        char path[MAXIMUM_PATH_LENGTH] = {};   /* Empty initialiser to placate
                                                * Valgrind.*/
  #endif

        /* Determining the predicate by OS. Value resolution is independent. */
  #ifdef _WIN32
        HMODULE hModule = GetModuleHandle(PNULL);
        if (hModule != PNULL)
        {
            GetModuleFileName(hModule, path, sizeof(path));

  #else       /* Unix-like way. If you want to harden this further, look at the
               * man page for readlink, at:
               * http://man7.org/linux/man-pages/man2/readlink.2.html#EXAMPLE */
        if (readlink("/proc/self/exe", path, MAXIMUM_PATH_LENGTH))
        {

  #endif
            return std::string(path);           // Need string call to deal with "Suspicious pointer conversion" in Borland
        }

        else
        {
            return "";
        }
    }
}
#endif /* _riscv */
/* ===========================================================================*/



/* =============================================================================
 * Template method to get a pointer
 * ===========================================================================*/
namespace OSFixes
{
    /*template <typename ptr>
    uint64_t getAddrAsUint(ptr (*tgtPtr)())
    {
        // This horrible double type cast is needed to get a sensible, warning-
        // free function pointers. void*s are not meant to point to functions,
        // only objects. void** is an object pointer that can be followed to get
        // a void*, which can then be used to get the function's address as a
        // uint64_t.
        return reinterpret_cast<uint64_t>(*(reinterpret_cast<void**>(&tgtPtr)));
    }*/

    // Borland and GCC disagree on which template definitions are ambiguous and
    // which are necessary.

    template <typename ptr>
    uint64_t getAddrAsUint(ptr *tgtPtr)
    {
    	// It's a "basic" pointer, so we just cast to a uint64_t
    	return reinterpret_cast<uint64_t>(tgtPtr);
    }

#ifndef __BORLANDC__
    template <typename ptr>
    uint64_t getAddrAsUint(ptr const *tgtPtr)
    {
        // We have some const-ness involved, so we need to get rid of it first.
        return reinterpret_cast<uint64_t>(*(const_cast<void**>(&tgtPtr)));
    }
#endif

#ifndef __BORLANDC__
    template <typename ptr>
    uint64_t getAddrAsUint(ptr& tgtPtr)
    {
    	// Someone gave us a value instead of a pointer.
    	return reinterpret_cast<uint64_t>(&tgtPtr);
    }
#endif

#ifdef __BORLANDC__
    template <typename ptr>
    uint64_t getAddrAsUint(ptr tgtPtr)
    {
    	// This deals with functions pointers in Borland.
    	return reinterpret_cast<uint64_t>(&tgtPtr);
    }
#endif
}
/* ===========================================================================*/


#endif /* __OSFIXES__H */
