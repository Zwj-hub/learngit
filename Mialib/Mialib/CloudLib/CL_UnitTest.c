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
*                   Filename:   CL_UnitTest.c                                 *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file CL_UnitTest.c
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
#include "CL_ParameterList.h"
#include "CL_MeasBuff.h"
#include "CL_Manager.h"
#include "CL_Utils.h"
#include "CL_DebugSerialiser.h"
#include "CL_ABBAbilitySerialiser.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
//-----------------------------------------------------------------------------
// 2) LOCAL ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------
#define TEST_FW_VER_DEF	"5.90.200.8"
#define TEST_GUID_DEF	"273e3b7f-ff62-4478-83f0-7ad91ab517c5"
#define TEST_TYPE_DEF	"panel@5.90"
#define TEST_SN_DEF	"B5520147WU"

//-----------------------------------------------------------------------------
// 3) LOCAL DATA STRUCTURES
//-----------------------------------------------------------------------------

typedef struct UnitTest
{
	int(*func)(void*);
	void * parama;
	const char * name;
}UnitTest;


typedef struct UnitTestSenario
{
	CL_Parameter param;
	CL_Meas meas;
}UnitTestSenario;

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

#if 0
CREATE_CL_ParameterList(testDevParaList, 6);
CREATE_CL_Manager(testMgr, 3, 30, "IoTPanel", "PanelIoTHub.azure-devices.net");

UnitTestSenario	Parameter_Senario[] = {
	{	{	"Saved energy_0",		1,		CL_UINT32	},	{	123,	1528959210	}	},
	{	{	"Pump/fan status_0",	34,		CL_UINT16	},	{	1000,	1528349210	}	},
	{	{	"Output frequency",		31,		CL_INT32	},	{	-2656,	1528342310	}	},
	{	{	"Saved energy_1",		2,		CL_UINT32	},	{	567,	1228359210	}	},
	{	{	"Pump/fan status_1",	32,		CL_UINT16	},	{	2000,	1524349210	}	},
	{	{	"Motor torque",			665,	CL_INT32	},	{	-3000,	1428359210 }	}
};

//-----------------------------------------------------------------------------
// 5) GLOBAL VARIABLES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 6) LOCAL FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------
static int testCLMgr(void * arg);
static int testCLParamList(void * arg);


//-----------------------------------------------------------------------------
// 7) FUNCTIONS
//-----------------------------------------------------------------------------

static int testCLParamList(void * arg)
{
	int ret = -1;
	CL_Parameter testParam;
	CL_ParameterList *pList = (CL_ParameterList *)arg;


	for (int i = 0; i< CL_ParameterListSize(pList); i++)
	{
		ret = CL_ParameterInit(&testParam, Parameter_Senario[i].param.name, Parameter_Senario[i].param.id, Parameter_Senario[i].param.type);
		if (ret < 0) {
			CONSOLE_PRINTER("CL_ParameterInit error\n");
			return ret;
		}
		ret = CL_ParameterListAddParam(pList, &testParam);
		if (ret < 0) {
			CONSOLE_PRINTER("CL_ParameterInit error\n");
			return ret;
		}
	}
	return 0;
}

static int testCLMgr(void * arg)
{
	int ret = -1;
	int i;
	CL_Manager *testMgr = (CL_Manager *)arg;
	CL_ParameterList *pList = NULL;

	for (i = 0; i < CL_ManagerGetDeviceSize(testMgr); i++) {
		ret = CL_DeviceInit(&testMgr->devices[i], TEST_GUID_DEF, TEST_SN_DEF, TEST_FW_VER_DEF, TEST_TYPE_DEF);
		if (ret < 0) {
			CONSOLE_PRINTER("CL Dev %d Init error\n", i);
			return ret;
		}

		CL_DeviceSetParameters(&testMgr->devices[i], &testDevParaList);
		pList = testMgr->devices[i].paraList;
		if (pList == NULL) {
			CONSOLE_PRINTER("CL Dev %d Parameter Init error\n", i);
			return ret;
		}

		for (int j = 0; j < sizeof(Parameter_Senario) / sizeof(UnitTestSenario); j++) {
			ret = CL_MgrAddMeas(testMgr, i, Parameter_Senario[j].param.id, &Parameter_Senario[j].meas);
			if (ret < 0) {
				CONSOLE_PRINTER("CL Dev %d Parameter %d name %s add Meas error\n", i, j, pList->list[j].name);
				return ret;
			}
		}
	}
	return 0;
}
#endif


