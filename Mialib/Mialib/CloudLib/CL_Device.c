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
*                   Filename:   CL_Device.c                                   *
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
#include "CL_Device.h"
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

static int DeviceSerialiser(CL_Device * dev, CL_Serialiser * serialiser)
{
	int ret;
	do
	{
		ret = CL_SERIALISER_ADD(serialiser, CL_SER_DEVICE_FW_VER, dev->fwVersion);
		if (ret) break;

		ret = CL_SERIALISER_ADD(serialiser, CL_SER_DEVICE_GUID, dev->guid);
		if (ret) break;

		ret = CL_SERIALISER_ADD(serialiser, CL_SER_DEVICE_SN, dev->sn);
		if (ret) break;

		ret = CL_SERIALISER_ADD(serialiser, CL_SER_DEVICE_TYPE, dev->type);
		if (ret) break;

		ret = CL_ParameterListSerialise(dev->paraList, serialiser);
		if (ret) break;

	} while (0);

	return ret;
}

//-----------------------------------------------------------------------------
// 7) GLOBAL FUNCTIONS
//-----------------------------------------------------------------------------

int CL_DeviceSetGUID(CL_Device *dev, const char *guid)
{
	if(!dev)
	{
		return -DEVICE_NOT_INITIALISED;
	}

	if (!guid)
	{
		guid = "";
	}

	if (strlen(guid) < MAX_GUID_SIZE) {
		strcpy(dev->guid, guid);
		return 0;
	}
	return -INTERNAL_BUFFER_FULL;
}

int CL_DeviceSetSN(CL_Device *dev, const char *sn)
{
	if (!sn)
	{
		sn = "";
	}

	if (strlen(sn) < MAX_SN_SIZE) {
		 strcpy(dev->sn, sn);
		 return 0;
	}
	return -INTERNAL_BUFFER_FULL;
}

int CL_DeviceSetType(CL_Device *dev, const char *type)
{
	if (!type)
	{
		type = "";
	}

	if (strlen(type) < MAX_DEVICETYPE_SIZE) {
		strcpy(dev->type, type);
		return 0;
	}
	return -INTERNAL_BUFFER_FULL;
}

int CL_DeviceSetFWVersion(CL_Device *dev, const char *fwVersion)
{
	if(!fwVersion)
	{
		fwVersion = "";
	}
	if (strlen(fwVersion) < MAX_FWVER_SIZE) {
		strcpy(dev->fwVersion, fwVersion);
		return 0;
	}
	return -INTERNAL_BUFFER_FULL;
}

int CL_DeviceSetParameters(CL_Device *dev, CL_ParameterList *paraList)
{
	dev->paraList = paraList;
	return 0;
}

int CL_DeviceInit(CL_Device * dev, const char *guid, const char *sn, const char *fwVersion, const char *type)
{
	int ret;

	do {
		ret = CL_DeviceSetGUID(dev, guid);
		if (ret)
			break;
		
		ret = CL_DeviceSetSN(dev, sn);
		if (ret)
			break;

		ret = CL_DeviceSetType(dev, type);
		if (ret)
			break;

		ret = CL_DeviceSetFWVersion(dev, fwVersion);
		if (ret)
			break;

	} while (0);

	return ret;
}

int CL_DeviceSerialise(CL_Device * dev, CL_Serialiser * serialiser)
{
	int err = 0;
	if(dev && dev->paraList)
	{
		CL_SERIALISER_DO_BLOCK(serialiser, CL_SER_DEVICE, DeviceSerialiser, dev, err);
	}
	return err;
}


/*! @} */ /* EOF, no more */
