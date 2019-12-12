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
*                   Filename:   CL_Manager.c                                  *
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
#include "CL_Manager.h"
#include "CL_ParameterList.h"
#include "CL_Meas.h"
#include "CL_Parameter.h"
#include "CL_Utils.h"

#include <string.h>
#include "CL_Error.h"
#include "CL_String.h"
#include "CL_Serialiser.h"

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


static int MgrSerialiser(CL_Manager *mgr, CL_Serialiser * serialiser)
{
	int i;
	int ret;

	ret = CL_SERIALISER_ADD(serialiser, CL_SER_HUB_DEVICE_ID, mgr->hubDeviceID);
	if (ret) return ret;

	ret = CL_SERIALISER_ADD(serialiser, CL_SER_HUB_URL, mgr->hubURL);
	if (ret) return ret;

	for (i = 0; i < CL_ManagerGetDeviceSize(mgr); i++)
	{
		int ret = CL_DeviceSerialise(&mgr->devices[i], serialiser);
		if (ret)
			return ret;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// 7) FUNCTIONS
//-----------------------------------------------------------------------------

int CL_MgrAddMeas(CL_Manager *mgr, CL_ParameterId parameterId, void * value, CL_Timestamp timestamp)
{
	int i;
	
	for( i = 0; i < CL_ManagerGetDeviceSize(mgr); i++)
	{
		CL_Parameter *param = CL_ParameterListGetParamByID(mgr->devices[i].paraList, parameterId);

		if (param) 
		{
			CL_Meas *meas = CL_MeasBuffReserve(mgr->measBuff);

			if (meas == NULL) 
			{
				return -MEAS_BUFFER_EMPTY;
			}
			
			CL_MeasInit(meas, value, timestamp);
			
			return CL_ParameterInsertMeas(param, meas);
		}
	}

	return -PARAMETER_ID_NOT_FOUND;
}

int CL_MgrDelMeas(CL_Manager *mgr)
{
	int i, j;

	for( i = 0; i < CL_ManagerGetDeviceSize(mgr); i++) // all devices
	{
		for(j = 0; j < CL_ParameterListSize(mgr->devices[i].paraList); j++) // All parameters
		{
			CL_Parameter *param = CL_ParameterListGetParamByIndex(mgr->devices[i].paraList, j);
		
			if (param == NULL) {
				break; // all params freed
			}
			else {
				while(1) // Free all measurements from parameter
				{
					CL_Meas *meas = CL_ParameterRemoveMeas(param);
					if(!meas)
						break;
					
					CL_MeasBuffFree(mgr->measBuff, meas);
				}
			}
		}
	}

	ASSERT(!CL_MeasBuffReserved(mgr->measBuff));

	if(CL_MeasBuffReserved(mgr->measBuff)) // Robust code
	{ 
		CL_MeasBuffFreeAll(mgr->measBuff);
	}

	return 0;
}



int CL_MgrSetDevice(CL_Manager *mgr, CL_Device * device, uint8_t index)
{
	if(index < CL_ManagerGetDeviceSize(mgr))
	{
		mgr->devices[index] = *device;
		return 0;
	}
	return -BAD_DEVICE_INDEX;
}

CL_Device * CL_MgrGetDevice(CL_Manager *mgr, uint8_t index)
{
	if(index < CL_ManagerGetDeviceSize(mgr))
	{
		return &mgr->devices[index];
	}
	else
	{ 
		return NULL;
	}
}


int CL_MgrPull(CL_Manager *mgr, const char *inData)
{
	return -1;
}

int CL_MgrPush(CL_Manager *mgr, char *output, uint16_t outputlen, CL_ABBAbilitySerialiserType type)
{
	int ret;
	CREATE_CL_String(out, output, outputlen);
	CREATE_CL_Serialiser(serialiser, &out, (void *)ABBAbilityGetSerialiserHandle(type));

	CL_SERIALISER_DO_BLOCK(&serialiser, CL_SER_MGR, MgrSerialiser, mgr, ret);
	
	if (ret == 0)
	{
		return  CL_StringLen(&out) + 1; // Return len of string plus terminator
	}
	else
	{
		return ret;
	}
}

int CL_MgrMeasCount(CL_Manager *mgr, uint16_t *meas, uint16_t *free)
{
	if (meas)
	{
		*meas = CL_MeasBuffReserved(mgr->measBuff);
	}

	if (free)
	{
		*free = CL_MeasBuffGetFreeSize(mgr->measBuff);
	}
	return 0;
}


/*! @} */ /* EOF, no more */
