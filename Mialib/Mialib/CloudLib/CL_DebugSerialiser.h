/*! @addtogroup CloudLib
 *  @{
 */
/******************************************************************************
*                                                                             *
*                   COPYRIGHT (C) 2018    ABB Beijing, DRIVES                 *
*                                                                             *
*******************************************************************************
*                                                                             *
*                                                                             *
*                     Daisy Assistant Panel SW                                *
*                                                                             *
*                                                                             *
*                  Subsystem:   CouldLib                                      *
*                                                                             *
*                   Filename:   CL_DebugSerial.h                              *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Christer Ekholm                               *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file CL_DebugSerial.h
 *  @par File description:
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
#ifndef CL_DEBUGSERIAL_INC  /* Allow multiple inclusions */
#define CL_DEBUGSERIAL_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include "CL_Serialiser.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

typedef enum {
	CREATE_DESCRIPTION = 10,
	CREATE_TELEMETRICS,
} CL_DebugSerialOutType;


#define CREATE_CL_DebugSerial(name, outType)\
	CL_DebugSerial name = {                  \
		CL_DebugSerialiserImpl,               \
		outType,                               \
	}


//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------
typedef struct CL_DebugSerial
{
	CL_SerialiserCmdType  parser;
	CL_DebugSerialOutType outType;

}CL_DebugSerial;


//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------
int CL_DebugSerialiserImpl(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * param);


#endif /* of CL_DEBUGSERIAL_INC */
/*! @} */ /* EOF, no more */
