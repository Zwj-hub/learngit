/*! @addtogroup azure iot port
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
*                  Subsystem:   azure iot port                                *
*                                                                             *
*                   Filename:   DPSConnection.c                               *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
*  @file DPSConnection.c
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
#include "AzureConnection.h"
#include "AzureOSConfig.h"
#include "iothub_device_client_ll.h"
#include "iothub_client_options.h"
#include "iothubtransportmqtt.h"
#include "azure_prov_client/prov_device_ll_client.h"
#include "azure_prov_client/prov_security_factory.h"
#include "azure_prov_client/iothub_security_factory.h"
#include "azure_prov_client/prov_transport_mqtt_client.h"
#include "azure_c_shared_utility/xlogging.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------------------------
// 2) LOCAL ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------
MU_DEFINE_ENUM_STRINGS(PROV_DEVICE_RESULT, PROV_DEVICE_RESULT_VALUE);

MU_DEFINE_ENUM_STRINGS(PROV_DEVICE_REG_STATUS, PROV_DEVICE_REG_STATUS_VALUES);

//-----------------------------------------------------------------------------
// 3) LOCAL DATA STRUCTURES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLES
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// 5) GLOBAL VARIABLES
//-----------------------------------------------------------------------------
char g_Azure_debug_level;

//-----------------------------------------------------------------------------
// 6) LOCAL FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 7) FUNCTIONS
//-----------------------------------------------------------------------------

static void AzureConnectStatInit(AzureConnectConf *AzureConf)
{
	memset(&AzureConf->stats, 0, sizeof(AzureConf->stats));
}

static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMsgCallback(IOTHUB_MESSAGE_HANDLE message, void* user_context)
{
	(void)message;
	AzureConnectConf *AzureConf = (AzureConnectConf*)user_context;

	AZURE_PRINTF_TRACE("Message recieved from IoTHub\r\n");

	if(AzureConf->evtRecv)
	{
		AzureConf->evtRecv(AzureConf, message);
	}

	AzureConf->stats.recvMsgCBCounter++;
	return IOTHUBMESSAGE_ACCEPTED;
}

static void DeviceTwinGetStateCallback(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size, void* user_context)
{
	AzureConnectConf *AzureConf = (AzureConnectConf*)user_context;
	AZURE_PRINTF_TRACE("Device Twins State recieved from IoTHub\r\n");

	if (AzureConf->statRecv)
	{
		AzureConf->statRecv(AzureConf, (const char*) payLoad);
	}

	AzureConf->stats.deviceTwinsCBCounter++;
    AZURE_PRINTF_TRACE("Device Twin properties received: update=%s payload=%s, size=%zu\n", MU_ENUM_TO_STRING(DEVICE_TWIN_UPDATE_STATE, update_state), payLoad, size);
}

static int DeviceMethodCallback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* user_context)
{
	int result = 0;
	AzureConnectConf *AzureConf = (AzureConnectConf*)user_context;
	AZURE_PRINTF_TRACE("Device Method recieved from IoTHub\r\n");

	if (AzureConf->methodRecv)
	{
		result = AzureConf->methodRecv(AzureConf, method_name, payload, size, response, resp_size);
	}

	AzureConf->stats.directMethodeCBCounter++;
	AZURE_PRINTF_TRACE("Device Method received: method name=%s, payload=%s, size=%zu\n", method_name, payload, size);
	return result;
}

static void RegistationStatusCallback(PROV_DEVICE_REG_STATUS reg_status, void* user_context)
{
	//AzureConnectConf *AzureConf = (AzureConnectConf*)user_context;

    AZURE_PRINTF_STATE("Provisioning Status: %s\r\n", MU_ENUM_TO_STRING(PROV_DEVICE_REG_STATUS, reg_status));
}

static void IothubConnectionStatus(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
{
	(void)reason;
	AzureConnectConf *AzureConf = (AzureConnectConf*)user_context;

	AzureConf->stats.conncetionStatCBCounter++;
	if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
	{
		AzureConf->connectionStat = IOTHUB_CONNECTED;

		if (AzureConf->connected)
		{
			AzureConf->connected(AzureConf);
		}
	}

	if (IOTHUB_CONNECTED == AzureConf->connectionStat && reason != IOTHUB_CLIENT_CONNECTION_OK)
	{
		AzureConf->connectionStat = IOTHUB_DISCONNECTED;
		if (AzureConf->disconnected)
		{
			AzureConf->disconnected(AzureConf);
		}
	}

}

static void RegisterDeviceCallback(PROV_DEVICE_RESULT register_result, const char* iothub_uri, const char* device_id, void* user_context)
{
	if(!user_context || !device_id || !iothub_uri)
	{
		AZURE_PRINTF_ERR("RegisterDeviceCallback parameter null\r\n");
		return;
	}
	
	AzureConnectConf* AzureConf = (AzureConnectConf*)user_context;
	if (register_result == PROV_DEVICE_RESULT_OK)
	{
		AZURE_PRINTF_STATE("Registration Information received from service: %s!\r\n", iothub_uri);
		mallocAndStrcpy_s(&AzureConf->iothub_uri, iothub_uri);
		mallocAndStrcpy_s(&AzureConf->device_id, device_id);

		AzureConf->connectionStat = IOTHUB_DISCONNECTED;

		AZURE_PRINTF_STATE("Creating IoTHub Device handle\r\n");
		if ((AzureConf->iotHubClientHandle = IoTHubDeviceClient_LL_CreateFromDeviceAuth(AzureConf->iothub_uri, AzureConf->device_id, MQTT_Protocol)) == NULL)
		{
			AZURE_PRINTF_ERR("failed create IoTHub client from connection string %s!\r\n", AzureConf->iothub_uri);
		}
		else
		{
			IoTHubDeviceClient_LL_SetConnectionStatusCallback(AzureConf->iotHubClientHandle, IothubConnectionStatus, AzureConf);
			
			IoTHubDeviceClient_LL_SetOption(AzureConf->iotHubClientHandle, OPTION_LOG_TRACE, &AzureConf->logtrace);

			IoTHubDeviceClient_LL_SetMessageCallback(AzureConf->iotHubClientHandle, ReceiveMsgCallback, AzureConf);

			IoTHubDeviceClient_LL_SetDeviceMethodCallback(AzureConf->iotHubClientHandle, DeviceMethodCallback, AzureConf);

			IoTHubDeviceClient_LL_SetDeviceTwinCallback(AzureConf->iotHubClientHandle, DeviceTwinGetStateCallback, AzureConf);
		}
	}
	else
	{
		AZURE_PRINTF_ERR("Failure encountered on registration!\r\n");
	}
}

static void SendEvtConfirmationCB(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* user_context)
{
	AzureConnectConf *AzureConf = (AzureConnectConf*)user_context;

	AzureConf->stats.sendEvtConfirmationCounter++;

	AZURE_PRINTF_TRACE("-->sendEvt conter %d confirmation status CallBack [%d]: Status_code = %d\r\n", AzureConf->stats.sendEvtCounter, AzureConf->stats.sendEvtConfirmationCounter, result);
}

static void AzureConnectLogger(LOG_CATEGORY log_category, const char* file, const char* func, int line, unsigned int options, const char* format, ...)
{
	va_list args;
	char printBuffer[80];
	char noDebug = 1;
#ifdef MIA_DEBUG
	noDebug = 0;
#endif

	if ((char)log_category + 1 > g_Azure_debug_level || noDebug)
	{
		return;
	}

	va_start(args, format);

	switch (log_category)
	{
	case AZ_LOG_INFO:
		_AZURE_PRINTF_MSG_(0, "Info: ");
		break;
	case AZ_LOG_ERROR:
	{
		time_t t = time(NULL);
		_AZURE_PRINTF_MSG_(0, "Error: Time:%.24s File:%s Func:%s Line:%d ", ctime(&t), file, func, line);

		break;
	}
	default:
		break;
	}

	unsigned int len = vsnprintf(printBuffer, sizeof(printBuffer), format, args);
	_AZURE_PRINTF_MSG_(0, "%s", printBuffer);

	if (len >= sizeof(printBuffer) - 1) // was line cut propably
	{
		options |= LOG_LINE;
	}

	va_end(args);

	if (options & LOG_LINE)
	{
		_AZURE_PRINTF_MSG_(0, "\n");
	}
}

void AzureClientRefresh(AzureConnectConf *AzureConf)
{
	if (AzureConf->connectionStat < IOTHUB_DISCONNECTED)
	{
		if (AzureConf->dpsHandle && AzureConf->connectionStat == DPS_DISCONNECTED)
		{
			Prov_Device_LL_DoWork(AzureConf->dpsHandle);
		}
	}
	else 
	{
		if (AzureConf->dpsHandle) {
			Prov_Device_LL_Destroy(AzureConf->dpsHandle);
			AzureConf->dpsHandle = NULL;
		}
		
		if (AzureConf->iotHubClientHandle) 
		{
			IoTHubDeviceClient_LL_DoWork(AzureConf->iotHubClientHandle);
		}
	}
}

int AzureClientConnectStat(AzureConnectConf *AzureConf)
{
	return AzureConf->connectionStat == IOTHUB_CONNECTED; // return !0 when connected
}

void AzureConnectDeinit(AzureConnectConf *AzureConf)
{
	if (AzureConf->device_id) 
	{
		free(AzureConf->device_id);
		AzureConf->device_id = NULL;
	}
	if (AzureConf->iothub_uri) 
	{
		free(AzureConf->iothub_uri);
		AzureConf->iothub_uri = NULL;
	}
	if (AzureConf->dpsHandle) 
	{
		Prov_Device_LL_Destroy(AzureConf->dpsHandle);
		AzureConf->dpsHandle = NULL;
	}
	if (AzureConf->iotHubClientHandle) 
	{
		IoTHubDeviceClient_LL_Destroy(AzureConf->iotHubClientHandle);
		AzureConf->iotHubClientHandle = NULL;
	}
	AzureConnectStatInit(AzureConf);
}

bool AzureClientGetSendMsgStat(AzureConnectConf *AzureConf)
{
	if (AzureConf->stats.sendEvtCounter == AzureConf->stats.sendEvtConfirmationCounter) 
	{
		return true;
	}
	else 
	{
		return false;
	}
}

int AzureClientSendMessageHandle(AzureConnectConf *AzureConf, IOTHUB_MESSAGE_HANDLE message)
{
	if (!AzureConf->iotHubClientHandle)
	{
		return -1;
	}

	if (message == NULL)
	{
		AZURE_PRINTF_ERR("unable to create a new IoTHubMessage\r\n");
		return -2;
	}

	if (IoTHubDeviceClient_LL_SendEventAsync(AzureConf->iotHubClientHandle, message, SendEvtConfirmationCB, AzureConf) != IOTHUB_CLIENT_OK)
	{
		AZURE_PRINTF_ERR("failed to hand over the message to IoTHubClient");
	}
	else
	{
		AZURE_PRINTF_TRACE("IoTHubClient accepted the message for delivery\r\n");
		AzureConf->stats.sendEvtCounter++;
	}
	// The message is copied to the sdk so the we can destroy it
	IoTHubMessage_Destroy(message);
	return 0;
}

int DPSConnectInit(AzureConnectConf *AzureConf)
{
	(void)prov_dev_security_init(SECURE_DEVICE_TYPE_X509);

	AzureConf->connectionStat = DPS_DISCONNECTED;

	(void)AzureConnectLogger; // Avoid warning about non used function if xlogging_set_log_function is defined empty
	xlogging_set_log_function(AzureConnectLogger);


	if ((AzureConf->dpsHandle = Prov_Device_LL_Create(AzureConf->globalProvUrl, AzureConf->idScope, Prov_Device_MQTT_Protocol)) == NULL)
	{
		AZURE_PRINTF_ERR("failed calling Prov_Device_LL_Create\r\n");
		return -1;
	}

	Prov_Device_LL_SetOption(AzureConf->dpsHandle, PROV_OPTION_LOG_TRACE, &AzureConf->logtrace);

	if (Prov_Device_LL_Register_Device(AzureConf->dpsHandle, RegisterDeviceCallback, AzureConf, RegistationStatusCallback, AzureConf) != PROV_DEVICE_RESULT_OK)
	{
		AZURE_PRINTF_ERR("failed calling Prov_Device_LL_Register_Device\r\n");
		return -1;
	}

	return 0;
}

int IoTHubConnectInit(AzureConnectConf *AzureConf)
{
	char connStr[256];
	iothub_security_init(IOTHUB_SECURITY_TYPE_X509);
	AzureConf->connectionStat = IOTHUB_DISCONNECTED;
   // const char* connectstring="HostName=drives-rcm-int-cn-ih.azure-devices.cn;DeviceId=test_http;SharedAccessKey=U5a/e9JMjSvPa2l4QCtIXDavfCC6FoCc/vqXNL2S7Cg=";//zwj

    snprintf(connStr, sizeof(connStr), "HostName=%s;DeviceId=%s;UseProvisioning=true", AzureConf->iothub_uri, AzureConf->device_id);
	AZURE_PRINTF_STATE("Creating IoTHub Device handle\r\n");

    if ((AzureConf->iotHubClientHandle = IoTHubDeviceClient_LL_CreateFromConnectionString(connStr, MQTT_Protocol)) == NULL)
   // if ((AzureConf->iotHubClientHandle = IoTHubDeviceClient_LL_CreateFromConnectionString(connectstring, MQTT_Protocol)) == NULL)//zwj
	{
		AZURE_PRINTF_ERR("failed create IoTHub client from connection string %s!\r\n", AzureConf->iothub_uri);
		return -1;
	}
	else
	{
		IoTHubDeviceClient_LL_SetConnectionStatusCallback(AzureConf->iotHubClientHandle, IothubConnectionStatus, AzureConf);

		IoTHubDeviceClient_LL_SetOption(AzureConf->iotHubClientHandle, OPTION_LOG_TRACE, &AzureConf->logtrace);

		IoTHubDeviceClient_LL_SetMessageCallback(AzureConf->iotHubClientHandle, ReceiveMsgCallback, AzureConf);

		IoTHubDeviceClient_LL_SetDeviceMethodCallback(AzureConf->iotHubClientHandle, DeviceMethodCallback, AzureConf);

		IoTHubDeviceClient_LL_SetDeviceTwinCallback(AzureConf->iotHubClientHandle, DeviceTwinGetStateCallback, AzureConf);
	}
	return 0;
}

int AzureConnectInit(AzureConnectConf *AzureConf)
{
	if ((AzureConf->iothub_uri!=NULL && AzureConf->iothub_uri[0]) && (AzureConf->device_id != NULL && AzureConf->device_id[0]))
	{
		return IoTHubConnectInit(AzureConf);
	}
	else 
	{
		return  DPSConnectInit(AzureConf);
	}
}

int AzureClinetDeviceTwinsDisable(AzureConnectConf *AzureConf)
{
	IoTHubDeviceClient_LL_SetDeviceTwinCallback(AzureConf->iotHubClientHandle, NULL, NULL);
	return 0;
}

/*! @} */ /* EOF, no more */
