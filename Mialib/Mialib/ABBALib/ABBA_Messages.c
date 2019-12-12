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
*                   Filename:   ABBA_Messages.c                               *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Christer Ekholm                               *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
*  @file ABBA_Messages.c
*  @par File description:
*     info model msg generation and debug
*
*  @par Related documents:
*
*  @note
*/
/******************************************************************************/

#include "ABBALib.h"
#include "azure_c_shared_utility/map.h"
#include "AzureOSConfig.h"

static char s_debugOutput;
extern void DLOG_vPrintF(char * format, ...);

#define ABBA_PR(...) DLOG_vPrintF(__VA_ARGS__)

IOTHUB_MESSAGE_HANDLE ABBA_MsgCreateTelemetrics(const char *jsonPayload)
{
	IOTHUB_MESSAGE_HANDLE out = IoTHubMessage_CreateFromByteArray((const unsigned char*)jsonPayload, strlen(jsonPayload)+1);
	//IOTHUB_MESSAGE_HANDLE out = IoTHubMessage_CreateFromString(jsonPayload);

	MAP_HANDLE handle = IoTHubMessage_Properties(out);
	Map_Add(handle, ABBA_KEY_MSG_TYPE, ABBA_VAL_MSG_TIMESERIE);

	return out;
}

void ABBA_MsgPrintMsg(IOTHUB_MESSAGE_HANDLE msg)
{
	char formatBuffer[32];
	const unsigned char* buffer;
	size_t size;

	STRING_HANDLE str = Map_ToJSON(IoTHubMessage_Properties(msg));

	ABBA_PR("Message parameters\n%s\nPayload\n", (str && STRING_c_str(str)) ? STRING_c_str(str) : ""); // Ce trust that

	IOTHUB_MESSAGE_RESULT result = IoTHubMessage_GetByteArray(msg, &buffer, &size);

	if(result == IOTHUB_MESSAGE_OK)
	{
		sprintf(formatBuffer, "%%.%lus\nlen:%lu\n", (unsigned long)size, (unsigned long)size);
		ABBA_PR(formatBuffer, buffer);
	}
	else
	{
		const char * data = IoTHubMessage_GetString(msg);
		if(data)
		{
			ABBA_PR("%s\nlen:%d\n", data, strlen(data));
		}
		else
		{
			ABBA_PR("Payload error\n");
		}
	}
}

int  ABBA_MsgDebug(IOTHUB_MESSAGE_HANDLE msg, ABBA_Debug_Control control)
{
	if(control == ABBA_CTRL_MSG)
	{
		int ret = s_debugOutput;
		s_debugOutput = !!msg;
		return ret;
	}
	else if(s_debugOutput)
	{
		ABBA_PR("%s message\n", control == ABBA_TX_MSG ? "TX" : "RX");
		ABBA_MsgPrintMsg(msg);
	}

	return 0;
}

