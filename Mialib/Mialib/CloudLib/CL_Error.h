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
#ifndef CL_ERROR_INC  /* Allow multiple inclusions */
#define CL_ERROR_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

typedef enum {
	CL_OK = 0,
	PARAMETER_ID_NOT_FOUND,
	PARAMETER_ID_INVALID,
	MEAS_BUFFER_EMPTY,
	BAD_DEVICE_INDEX,
	OUTPUT_BUFFER_FULL,
	INTERNAL_BUFFER_FULL,
	DEVICE_NOT_INITIALISED,
	SERIALISER_ERROR,
} CL_ERRORS;

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------


#endif /* of CL_ERROR_INC */
/*! @} */ /* EOF, no more */
