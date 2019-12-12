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
*                   Filename:   CL_ABBAbilityTelemetry.c                      *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Christer Ekholm                               *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
*  @file CL_ABBAbilityTelemetry.c
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
#include <time.h>       /* time_t, struct tm, time, localtime, strftime */
#include <string.h>

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------

static const char *teleheader =
"[\n";


#if 0
{
	"objectId": "2e57962f-2b6c-43c0-8fd8-c0d50d140011",
		"model" : "abb.ability.device",
		"timestamp" : "2018-05-01T15:12:19.474Z",
		"variable" : "kurtosis.x",
		"value" : 1.2
},
		{
			"objectId": "c0b4336b-fa23-4a7f-b92b-d971d8b041cd",
			"model" : "abb.ability.device",
			"timestamp" : "2018-05-01T15:12:19.476Z",
			"variable" : "kurtosis.y",
			"value" : 2.5,
			"quality" : 0
		}
#endif

static const char *telefooter = "]\n";
static ParserVar v;

#if 0
		{
			"objectId": "2e57962f-2b6c-43c0-8fd8-c0d50d140011",
				"model" : "abb.ability.device",
				"timestamp" : "2018-05-01T15:12:19.474Z",
				"variable" : "kurtosis.x",
				"value" : 1.2
		},
#endif


static void init_parser_v() 
{
	memset(&v, 0, sizeof(ParserVar));
	v.addMeasSemicolon = 0;
	v.telemetricsHeaderDone = 0;
}


//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------
int ABBAbilityTelemetry(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * parameter)
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
		init_parser_v();
		break;
	}
	case CL_SER_MGR_END:
	{
		if (v.telemetricsHeaderDone)
		{
			CL_SERIALISER_ADD_TEXT(ser, err, "%s", telefooter);
		}
		break;
	}
	case CL_SER_DEVICE_START:
	{
		break;
	}

	case CL_SER_DEVICE_END:
	{
		break;
	}

	case CL_SER_DEVICE_FW_VER:
	{
		v.deviceD.fw = &param[0];
		break;
	}
	case CL_SER_DEVICE_GUID:
	{
		v.deviceD.guid = &param[0];
		break;
	}
	case CL_SER_DEVICE_SN:
	{
		v.deviceD.sn = &param[0];
		break;
	}
	case CL_SER_DEVICE_TYPE:
	{
		v.deviceD.type = &param[0];
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
		if (!v.telemetricsHeaderDone) {
			CL_SERIALISER_ADD_TEXT(ser, err, "%s", teleheader);
			v.telemetricsHeaderDone = 1;
		}
		if (v.addMeasSemicolon)
		{
			CL_SERIALISER_ADD_TEXT(ser, err, ",\n");
		}
		if (v.deviceD.guid != NULL) {
			CL_SERIALISER_ADD_TEXT(ser, err, "\t\t{\n\t\t\t\"objectId\": \"%s\",\n", v.deviceD.guid);
		}
		if (v.deviceD.type != NULL) {
			CL_SERIALISER_ADD_TEXT(ser, err, "\t\t\t\"model\" : \"%s\",\n", "abb.ability.device");
		}
		if (v.parameterD.name != NULL) {
			char buffer[64] = "";
			int i = 0;
			while(i < (sizeof(buffer) - 1) && v.parameterD.name[i] && v.parameterD.name[i] != '\"')
			{
				buffer[i] = v.parameterD.name[i];
				i++;
			}
			buffer[i] = 0;

			CL_SERIALISER_ADD_TEXT(ser, err, "\t\t\t\"variable\" : \"%s\",\n", buffer);
		}

		break;
	}

	case CL_SER_PARAMETER_MEAS_END:
	{
		CL_SERIALISER_ADD_TEXT(ser, err, "\t\t}");
		v.addMeasSemicolon = 1;
		break;
	}

	case CL_SER_PARAMETER_MEAS_TIMESTAMP:
	{
		time_t rawtime = value;
		struct tm *timeinfo = localtime(&rawtime);
		char buffer[180];

		strftime(buffer, sizeof(buffer), "%FT%T%z", timeinfo);

		CL_SERIALISER_ADD_TEXT(ser, err, "\t\t\t\"timestamp\" : \"%s\",\n", buffer);


		break;
	}
	case CL_SER_PARAMETER_MEAS_VALUE:
	{
		// Parse unit
		int i = 0;

		CL_SERIALISER_ADD_TEXT(ser, err, "\t\t\t\"value\" : %s", param);

		while(v.parameterD.name[i] && v.parameterD.name[i] != '\"') // Skip  parameter name
		{ 
			i++;
		}; 

		if(v.parameterD.name[i]) // if unit available
		{
			int j = 0;
			char buffer[64] = "";

			i++; // Skip separator \"
			while(j < (sizeof(buffer) - 1) && v.parameterD.name[i + j])
			{
				buffer[j] = v.parameterD.name[i + j];
				j++;
			}
			buffer[j] = 0;
			CL_SERIALISER_ADD_TEXT(ser, err, ",\n\t\t\t\"unit\" : \"%s\"\n", buffer);
		}
		else 
		{
			CL_SERIALISER_ADD_TEXT(ser, err, "\n");
		}

		

		break;
	}
	case CL_SER_PARAMETER_ID:
	{
		break;
	}
	case CL_SER_PARAMETER_NAME:
	{
		v.parameterD.name = &param[0];
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


/*! @} */ /* EOF, no more */
