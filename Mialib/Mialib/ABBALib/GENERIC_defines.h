/*! @addtogroup GENERIC
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
*                  Subsystem:   Generic                                       *
*                                                                             *
*                   Filename:   GENERIC_defines.h                             *
*                       Date:   2009-02-11                                    *
*                                                                             *
*                 Programmer:   ?Janne Mäkihonka / Espotel Oy Nivala          *
*                     Target:   ?                                             *
*                                                                             *
*******************************************************************************/
/*!
 *  @file GENERIC_defines.h
 *  @par File description:
 *    File contains sysmem wide general defines etc...
 *
 *  @par Related documents:
 *    List here...
 *
 *  @note
 *    START HERE!!!
 */
/******************************************************************************/
#ifndef GENERIC_DEFINES_INC  /* Allow multiple inclusions */
#define GENERIC_DEFINES_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include <stdint.h>
#include "GENERIC_defines_conf.h"


//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

typedef unsigned char       boolean;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef float               real32;
typedef double              real64;


#define MODE_READ           ( 1 << 0 )
#define MODE_WRITE          ( 1 << 1 )
#define MODE_READWRITE      MODE_READ | MODE_WRITE

#define MODE_NON_BLOCKING   ( 1 << 2 )
#define MODE_BLOCKING       ( 1 << 3 )

#define DIRECTION_IN        MODE_READ
#define DIRECTION_OUT       MODE_WRITE
#define DIRECTION_INOUT     MODE_READWRITE

#ifndef FALSE
    #define FALSE           0
#endif
#ifndef TRUE
    #define TRUE            1
#endif

#define HAL_DISABLE         0
#define HAL_ENABLE          1

#ifdef __IAR_SYSTEMS_ICC__
/* Objects of atomic types are the only objects that are free from data races,
   that is, they may be modified by two threads concurrently or modified by one and read by another. */
#define ATOMIC_VAR(var) _Atomic(var)
#else
#define ATOMIC_VAR(var) var
#endif

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 4) GLOBAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------


#endif  /* of GENERIC_DEFINES_INC */

/*! @} */ /* EOF, no more */