static int testCloudLib(void * arg)
{
	int i;

	CREATE_CL_Manager(mgr, 2, 10, "IoTpanel", "PanelIoTHub.azure-devices.net");

	CL_Device devices[2];
	CREATE_CL_ParameterList(paraList0, 5);
	CREATE_CL_ParameterList(paraList1, 5);


	EXPECT_ZERO(CL_DeviceInit(&devices[0], "GUID0", "SN0", "FWVER0", "TYPE0"));
	EXPECT_ZERO(CL_DeviceInit(&devices[1], "GUID1", "SN1", "FWVER1", "TYPE1"));

	EXPECT_NON_ZERO(CL_DeviceInit(&devices[1], "GUID1", "SN1_____________________", "FWVER1123456789987", "TYPE1"));


	for(i = 0; i < 5; i++)
	{
		CL_Parameter parameter0;
		CL_Parameter parameter1;
		char parName0[] = "parName0     ";
		char parName1[] = "parName1     ";
		parName0[9] = '0' + i;
		parName1[9] = '0' + i;

		EXPECT_ZERO(CL_ParameterInit(&parameter0, parName0, i + 10, CL_INT8 + i));
		EXPECT_ZERO(CL_ParameterInit(&parameter1, parName1, i + 30, CL_INT8 + i));

		EXPECT_ZERO(CL_ParameterListAddParam(&paraList0, &parameter0));
		EXPECT_ZERO(CL_ParameterListAddParam(&paraList1, &parameter1));
	}

	if (1)
	{
		int i = 2;
		CL_Parameter parameter0;
		CL_Parameter parameter1;
		char parName0[] = "fail     ";
		char parName1[] = "fail2     ";

		EXPECT_ZERO(CL_ParameterInit(&parameter0, parName0, i + 101, CL_INT8 + i));
		EXPECT_ZERO(CL_ParameterInit(&parameter1, parName1, i + 301, CL_INT8 + i));

		EXPECT_NON_ZERO(CL_ParameterListAddParam(&paraList0, &parameter0));
		EXPECT_NON_ZERO(CL_ParameterListAddParam(&paraList1, &parameter1));
	}


	EXPECT_ZERO(CL_DeviceSetParameters(&devices[0], &paraList0));
	EXPECT_ZERO(CL_DeviceSetParameters(&devices[1], &paraList1));

	EXPECT_ZERO(CL_MgrSetDevice(&mgr, &devices[0], 0));
	EXPECT_ZERO(CL_MgrSetDevice(&mgr, &devices[1], 1));

	for(i = 0; i < 5; i++)
	{
		int meas = 5 + i;
		EXPECT_ZERO(CL_MgrAddMeas(&mgr, i + 10, &meas, 1000 + i));
		EXPECT_ZERO(CL_MgrAddMeas(&mgr, i + 30, &meas, 1000 + i));
	}

	for (i = 0; i < 5; i++)
	{
		int meas = 5 + i;
		EXPECT_NON_ZERO(CL_MgrAddMeas(&mgr, i + 10, &meas, 1000 + i));
		EXPECT_NON_ZERO(CL_MgrAddMeas(&mgr, i + 30, &meas, 1000 + i));
	}



	if(1)
	{
		int ret;
		char buffer[4086];
		const char cmp[] = 
			"[\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName0 0   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:40+0800\",\n"
			"\t\t\t\"value\" : 5\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName0 1   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:41+0800\",\n"
			"\t\t\t\"value\" : 6\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName0 2   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:42+0800\",\n"
			"\t\t\t\"value\" : 7\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName0 3   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:43+0800\",\n"
			"\t\t\t\"value\" : 8\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName0 4   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:44+0800\",\n"
			"\t\t\t\"value\" : 9\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID1\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName1 0   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:40+0800\",\n"
			"\t\t\t\"value\" : 5\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID1\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName1 1   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:41+0800\",\n"
			"\t\t\t\"value\" : 6\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID1\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName1 2   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:42+0800\",\n"
			"\t\t\t\"value\" : 7\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID1\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName1 3   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:43+0800\",\n"
			"\t\t\t\"value\" : 8\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID1\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName1 4   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:44+0800\",\n"
			"\t\t\t\"value\" : 9\n"
			"\t\t}]\n"
			"";
		EXPECT_POSITIVE(CL_MgrPush(&mgr, buffer, sizeof(buffer), CL_ABB_ABILITY_CREATE_TELEMETRICS));
		ret = strncmp(buffer, cmp, sizeof(buffer));
		EXPECT_ZERO(ret);
		/* TODO compare output */
	}

	EXPECT_ZERO(CL_MgrDelMeas(&mgr));

	for (i = 0; i < 2; i++) // Two measurements to parameter
	{
		int meas = 5 + i;
		EXPECT_ZERO(CL_MgrAddMeas(&mgr, i + 10, &meas, 1000 + i));
		EXPECT_ZERO(CL_MgrAddMeas(&mgr, i + 30, &meas, 1000 + i));

		meas = 10 + i;
		EXPECT_ZERO(CL_MgrAddMeas(&mgr, i + 10, &meas, 2000 + i));
		EXPECT_ZERO(CL_MgrAddMeas(&mgr, i + 30, &meas, 2000 + i));
	}

	if (1)
	{
		int ret;
		char buffer[8086];
		char cmp[] =
			"[\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName0 0   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:33:20+0800\",\n"
			"\t\t\t\"value\" : 10\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName0 0   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:40+0800\",\n"
			"\t\t\t\"value\" : 5\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName0 1   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:33:21+0800\",\n"
			"\t\t\t\"value\" : 11\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName0 1   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:41+0800\",\n"
			"\t\t\t\"value\" : 6\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID1\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName1 0   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:33:20+0800\",\n"
			"\t\t\t\"value\" : 10\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID1\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName1 0   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:40+0800\",\n"
			"\t\t\t\"value\" : 5\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID1\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName1 1   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:33:21+0800\",\n"
			"\t\t\t\"value\" : 11\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID1\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"parName1 1   \",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:41+0800\",\n"
			"\t\t\t\"value\" : 6\n"
			"\t\t}]\n"
			"";
		EXPECT_POSITIVE(CL_MgrPush(&mgr, buffer, sizeof(buffer), CL_ABB_ABILITY_CREATE_TELEMETRICS));
		ret = strncmp(buffer, cmp, sizeof(buffer));
		EXPECT_ZERO(ret);
		EXPECT_POSITIVE(CL_MgrPush(&mgr, buffer, sizeof(buffer), CL_ABB_ABILITY_CREATE_DEVICEREGISTRATION));
		char cmp2[] = "{\n"
			"\t\"property\": {\n"
			"\t\t\"ability-messagetype\": \"platformEvent\",\n"
			"\t\t\"eventType\": \"Abb.Ability.Device.Created\"\n"
			"\t},\n"
			"\t\"body\" : {\n"
			"\t\t\"type\":\"Abb.Ability.Device.Panel\",\n"
			"\t\t\"property\":{\n"
			"\t\t\t\"deviceId\":{\n"
			"\t\t\t\"value\":\"IoTpanel\"},\n"
			"\t\t\t\"hubName\":{\n"
			"\t\t\t\"value\":\"PanelIoTHub.azure-devices.net\"}\n"
			"\t\t},\n"
			"\t\t\"variables\":[\n"
			"\t\t\t{\"firmwareVersion\":\"FWVER0\"},\n"
			"\t\t\t{\"objectId\":\"GUID0\"},\n"
			"\t\t\t{\"sn\":\"SN0\"},\n"
			"\t\t\t{\"deviceType\":\"TYPE0\"}\n"
			"\t\t]\n"
			"\t\t\"variables\":[\n"
			"\t\t\t{\"firmwareVersion\":\"FWVER1\"},\n"
			"\t\t\t{\"objectId\":\"GUID1\"},\n"
			"\t\t\t{\"sn\":\"SN1_____________________\"},\n"
			"\t\t\t{\"deviceType\":\"TYPE1\"}\n"
			"\t\t]\n"
			"\n"
			"\t}\n"
			"}\n"
			"";
		ret = strncmp(buffer, cmp2, sizeof(buffer));
		EXPECT_ZERO(ret);
		/* TODO compare output */
	}

	CL_MgrDelMeas(&mgr);

	for(i = 0; i < 5; i++)
	{
		int meas = 5 + i;
		EXPECT_NON_ZERO(CL_MgrAddMeas(&mgr, i + 100, &meas, 1000 + i));
		EXPECT_NON_ZERO(CL_MgrAddMeas(&mgr, i + 300, &meas, 1000 + i));
	}

	return 0; // Great success
}

