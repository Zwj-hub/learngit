/*! @addtogroup ABBA_Lib
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
 *                  Subsystem:   ABBALib                                       *
 *                                                                             *
 *                   Filename:   ABBA_Unittest.c                               *
 *                       Date:                                                 *
 *                                                                             *
 *                 Programmer:   Christer Ekholm                               *
 *                     Target:   All where is lot of stack                     *
 *                                                                             *
 *******************************************************************************/
 /*!
  *  @file ABBA_Unittest.c
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
#include "ABBALib.h"

#include "azure_c_shared_utility/map.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
//-----------------------------------------------------------------------------
// 2) LOCAL ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 3) LOCAL DATA STRUCTURES
//-----------------------------------------------------------------------------

typedef struct UnitTest
{
	int(*func)(void*);
	void * parama;
	const char * name;
}UnitTest;



#ifndef CONSOLE_PRINTER
#define CONSOLE_PRINTER printf
#endif

#define EXPECT_MATCH(a,b) do{ if((a) != (b)){ CONSOLE_PRINTER("Unit test failed. " #a " == " #b ". Result %d != %d at %s:%d.\n", (int)(a), (int)(b), __FILE__, __LINE__); return -__LINE__; } } while(0)

#define EXPECT_POSITIVE(a) do{ if(a < 1){ CONSOLE_PRINTER("Unit test failed. " #a " > 0. Result %d <= 0 at %s:%d.\n", (int)(a), __FILE__, __LINE__); return -__LINE__; } } while(0)

#define EXPECT_ZERO(a) do{ if(a){ CONSOLE_PRINTER("Unit test failed. " #a " == 0. Result %d != 0 at %s:%d.\n", (int)(a), __FILE__, __LINE__); return -__LINE__; } } while(0)

#define EXPECT_NON_ZERO(a) do{ if(!(a)){ CONSOLE_PRINTER("Unit test failed. " #a " != 0. Result %d == 0 at %s:%d.\n", (int)(a), __FILE__, __LINE__); return -__LINE__; } } while(0)


//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLES
//-----------------------------------------------------------------------------
static int failed = 0;
static int success = 0;


//-----------------------------------------------------------------------------
// 5) GLOBAL VARIABLES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 6) LOCAL FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 7) FUNCTIONS
//-----------------------------------------------------------------------------

#define GW_SERIAL_NUMBER "gw123"
#define DRIVE_SERIAL_NUMBER "dr123"
#define GW_IOT_HUB_ID "gwHubID123"
#define GW_IOT_HUB_NAME "gwHubNameAzureChina"
#define GW_FWVER "GPAPR_v5.90.200.35"
#define DRIVE_FWVER "AINF7 v2.62"
#define DRIVE_DEVTYPE "ACS880"
#define DRIVE_DEVMODE "ACS880"

#define REGISTER_INIT_CMD()  \
	ABBA_RegisterSet(&abbLib, \
		GW_SERIAL_NUMBER,      \
		1,                      \
		drives,                  \
		GW_IOT_HUB_ID,            \
		GW_IOT_HUB_NAME,           \
		GW_FWVER,                   \
		DRIVE_FWVER,                 \
		DRIVE_DEVTYPE,                \
		DRIVE_DEVMODE                  \
	)

static int testAbbaGwRegister(void * arg)
{
	(void)arg;
	ABBA_Data abbLib;

	const char *drives[] = { DRIVE_SERIAL_NUMBER };

	IOTHUB_MESSAGE_HANDLE out = NULL;
	IOTHUB_MESSAGE_HANDLE in = NULL;
	const unsigned char *msg;
	size_t size;
	MAP_HANDLE mHandle;

	int ret = REGISTER_INIT_CMD();

	abbLib.registerState = ABBA_INFO_MODEL_GW_REGISTERING ;

	ret = ABBA_Register_StateMachine(&abbLib, &out, NULL);

	EXPECT_MATCH(ret, 1);

	EXPECT_MATCH(IoTHubMessage_GetByteArray(out, &msg, &size), IOTHUB_MESSAGE_OK);
	//gw register msg
	const char cmp[] = 
		"{\n"
		"\"type\": \"abb.drives.iotpanel@1\",\n"
		"\"properties\": {\n"
		"\t\"deviceId\":{ \"value\": \"gwHubID123\" },\n"
		"\t\"hubName\":{ \"value\": \"gwHubNameAzureChina\" },\n"
		"\t\"serialNumberIotPanel\": { \"value\": \"gw123\" },\n"
		"\t\"firmwareVersion\": { \"value\": \"GPAPR_v5.90.200.35\" },\n"
		"\t\"simIccid\": { \"value\": \"\" },\n"
		"\t\"networkOperator\": { \"value\": \"\" },\n"
		"\t\"signalStrength\": { \"value\": \"No Signal\" }\n"
		"}\n"
		"}\n"
		"";
	EXPECT_ZERO(strcmp((const char *)msg, cmp));

	abbLib.registerState = ABBA_INFO_MODEL_GW_REGISTERED;

	const char regRecvMsg[] = 
		"{\n"
		"\"objectId\":\"f8f0821f-ff74-4943-800a-7ce0e0d405f7\",\n"
		"\"type\" : \"abb.drives.iotpanel@1\", \n"
		"\"properties\" : \n"
		"{\"serialNumberIotPanel\":{\"value\":\"gw123\"}}, \n"
		"\"version\" : 1}";

	in = IoTHubMessage_CreateFromByteArray((const unsigned char *)regRecvMsg, strlen(regRecvMsg));

	mHandle = IoTHubMessage_Properties(in);

	Map_Add(mHandle, "ability-messagetype", "platformEvent");
	Map_Add(mHandle, "eventType", "Abb.Ability.InformationModel.ObjectModelCreated");
	Map_Add(mHandle, "objectId", "f8f0821f-ff74-4943-800a-7ce0e0d405f7");
	Map_Add(mHandle, "model", "abb.ability.device");

	ret = ABBA_Register_StateMachine(&abbLib, NULL, &in);  // receive gw registered

	EXPECT_ZERO(ret);
	EXPECT_ZERO(strcmp(abbLib.ABB_GW_Id, "f8f0821f-ff74-4943-800a-7ce0e0d405f7"));

	//drive reg msg
	abbLib.registerState = ABBA_INFO_MODEL_GW_CONFIGURED;

	const char cfgRecvMsg[] =
	"{\n"
		"\"type\":\"abb.drives.iotpanel@1\",\n"
		"\"objectId\":\"f8f0821f-ff74-4943-800a-7ce0e0d405f7\",\n"
		"\"version\":1,\n"
		"\"model\":\"abb.ability.configuration\",\n"
		"\"properties\":\n"
		"{\n"
			"\"serialNumberIotPanel\":{\"value\":\"gw123\"},\n"
			"\"firmwareVersion\":{\"mandatory\":true,\"dataType\":\"string\"},\n"
			"\"simIccid\":{\"mandatory\":true,\"dataType\":\"string\"},\n"
			"\"networkOperator\":{\"mandatory\":true,\"dataType\":\"string\"},\n"
			"\"driveTypes\":\n"
			"{\n"
				"\"driveType_1\":\n"
				"{\n"
					"\"description\":\"ACS580\",\n"
					"\"mandatory\":true,\"dataType\":\"string\",\n"
					"\"value\":\"abb.drives.acs580.4@1\",\"firmware\":\"ASCD*\",\"supported\":true\n"
				"},\n"
				"\"driveType_2\":\n"
				"{\n"
					"\"description\":\"ACS880 single drive or INU\",\"mandatory\":true,\"dataType\":\"string\",\n"
					"\"value\":\"abb.drives.acs880.inu@1\",\"firmware\":\"AINF*\",\"supported\":true\n"
				"}\n"
			"}\n"
		"}\n"
	"}"
	;

	in = IoTHubMessage_CreateFromByteArray((const unsigned char *)cfgRecvMsg, strlen(cfgRecvMsg));

	mHandle = IoTHubMessage_Properties(in);

	Map_Add(mHandle, "ability-messagetype", "platformEvent");
	Map_Add(mHandle, "eventType", "Abb.Ability.InformationModel.ObjectModelCreated");
	Map_Add(mHandle, "objectId", "f8f0821f-ff74-4943-800a-7ce0e0d405f7");
	Map_Add(mHandle, "model", "abb.ability.configuration");

	ret = ABBA_Register_StateMachine(&abbLib, NULL, &in);  // receive gw configured

	EXPECT_ZERO(ret);
	EXPECT_ZERO(strcmp(abbLib.ABB_GW_Id, "f8f0821f-ff74-4943-800a-7ce0e0d405f7"));
	
	return 0;
}

static int testAbbaDrvRegister(void * arg)
{
	(void)arg;
	ABBA_Data abbLib;

	const char *drives[] = { DRIVE_SERIAL_NUMBER };

	IOTHUB_MESSAGE_HANDLE out = NULL;
	IOTHUB_MESSAGE_HANDLE in = NULL;
	const unsigned char *msg;
	size_t size;
	MAP_HANDLE mHandle;

	int ret = REGISTER_INIT_CMD();
	
	abbLib.registerState = ABBA_INFO_MODEL_DRIVE_REGISTERING;
	strncpy(abbLib.ABB_GW_Id, "f8f0821f-ff74-4943-800a-7ce0e0d405f7", sizeof(abbLib.ABB_GW_Id));

	ret = ABBA_Register_StateMachine(&abbLib, &out, NULL);

	EXPECT_MATCH(ret, 1);

	const char cmp[] = 
		"{\n"
		"\"type\": \"abb.drives.iotpanel@1\",\n"
		"\"objectId\": \"f8f0821f-ff74-4943-800a-7ce0e0d405f7\",\n"
		"\"model\": \"abb.ability.device\",\n"
		"\"properties\": {\n"
		"\t\"deviceId\":{ \"value\": \"gwHubID123\" },\n"
		"\t\"hubName\":{ \"value\": \"gwHubNameAzureChina\" },\n"
		"\t\"serialNumberIotPanel\": { \"value\": \"gw123\" }\n"
		"},\n"
		"\"references\": {\n"
		"\t\t\"Drive\": [\n"
		"\t\t{\n"
		"\t\t\t\"model\": \"abb.ability.device\",\n"
		"\t\t\t\"type\" : \"abb.drives.acs880.inu@1\",\n"
		"\t\t\t\"properties\" : {\n"
		"\t\t\t\t\"serialNumberDrive\" : {\"value\": \"dr123\"},\n"
		"\t\t\t\t\"firmwareVersion\" : {\"value\": \"AINF7 v2.62\"},\n"
		"\t\t\t\t\"deviceType\" : {\"value\": \"ACS880\"},\n"
		"\t\t\t\t\"deviceVersion\" : {\"value\": \"ACS880\"}\n"
		"\t\t\t},\n"
		"\t\t\t\"createIfMissing\": true\n"
		"\t\t}\n"
		"\t]\n"
		"}\n"
		"\n"
		"}\n"
		"";

	EXPECT_MATCH(IoTHubMessage_GetByteArray(out, &msg, &size), IOTHUB_MESSAGE_OK);
	EXPECT_ZERO(strcmp((const char *)msg, cmp));

	abbLib.registerState = ABBA_INFO_MODEL_DRIVE_REGISTERED;

	const char regRecvMsg[] =
		"{\"objectId\":\"9353cbb9-fdf1-45cf-a97b-277cc4d699c3\",\n"
		"\"type\":\"abb.drives.acs880.inu@1\",\n"
		"\"properties\":{\"serialNumberDrive\":{\"value\":\"dr123\"}},\"version\":1}";

	in = IoTHubMessage_CreateFromByteArray((const unsigned char *)regRecvMsg, strlen(regRecvMsg));

	mHandle = IoTHubMessage_Properties(in);

	Map_Add(mHandle, "ability-messagetype", "platformEvent");
	Map_Add(mHandle, "eventType", "Abb.Ability.InformationModel.ObjectModelCreated");
	Map_Add(mHandle, "objectId", "9353cbb9-fdf1-45cf-a97b-277cc4d699c3");
	Map_Add(mHandle, "model", "abb.ability.device");
	Map_Add(mHandle, "type", ABBA_VAL_DRV_ACS_880_TYPE);

	ret = ABBA_Register_StateMachine(&abbLib, NULL, &in);  // receive gw registered

	EXPECT_ZERO(ret);
	EXPECT_ZERO(strcmp(abbLib.ABB_Drive_Id[0], "9353cbb9-fdf1-45cf-a97b-277cc4d699c3"));
	
	//drive reg msg
	abbLib.registerState = ABBA_INFO_MODEL_GW_CONFIGURED;
	const char cfgRecvMsg[] = 
	"{\n"
		"\"type\":\"abb.drives.acs880.inu@1\",\n"
		"\"objectId\":\"9353cbb9-fdf1-45cf-a97b-277cc4d699c3\",\n"
		"\"version\":1,\n"
		"\"model\":\"abb.ability.configuration\",\n"
		"\"properties\":\n"
		"{\n"
			"\"serialNumberDrive\":{\"value\":\"drv123\"},\n"
			"\"cooling\":{\"mandatory\":true,\"dataType\":\"string\",\"enum\":[\"air\",\"water\"],\"value\":\"air\"},\n"
			"\"driveCategory\":{\"mandatory\":true,\"dataType\":\"string\",\"enum\":[\"LV\",\"MV\"],\"value\":\"LV\"},\n"
			"\"packageType\":{\"mandatory\":true,\"dataType\":\"string\",\"enum\":[\"INU\",\"ISU\"],\"value\":\"INU\"},\n"
			"\"schedules\":\n"
			"{\n"
				"\"basic\":\n"
				"{\n"
					"\"cycle_1\":{\"dataType\":\"array\",\"value\":[\"01_01\",\"01_10\",\"01_15\",\"06_11\"],\"interval\":\"10s\",\"onlyOnChange\":true},\n"
					"\"cycle_2\":{\"dataType\":\"array\",\"value\":[\"01_07\",\"01_11\",\"01_31\",\"05_111\",\"06_16\"],\"interval\":\"1m\",\"onlyOnChange\":true},\n"
					"\"cycle_3\":{\"dataType\":\"array\",\"value\":[\"01_35\",\"01_36\",\"01_37\",\"05_41\",\"05_42\",\"05_121\",\"30_11\",\"30_12\",\"30_13\",\"30_14\",\"30_17\",\"30_26\",\"30_27\",\"96_01\"],\"interval\":\"1h\",\"onlyOnChange\":true},\n"
					"\"cycle_4\":{\"dataType\":\"array\",\"value\":[\"05_01\",\"05_02\",\"45_01\",\"45_02\",\"45_03\",\"99_03\",\"99_04\",\"99_06\",\"99_07\",\"99_08\",\"99_09\",\"99_10\",\"99_12\"],\"interval\":\"1d\",\"onlyOnChange\":true}\n"
				"},\n"
				"\"advanced\":\n"
				"{\n"
					"\"cycle_1\":{\"dataType\":\"array\",\"value\":[\"01_01\",\"01_10\",\"01_15\",\"05_11\",\"05_13\",\"06_11\"],\"interval\":\"3s\",\"onlyOnChange\":true},\n"
					"\"cycle_2\":{\"dataType\":\"array\",\"value\":[\"01_07\",\"01_11\",\"01_31\",\"05_111\",\"06_16\"],\"interval\":\"1m\",\"onlyOnChange\":true},\n"
					"\"cycle_3\":{\"dataType\":\"array\",\"value\":[\"01_35\",\"01_36\",\"01_37\",\"05_41\",\"05_42\",\"05_121\",\"30_11\",\"30_12\",\"30_13\",\"30_14\",\"30_17\",\"30_26\",\"30_27\",\"96_01\"],\"interval\":\"1h\",\"onlyOnChange\":true},\n"
					"\"cycle_4\":{\"dataType\":\"array\",\"value\":[\"05_01\",\"05_02\",\"45_01\",\"45_02\",\"45_03\",\"99_03\",\"99_04\",\"99_06\",\"99_07\",\"99_08\",\"99_09\",\"99_10\",\"99_12\"],\"interval\":\"1d\",\"onlyOnChange\":true}\n"
				"},\n"
				"\"full\":\n"
				"{\n"
					"\"cycle_1\":{\"dataType\":\"array\",\"value\":[\"01_01\",\"01_10\",\"01_15\",\"05_11\",\"05_13\",\"06_11\"],\"interval\":\"1s\",\"onlyOnChange\":false},\n"
					"\"cycle_2\":{\"dataType\":\"array\",\"value\":[\"01_02\",\"01_06\",\"01_07\",\"01_11\",\"01_13\",\"01_14\",\"01_24\",\"01_31\",\"05_12\",\"05_14\",\"05_15\",\"05_16\",\"05_17\",\"05_111\",\"06_01\",\"06_02\",\"06_03\",\"06_04\",\"06_05\",\"06_16\",\"06_17\",\"06_18\",\"06_19\",\"06_21\",\"06_25\",\"10_01\",\"10_21\",\"11_01\",\"12_11\",\"12_21\",\"30_01\",\"30_02\"],\"interval\":\"1m\",\"onlyOnChange\":false},\n"
					"\"cycle_3\":{\"dataType\":\"array\",\"value\":[\"01_35\",\"01_36\",\"01_37\",\"05_30\",\"05_31\",\"05_41\",\"05_42\",\"05_121\",\"30_11\",\"30_12\",\"30_13\",\"30_14\",\"30_17\",\"30_26\",\"30_27\",\"96_01\"],\"interval\":\"1h\",\"onlyOnChange\":false},\n"
					"\"cycle_4\":{\"dataType\":\"array\",\"value\":[\"05_01\",\"05_02\",\"05_04\",\"45_01\",\"45_02\",\"45_03\",\"99_03\",\"99_04\",\"99_06\",\"99_07\",\"99_08\",\"99_09\",\"99_10\",\"99_12\"],\"interval\":\"1d\",\"onlyOnChange\":false}\n"
				"}\n"
			"},\n"
			"\"constants\":\n"
			"{\n"
				"\"constant_1\":{\"dataType\":\"number\",\"symbolicName\":\"max_torque_limit\",\"value\":300,\"unit\":\"%\"},\n"
				"\"constant_2\":{\"dataType\":\"number\",\"symbolicName\":\"min_torque_limit\",\"value\":-300,\"unit\":\"%\"}\n"
			"},\n"
			"\"algorithms\":\n"
			"{\n"
				"\"algorithm_1\":{\"dataType\":\"string\",\"value\":\"InuChargingCounter\"},\n"
				"\"algorithm_2\":{\"dataType\":\"string\",\"value\":\"InuHistogramTorque\"},\n"
				"\"algorithm_3\":{\"dataType\":\"string\",\"value\":\"InuHistogramSpeed\"},\n"
				"\"algorithm_4\":{\"dataType\":\"string\",\"value\":\"InuHistogramPower\"},\n"
				"\"algorithm_5\":{\"dataType\":\"string\",\"value\":\"InuHistogramNmTorque\"},\n"
				"\"algorithm_6\":{\"dataType\":\"string\",\"value\":\"InuIndexLVStress\"},\n"
				"\"algorithm_7\":{\"dataType\":\"string\",\"value\":\"InuEnergyConsumption\"},\n"
				"\"algorithm_8\":{\"dataType\":\"string\",\"value\":\"InuAvailability\"},\n"
				"\"algorithm_9\":{\"dataType\":\"string\",\"value\":\"InuSignalSummary\"},\n"
				"\"algorithm_10\":{\"dataType\":\"string\",\"value\":\"InuScatterTorqueSpeed\"},\n"
				"\"algorithm_11\":{\"dataType\":\"string\",\"value\":\"InuScatterPowerSpeed\"},\n"
				"\"algorithm_12\":{\"dataType\":\"string\",\"value\":\"InuSaveFaultResetTime\"},\n"
				"\"algorithm_13\":{\"dataType\":\"string\",\"value\":\"InuCavitySpeed\"},\n"
				"\"algorithm_14\":{\"dataType\":\"string\",\"value\":\"InuCavityTorque\"},\n"
				"\"algorithm_15\":{\"dataType\":\"string\",\"value\":\"InuIndexTemperature\"}\n"
			"},\n"
			"\"trends\":\n"
			"{\n"
				"\"trend_1\":{\"dataType\":\"string\",\"value\":\"Power\",\"unit\":\"kW\"},\n"
				"\"trend_2\":{\"dataType\":\"string\",\"value\":\"Torque\",\"unit\":\"Nm\"},\n"
				"\"trend_3\":{\"dataType\":\"string\",\"value\":\"Speed\",\"unit\":\"Rpm\"},\n"
				"\"trend_4\":{\"dataType\":\"string\",\"value\":\"Temperature\",\"unit\":\"C\"},\n"
				"\"trend_5\":{\"dataType\":\"string\",\"value\":\"Current\",\"unit\":\"A\"},\n"
				"\"trend_6\":{\"dataType\":\"string\",\"value\":\"EnergyConsumption\",\"unit\":\"kWh\"},\n"
				"\"trend_7\":{\"dataType\":\"string\",\"value\":\"Pumpcavitationspeed\",\"unit\":\"Rpm\"},\n"
				"\"trend_8\":{\"dataType\":\"string\",\"value\":\"Pumpcavitationtorque\",\"unit\":\"Nm\"},\n"
				"\"trend_9\":{\"dataType\":\"string\",\"value\":\"extambtemperature\",\"unit\":\"C\"},\n"
				"\"trend_10\":{\"dataType\":\"string\",\"value\":\"extambpressure\",\"unit\":\"mbar\"},\n"
				"\"trend_11\":{\"dataType\":\"string\",\"value\":\"extambhumidity\",\"unit\":\"% RH\"}\n"
			"},\n"
			"\"scatters\":\n"
			"{\n"
				"\"scatter_1\":{\"dataType\":\"string\",\"value\":\"torque,speed\",\"unit\":\"Nm|Rpm\"},\n"
				"\"scatter_2\":{\"dataType\":\"string\",\"value\":\"power,speed\",\"unit\":\"kW|Rpm\"},\n"
				"\"scatter_3\":{\"dataType\":\"string\",\"value\":\"igbt,power\",\"unit\":\"%|kW\"}\n"
			"},\n"
			"\"histograms\":\n"
			"{\n"
				"\"histogram_1\":{\"dataType\":\"string\",\"value\":\"Speed\",\"unit\":\"Rpm\"},\n"
				"\"histogram_2\":{\"dataType\":\"string\",\"value\":\"power\",\"unit\":\"kW\"},\n"
				"\"histogram_3\":{\"dataType\":\"string\",\"value\":\"temperature\",\"unit\":\"C\"},\n"
				"\"histogram_4\":{\"dataType\":\"string\",\"value\":\"torque\",\"unit\":\"Nm\"}\n"
			"},\n"
			"\"serviceCounters\":\n"
			"{\n"
				"\"serviceCounter_1\":{\"dataType\":\"string\",\"value\":\"fanagingindicator\",\"unit\":\"%\"}\n"
			"},\n"
			"\"thermalLimits\":\n"
			"{\n"
				"\"yellow\":\n"
				"{\n"
					"\"threshold\":{\"mandatory\":true,\"dataType\":\"number\",\"value\":50},\n"
					"\"mean\":{\"mandatory\":true,\"dataType\":\"number\",\"value\":32.5},\n"
					"\"stdDev\":{\"mandatory\":true,\"dataType\":\"number\",\"value\":4}\n"
				"},\n"
				"\"red\":\n"
				"{\n"
					"\"threshold\":{\"mandatory\":true,\"dataType\":\"number\",\"value\":50},\n"
					"\"mean\":{\"mandatory\":true,\"dataType\":\"number\",\"value\":50},\n"
					"\"stdDev\":{\"mandatory\":true,\"dataType\":\"number\",\"value\":4}\n"
				"}\n"
			"}\n"
		"}\n"
	"}";
	in = IoTHubMessage_CreateFromByteArray((const unsigned char *)cfgRecvMsg, strlen(cfgRecvMsg) +1);

	mHandle = IoTHubMessage_Properties(in);

	Map_Add(mHandle, "ability-messagetype", "platformEvent");
	Map_Add(mHandle, "eventType", "Abb.Ability.InformationModel.ObjectModelCreated");
	Map_Add(mHandle, "objectId", "9353cbb9-fdf1-45cf-a97b-277cc4d699c3");
	Map_Add(mHandle, "model", "abb.ability.device");
	Map_Add(mHandle, "type", ABBA_VAL_DRV_ACS_880_TYPE);

	ret = ABBA_Register_StateMachine(&abbLib, NULL, &in);  // receive gw registered

	EXPECT_ZERO(ret);
	EXPECT_ZERO(strcmp(abbLib.ABB_Drive_Id[0], "9353cbb9-fdf1-45cf-a97b-277cc4d699c3"));
	return 0;
}

static int parametersCount;

static int checkParameter(ABBA_Data_CB_Parameter_Data * data)
{
	EXPECT_NON_ZERO(data);

	EXPECT_NON_ZERO(data->cycleType);
	EXPECT_NON_ZERO(data->cycleType[0]);

	EXPECT_NON_ZERO(data->interval);

	EXPECT_NON_ZERO(data->onlyOnChange >= 0 ? 1 : 0);

	EXPECT_NON_ZERO(data->parameterName);

	EXPECT_NON_ZERO(data->parameterName[0]);

	return 0;
}

static void parameterCB(struct ABBA_Data * This, ABBA_Data_CB_Parameter par, ABBA_Data_CB_Parameter_Data * data)
{
	if(par == ABBA_PAR_FIRST)
	{
		if(data)
		{
			parametersCount = 1110;
		}
		else
		{ 
			parametersCount = 0;
		}
		
	}

	if(par == ABBA_PAR_ADD)
	{
		if(!checkParameter(data))
		{ 
			parametersCount++;
		}
	}
}

static int testAbbaSyncConfig(void * arg)
{
	(void)arg;
	ABBA_Data abbLib;

	const char *drives[] = { DRIVE_SERIAL_NUMBER };

	IOTHUB_MESSAGE_HANDLE out = NULL;
	IOTHUB_MESSAGE_HANDLE in = NULL;
	const unsigned char *msg;
	size_t size;
	MAP_HANDLE mHandle;

	int ret = REGISTER_INIT_CMD();

	abbLib.cb.parameter = parameterCB;

	abbLib.registerState = ABBA_INFO_MODEL_SYNC_CONFIGURING;
	strncpy(abbLib.ABB_GW_Id, "f8f0821f-ff74-4943-800a-7ce0e0d405f7", sizeof(abbLib.ABB_GW_Id));

	ret = ABBA_Register_StateMachine(&abbLib, &out, NULL);

	EXPECT_MATCH(ret, 1);

	const char cmp[] ="{\"recursive\":false,\"contained\":false,\"references\":[null],\"related\":[\"abb.ability.configuration\"]}";

	EXPECT_MATCH(IoTHubMessage_GetByteArray(out, &msg, &size), IOTHUB_MESSAGE_OK);
	EXPECT_ZERO(strcmp((const char *)msg, cmp));

	//drive cfg msg
	abbLib.registerState = ABBA_INFO_MODEL_SYNC_CONFIGURED;
	const char cfgRecvMsg[] =
		"{\n"
		"\"type\":\"abb.drives.acs880.inu@1\",\n"
		"\"objectId\":\"9353cbb9-fdf1-45cf-a97b-277cc4d699c3\",\n"
		"\"version\":1,\n"
		"\"model\":\"abb.ability.configuration\",\n"
		"\"properties\":\n"
		"{\n"
		"\"serialNumberDrive\":{\"value\":\"drv123\"},\n"
		"\"cooling\":{\"mandatory\":true,\"dataType\":\"string\",\"enum\":[\"air\",\"water\"],\"value\":\"air\"},\n"
		"\"driveCategory\":{\"mandatory\":true,\"dataType\":\"string\",\"enum\":[\"LV\",\"MV\"],\"value\":\"LV\"},\n"
		"\"packageType\":{\"mandatory\":true,\"dataType\":\"string\",\"enum\":[\"INU\",\"ISU\"],\"value\":\"INU\"},\n"
		"\"monitoring\":{\"value\":\"Advanced\"},\n"
		"\"registration\":{\"value\":\"Registered\"},\n"
		"\"registrationSystem\":{\"value\":\"DriveInstalledBase\"},\n"
		"\"schedules\":\n"
		"{\n"
		"\"basic\":\n"
		"{\n"
		"\"cycle_1\":{\"dataType\":\"array\",\"value\":[\"01_01\",\"01_10\",\"01_15\",\"06_11\"],\"interval\":\"10s\",\"onlyOnChange\":true},\n"
		"\"cycle_2\":{\"dataType\":\"array\",\"value\":[\"01_07\",\"01_11\",\"01_31\",\"05_111\",\"06_16\"],\"interval\":\"1m\",\"onlyOnChange\":true},\n"
		"\"cycle_3\":{\"dataType\":\"array\",\"value\":[\"01_35\",\"01_36\",\"01_37\",\"05_41\",\"05_42\",\"05_121\",\"30_11\",\"30_12\",\"30_13\",\"30_14\",\"30_17\",\"30_26\",\"30_27\",\"96_01\"],\"interval\":\"1h\",\"onlyOnChange\":true},\n"
		"\"cycle_4\":{\"dataType\":\"array\",\"value\":[\"05_01\",\"05_02\",\"45_01\",\"45_02\",\"45_03\",\"99_03\",\"99_04\",\"99_06\",\"99_07\",\"99_08\",\"99_09\",\"99_10\",\"99_12\"],\"interval\":\"1d\",\"onlyOnChange\":true}\n"
		"},\n"
		"\"advanced\":\n"
		"{\n"
		"\"cycle_1\":{\"dataType\":\"array\",\"value\":[\"01_01\",\"01_10\",\"01_15\",\"05_11\",\"05_13\",\"06_11\"],\"interval\":\"3s\",\"onlyOnChange\":true},\n"
		"\"cycle_2\":{\"dataType\":\"array\",\"value\":[\"01_07\",\"01_11\",\"01_31\",\"05_111\",\"06_16\"],\"interval\":\"1m\",\"onlyOnChange\":true},\n"
		"\"cycle_3\":{\"dataType\":\"array\",\"value\":[\"01_35\",\"01_36\",\"01_37\",\"05_41\",\"05_42\",\"05_121\",\"30_11\",\"30_12\",\"30_13\",\"30_14\",\"30_17\",\"30_26\",\"30_27\",\"96_01\"],\"interval\":\"1h\",\"onlyOnChange\":true},\n"
		"\"cycle_4\":{\"dataType\":\"array\",\"value\":[\"05_01\",\"05_02\",\"45_01\",\"45_02\",\"45_03\",\"99_03\",\"99_04\",\"99_06\",\"99_07\",\"99_08\",\"99_09\",\"99_10\",\"99_12\"],\"interval\":\"1d\",\"onlyOnChange\":true}\n"
		"},\n"
		"\"full\":\n"
		"{\n"
		"\"cycle_1\":{\"dataType\":\"array\",\"value\":[\"01_01\",\"01_10\",\"01_15\",\"05_11\",\"05_13\",\"06_11\"],\"interval\":\"1s\",\"onlyOnChange\":false},\n"
		"\"cycle_2\":{\"dataType\":\"array\",\"value\":[\"01_02\",\"01_06\",\"01_07\",\"01_11\",\"01_13\",\"01_14\",\"01_24\",\"01_31\",\"05_12\",\"05_14\",\"05_15\",\"05_16\",\"05_17\",\"05_111\",\"06_01\",\"06_02\",\"06_03\",\"06_04\",\"06_05\",\"06_16\",\"06_17\",\"06_18\",\"06_19\",\"06_21\",\"06_25\",\"10_01\",\"10_21\",\"11_01\",\"12_11\",\"12_21\",\"30_01\",\"30_02\"],\"interval\":\"1m\",\"onlyOnChange\":false},\n"
		"\"cycle_3\":{\"dataType\":\"array\",\"value\":[\"01_35\",\"01_36\",\"01_37\",\"05_30\",\"05_31\",\"05_41\",\"05_42\",\"05_121\",\"30_11\",\"30_12\",\"30_13\",\"30_14\",\"30_17\",\"30_26\",\"30_27\",\"96_01\"],\"interval\":\"1h\",\"onlyOnChange\":false},\n"
		"\"cycle_4\":{\"dataType\":\"array\",\"value\":[\"05_01\",\"05_02\",\"05_04\",\"45_01\",\"45_02\",\"45_03\",\"99_03\",\"99_04\",\"99_06\",\"99_07\",\"99_08\",\"99_09\",\"99_10\",\"99_12\"],\"interval\":\"1d\",\"onlyOnChange\":false}\n"
		"}\n"
		"},\n"
		"\"constants\":\n"
		"{\n"
		"\"constant_1\":{\"dataType\":\"number\",\"symbolicName\":\"max_torque_limit\",\"value\":300,\"unit\":\"%\"},\n"
		"\"constant_2\":{\"dataType\":\"number\",\"symbolicName\":\"min_torque_limit\",\"value\":-300,\"unit\":\"%\"}\n"
		"},\n"
		"\"algorithms\":\n"
		"{\n"
		"\"algorithm_1\":{\"dataType\":\"string\",\"value\":\"InuChargingCounter\"},\n"
		"\"algorithm_2\":{\"dataType\":\"string\",\"value\":\"InuHistogramTorque\"},\n"
		"\"algorithm_3\":{\"dataType\":\"string\",\"value\":\"InuHistogramSpeed\"},\n"
		"\"algorithm_4\":{\"dataType\":\"string\",\"value\":\"InuHistogramPower\"},\n"
		"\"algorithm_5\":{\"dataType\":\"string\",\"value\":\"InuHistogramNmTorque\"},\n"
		"\"algorithm_6\":{\"dataType\":\"string\",\"value\":\"InuIndexLVStress\"},\n"
		"\"algorithm_7\":{\"dataType\":\"string\",\"value\":\"InuEnergyConsumption\"},\n"
		"\"algorithm_8\":{\"dataType\":\"string\",\"value\":\"InuAvailability\"},\n"
		"\"algorithm_9\":{\"dataType\":\"string\",\"value\":\"InuSignalSummary\"},\n"
		"\"algorithm_10\":{\"dataType\":\"string\",\"value\":\"InuScatterTorqueSpeed\"},\n"
		"\"algorithm_11\":{\"dataType\":\"string\",\"value\":\"InuScatterPowerSpeed\"},\n"
		"\"algorithm_12\":{\"dataType\":\"string\",\"value\":\"InuSaveFaultResetTime\"},\n"
		"\"algorithm_13\":{\"dataType\":\"string\",\"value\":\"InuCavitySpeed\"},\n"
		"\"algorithm_14\":{\"dataType\":\"string\",\"value\":\"InuCavityTorque\"},\n"
		"\"algorithm_15\":{\"dataType\":\"string\",\"value\":\"InuIndexTemperature\"}\n"
		"},\n"
		"\"trends\":\n"
		"{\n"
		"\"trend_1\":{\"dataType\":\"string\",\"value\":\"Power\",\"unit\":\"kW\"},\n"
		"\"trend_2\":{\"dataType\":\"string\",\"value\":\"Torque\",\"unit\":\"Nm\"},\n"
		"\"trend_3\":{\"dataType\":\"string\",\"value\":\"Speed\",\"unit\":\"Rpm\"},\n"
		"\"trend_4\":{\"dataType\":\"string\",\"value\":\"Temperature\",\"unit\":\"C\"},\n"
		"\"trend_5\":{\"dataType\":\"string\",\"value\":\"Current\",\"unit\":\"A\"},\n"
		"\"trend_6\":{\"dataType\":\"string\",\"value\":\"EnergyConsumption\",\"unit\":\"kWh\"},\n"
		"\"trend_7\":{\"dataType\":\"string\",\"value\":\"Pumpcavitationspeed\",\"unit\":\"Rpm\"},\n"
		"\"trend_8\":{\"dataType\":\"string\",\"value\":\"Pumpcavitationtorque\",\"unit\":\"Nm\"},\n"
		"\"trend_9\":{\"dataType\":\"string\",\"value\":\"extambtemperature\",\"unit\":\"C\"},\n"
		"\"trend_10\":{\"dataType\":\"string\",\"value\":\"extambpressure\",\"unit\":\"mbar\"},\n"
		"\"trend_11\":{\"dataType\":\"string\",\"value\":\"extambhumidity\",\"unit\":\"% RH\"}\n"
		"},\n"
		"\"scatters\":\n"
		"{\n"
		"\"scatter_1\":{\"dataType\":\"string\",\"value\":\"torque,speed\",\"unit\":\"Nm|Rpm\"},\n"
		"\"scatter_2\":{\"dataType\":\"string\",\"value\":\"power,speed\",\"unit\":\"kW|Rpm\"},\n"
		"\"scatter_3\":{\"dataType\":\"string\",\"value\":\"igbt,power\",\"unit\":\"%|kW\"}\n"
		"},\n"
		"\"histograms\":\n"
		"{\n"
		"\"histogram_1\":{\"dataType\":\"string\",\"value\":\"Speed\",\"unit\":\"Rpm\"},\n"
		"\"histogram_2\":{\"dataType\":\"string\",\"value\":\"power\",\"unit\":\"kW\"},\n"
		"\"histogram_3\":{\"dataType\":\"string\",\"value\":\"temperature\",\"unit\":\"C\"},\n"
		"\"histogram_4\":{\"dataType\":\"string\",\"value\":\"torque\",\"unit\":\"Nm\"}\n"
		"},\n"
		"\"serviceCounters\":\n"
		"{\n"
		"\"serviceCounter_1\":{\"dataType\":\"string\",\"value\":\"fanagingindicator\",\"unit\":\"%\"}\n"
		"},\n"
		"\"thermalLimits\":\n"
		"{\n"
		"\"yellow\":\n"
		"{\n"
		"\"threshold\":{\"mandatory\":true,\"dataType\":\"number\",\"value\":50},\n"
		"\"mean\":{\"mandatory\":true,\"dataType\":\"number\",\"value\":32.5},\n"
		"\"stdDev\":{\"mandatory\":true,\"dataType\":\"number\",\"value\":4}\n"
		"},\n"
		"\"red\":\n"
		"{\n"
		"\"threshold\":{\"mandatory\":true,\"dataType\":\"number\",\"value\":50},\n"
		"\"mean\":{\"mandatory\":true,\"dataType\":\"number\",\"value\":50},\n"
		"\"stdDev\":{\"mandatory\":true,\"dataType\":\"number\",\"value\":4}\n"
		"}\n"
		"}\n"
		"}\n"
		"}";
	in = IoTHubMessage_CreateFromByteArray((const unsigned char *)cfgRecvMsg, strlen(cfgRecvMsg) + 1);

	mHandle = IoTHubMessage_Properties(in);

	Map_Add(mHandle, "ability-messagetype", "platformEvent");
	Map_Add(mHandle, "eventType", "Abb.Ability.InformationModel.ObjectModelCreated");
	Map_Add(mHandle, "objectId", "9353cbb9-fdf1-45cf-a97b-277cc4d699c3");
	Map_Add(mHandle, "model", "abb.ability.device");
	Map_Add(mHandle, "type", ABBA_VAL_DRV_ACS_880_TYPE);

	ret = ABBA_Register_StateMachine(&abbLib, NULL, &in);  // receive gw registered

	EXPECT_ZERO(ret);
	
	EXPECT_MATCH(parametersCount, 38);//

	return 0;
}

static int testAbbaDbgMsg(void * arg)
{
	int ret;

	IOTHUB_MESSAGE_HANDLE in = IoTHubMessage_CreateFromByteArray("TEST MESSAGE", strlen("TEST MESSAGE"));

	MAP_HANDLE mHandle = IoTHubMessage_Properties(in);

	Map_Add(mHandle, "ability-messagetype", "timeSeries");
	//CPM_createSendCIDEvent("1222222222222222222");

	ret = ABBA_MsgDebug((IOTHUB_MESSAGE_HANDLE)0, ABBA_CTRL_MSG);
	ABBA_MsgDebug(in, ABBA_RX_MSG);
	ABBA_MsgDebug(in, ABBA_TX_MSG);
	ABBA_MsgDebug((IOTHUB_MESSAGE_HANDLE) 1, ABBA_CTRL_MSG);
	ABBA_MsgDebug(in, ABBA_RX_MSG);
	ABBA_MsgDebug(in, ABBA_TX_MSG);
	ABBA_MsgDebug((IOTHUB_MESSAGE_HANDLE)0, ABBA_CTRL_MSG);
	ABBA_MsgDebug(in, ABBA_RX_MSG);
	ABBA_MsgDebug(in, ABBA_TX_MSG);
	ABBA_MsgDebug((IOTHUB_MESSAGE_HANDLE)ret, ABBA_CTRL_MSG);

	return 0;
}

static int testAbbaRegisterInvalid(void * arg)
{
	(void)arg;

	return 0;
}


static UnitTest testAbbalib[] =
{
	{ testAbbaGwRegister, NULL, "test ABBA IoT Panel register " },
	{ testAbbaDrvRegister, NULL, "test ABBA drive register " },
	{ testAbbaSyncConfig, NULL, "test ABBA sync config " },
	{ testAbbaDbgMsg, NULL, "test ABBA debug output" },
	{ testAbbaRegisterInvalid, NULL, "test ABBA register rainy day" },
	{ NULL, NULL, NULL }
};

int ABBALIB_UnitTestDo(void)
{
	UnitTest *p = &testAbbalib[0];

	for (; p->func && p->name; p++)
	{
		int testValue = p->func(p->parama);
		if (testValue)
		{
			CONSOLE_PRINTER("ABBALib UT FAILED %s == %d\n", p->name, testValue);
			failed++;
		}
		else
		{
			success++;
		}
	}

	CONSOLE_PRINTER("ABBALib_UnitTestDo failed %d, success %d\n", failed, success);

	return failed;
}
