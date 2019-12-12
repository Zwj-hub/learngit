/*! @addtogroup azure iot
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
*                   Filename:   AzureConnection.h                             *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file 
 *  @par File description:
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
#ifndef AZURECONNECTION_INC  /* Allow multiple inclusions */
#define AZURECONNECTION_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------

#include <stdint.h>
#include "azure_prov_client/prov_device_ll_client.h"
#include "iothub_device_client_ll.h"


//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------

typedef enum CLIENT_CONNECTION_STAT_TAG
{
	DPS_DISCONNECTED = 0,
	IOTHUB_DISCONNECTED,
	IOTHUB_CONNECTED

}CLIENT_CONNECTION_STAT;

typedef struct AzureConnectStat_
{
	uint32_t sendEvtCounter;
	uint32_t sendEvtConfirmationCounter;
	uint32_t sendReportedCounter;
	uint32_t sendReportedComfirmationCounter;
	uint32_t recvMsgCBCounter;
	uint32_t conncetionStatCBCounter;
	uint32_t directMethodeCBCounter;
	uint32_t deviceTwinsCBCounter;

}AzureConnectStat_;

struct AzureConnectConf;

typedef struct AzureConnectConf
{
	uint16_t keepalive;
	bool logtrace;
	
	AzureConnectStat_ stats;

	const char* globalProvUrl;
	const char* idScope;

	void(*connected)(struct AzureConnectConf * conf);
	void(*disconnected)(struct AzureConnectConf * conf);
	int(*statRecv)(struct AzureConnectConf * conf, const char * message);
	int(*evtRecv)(struct AzureConnectConf * conf, IOTHUB_MESSAGE_HANDLE message); // return 0 if message was used. 1 if not used
	int(*methodRecv)(struct AzureConnectConf * azure, const char* method_name, const unsigned char* payLoad, size_t size, unsigned char** response, size_t* resp_size);

	char* iothub_uri;
	char* device_id;
	IOTHUB_DEVICE_CLIENT_LL_HANDLE iotHubClientHandle;
	CLIENT_CONNECTION_STAT connectionStat;
	PROV_DEVICE_LL_HANDLE dpsHandle;

}AzureConnectConf;


//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

int AzureConnectInit(AzureConnectConf *AzureConf);
void AzureClientRefresh(AzureConnectConf *AzureConf);
void AzureConnectDeinit(AzureConnectConf *AzureConf);
bool AzureClientGetSendMsgStat(AzureConnectConf *AzureConf);
int AzureClientSendMessageHandle(AzureConnectConf *AzureConf, IOTHUB_MESSAGE_HANDLE message);
int AzureClientConnectStat(AzureConnectConf *AzureConf);
int AzureClinetDeviceTwinsDisable(AzureConnectConf *AzureConf);

#endif /* of AZURECONNECTION_INC */
/*! @} */ /* EOF, no more */
