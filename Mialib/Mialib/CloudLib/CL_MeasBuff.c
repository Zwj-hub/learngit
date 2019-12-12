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
*                   Filename:   CL_MeasBuff.c                                 *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file CL_MeasBuff.c
 *  @par File description:
 *   
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/


//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include "CL_MeasBuff.h"
#include "CL_Utils.h"

//-----------------------------------------------------------------------------
// 2) LOCAL ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 3) LOCAL DATA STRUCTURES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 5) GLOBAL VARIABLES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 6) LOCAL FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

int CL_MeasBuffGetFreeSize(CL_MeasBuff *measBuff)
{
	return CL_MeasBuffSize(measBuff) - CL_MeasBuffReserved(measBuff);
}

static void init(CL_MeasBuff *measBuff)
{
	int i;
	// Fill free list
	for(i = 0; i < CL_MeasBuffSize(measBuff); i++)
	{
		measBuff->meas[i].next = NULL;
		CL_MeasAdd(&measBuff->freeList, &measBuff->meas[i]);
	}
	measBuff->reserved = 0;
}

CL_Meas * CL_MeasBuffReserve(CL_MeasBuff *measBuff)
{
	if(!measBuff->reserved && !measBuff->freeList) // No reservation but freelist is empty. Add items from buffer to freelist
	{
		init(measBuff);
	}
	
	CL_Meas * res = CL_MeasPop(&measBuff->freeList);
	
	if(res)
	{
		measBuff->reserved++;
	}
	
	ASSERT(CL_MeasBuffReserved(measBuff) <= CL_MeasBuffSize(measBuff));
	
	return res;
}

int CL_MeasBuffFreeAll(CL_MeasBuff *measBuff)
{
	init(measBuff);
	return 0;
}

int CL_MeasBuffFree(CL_MeasBuff *measBuff, CL_Meas *meas)
{
	CL_MeasAdd(&measBuff->freeList, meas);
	
	ASSERT(measBuff->reserved);
	
	measBuff->reserved--;
	
	return 0;
}

/*! @} */ /* EOF, no more */
