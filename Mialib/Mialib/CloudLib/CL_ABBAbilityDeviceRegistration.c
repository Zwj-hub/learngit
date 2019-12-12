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
*                   Filename:   CL_ABBAbilityDeviceRegistration.c             *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Christer Ekholm                               *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
*  @file CL_ABBAbilityDeviceRegistration.h
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
#include "CL_ABBAbilitySerialiser.h"
#include "CL_Serialiser.h"
#include <stdint.h>


//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------


static const char *devRegHeader =
"{\n"
"\t\"property\": {\n"
"\t\t\"ability-messagetype\": \"platformEvent\",\n"
"\t\t\"eventType\": \"Abb.Ability.Device.Created\"\n"
"\t},\n"
"\t\"body\" : {\n"
"\t\t\"type\":\"Abb.Ability.Device.Panel\",";


static const char *devRegFooter ="\n\t}\n}";

#if 0
{
	"properties": {
		"ability-messagetype": "platformEvent",
		"eventType" : "Abb.Ability.Device.Created"
	},
	"body" : {
		"type": "Abb.Ability.Device.Panel@1.0.0",
		"properties" : {
			"deviceId": {
				"value": "IoTPanel"
			},
			"hubName" : {
				"value": "PanelIoTHub.azure-devices.net"
			},
			"variables" : [
				{
					"variable": "iccid",
					"value" : "873249822123123"
				},
				{
					"variable": "network operator",
					"value" : "Orange"
				}
			]
		}
	}
}
#endif


int ABBABbilityDeviceRegistration(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * parameter)
{
	int err = 0;
	const char * param = parameter;

	switch (type)
	{
	case CL_SER_MGR_START:
	{
		CL_SERIALISER_ADD_TEXT(ser, err, "%s\n", devRegHeader);
		CL_SERIALISER_ADD_TEXT(ser, err, "\t\t\"property\":{\n");
		break;
	}
	case CL_SER_MGR_END:
	{
		CL_SERIALISER_ADD_TEXT(ser, err, "%s\n", devRegFooter);
		break;
	}
	case CL_SER_DEVICE_START:
	{
		CL_SERIALISER_ADD_TEXT(ser, err, "\t\t\"variables\":[\n");
		break;
	}

	case CL_SER_DEVICE_END:
	{
		CL_SERIALISER_ADD_TEXT(ser, err, "\t\t]\n");
		break;
	}

	case CL_SER_HUB_DEVICE_ID:
	{
		CL_SERIALISER_ADD_TEXT(ser, err, "\t\t\t\"deviceId\":{\n");
		CL_SERIALISER_ADD_TEXT(ser, err, "\t\t\t\"value\":\"%s\"},\n", param);

		break;
	}
	
	case CL_SER_HUB_URL:
	{
		CL_SERIALISER_ADD_TEXT(ser, err, "\t\t\t\"hubName\":{\n");
		CL_SERIALISER_ADD_TEXT(ser, err, "\t\t\t\"value\":\"%s\"}\n", param);
		CL_SERIALISER_ADD_TEXT(ser, err, "\t\t},\n");

		break;
	}
	case CL_SER_DEVICE_FW_VER:
	{
		CL_SERIALISER_ADD_TEXT(ser, err, "\t\t\t{\"firmwareVersion\":\"%s\"},\n", param);
		break;
	}
	case CL_SER_DEVICE_GUID:
	{
		CL_SERIALISER_ADD_TEXT(ser, err, "\t\t\t{\"objectId\":\"%s\"},\n", param);
		break;
	}
	case CL_SER_DEVICE_SN:
	{
		CL_SERIALISER_ADD_TEXT(ser, err, "\t\t\t{\"sn\":\"%s\"},\n", param);
		break;
	}
	case CL_SER_DEVICE_TYPE:
	{	CL_SERIALISER_ADD_TEXT(ser, err, "\t\t\t{\"deviceType\":\"%s\"}\n", param);
		break;
	}

	case CL_SER_PARAMETER_LIST_START:
	{
		break;
	}
	case CL_SER_PARAMETER_LIST_END:
	{
		break;
	}
	case CL_SER_PARAMETER_START:
	{
		break;
	}
	case CL_SER_PARAMETER_END:
	{
		break;
	}

	case CL_SER_PARAMETER_MEAS_START:
	{
		break;
	}

	case CL_SER_PARAMETER_MEAS_END:
	{
		break;
	}

	case CL_SER_PARAMETER_MEAS_TIMESTAMP:
	{
		break;
	}
	case CL_SER_PARAMETER_MEAS_VALUE:
	{
		break;
	}
	case CL_SER_PARAMETER_ID:
	{
		break;
	}
	case CL_SER_PARAMETER_NAME:
	{
		break;
	}
	case CL_SER_PARAMETER_TYPE:
	{
		break;
	}
	default:
		;
	}

	return err;
}



//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

/*! @} */ /* EOF, no more */
