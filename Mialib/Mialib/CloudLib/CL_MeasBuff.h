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
*                   Filename:   CL_MeasBuff.h                                 *
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
#ifndef CL_MEASBUFF_INC  /* Allow multiple inclusions */
#define CL_MEASBUFF_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include "CL_Meas.h"
//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------


#define CREATE_CL_MeasBuff(name, size)    \
	CL_Meas name ## _mbuf[size];          \
	CL_MeasBuff name = {                  \
		name ## _mbuf,                    \
		NULL,                             \
		size,                             \
		0                                 \
	}

#define CL_MeasBuffSize(mbuff)     ((mbuff)->measCnt)
#define CL_MeasBuffReserved(mbuff) ((mbuff)->reserved)
//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------
typedef struct CL_MeasBuff
{
	CL_Meas *meas; // Buffer for measurements
	CL_Meas *freeList; // Linked list for measurements which are free to use for parameters
	uint16_t measCnt;
	uint16_t reserved;
}CL_MeasBuff;


//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

int CL_MeasBuffGetFreeSize(CL_MeasBuff *measBuff);
CL_Meas * CL_MeasBuffReserve(CL_MeasBuff *measBuff);
int CL_MeasBuffFree(CL_MeasBuff *measBuff, CL_Meas *meas);
int CL_MeasBuffFreeAll(CL_MeasBuff *measBuff);

#endif /* of CL_MEASBUFF_INC */
/*! @} */ /* EOF, no more */
