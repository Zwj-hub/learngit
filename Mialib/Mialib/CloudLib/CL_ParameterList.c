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
*                   Filename:   CL_ParameterList.c                            *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file AzureConnection.c
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
#include "CL_ParameterList.h"
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

static int parameterListSerialiser(CL_ParameterList *paraList, CL_Serialiser * serialiser)
{
	int i;
	int ret = 0;

	for (i = 0; i < CL_ParameterListSize(paraList); i++)
	{
		ret = CL_ParameterSerialise(&paraList->list[i], serialiser);
		if (ret)
			break;
	}

	return ret;
}


//-----------------------------------------------------------------------------
// 7) FUNCTIONS
//-----------------------------------------------------------------------------

int CL_ParameterListAddParam(CL_ParameterList *paraList, CL_Parameter *param )
{
	int i;
	int ret = -1;

	for (i = 0; i < CL_ParameterListSize(paraList); i++) {
		if (paraList->list[i].id) {
			continue;
		}
		else {
			paraList->list[i] = *param;
			ret = 0;
			break;
		}
	}
	return ret;
}

CL_Parameter * CL_ParameterListGetParamByID (CL_ParameterList *paraList, ParameterID id)
{
	int i;
	
	for (i = 0; i < CL_ParameterListSize(paraList); i++) {
		if (paraList->list[i].id == id) {
			return &paraList->list[i];
		}
	}
	return NULL;
}


CL_Parameter * CL_ParameterListGetParamByIndex(CL_ParameterList *paraList, uint16_t index)
{
	if(index < CL_ParameterListSize(paraList) && paraList->list[index].id)
	{
		return &paraList->list[index];
	}

	return NULL;
}



int CL_ParameterListSerialise(CL_ParameterList *paraList, CL_Serialiser * serialiser)
{
	int err;
	CL_SERIALISER_DO_BLOCK(serialiser, CL_SER_PARAMETER_LIST, parameterListSerialiser, paraList, err);
	return err;
}


/*! @} */ /* EOF, no more */
