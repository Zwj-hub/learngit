#ifndef MIA_BASE_QNX_H_
#define MIA_BASE_QNX_H_

/*
 * Mia_Base_QNX.h
 *
 *  Created on:
 *      Author: Christer Ekholm
 */

/* XXXX Not tested in QNX environment. May contain copy paste errors */

//  GCC
#define Mia_EXPORT __attribute__((visibility("default")))
#define Mia_IMPORT

#define Mia_TEXT2(x)	reinterpret_cast<const char16*>(L##x)

#define Mia_L64(x)		x##LL
#define Mia_LU64(x)	x##LLU

#define Mia_TEXT(x)	Mia_TEXT2(x)

#define CSCommon_TCHAR_IMPLEMENTATION 1
typedef unsigned short tchar;

typedef std::wstringbuf tstringbuf;

typedef std::wistream tistream;
typedef std::wostream tostream;
typedef char int8;
typedef short int16;
typedef int int32;

// 32-bit x86 GCC needs to be told to align int64, uint64 and double by 8 bytes. Default alignment is 4 bytes.
#ifdef __GNUC__
typedef long long __attribute__((aligned(8))) int64;
typedef unsigned long long __attribute__((aligned(8))) uint64;
typedef double __attribute__((aligned(8))) float64;
#else
typedef long long int64;
typedef unsigned long long uint64;
typedef double float64;
#endif

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef float float32;

typedef long double float80;

#define CSCommon_SAFESTRING_IMPLEMENTATION 0
typedef std::basic_string<unsigned short> T_SafeString;
typedef T_SafeString tstring;

const tchar g_PathSeparator = Mia_TEXT('/');

#define G_MIA_STRTOLL(a) std::atoll(a)

#endif
