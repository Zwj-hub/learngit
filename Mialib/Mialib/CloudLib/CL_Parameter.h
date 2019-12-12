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
*                   Filename:   Parameter.h                                   *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
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
#ifndef PARAMETER_INC  /* Allow multiple inclusions */
#define PARAMETER_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include <stdint.h>
#include "CL_Meas.h"
#include "CL_Serialiser.h"
#include "CL_MeasureTypes.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

#define PARAMETER_NAME_SIZE 64

typedef uint16_t CL_ParameterId;

typedef uint16_t ParameterID;

typedef struct CL_Parameter
{
	char name[PARAMETER_NAME_SIZE];
	ParameterID id;
	CL_ParameterMeasType type;
	CL_Meas *meas;
}CL_Parameter;


//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

int CL_ParameterInit(CL_Parameter * parameter, const char *name, ParameterID id, CL_ParameterMeasType type);

int CL_ParameterInsertMeas(CL_Parameter *parameter, CL_Meas *meas);

CL_Meas * CL_ParameterRemoveMeas(CL_Parameter *param);

int CL_ParameterSerialise(CL_Parameter * parameter, CL_Serialiser * serialiser);

#endif /* of PARAMETER_INC */
/*! @} */ /* EOF, no more */
