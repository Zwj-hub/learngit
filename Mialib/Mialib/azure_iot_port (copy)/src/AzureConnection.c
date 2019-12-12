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
*                  Subsystem:   azure iot                                     *
*                                                                             *
*                   Filename:   AzureConnection.c                             *
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
#if USE_CLOUDLIB_AZURE && USE_IOTHUB

#define AZURECONNECTION

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include "AzureOSConfig.h"
#include "AzureConnection.h"
#include "iothub_message.h"
#include "iothubtransportmqtt.h"
#if 0
#include "certificates.h"
#endif


//-----------------------------------------------------------------------------
// 2) LOCAL ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 3) LOCAL DATA STRUCTURES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLES
//-----------------------------------------------------------------------------
static AzureConnectStat AzureStat;

//-----------------------------------------------------------------------------
// 5) GLOBAL VARIABLES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 6) LOCAL FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

static void AzureConnectStatInit(void);
static void DeviceTwinStatusCB(DEVICE_TWIN_UPDATE_STATE status_code, const unsigned char* payload, size_t size, void* userContextCallback);
static void SendEvtConfirmationCB(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback);
static void ConnectionStatCB(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* userContextCallback);
static IOTHUBMESSAGE_DISPOSITION_RESULT RecvMsgCB(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback);
#if 0
static void SendReportedConfirmationCB(int status_code, void* userContextCallback);
#endif

//-----------------------------------------------------------------------------
// 7) FUNCTIONS
//-----------------------------------------------------------------------------

static void AzureConnectStatInit(void) 
{
	AzureStat.sendEvtCounter = 0;
	AzureStat.sendEvtConfirmationCounter = 0;
	AzureStat.sendReportedCounter = 0;
	AzureStat.sendReportedComfirmationCounter = 0;
	AzureStat.recvMsgCBCounter = 0;
	AzureStat.conncetionStatCBCounter = 0;
	AzureStat.directMethodeCBCounter = 0;
	AzureStat.deviceTwinsCBCounter = 0;
	AzureStat.connectionStat = false;
}

#if 0
static void SendReportedConfirmationCB(int status_code, void* userContextCallback)
{
	AzureStat.sendReportedComfirmationCounter++;
	AZURE_PRINTF("-->repoted status CallBack [%d]: Status_code = %d\r\n", AzureStat.sendReportedComfirmationCounter, status_code);
}
#endif

static IOTHUBMESSAGE_DISPOSITION_RESULT RecvMsgCB(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
	IOTHUBMESSAGE_DISPOSITION_RESULT result;
	const unsigned char* buffer;
	size_t size;
	if (IoTHubMessage_GetByteArray(message, &buffer, &size) != IOTHUB_MESSAGE_OK) {
		result = IOTHUBMESSAGE_ABANDONED;
		AZURE_PRINTF("unable to retrieve the message data, ret %d\r\n", result);
	}
	else if(size < 1024) {
		/*buffer is not zero terminated*/
		AZURE_PRINTF("Received Message with Data: <<<%.*s>>> & Size=%d\r\n", (int)size, buffer, (int)size);
		result = IOTHUBMESSAGE_ACCEPTED;
	}
	else
	{
		AZURE_PRINTF("Received Message with Data: <<<%.*s>>> & Size=%d. Message Too Large!!\r\n", (int)size, buffer, (int)size);
		return IOTHUBMESSAGE_REJECTED;
	}
	return result;
}

static void SendEvtConfirmationCB(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
	AzureStat.sendEvtConfirmationCounter++;
	AZURE_PRINTF("-->sendEvt conter %d confirmation status CallBack [%d]: Status_code = %d\r\n", AzureStat.sendEvtCounter ,AzureStat.sendEvtConfirmationCounter, result);
}

static void DeviceTwinStatusCB(DEVICE_TWIN_UPDATE_STATE status_code,  const unsigned char* payload, size_t size, void* userContextCallback)
{
	AzureStat.deviceTwinsCBCounter++;
	AZURE_PRINTF("DeviceTwinCallbackStatus[%d] length %d Method payload: %s\r\nStatus_code = %d\r\n", AzureStat.deviceTwinsCBCounter, size,(const char*)payload,status_code);
}

static void ConnectionStatCB(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* userContextCallback)
{
	AzureStat.conncetionStatCBCounter++;
	if (reason == IOTHUB_CLIENT_CONNECTION_OK) {
		AzureStat.connectionStat = true;
	}
	else {
		AzureStat.connectionStat = false;
	}
	AZURE_PRINTF("-->conncetion status CallBack [%d]: Status_code = %d and Reasion = %d\r\n", AzureStat.conncetionStatCBCounter, result, reason);
}

