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
*                   Filename:   CL_Meas.c                                     *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file CL_Meas.c
 *  @par File description:
 *    Upload parameter using Azure IoT Device SDK
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include "CL_Meas.h"
#include "CL_Utils.h"
#include "CL_Serialiser.h"
#include "CL_MeasureTypes.h"
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

static int measSerialiser(void  * parameters[], CL_Serialiser * serialiser)
{
	CL_Meas * meas = (CL_Meas*)parameters[0];
	CL_ParameterMeasType type = *((CL_ParameterMeasType*)parameters[1]);

	int ret = -1;

	do
	{
		char valueStr[32] = "";

		switch (type)
		{
		case CL_REAL64:
			sprintf(valueStr, "%llf", meas->value.vReal64);
			break;
		case CL_REAL32:
			sprintf(valueStr, "%f", meas->value.vReal32);
			break;
		case CL_INT64:
			sprintf(valueStr, "%lld", meas->value.vInt64);
			break;
		case CL_INT32:
			sprintf(valueStr, "%d", meas->value.vInt32);
			break;
		case CL_INT16:
			sprintf(valueStr, "%d", meas->value.vInt16);
			break;
		case CL_INT8:
			sprintf(valueStr, "%d", meas->value.vInt8);
			break;
		case CL_UINT64:
			sprintf(valueStr, "%llu", meas->value.vUint64);
			break;
		case CL_UINT32:
			sprintf(valueStr, "%u", meas->value.vUint32);
			break;
		case CL_UINT16:
			sprintf(valueStr, "%u", meas->value.vUint16);
			break;
		case CL_UINT8:
			sprintf(valueStr, "%u", meas->value.vUint8);
			break;
		default:
			sprintf(valueStr, "%u", meas->value.vUint32);
		}

		ret = CL_SERIALISER_ADD(serialiser, CL_SER_PARAMETER_MEAS_TIMESTAMP, &meas->timestamp);
		if (ret) break;

		ret = CL_SERIALISER_ADD(serialiser, CL_SER_PARAMETER_MEAS_VALUE, valueStr);
		if (ret) break;

	} while (0);

	return ret;
}

//-----------------------------------------------------------------------------
// 7) FUNCTIONS
//-----------------------------------------------------------------------------


void CL_MeasInit(CL_Meas *meas, void * value, CL_Timestamp timestamp)
{
	memset(meas, 0, sizeof(*meas));
	
	memcpy(&meas->value.vUint64, value, sizeof(meas->value.vUint64));
	
	meas->timestamp = timestamp;
}

void CL_MeasAdd(CL_Meas **measList, CL_Meas *new)
{
	CL_Meas *first;
	
	if(!*measList)
	{
		// List empty
		*measList = new;
		return;
	}
	
	// Get old first
	first = (*measList);
	
	// Set new to be first
	*measList = new;
	
	// Set old first to be second
	new->next = first;
}

CL_Meas * CL_MeasPop(CL_Meas **measList)
{
	CL_Meas * first; // Current first item in list

	if(!*measList)  // List empty?
	{
		return NULL;
	}
	
	// Get first
	first = (*measList);
	
	// Set second to be new first
	*measList = first->next;
	
	// Remove link to others
	first->next = NULL;
	
	// return removed item
	return first;
}

int CL_MeasSerialise(CL_Meas *meas, CL_Serialiser * serialiser, CL_ParameterMeasType type)
{
	void *parameters[] = { meas, &type };
	int err;
	CL_SERIALISER_DO_BLOCK(serialiser, CL_SER_PARAMETER_MEAS, measSerialiser, parameters, err);
	return err;
}

/*! @} */ /* EOF, no more */
