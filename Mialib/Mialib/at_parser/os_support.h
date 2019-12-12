#ifndef OS_SUPPORT_H_
#define OS_SUPPORT_H_


#ifdef __linux
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#define SLEEP(a) usleep(a * 1000)

#define GET_TIME_SECONDS() ((unsigned int)time(NULL))

#define KRED  "\x1B[31m"
#define KMAG  "\x1B[35m"
#define KNRM  "\x1B[0m"

#elif defined(OSAL_OS)

#include "OSAL_Time.h"

#define SLEEP(a) do{ uint16 ms = a%1000; if(a/1000)OSAL_eTimeDelay(a/1000); if(ms)OSAL_eTimeDelayMS( ms < 5 ? 5 : ms); }while(0)


#ifdef WIN32
#include <stdio.h>
void DLOG_vPrintF(char * format, ...);
#define DEBUG_ERR 1
#define printf DLOG_vPrintF
#define INLINE __inline
#else
#define INLINE inline
#endif

#ifdef INLINE_NOT_SUPPORTED
#undef INLINE
#define INLINE
#endif

static INLINE unsigned int __osal_get_time(void) { OSAL_T_TimeCounter _counter; OSAL_eTimeGetCounter(&_counter); return  _counter.u32MilliSeconds / 1000; }

#define GET_TIME_SECONDS() __osal_get_time()

#else

#error OS not handeled

#endif

#ifndef KRED
#define KRED  ""
#define KMAG  ""
#define KNRM  ""
#endif

#ifndef MODEM_DEBUG
#define MODEM_DEBUG 0
#endif

#if MODEM_DEBUG
#define MODEM_DEBUG_LIGHT 1
#endif

#ifndef MODEM_DEBUG_ERR
#define MODEM_DEBUG_ERR 0
#endif

#ifndef MODEM_DEBUG_LIGHT
#define MODEM_DEBUG_LIGHT 0
#endif

extern uint8_t g_modem_output;
#define MODEM_DEBUG_OUT_FLAG 1
#define MODEM_LIGHT_OUT_FLAG 2
#define MODEM_ERR_OUT_FLAG 4

/*#define DBG_PR_RAW(...) if(MODEM_DEBUG && (g_modem_output & MODEM_DEBUG_OUT_FLAG)) do{printf(__VA_ARGS__);}while(0)
#define DBG_PR(...) if(MODEM_DEBUG && (g_modem_output & MODEM_DEBUG_OUT_FLAG)) do{printf(KMAG MODULE_NAME ":"); printf(__VA_ARGS__); printf(KNRM);}while(0)
#define ERR_PR(...) if((MODEM_DEBUG || MODEM_DEBUG_ERR) && (g_modem_output & MODEM_ERR_OUT_FLAG))do{printf(KRED MODULE_NAME ":"); printf(__VA_ARGS__); printf(KNRM);}while(0)
#define LIGHT_DBG_PR(...) if(MODEM_DEBUG_LIGHT && (g_modem_output & MODEM_LIGHT_OUT_FLAG)) do{printf(KMAG "%03u:" MODULE_NAME ":", GET_TIME_SECONDS()%1000 ); printf(__VA_ARGS__); printf(KNRM);}while(0)
*/
#define DBG_PR_RAW(...)  do{printf(__VA_ARGS__);}while(0)
#define DBG_PR(...)  do{printf(KMAG MODULE_NAME ":"); printf(__VA_ARGS__); printf(KNRM);}while(0)
#define ERR_PR(...) do{printf(KRED MODULE_NAME ":"); printf(__VA_ARGS__); printf(KNRM);}while(0)
#define LIGHT_DBG_PR(...) do{printf(KMAG "%03u:" MODULE_NAME ":", GET_TIME_SECONDS()%1000 ); printf(__VA_ARGS__); printf(KNRM);}while(0)
#endif

