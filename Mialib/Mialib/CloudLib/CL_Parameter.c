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
*                   Filename:   CL_Parameter.c                                *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file CL_Parameter.c
 *  @par File description:
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include "CL_Parameter.h"
#include "CL_Utils.h"
#include "CL_Serialiser.h"

#include <string.h>
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

static int ParameterSerialiserMeas(CL_Parameter * parameter, CL_Serialiser * serialiser)
{
	int ret = 0;
	CL_Meas *meas;

	for (meas = parameter->meas; meas; meas = meas->next)
	{
		ret = CL_MeasSerialise(meas, serialiser, parameter->type);
		if (ret) 
			break;
	}

	return ret;
}

static int ParameterSerialiser(CL_Parameter * parameter, CL_Serialiser * serialiser)
{
	int ret = -1;

	do
	{
		ret = CL_SERIALISER_ADD(serialiser, CL_SER_PARAMETER_ID, &parameter->id);
		if (ret) 
			break;

		ret = CL_SERIALISER_ADD(serialiser, CL_SER_PARAMETER_NAME, parameter->name);
		if (ret) 
			break;

		ret = CL_SERIALISER_ADD(serialiser, CL_SER_PARAMETER_TYPE, &parameter->type);
		if (ret) 
			break;

		ret = ParameterSerialiserMeas(parameter, serialiser);
		if (ret) 
			break;

	} while (0);

	return ret;
}

//-----------------------------------------------------------------------------
// 7) FUNCTIONS
//-----------------------------------------------------------------------------

int CL_ParameterInsertMeas(CL_Parameter *parameter, CL_Meas *meas)
{
	CL_MeasAdd(&parameter->meas, meas);
	return 0;
}

CL_Meas * CL_ParameterRemoveMeas(CL_Parameter *parameter)
{
	return CL_MeasPop(&parameter->meas);
}

int CL_ParameterInit(CL_Parameter *parameter, const char *name, ParameterID id, CL_ParameterMeasType type)
{
	if (strlen(name) >= PARAMETER_NAME_SIZE) {
		return -INTERNAL_BUFFER_FULL;
	}

	if (!id) { // Id == 0 is reseved for invalid
		return -PARAMETER_ID_INVALID;
	}

	memset(parameter, 0, sizeof(*parameter));
	
	strcpy(parameter->name, name);
	
	parameter->type = type;
	parameter->id   = id;
	
	return 0;
}

int CL_ParameterSerialise(CL_Parameter * parameter, CL_Serialiser * serialiser)
{
	int err = 0;
	if(parameter->id && parameter->name[0])
	{
		CL_SERIALISER_DO_BLOCK(serialiser, CL_SER_PARAMETER, ParameterSerialiser, parameter, err);
	}
	
	return err;
}

/*! @} */ /* EOF, no more */
