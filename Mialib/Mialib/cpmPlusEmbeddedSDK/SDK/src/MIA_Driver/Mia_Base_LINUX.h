#ifndef MIA_BASE_LINUX_H_
#define MIA_BASE_LINUX_H_

/*
 * Mia_Base_LINUX.h
 *
 *  Created on:
 *      Author:
 */

#include <string>          //zwj
#include "GENERIC_defines.h"  //zwj
#include "define.h"             //zwj



#define Mia_EXPORT __attribute__((visibility("default")))
#define Mia_IMPORT
#define Mia_TEXT2(x)    x
#define Mistringa_L64(x)      x##LL
#define Mia_LU64(x)     x##LLU

#define Mia_TEXT(x)	Mia_TEXT2(x)

#define CSCommon_TCHAR_IMPLEMENTATION 2
typedef char tchar;

typedef std::wstringbuf tstringbuf;

typedef std::basic_istringstream<tchar> tistringstream; // Probably causes std::bad_cast when used.
typedef std::basic_ostringstream<tchar> tostringstream;
typedef std::basic_stringstream<tchar> tstringstream;
typedef std::wistream tistream;
typedef std::wostream tostream;
//typedef char int8;
typedef short int16;
typedef int int32;

// 32-bit x86 GCC needs to be told to align int64, uint64 and double by 8 bytes. Default alignment is 4 bytes.
#ifdef __GNUC__
//typedef long long __attribute__((aligned(8))) int64;
//typedef unsigned long long __attribute__((aligned(8))) uint64;
typedef double __attribute__((aligned(8))) float64;
#else
typedef long long int64;
typedef unsigned long long uint64;
typedef double float64;
#endif

#define g_INFINITE ~1

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
typedef std::string T_SafeString;
typedef T_SafeString tstring;

static const tchar g_PathSeparator = Mia_TEXT('/');

#define G_MIA_STRTOLL(a) std::atoll(a)

#define UNUSED_VAR __attribute__((unused))

//typedef int  t_EventHandle;      //zwj
typedef void*  t_EventHandle;     //zwj
typedef void* t_ThreadHandle;

#endif
