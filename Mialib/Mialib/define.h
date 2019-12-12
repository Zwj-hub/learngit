/*! @addtogroup OS_Abstraction
 *  @{
 */
/******************************************************************************
*                                                                             *
*                   COPYRIGHT (C) 2009    ABB OY, DRIVES                      *
*                                                                             *
*******************************************************************************
*                                                                             *
*                                                                             *
*                     Daisy Assistant Panel SW                                *
*                                                                             *
*                                                                             *
*                  Subsystem:   OS Abstraction                                *
*                                                                             *
*                   Filename:   os_cpu.h                                      *
*                       Date:   2009-03-30                                    *
*                                                                             *
*                 Programmer:   Miikka Mättö                                  *
*                     Target:   PC                                            *
*                                                                             *
*******************************************************************************/
/*!
 *  @file os_cpu.h
 *  @par File description:
 *    OS Abstraction layer uC/OS II WIN32 port header file.
 *
 *  @par Related documents:
 *
 *  @note
 */


/*! Data types used by OS */
#ifndef DEFINE_H_
#define DEFINE_H_
#include <stdint.h>
typedef	unsigned char       BOOLEAN;
typedef unsigned char       INT8U;
typedef signed char         INT8S;
typedef unsigned short      INT16U;
typedef signed short        INT16S;
typedef unsigned long       INT32U;
typedef signed long         INT32S;
typedef float               FP32;
typedef double              FP54;

/*! Data type of OS stack item */
typedef INT16U              OS_STK;

typedef struct os_event {
    INT8U    OSEventType;                    /* Type of event control block (see OS_EVENT_TYPE_xxxx)    */
    INT8U    OSEventReservee;                /* Task prio that has last reserved a semaphore            */
    void    *OSEventPtr;                     /* Pointer to message or queue structure                   */
    INT16U   OSEventCnt;                     /* Semaphore Count (not used if other EVENT type)          */

    INT8U    OSEventGrp;                     /* Group corresponding to tasks waiting for event to occur */

    INT8U    OSEventTbl[6];    /* List of tasks waiting for event to occur                */

    INT8U    OSEventName[4];

} OS_EVENT;

typedef int8_t int8;
//typedef int64_t int64;
//typedef uint64_t uint64;
typedef long long __attribute__((aligned(8))) int64;
typedef unsigned long long __attribute__((aligned(8))) uint64;
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
#define OS_ERR_NONE 0
#endif
