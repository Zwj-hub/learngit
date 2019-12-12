/*! @addtogroup ABBA Lib
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
*                  Subsystem:   ABBA Lib                                      *
*                                                                             *
*                   Filename:   ABBA_Configuration.c                          *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Christer Ekholm                               *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
*  @file ABBA_Configuration.c
*  @par File description:
*    Parse ABBA server configurations
*
*  @par Related documents:
*
*  @note
*/
/******************************************************************************/

#include "ABBA_Configuration.h"
#include "parson.h"

#define PARAM_CB(me, a, b) do{if((me)->cb.parameter){(me)->cb.parameter((me), (a), (b));}}while(0)

static int CreateRoot(IOTHUB_MESSAGE_HANDLE in, JSON_Value ** value, JSON_Object ** object )
{
	IOTHUBMESSAGE_CONTENT_TYPE content_type = IoTHubMessage_GetContentType(in);

	if (content_type == IOTHUBMESSAGE_BYTEARRAY)
	{
		const unsigned char *msg;
		size_t size;
		if (IoTHubMessage_GetByteArray(in, &msg, &size) != IOTHUB_MESSAGE_OK)
		{
			return -ABBA_E_SERVER_ERROR;
		}
		*value  =  json_parse_string((const char *)msg);
	}
	else
	{
		const char *msg = IoTHubMessage_GetString(in);
		if (msg == NULL)
		{
			return -ABBA_E_SERVER_ERROR;
		}
		*value  =  json_parse_string(msg);
	}

	*object =  json_value_get_object(*value);
	if(!*value || !*object)
	{
		json_object_clear(*object);
		json_value_free(*value);
		return -ABBA_E_SERVER_ERROR;
	}

	return 0;
}
#define MAX_CYCLES 6
#define MAX_TIME_BUFFER_SIZE 31
#define DEFINE_FORMAT_STRING_LENGTH(len) #len
#define DEFINE_DEFINE_FORMAT_STRING_LENGTH(len) DEFINE_FORMAT_STRING_LENGTH(len)

static int convertTime(const char *value)
{
	int i;
	int number;
	char buffer[MAX_TIME_BUFFER_SIZE+1];
	const char   units[] = { 's', 'm',   'h',      'd' };
	const int    multi[] = {   1,  60,  3600, (24*3600) };

	if(strlen(value) >= sizeof(buffer))
	{
		return -1;
	}

	if (sscanf(value, "%d%" DEFINE_DEFINE_FORMAT_STRING_LENGTH(MAX_TIME_BUFFER_SIZE) "s", &number, buffer) != 2)
	{
		return -1;
	}

	for( i = 0; i < sizeof(units) / sizeof(units[0]); i++ )
	{
		if(units[i] == buffer[0])
			return number * multi[i];
	}

	return -1;
}

static int parseParemeterCycle(ABBA_Data * This, const JSON_Object * cycle, const char * cycleType)
{
	int i;
	ABBA_Data_CB_Parameter_Data data;

	const JSON_Array * values   = json_object_dotget_array(cycle,   "value");
	const char *       interval = json_object_dotget_string(cycle,  "interval");
	int                change   = json_object_dotget_boolean(cycle, "onlyOnChange");
	int                elements = json_array_get_count(values);

	if(!values || !interval || change < 0 || elements < 1)
	{
		return -1;
	}

	data.cycleType = cycleType;
	data.parameterName = "";
	data.interval = convertTime(interval);
	data.onlyOnChange = change;

	for(i = 0; i < elements; i++)
	{
		data.parameterName = json_array_get_string(values, i);
		if(!data.parameterName || !data.parameterName[0])
		{
			return -1;
		}

		PARAM_CB(This, ABBA_PAR_ADD, &data);
	}

	return 0;
}

static int parseParemeterConf(ABBA_Data * This, JSON_Object * root)
{
	int i;
	const char *monitorType;

	monitorType = json_object_dotget_string(root, "properties.monitoring.value");

	if (!monitorType) 	return -ABBA_E_MISSING_CONFIGURATION;

	for(i = 1; i < MAX_CYCLES; i++)
	{
		char jsonDot[64];
		snprintf(jsonDot, sizeof(jsonDot), "properties.schedules.%s.cycle_%d", monitorType, i);
		const JSON_Object * cycle = json_object_dotget_object(root, jsonDot);

		if(!cycle)
		{
			if(i == 1)
			{
				return -ABBA_E_MISSING_CONFIGURATION;
			}
			else
			{
				break;
			}
		}

		if(i == 1)
		{
			PARAM_CB(This, ABBA_PAR_FIRST, NULL);
		}

		int ret = parseParemeterCycle(This, cycle, monitorType);

		if(ret)
		{
			break;
		}
	}

	PARAM_CB(This, ABBA_PAR_LAST, NULL);

	return ABBA_E_OK;
}