static int testCloudLibData(void * arg)
{
	int i;

	CREATE_CL_Manager(mgr, 1, 10, "IoTpanel", "PanelIoTHub.azure-devices.net");

	CL_Device devices[1];
	CREATE_CL_ParameterList(paraList0, 5);

	EXPECT_ZERO(CL_DeviceInit(&devices[0], "GUID0", "SN0", "FWVER0", "TYPE0"));

	CL_Parameter parameter0;
	CL_ParameterInit(&parameter0, "output speed1", 2, CL_REAL64);
	EXPECT_ZERO(CL_ParameterListAddParam(&paraList0, &parameter0));
	CL_ParameterInit(&parameter0, "output speed2", 3, CL_REAL32);
	EXPECT_ZERO(CL_ParameterListAddParam(&paraList0, &parameter0));
	CL_ParameterInit(&parameter0, "output speed3", 4, CL_INT32);
	EXPECT_ZERO(CL_ParameterListAddParam(&paraList0, &parameter0));

	EXPECT_ZERO(CL_DeviceSetParameters(&devices[0], &paraList0));

	EXPECT_ZERO(CL_MgrSetDevice(&mgr, &devices[0], 0));

	double meas1 = 5;
	CL_MgrAddMeas(&mgr, 2, &meas1, 1000);
	float meas2 = 5;
	CL_MgrAddMeas(&mgr, 3, &meas2, 2000);
	int meas3 = -5;
	CL_MgrAddMeas(&mgr, 4, &meas3, 3000);


	if (1)
	{
		char buffer[4086];
		EXPECT_POSITIVE(CL_MgrPush(&mgr, buffer, sizeof(buffer), CL_ABB_ABILITY_CREATE_TELEMETRICS));

		const char cmp[] =
			"[\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"output speed1\",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:40+0800\",\n"
			"\t\t\t\"value\" : 5.000000\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"output speed2\",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:33:20+0800\",\n"
			"\t\t\t\"value\" : 5.000000\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"output speed3\",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:50:00+0800\",\n"
			"\t\t\t\"value\" : -5\n"
			"\t\t}]\n"
			;
		int ret = strcmp(buffer, cmp);
		return ret;
	}
	return 0; // Great success
}

