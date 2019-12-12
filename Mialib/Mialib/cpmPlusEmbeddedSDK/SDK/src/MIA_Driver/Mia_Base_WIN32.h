#ifndef MIA_BASE_WIN32_H_
#define MIA_BASE_WIN32_H_

/*
 * Mia_Base_WIN32.h
 *
 *  Created on:
 *      Author: Christer Ekholm
 */


#include <stdint.h>
#include <time.h>
#include <iomanip>
#include <sstream>
#include <Windows.h> // For DWORD WINAPI


#define Mia_EXPORT __declspec(dllexport)
#define Mia_IMPORT __declspec(dllimport)

// To write a wide character literal character string
#define Mia_TEXT2(x)	L##x
// To write a 64-bit integer literal
#define Mia_L64(x)		x##i64
#define Mia_LU64(x)	x##ui64

#define Mia_TEXT(x)	Mia_TEXT2(x)

#define CSCommon_TCHAR_IMPLEMENTATION 0
typedef wchar_t tchar;
typedef HANDLE t_EventHandle;
typedef HANDLE t_ThreadHandle;

//		typedef std::wstring				tstring;
typedef std::wstringbuf tstringbuf;
typedef std::wistringstream tistringstream;
typedef std::wostringstream tostringstream;
typedef std::wstringstream tstringstream;
typedef std::wistream tistream;
typedef std::wostream tostream;
typedef int8_t int8;
typedef int16_t int16;
typedef __int64 int64;
typedef int32_t  int32;
typedef uint32_t uint32;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned __int64 uint64;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef float float32;
typedef double float64;
typedef long double float80;
//typedef GUID t_UUID;

#define CSCommon_SAFESTRING_IMPLEMENTATION 0
typedef std::wstring T_SafeString;
typedef T_SafeString tstring;

static const tchar g_PathSeparator = Mia_TEXT('\\');

#define strtoll _strtoi64
#define g_INFINITE INFINITE
#define G_MIA_STRTOLL(a) _atoi64(a)

#ifndef SYSTEM_THREAD_PARAM
	#define SYSTEM_THREAD_RET   DWORD WINAPI
	#define SYSTEM_THREAD_PARAM LPVOID
#endif

#endif //MIA_BASE_WIN32_H_