int ABBA_ParseConf(ABBA_Data * This, IOTHUB_MESSAGE_HANDLE in)
{
	JSON_Value   *value;
	JSON_Object  *object;
	int ret = CreateRoot(in, &value, &object);

	if(ret)
	{
		return ret;
	}

	ret = parseParemeterConf(This, object);

	json_object_clear(object);
	json_value_free(value);

	return ret;
}

#if 0

{
	"ability-messagetype":"platformEvent",
		"eventType" : "Abb.Ability.InformationModel.ObjectModelCreated",
		"model" : "abb.ability.configuration",
		"type" : "abb.drives.acs880.inu.4%401",
		"objectId" : "9353cbb9-fdf1-45cf-a97b-277cc4d699c3"
}

properties.schedules.advanced.cycle_1.value =
["01_01",
"01_10",
"01_15",
"05_11",
"05_13",
"06_11"
]
properties.schedules.advanced.cycle_1.interval = "3s",
properties.schedules.advanced.cycle_1.onlyOnChange = trye


"advanced":{
	"cycle_1":{
		"dataType":"array",
			"value" : [
				"01_01",
					"01_10",
					"01_15",
					"05_11",
					"05_13",
					"06_11"
			],
			"interval":"3s",
					"onlyOnChange" : true
	},
		"cycle_2":{
					"dataType":"array",
						"value" : [
							"01_07",
								"01_11",
								"01_31",
								"05_111",
								"06_16"
						],
						"interval":"1m",
								"onlyOnChange" : true
				},
					"cycle_3":{
								"dataType":"array",
									"value" : [
										"01_35",
											"01_36",
											"01_37",
											"05_41",
											"05_42",
											"05_121",
											"30_11",
											"30_12",
											"30_13",
											"30_14",
											"30_17",
											"30_26",
											"30_27",
											"96_01"
									],
									"interval":"1h",
											"onlyOnChange" : true
							},
								"cycle_4":{
											"dataType":"array",
												"value" : [
													"05_01",
														"05_02",
														"45_01",
														"45_02",
														"45_03",
														"99_03",
														"99_04",
														"99_06",
														"99_07",
														"99_08",
														"99_09",
														"99_10",
														"99_12"
												],
												"interval":"1d",
														"onlyOnChange" : true
										}
},

