#ifndef MIA_BASE_LINUX_H_
#define MIA_BASE_LINUX_H_

/*
 * Mia_Base_OS.h
 *
 *  Created on:
 *      Author:
 */

#include <stdint.h>
#include <string>          //zwj

#define Mia_EXPORT 
#define Mia_IMPORT
#define Mia_TEXT2(x)    x
#define Mia_L64(x)      x##LL
#define Mia_LU64(x)     x##LLU

#define Mia_TEXT(x)	Mia_TEXT2(x)

#define CSCommon_TCHAR_IMPLEMENTATION 2
typedef char tchar;

typedef std::basic_stringstream<tchar> tstringstream;
typedef std::wstringbuf tstringbuf;

typedef std::basic_istringstream<tchar> tistringstream; // Probably causes std::bad_cast when used.
typedef std::basic_ostringstream<tchar> tostringstream;
typedef std::basic_stringstream<tchar> tstringstream;
typedef std::wistream tistream;
typedef std::wostream tostream;
/*typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;

typedef int64_t int64;
typedef uint64_t uint64;
typedef double float64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint8_t uchar;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint64_t ulong;
typedef float    float32;
typedef long double float80;
*/
#define CSCommon_SAFESTRING_IMPLEMENTATION 0
typedef std::string T_SafeString;
typedef T_SafeString tstring;

static const tchar g_PathSeparator = Mia_TEXT('/');

#ifndef __TIMESTAMP__
#define __TIMESTAMP__ ""
#endif

#define UNUSED_VAR __attribute__((unused))

#define G_MIA_STRTOLL(a) std::atoll(a)

typedef void* t_EventHandle;
typedef void* t_ThreadHandle;

#define g_INFINITE ~1

#endif