static int testCloudLibTwoDevs(void * arg)
{
	int i;

	CREATE_CL_Manager(mgr, 1, 5, "IoTpanel", "PanelIoTHub.azure-devices.net");

	CL_Device devices[2];
	CREATE_CL_ParameterList(paraList, 5);

	EXPECT_ZERO(CL_DeviceInit(&devices[0], "GUID0", "SN0", "FWVER0", "TYPE0"));
	EXPECT_ZERO(CL_DeviceInit(&devices[1], "GUID1", "SN1", "FWVER1", "TYPE1"));

	for (i = 0; i < 5; i++)
	{
		CL_Parameter parameter[5];
		char parName[16];

		memset(parName, 0, 16);
		snprintf(parName, 16, "testParaName__%d", i);

		EXPECT_ZERO(CL_ParameterInit(&parameter[i], parName, i + 10, CL_INT8 + i));
		EXPECT_ZERO(CL_ParameterListAddParam(&paraList, &parameter[i]));
	}


	for (i = 0; i < 2; i++)
	{
		EXPECT_ZERO(CL_DeviceSetParameters(&devices[i], &paraList));
		EXPECT_ZERO(CL_MgrSetDevice(&mgr, &devices[i], i));

		for (i = 0; i < 5; i++)
		{
			int meas = 5 + i;
			EXPECT_ZERO(CL_MgrAddMeas(&mgr, i + 10, &meas, 1000 + i));
		}

		for (i = 0; i < 5; i++)
		{
			int meas = 5 + i;
			EXPECT_NON_ZERO(CL_MgrAddMeas(&mgr, i + 10, &meas, 1000 + i));
		}

		if (1)
		{
			int ret;
			char buffer[4086];
			char cmp[] =
				"[\n"
				"\t\t{\n"
				"\t\t\t\"objectId\": \"GUID0\",\n"
				"\t\t\t\"model\" : \"abb.ability.device\",\n"
				"\t\t\t\"variable\" : \"testParaName__0\",\n"
				"\t\t\t\"timestamp\" : \"1970-01-01T08:16:40+0800\",\n"
				"\t\t\t\"value\" : 5\n"
				"\t\t},\n"
				"\t\t{\n"
				"\t\t\t\"objectId\": \"GUID0\",\n"
				"\t\t\t\"model\" : \"abb.ability.device\",\n"
				"\t\t\t\"variable\" : \"testParaName__1\",\n"
				"\t\t\t\"timestamp\" : \"1970-01-01T08:16:41+0800\",\n"
				"\t\t\t\"value\" : 6\n"
				"\t\t},\n"
				"\t\t{\n"
				"\t\t\t\"objectId\": \"GUID0\",\n"
				"\t\t\t\"model\" : \"abb.ability.device\",\n"
				"\t\t\t\"variable\" : \"testParaName__2\",\n"
				"\t\t\t\"timestamp\" : \"1970-01-01T08:16:42+0800\",\n"
				"\t\t\t\"value\" : 7\n"
				"\t\t},\n"
				"\t\t{\n"
				"\t\t\t\"objectId\": \"GUID0\",\n"
				"\t\t\t\"model\" : \"abb.ability.device\",\n"
				"\t\t\t\"variable\" : \"testParaName__3\",\n"
				"\t\t\t\"timestamp\" : \"1970-01-01T08:16:43+0800\",\n"
				"\t\t\t\"value\" : 8\n"
				"\t\t},\n"
				"\t\t{\n"
				"\t\t\t\"objectId\": \"GUID0\",\n"
				"\t\t\t\"model\" : \"abb.ability.device\",\n"
				"\t\t\t\"variable\" : \"testParaName__4\",\n"
				"\t\t\t\"timestamp\" : \"1970-01-01T08:16:44+0800\",\n"
				"\t\t\t\"value\" : 9\n"
				"\t\t}]\n"
				"";
			EXPECT_POSITIVE(CL_MgrPush(&mgr, buffer, sizeof(buffer), CL_ABB_ABILITY_CREATE_TELEMETRICS));
			ret = strncmp(buffer, cmp, sizeof(buffer));
			EXPECT_ZERO(ret);
			/* TODO compare output */
		}

		EXPECT_ZERO(CL_MgrDelMeas(&mgr));

		for (i = 0; i < 2; i++) // Two measurements to parameter
		{
			int meas = 5 + i;
			EXPECT_ZERO(CL_MgrAddMeas(&mgr, i + 10, &meas, 1000 + i));

			meas = 10 + i;
			EXPECT_ZERO(CL_MgrAddMeas(&mgr, i + 10, &meas, 2000 + i));
		}

		if (1)
		{
			int ret;
			char buffer[8086];
			char cmp[] =
				"[\n"
				"\t\t{\n"
				"\t\t\t\"objectId\": \"GUID0\",\n"
				"\t\t\t\"model\" : \"abb.ability.device\",\n"
				"\t\t\t\"variable\" : \"testParaName__0\",\n"
				"\t\t\t\"timestamp\" : \"1970-01-01T08:33:20+0800\",\n"
				"\t\t\t\"value\" : 10\n"
				"\t\t},\n"
				"\t\t{\n"
				"\t\t\t\"objectId\": \"GUID0\",\n"
				"\t\t\t\"model\" : \"abb.ability.device\",\n"
				"\t\t\t\"variable\" : \"testParaName__0\",\n"
				"\t\t\t\"timestamp\" : \"1970-01-01T08:16:40+0800\",\n"
				"\t\t\t\"value\" : 5\n"
				"\t\t},\n"
				"\t\t{\n"
				"\t\t\t\"objectId\": \"GUID0\",\n"
				"\t\t\t\"model\" : \"abb.ability.device\",\n"
				"\t\t\t\"variable\" : \"testParaName__1\",\n"
				"\t\t\t\"timestamp\" : \"1970-01-01T08:33:21+0800\",\n"
				"\t\t\t\"value\" : 11\n"
				"\t\t},\n"
				"\t\t{\n"
				"\t\t\t\"objectId\": \"GUID0\",\n"
				"\t\t\t\"model\" : \"abb.ability.device\",\n"
				"\t\t\t\"variable\" : \"testParaName__1\",\n"
				"\t\t\t\"timestamp\" : \"1970-01-01T08:16:41+0800\",\n"
				"\t\t\t\"value\" : 6\n"
				"\t\t}]\n"
				"";
			EXPECT_POSITIVE(CL_MgrPush(&mgr, buffer, sizeof(buffer), CL_ABB_ABILITY_CREATE_TELEMETRICS));
			ret = strncmp(buffer, cmp, sizeof(buffer));
			EXPECT_ZERO(ret);
			EXPECT_POSITIVE(CL_MgrPush(&mgr, buffer, sizeof(buffer), CL_ABB_ABILITY_CREATE_DEVICEREGISTRATION));
			char cmp2[] =
				"{\n"
				"\t\"property\": {\n"
				"\t\t\"ability-messagetype\": \"platformEvent\",\n"
				"\t\t\"eventType\": \"Abb.Ability.Device.Created\"\n"
				"\t},\n"
				"\t\"body\" : {\n"
				"\t\t\"type\":\"Abb.Ability.Device.Panel\",\n"
				"\t\t\"property\":{\n"
				"\t\t\t\"deviceId\":{\n"
				"\t\t\t\"value\":\"IoTpanel\"},\n"
				"\t\t\t\"hubName\":{\n"
				"\t\t\t\"value\":\"PanelIoTHub.azure-devices.net\"}\n"
				"\t\t},\n"
				"\t\t\"variables\":[\n"
				"\t\t\t{\"firmwareVersion\":\"FWVER0\"},\n"
				"\t\t\t{\"objectId\":\"GUID0\"},\n"
				"\t\t\t{\"sn\":\"SN0\"},\n"
				"\t\t\t{\"deviceType\":\"TYPE0\"}\n"
				"\t\t]\n"
				"\n"
				"\t}\n"
				"}\n"
				"";
			ret = strncmp(buffer, cmp2, sizeof(buffer));
			EXPECT_ZERO(ret);
			/* TODO compare output */
		}
	}

	CL_MgrDelMeas(&mgr);

	for (i = 0; i < 5; i++)
	{
		int meas = 5 + i;
		EXPECT_NON_ZERO(CL_MgrAddMeas(&mgr, i + 100, &meas, 1000 + i));
		EXPECT_NON_ZERO(CL_MgrAddMeas(&mgr, i + 300, &meas, 1000 + i));
	}

	return 0; // Great success
}

