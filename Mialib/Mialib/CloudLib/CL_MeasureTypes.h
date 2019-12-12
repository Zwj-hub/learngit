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
*                   Filename:   CL_MeasureTypes.h                             *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Christer Ekholm                               *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
*  @file ParameterList.h
*  @par File description:
*
*  @par Related documents:
*
*  @note
*/
/******************************************************************************/
#ifndef CL_MEASURE_TYPES_INC  /* Allow multiple inclusions */
#define CL_MEASURE_TYPES_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------
typedef enum CL_ParameterMeasType {
	CL_INT8,
	CL_UINT8,
	CL_INT16,
	CL_UINT16,
	CL_INT32,
	CL_UINT32,
	CL_INT64,
	CL_UINT64,
	CL_REAL32,
	CL_REAL64,
	CL_BOOL,
	CL_STRING,
	CL_CBMEAS
}CL_ParameterMeasType; 

#endif // CL_MEASURE_TYPES_INC