{
	"type":"abb.drives.acs880.inu@1",
	"objectId" : "9353cbb9-fdf1-45cf-a97b-277cc4d699c3",
	"version" : 1,
	"model" : "abb.ability.configuration",
	"properties" : {
		"serialNumberDrive":{
			"value":"Drive-5566677778888"
		},
		"cooling" : {
			"mandatory":true,
			"dataType" : "string",
			"enum" : [
			"air",
			"water"
			],
			"value" : "air"
		},
		"driveCategory":{
			"mandatory":true,
			"dataType" : "string",
			"enum" : [
			"LV",
			"MV"
			],
			"value" : "LV"
		},
		"packageType":{
			"mandatory":true,
			"dataType" : "string",
			"enum" : [
			"INU",
			"ISU"
			],
			"value" : "INU"
		},
		"schedules":{
			"basic":{
			"cycle_1":{
				"dataType":"array",
				"value" : [
					"01_01",
					"01_10",
					"01_15",
					"06_11"
				],
				"interval":"10s",
				"onlyOnChange" : true
			},
			"cycle_2":{
				"dataType":"array",
				"value" : [
					"01_07",
					"01_11",
					"01_31",
					"05_111",
					"06_16"
				],
				"interval":"1m",
				"onlyOnChange" : true
			},
			"cycle_3":{
				"dataType":"array",
				"value" : [
					"01_35",
					"01_36",
					"01_37",
					"05_41",
					"05_42",
					"05_121",
					"30_11",
					"30_12",
					"30_13",
					"30_14",
					"30_17",
					"30_26",
					"30_27",
					"96_01"
				],
				"interval":"1h",
				"onlyOnChange" : true
			},
			"cycle_4":{
				"dataType":"array",
				"value" : [
					"05_01",
					"05_02",
					"45_01",
					"45_02",
					"45_03",
					"99_03",
					"99_04",
					"99_06",
					"99_07",
					"99_08",
					"99_09",
					"99_10",
					"99_12"
				],
				"interval":"1d",
				"onlyOnChange" : true
			}
			},
				advanced:{
			"cycle_1":{
				"dataType":"array",
				"value" : [
					"01_01",
					"01_10",
					"01_15",
					"05_11",
					"05_13",
					"06_11"
				],
				"interval":"3s",
				"onlyOnChange" : true
			},
			"cycle_2":{
				"dataType":"array",
				"value" : [
					"01_07",
					"01_11",
					"01_31",
					"05_111",
					"06_16"
				],
				"interval":"1m",
				"onlyOnChange" : true
			},
			"cycle_3":{
				"dataType":"array",
				"value" : [
					"01_35",
					"01_36",
					"01_37",
					"05_41",
					"05_42",
					"05_121",
					"30_11",
					"30_12",
					"30_13",
					"30_14",
					"30_17",
					"30_26",
					"30_27",
					"96_01"
				],
				"interval":"1h",
				"onlyOnChange" : true
			},
			"cycle_4":{
				"dataType":"array",
				"value" : [
					"05_01",
					"05_02",
					"45_01",
					"45_02",
					"45_03",
					"99_03",
					"99_04",
					"99_06",
					"99_07",
					"99_08",
					"99_09",
					"99_10",
					"99_12"
				],
				"interval":"1d",
				"onlyOnChange" : true
			}
			},
			"full":{
			"cycle_1":{
				"dataType":"array",
				"value" : [
					"01_01",
					"01_10",
					"01_15",
					"05_11",
					"05_13",
					"06_11"
				],
				"interval":"1s",
				"onlyOnChange" : false
			},
			"cycle_2":{
				"dataType":"array",
				"value" : [
					"01_02",
					"01_06",
					"01_07",
					"01_11",
					"01_13",
					"01_14",
					"01_24",
					"01_31",
					"05_12",
					"05_14",
					"05_15",
					"05_16",
					"05_17",
					"05_111",
					"06_01",
					"06_02",
					"06_03",
					"06_04",
					"06_05",
					"06_16",
					"06_17",
					"06_18",
					"06_19",
					"06_21",
					"06_25",
					"10_01",
					"10_21",
					"11_01",
					"12_11",
					"12_21",
					"30_01",
					"30_02"
				],
				"interval":"1m",
				"onlyOnChange" : false
			},
			"cycle_3":{
				"dataType":"array",
				"value" : [
					"01_35",
					"01_36",
					"01_37",
					"05_30",
					"05_31",
					"05_41",
					"05_42",
					"05_121",
					"30_11",
					"30_12",
					"30_13",
					"30_14",
					"30_17",
					"30_26",
					"30_27",
					"96_01"
				],
				"interval":"1h",
				"onlyOnChange" : false
			},
			"cycle_4":{
				"dataType":"array",
				"value" : [
					"05_01",
					"05_02",
					"05_04",
					"45_01",
					"45_02",
					"45_03",
					"99_03",
					"99_04",
					"99_06",
					"99_07",
					"99_08",
					"99_09",
					"99_10",
					"99_12"
				],
				"interval":"1d",
				"onlyOnChange" : false
			}
			}
		},
		"constants":{
			"constant_1":{
			"dataType":"number",
			"symbolicName" : "max_torque_limit",
			"value" : 300,
			"unit" : "%"
			},
			"constant_2":{
			"dataType":"number",
			"symbolicName" : "min_torque_limit",
			"value" : -300,
			"unit" : "%"
			}
		},
		"algorithms":{
			"algorithm_1":{
			"dataType":"string",
			"value" : "InuChargingCounter"
			},
			"algorithm_2" : {
			"dataType":"string",
			"value" : "InuHistogramTorque"
			},
			"algorithm_3" : {
			"dataType":"string",
			"value" : "InuHistogramSpeed"
			},
			"algorithm_4" : {
			"dataType":"string",
			"value" : "InuHistogramPower"
			},
			"algorithm_5" : {
			"dataType":"string",
			"value" : "InuHistogramNmTorque"
			},
			"algorithm_6" : {
			"dataType":"string",
			"value" : "InuIndexLVStress"
			},
			"algorithm_7" : {
			"dataType":"string",
			"value" : "InuEnergyConsumption"
			},
			"algorithm_8" : {
			"dataType":"string",
			"value" : "InuAvailability"
			},
			"algorithm_9" : {
			"dataType":"string",
			"value" : "InuSignalSummary"
			},
			"algorithm_10" : {
			"dataType":"string",
			"value" : "InuScatterTorqueSpeed"
			},
			"algorithm_11" : {
			"dataType":"string",
			"value" : "InuScatterPowerSpeed"
			},
			"algorithm_12" : {
			"dataType":"string",
			"value" : "InuSaveFaultResetTime"
			},
			"algorithm_13" : {
			"dataType":"string",
			"value" : "InuCavitySpeed"
			},
			"algorithm_14" : {
			"dataType":"string",
			"value" : "InuCavityTorque"
			},
			"algorithm_15" : {
			"dataType":"string",
			"value" : "InuIndexTemperature"
			}
		},
		"trends":{
			"trend_1":{
			"dataType":"string",
			"value" : "Power",
			"unit" : "kW"
			},
			"trend_2" : {
			"dataType":"string",
			"value" : "Torque",
			"unit" : "Nm"
			},
			"trend_3" : {
			"dataType":"string",
			"value" : "Speed",
			"unit" : "Rpm"
			},
			"trend_4" : {
			"dataType":"string",
			"value" : "Temperature",
			"unit" : "C"
			},
			"trend_5" : {
			"dataType":"string",
			"value" : "Current",
			"unit" : "A"
			},
			"trend_6" : {
			"dataType":"string",
			"value" : "EnergyConsumption",
			"unit" : "kWh"
			},
			"trend_7" : {
			"dataType":"string",
			"value" : "Pumpcavitationspeed",
			"unit" : "Rpm"
			},
			"trend_8" : {
			"dataType":"string",
			"value" : "Pumpcavitationtorque",
			"unit" : "Nm"
			},
			"trend_9" : {
			"dataType":"string",
			"value" : "extambtemperature",
			"unit" : "C"
			},
			"trend_10" : {
			"dataType":"string",
			"value" : "extambpressure",
			"unit" : "mbar"
			},
			"trend_11" : {
			"dataType":"string",
			"value" : "extambhumidity",
			"unit" : "% RH"
			}
		},
		"scatters":{
			"scatter_1":{
			"dataType":"string",
			"value" : "torque,speed",
			"unit" : "Nm|Rpm"
			},
			"scatter_2" : {
			"dataType":"string",
			"value" : "power,speed",
			"unit" : "kW|Rpm"
			},
			"scatter_3" : {
			"dataType":"string",
			"value" : "igbt,power",
			"unit" : "%|kW"
			}
		},
		"histograms":{
			"histogram_1":{
			"dataType":"string",
			"value" : "Speed",
			"unit" : "Rpm"
			},
			"histogram_2" : {
			"dataType":"string",
			"value" : "power",
			"unit" : "kW"
			},
			"histogram_3" : {
			"dataType":"string",
			"value" : "temperature",
			"unit" : "C"
			},
			"histogram_4" : {
			"dataType":"string",
			"value" : "torque",
			"unit" : "Nm"
			}
		},
		"serviceCounters":{
			"serviceCounter_1":{
			"dataType":"string",
			"value" : "fanagingindicator",
			"unit" : "%"
			}
		},
		"thermalLimits":{
			"yellow":{
			"threshold":{
				"mandatory":true,
				"dataType" : "number",
				"value" : 50
			},
			"mean" : {
				"mandatory":true,
				"dataType" : "number",
				"value" : 32.5
			},
			"stdDev" : {
				"mandatory":true,
				"dataType" : "number",
				"value" : 4
			}
			},
			"red":{
			"threshold":{
				"mandatory":true,
				"dataType" : "number",
				"value" : 50
			},
			"mean" : {
				"mandatory":true,
				"dataType" : "number",
				"value" : 50
			},
			"stdDev" : {
				"mandatory":true,
				"dataType" : "number",
				"value" : 4
			}
			}
		}
	}
}
#endif