#define TEST_PARA_MAX CL_BOOL
static int testCloudLibAllType(void * arg)
{
	int i;

	CREATE_CL_Manager(mgr, 1, 10, "IoTpanel", "PanelIoTHub.azure-devices.net");

	CL_Device devices[1];
	CREATE_CL_ParameterList(paraList0, TEST_PARA_MAX);

	EXPECT_ZERO(CL_DeviceInit(&devices[0], "GUID0", "SN0", "FWVER0", "TYPE0"));

	CL_Parameter parameters[TEST_PARA_MAX];
	for (i = 0; i < TEST_PARA_MAX; i++)
	{
		char paraName[32];
		snprintf(paraName, 32, "testOutVoltage_%d", i);
		CL_ParameterInit(&parameters[i], paraName, i + 10 , CL_INT8 + i);
		EXPECT_ZERO(CL_ParameterListAddParam(&paraList0, &parameters[i]));
	}

	EXPECT_ZERO(CL_DeviceSetParameters(&devices[0], &paraList0));

	EXPECT_ZERO(CL_MgrSetDevice(&mgr, &devices[0], 0));

	for (i = 0; i < CL_BOOL; i++)
	{
		switch (i)
		{
			case CL_INT8: {
				int8_t meas = -127;
				CL_MgrAddMeas(&mgr, i + 10, &meas, i + 1000);
				break;
			}
			case CL_UINT8: {
				uint8_t meas = 255;
				CL_MgrAddMeas(&mgr, i + 10, &meas, i + 1000);
				break;
			}
			case CL_INT16: {
				int16_t meas = -32767;
				CL_MgrAddMeas(&mgr, i + 10, &meas, i + 1000);
				break;
			}
			case CL_UINT16: {
				uint16_t meas = 65535;
				CL_MgrAddMeas(&mgr, i + 10, &meas, i + 1000);
				break;
			}
			case CL_INT32: {
				int32_t meas = 32768;
				CL_MgrAddMeas(&mgr, i + 10, &meas, i + 1000);
				break;
			}
			case CL_UINT32: {
				uint32_t meas = 4294967295;
				CL_MgrAddMeas(&mgr, i + 10, &meas, i + 1000);
				break;
			}
			case CL_INT64: {
				int64_t meas = -42949672;
				CL_MgrAddMeas(&mgr, i + 10, &meas, i + 1000);
				break;
			}
			case CL_UINT64: {
				uint64_t meas = 1844674407370955161;
				CL_MgrAddMeas(&mgr, i + 10, &meas, i + 1000);
				break;
			}
			case CL_REAL32: {
				float meas = 2;
				CL_MgrAddMeas(&mgr, i + 10, &meas, i + 1000);
				break;
			}
			case CL_REAL64: {
				double meas = 1.234;
				CL_MgrAddMeas(&mgr, i + 10, &meas, i + 1000);
				break;
			}
		}
	}

	if (1)
	{
		char buffer[8086];
		EXPECT_POSITIVE(CL_MgrPush(&mgr, buffer, sizeof(buffer), CL_ABB_ABILITY_CREATE_TELEMETRICS));

		const char cmp[] =
			"[\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"testOutVoltage_0\",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:40+0800\",\n"
			"\t\t\t\"value\" : -127\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"testOutVoltage_1\",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:41+0800\",\n"
			"\t\t\t\"value\" : 255\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"testOutVoltage_2\",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:42+0800\",\n"
			"\t\t\t\"value\" : -32767\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"testOutVoltage_3\",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:43+0800\",\n"
			"\t\t\t\"value\" : 65535\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"testOutVoltage_4\",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:44+0800\",\n"
			"\t\t\t\"value\" : 32768\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"testOutVoltage_5\",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:45+0800\",\n"
			"\t\t\t\"value\" : 4294967295\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"testOutVoltage_6\",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:46+0800\",\n"
			"\t\t\t\"value\" : -42949672\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"testOutVoltage_7\",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:47+0800\",\n"
			"\t\t\t\"value\" : 1844674407370955161\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"testOutVoltage_8\",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:48+0800\",\n"
			"\t\t\t\"value\" : 2.000000\n"
			"\t\t},\n"
			"\t\t{\n"
			"\t\t\t\"objectId\": \"GUID0\",\n"
			"\t\t\t\"model\" : \"abb.ability.device\",\n"
			"\t\t\t\"variable\" : \"testOutVoltage_9\",\n"
			"\t\t\t\"timestamp\" : \"1970-01-01T08:16:49+0800\",\n"
			"\t\t\t\"value\" : 1.234000\n"
			"\t\t}]\n"
			"";

		int ret = strcmp(buffer, cmp);
		return ret;

	}

	return 0; // Great success
}