int AzureConnectInit(AzureConnectConf *AzureConf)
{
	AzureConnectStatInit();

	if((AzureConf->iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(AZUREDEVICECONNECTIONSTRING, MQTT_Protocol)) == NULL){
		AZURE_PRINTF(" iotHubClientHandle is NULL!\r\n");
		AZURE_PRINTF("\tchange AZUREDEVICECONNECTIONSTRING on Inc/AzureOSConfig.h file\r\n");
		return -1;
	}
	else {
		AZURE_PRINTF("iotHubClientHandle Created\r\n");
	}
	
	if(IoTHubClient_LL_SetOption(AzureConf->iotHubClientHandle, "logtrace", &(AzureConf->logtrace))==IOTHUB_CLIENT_OK) {
		AZURE_PRINTF("iotHubClientSetOption logtrace\r\n");
	}
	if(IoTHubClient_LL_SetOption(AzureConf->iotHubClientHandle, "keepalive", &(AzureConf->keepalive))==IOTHUB_CLIENT_OK) {
		AZURE_PRINTF("iotHubClientSetOption keepalive \r\n");
	}
#if 0
	/* Load certificates */
	if (IoTHubClient_LL_SetOption(AzureConf->iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK) {
		AZURE_PRINTF("IoTHubClient_LL_SetOption(certificates)..........FAILED!\r\n");
		return -1;
	}
	else {
		AZURE_PRINTF("IoTHubClient_LL_SetOption(certificates)...OK.\r\n");
	}
#endif
	/* Setting Message call back, so we can receive Commands. */
	if (IoTHubClient_LL_SetMessageCallback(AzureConf->iotHubClientHandle, RecvMsgCB, NULL) != IOTHUB_CLIENT_OK){
		AZURE_PRINTF( " IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
		return -1;
	} else {
		AZURE_PRINTF("IoTHubClient_LL_SetMessageCallback..OK.\r\n");
	}

	if (IoTHubClient_LL_SetDeviceTwinCallback(AzureConf->iotHubClientHandle, DeviceTwinStatusCB, NULL) != IOTHUB_CLIENT_OK) {
		AZURE_PRINTF("IoTHubClient_LL_SetDeviceTwinCallback..........FAILED!\r\n");
		return -1;
	}
	else {
		AZURE_PRINTF("IoTHubClient_LL_SetDeviceTwinCallback...OK.\r\n");
	}

	if (IoTHubClient_LL_SetConnectionStatusCallback(AzureConf->iotHubClientHandle, ConnectionStatCB, NULL) != IOTHUB_CLIENT_OK) {
		AZURE_PRINTF("IoTHubClient_LL_SetConnectionStatCallback..........FAILED!\r\n");
		return -1;
	}
	else {
		AZURE_PRINTF("IoTHubClient_LL_SetConnectionStatCallback...OK.\r\n");
	}
#if 0
	if(IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, Azure1) != IOTHUB_CLIENT_OK){
		AZURE_PRINTF("IoTHubClient_LL_SetDeviceMethodCallback..........FAILED!\r\n");
		AzureExit();
	} else {
	AZURE_PRINTF( "IoTHubClient_LL_SetDeviceMethodCallback...OK.\r\n");
	}
#endif
	return 0;
}

/******************************************************************************
*                                                                            *
*  Function:
* @brief  deinit MQTT application for Telemetry and Device Management
* @param  None
* @retval None
*                                                                            *
******************************************************************************/
void AzureConnectDeinit(AzureConnectConf *AzureConf)
{
	if (AzureConf->iotHubClientHandle)
	{
		IoTHubClient_LL_Destroy(AzureConf->iotHubClientHandle);
		AZURE_PRINTF("iotHubClientHandle Destroied\r\n");
	}
	AzureConf->iotHubClientHandle = NULL;
}

/******************************************************************************
*                                                                            *
*  Function:
* @brief  refrsh MQTT application for Telemetry and Device Management
* @param  None
* @retval None
*                                                                            *
******************************************************************************/
void AzureClientRefresh(AzureConnectConf *AzureConf)
{
	if (AzureConf->iotHubClientHandle)
	{
		IoTHubClient_LL_DoWork(AzureConf->iotHubClientHandle);
	}
}

int AzureClientConnectStat(AzureConnectConf *AzureConf)
{
	return AzureStat.connectionStat == IOTHUB_CONNECTED; // return !0 when connected
}

void AzureClientSendMessage(AzureConnectConf *AzureConf, const char* buffer, size_t size)
{
	if (!AzureConf->iotHubClientHandle)
	{
		return;
	}

	IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray((unsigned char *)buffer, size);
	if (messageHandle == NULL)
	{
		AZURE_PRINTF("unable to create a new IoTHubMessage\r\n");
	}
	else
	{
		if (IoTHubClient_LL_SendEventAsync(AzureConf->iotHubClientHandle, messageHandle, SendEvtConfirmationCB, NULL) != IOTHUB_CLIENT_OK)
		{
			AZURE_PRINTF("failed to hand over the message to IoTHubClient");
		}
		else
		{
			AZURE_PRINTF("IoTHubClient accepted the message for delivery\r\n");
			AzureStat.sendEvtCounter++;
		}
		IoTHubMessage_Destroy(messageHandle);
	}
}

bool AzureClientGetSendMsgStat() 
{
	if (AzureStat.sendEvtCounter == AzureStat.sendEvtConfirmationCounter) {
		return true;
	}
	else {
		return false;
	}
}

#endif
/*! @} */ /* EOF, no more */
