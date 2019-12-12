/*! @addtogroup CouldLib
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
*                   Filename:   CL_Meas.h                                     *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file CL_MeasBuff.h
 *  @par File description:
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
#ifndef CL_MEAS_INC  /* Allow multiple inclusions */
#define CL_MEAS_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include <stdint.h>
#include "CL_Serialiser.h"
#include "CL_MeasureTypes.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

typedef uint32_t CL_Timestamp;

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------

typedef union CL_MeasValue
{
	uint8_t  vBool;
	uint8_t  vUint8;
	uint16_t vUint16;
	uint32_t vUint32;
	uint64_t vUint64;
	int64_t vInt64;
	int32_t vInt32;
	int16_t vInt16;
	int8_t vInt8;
	float vReal32;
	double vReal64;
	// TODO data CB. Look architecture speck
}CL_MeasValue;

struct CL_Meas; // For linked list

typedef struct CL_Meas
{
	struct CL_Meas *next;
	CL_MeasValue value;
	CL_Timestamp timestamp;
}CL_Meas;

//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

// Initialise Meas
void CL_MeasInit(CL_Meas *meas, void * value, CL_Timestamp timestamp);


// Add new new item to previous measure
void CL_MeasAdd(CL_Meas **measList, CL_Meas *meas);

// Remove item from meas list
CL_Meas * CL_MeasPop(CL_Meas **measList);

int CL_MeasSerialise(CL_Meas *meas, CL_Serialiser * serialiser, CL_ParameterMeasType measType);

#endif /* of CL_MEAS_INC */
/*! @} */ /* EOF, no more */