static UnitTest testCL[] =
{
#if 0
	{ testCLParamList, NULL, "comfirm ParameterList test" },
	{ testCLMgr, NULL, "verify CL Manager" },
#endif
	{ testCloudLib, NULL, "test meas validity" },
	{ testCloudLibData, NULL, "test data validity" },
	{ testCloudLibTwoDevs, NULL, "test single dev meas validity" },
	{ testCloudLibAllType , NULL, "test all data type" },
	{ NULL, NULL, NULL }
};

int CL_UnitTestDo(void)
{
	UnitTest *p = &testCL[0];

	for (;p->name != NULL; p++ )
	{
		int testValue = p->func(p->parama);
		if (testValue)
		{
			CONSOLE_PRINTER("CL UT FAILED %s == %d\n", p->name, testValue);
			failed++;
		}
		else
		{
			success++;
		}
	}

	CONSOLE_PRINTER("CL_UnitTestDo failed %d, success %d\n", failed, success);

	return failed;
}

//#define CL_CLOUD_LIB_UNIT_TESTS_MAIN
#ifdef CL_CLOUD_LIB_UNIT_TESTS_MAIN
int main() {
	CL_UnitTestDo();
	while (1);
}
#endif
// This not as good as Google test or something but it is still much better than no unit tests.
/*! @} */ /* EOF, no more */
