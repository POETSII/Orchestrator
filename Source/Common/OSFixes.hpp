#ifndef OSFixes_H
#define OSFixes_H

/* =============================================================================
 * Avoid format warnings on MINGW from it trying to use old MSVc printf *============================================================================*/
#if defined(__MINGW64__) || defined(__MINGW32__)
 #define __USE_MINGW_ANSI_STDIO  1
#endif


/* =============================================================================
 * Headers for fixed-width ints (i.e. uint64_t) depend on c++ version. 
 * These are needed for addresses.
 *============================================================================*/
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
 *============================================================================*/
#define PTR_FMT     "#018" PRIx64
#define SYMA_FMT    "#018" PRIx64
#define HWA_FMT     "#010" PRIx32
#define SWA_FMT     "#010" PRIx32


/* =============================================================================
 * Handle NULL<>nullptr for C++98 and C++11+ compatibility.
 *
 * C++11 complains about using NULL as it compares a pointer to 0, which is NOT
 * portable according to the standard. C++98 does not have nullptr.
 *============================================================================*/
#if __cplusplus >= 201103   // C++11+ land
 #define PNULL nullptr
#else
 #define PNULL NULL
#endif



/* =============================================================================
 * Offer a to_string method for C++98
 *============================================================================*/
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


#endif /* OSFixes_H */