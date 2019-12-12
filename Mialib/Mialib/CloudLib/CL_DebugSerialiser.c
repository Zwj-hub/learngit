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
*                   Filename:   CL_DebugSerialiser.h                          *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Christer Ekholm                               *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
*  @file CL_DebugSerialiser.h
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
#include "CL_DebugSerialiser.h"
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

static int doIndent(CL_Serialiser *ser)
{
	return CL_SerialiserIndent(ser, '\t', 1);
}

static int debugSerialiser(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * parameter)
{
	int err = 0;
	const char * param = parameter;
	uint32_t value = 0;

	if (parameter)
	{
		value = *((uint32_t*)parameter);
	}

	switch (type)
	{
	case CL_SER_MGR_START:
	{
		CL_SERIALISER_ADD_TEXT(ser, err,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		CL_SERIALISER_ADD_TEXT(ser, err,"<mgr>\n");
		break;
	}
	case CL_SER_MGR_END:
	{
		CL_SERIALISER_ADD_TEXT(ser, err, "</mgr>\n");
		CL_SERIALISER_ADD_TEXT(ser, err, "<!-- end of demo device description xml -->\n");
		break;
	}
	case CL_SER_DEVICE_START:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<device>\n");
		break;
	}

	case CL_SER_DEVICE_END:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "</device>\n");
		break;
	}

	case CL_SER_DEVICE_FW_VER:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<fw>\"%s\"</fw>\n",param);
		break;
	}
	case CL_SER_DEVICE_GUID:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<guid>\"%s\"</guid>\n", param);
		break;
	}
	case CL_SER_DEVICE_SN:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<sn>\"%s\"</sn>\n", param);
		break;
	}
	case CL_SER_DEVICE_TYPE:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<type>\"%s\"</type>\n", param);
		break;
	}

	case CL_SER_PARAMETER_LIST_START:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<list>\n");
		break;
	}
	case CL_SER_PARAMETER_LIST_END:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "</list>\n");
		break;
	}

	case CL_SER_PARAMETER_START:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<parameter>\n");
		break;
	}
	case CL_SER_PARAMETER_END:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "</parameter>\n");
		break;
	}

	case CL_SER_PARAMETER_MEAS_START:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<meas>\n");
		break;
	}

	case CL_SER_PARAMETER_MEAS_END:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "</meas>\n");
		break;
	}

	case CL_SER_PARAMETER_MEAS_TIMESTAMP:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<timestamp>%u</timestamp>\n", value);
		break;
	}
	case CL_SER_PARAMETER_MEAS_VALUE:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<value>\"%s\"</value>\n", param);
		break;
	}
	case CL_SER_PARAMETER_ID:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<id>%u</id>\n", value);
		break;
	}
	case CL_SER_PARAMETER_NAME:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<name>\"%s\"</name>\n", param);
		break;
	}
	case CL_SER_PARAMETER_TYPE:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<type>%u</type>\n", value);
		break;
	}
	default:
		;
	}

	return err;
}

static int debugSerialiserTelemetrics(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * parameter)
{
	int err = 0;
	const char * param = parameter;
	uint32_t value = 0;


	if (parameter)
	{
		value = *((uint32_t*)parameter);
	}

	switch (type)
	{
	case CL_SER_MGR_START:
	{
		CL_SERIALISER_ADD_TEXT(ser, err, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		CL_SERIALISER_ADD_TEXT(ser, err, "<mgr>\n");
		break;
	}
	case CL_SER_MGR_END:
	{
		CL_SERIALISER_ADD_TEXT(ser, err, "</mgr>\n");
		CL_SERIALISER_ADD_TEXT(ser, err, "<!-- end of demo telemetrics xml -->\n");
		break;
	}
	case CL_SER_DEVICE_START:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<device>\n");
		break;
	}

	case CL_SER_DEVICE_END:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "</device>\n");
		break;
	}

	case CL_SER_DEVICE_FW_VER:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<fw>\"%s\"</fw>\n", param);
		break;
	}
	case CL_SER_DEVICE_GUID:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<guid>\"%s\"</guid>\n", param);
		break;
	}
	case CL_SER_DEVICE_SN:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<sn>\"%s\"</sn>\n", param);
		break;
	}
	case CL_SER_DEVICE_TYPE:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<type>\"%s\"</type>\n", param);
		break;
	}

	case CL_SER_PARAMETER_LIST_START:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<list>\n");
		break;
	}
	case CL_SER_PARAMETER_LIST_END:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "</list>\n");
		break;
	}

	case CL_SER_PARAMETER_START:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<parameter>\n");
		break;
	}
	case CL_SER_PARAMETER_END:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "</parameter>\n");
		break;
	}

	case CL_SER_PARAMETER_MEAS_START:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<meas>\n");
		break;
	}

	case CL_SER_PARAMETER_MEAS_END:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "</meas>\n");
		break;
	}

	case CL_SER_PARAMETER_MEAS_TIMESTAMP:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<timestamp>%u</timestamp>\n", value);
		break;
	}
	case CL_SER_PARAMETER_MEAS_VALUE:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<value>\"%s\"</value>\n", param);
		break;
	}
	case CL_SER_PARAMETER_ID:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<id>%u</id>\n", value);
		break;
	}
	case CL_SER_PARAMETER_NAME:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<name>\"%s\"</name>\n", param);
		break;
	}
	case CL_SER_PARAMETER_TYPE:
	{
		doIndent(ser);
		CL_SERIALISER_ADD_TEXT(ser, err, "<type>%u</type>\n", value);
		break;
	}
	default:
		;
	}

	return err;
}

static int debugSerialiserSw(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * parameter)
{
	CL_DebugSerial *mode = (CL_DebugSerial*)ser->param;
	switch (mode->outType)
	{
	case CREATE_DESCRIPTION:
		return  debugSerialiser(ser, type, parameter);
	case CREATE_TELEMETRICS:
		return debugSerialiserTelemetrics(ser, type, parameter);
	default:
		return -SERIALISER_ERROR;
	}
}

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

#define USE_DEBUG_SERIALISER
#ifdef USE_DEBUG_SERIALISER

int CL_DebugSerialiserImpl(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * param)
{
	return debugSerialiserSw(ser, type, param);
}
#endif


/*! @} */ /* EOF, no more */
