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
*                   Filename:   ABBA_EventMsg.c                               *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
*  @file ABBA_EventMsg.c
*  @par File description:
*     event msg handle
*
*  @par Related documents:
*
*  @note
*/
/******************************************************************************/

#include "ABBALib.h"
#include "ABBA_EventMsg.h"
#include <time.h>
#include <stdlib.h>

static uint32_t getEvtId(const char *buff)
{
	int i;
	char evtId[16];
	int ret = 25855; // "fault reset(0x64FF)"
	char *pData = strstr(buff, "Base:0X");

	if (pData == NULL) return ret;
	
	pData += sizeof("Base");
	for (i = 0; i < sizeof(evtId); i++)
	{
		if (*pData == ' ') break;
		evtId[i] = *pData++;
	}
	return strtoul(evtId, NULL, 16);
}

static int getEvtMsg(ABBA_Data * This, char *buffer, int size, const char *evtMsg, const char *srcMsg, const char *severity, char *devTimestamp)
{
	char timeStr[180];
	time_t timeNow = time(NULL);

	strftime(timeStr, sizeof(timeStr), "%FT%T%z", localtime(&timeNow));

	return snprintf(
		buffer,
		size,
		"[{\n"
		"\t\"" ABBA_KEY_OBJ_ID "\": \"%s\",\n"
		"\t\"" ABBA_KEY_MODEL "\": \"" ABBA_VAL_MODEL_DEV "\",\n"
		"\t\"" ABBA_KEY_TIMESTAMP "\": \"%s\",\n"
		"\t\"" ABBA_KEY_EVENT "\": \"" ABBA_VAL_EVT_DEVEVT "\",\n"
		"\t\"" ABBA_KEY_VAL "\": {\n"
		"\t\t\"" ABBA_KEY_EVENTVAL_ID "\" : %u,\n"
		"\t\t\"" ABBA_KEY_EVENTVAL_MSG "\" : \"%s\",\n"
		"\t\t\"" ABBA_KEY_EVENTVAL_SRC "\" : \"%s\",\n"
		"\t\t\"" ABBA_KEY_EVENTVAL_SEVERITY "\" : \"%s\",\n"
		"\t\t\"" ABBA_KEY_EVENTVAL_DEVTIMESTAMP "\" : \"%s\"\n"
		"\t}\n"
		"}]\n"
		,
		"4320ca19-f5c9-4d53-85f4-d2535f5a9ab9",//"69c13190-68a3-4daf-8ee4-0a8e677b345e",//zwjThis->ABB_Drive_Id[0],
		timeStr,
		getEvtId(evtMsg),
		evtMsg,
		srcMsg,
		severity,
		devTimestamp
	);
}

int ABBA_EventMsg(ABBA_Data * This, IOTHUB_MESSAGE_HANDLE *out, const char *evtMsg, const char *srcMsg, const char *severity, const struct tm *devTimestamp)
{
	char outData[1024];
	char timeStr[180];

	if (!This->gwSerialNumber)
		return -ABBA_E_MISSING_SERIAL_NUMBER;

	if (!This->hubId)
		return -ABBA_E_MISSING_GW_ID;

	if (!This->hubName)
		return -ABBA_E_MISSING_HUB_NAME;//zwj

	strftime(timeStr, sizeof(timeStr), "%FT%T%z", devTimestamp);

	getEvtMsg(This, outData, sizeof(outData), evtMsg, srcMsg, severity, timeStr);

	*out = IoTHubMessage_CreateFromByteArray((const unsigned char*)outData, strlen(outData) + 1);

	MAP_HANDLE mHandle = IoTHubMessage_Properties(*out);

	Map_Add(mHandle, ABBA_KEY_MSG_TYPE, ABBA_VAL_MSG_EVT);
	return ABBA_E_OK;
}
