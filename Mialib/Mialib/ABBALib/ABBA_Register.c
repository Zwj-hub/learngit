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
*                   Filename:   ABBA_Register.c                               *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Christer Ekholm                               *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
*  @file ABBA_Register.c
*  @par File description:
*    device provisioning info model state machine
*
*  @par Related documents:
*
*  @note
*/
/******************************************************************************/

#include "ABBALib.h"
#include "ABBA_Configuration.h"
#include "ABBA_PanelInfo.h"

#include "azure_c_shared_utility/map.h"
#include "parson.h"

static IOTHUB_MESSAGE_HANDLE createFromString(ABBA_Data * This, const char *outData)
{
	//IoTHubMessage_CreateFromByteArray(const unsigned char* byteArray, size_t size)
	//return IoTHubMessage_CreateFromString(outData);
	return IoTHubMessage_CreateFromByteArray((const unsigned char*)outData, strlen(outData) + 1);
}

static int parseJsonStr(IOTHUB_MESSAGE_HANDLE *msg, const char *key, char *val, size_t valSize)
{
	JSON_Value* root_value;
	JSON_Object* root_object;
	const unsigned char* buffer;
	size_t size;
	const char * jsonVal;
	int ret = ABBA_E_OK;
	
	//payload
	if (IOTHUB_MESSAGE_OK != IoTHubMessage_GetByteArray(*msg, &buffer, &size))
	{
		ret =  -ABBA_E_SERVER_ERROR;
	}
	else
	{
		if ((root_value = json_parse_string((const char*)buffer)) == NULL)
		{
			ret = -ABBA_E_SERVER_ERROR;
		}
		else if ((root_object = json_value_get_object(root_value)) == NULL)
		{
			json_value_free(root_value);
			ret = -ABBA_E_SERVER_ERROR;
		}
		else
		{
			jsonVal = json_object_dotget_string(root_object, key);
			if (jsonVal == NULL)
			{
				ret = -ABBA_E_SERVER_ERROR;
			}
			else 
			{
				strncpy(val, jsonVal, valSize);
				val[valSize-1] = '\0';
			}
			json_object_clear(root_object);
			json_value_free(root_value);
		}
	}
	return ret;
}

static int compareMsgHeader(MAP_HANDLE mHandle, const char *key, const char *val)
{
	const char *data = Map_GetValueFromKey(mHandle, key);
	if (!strncmp(val, data, strlen(val)))
	{
		return ABBA_E_OK;
	}
	return -ABBA_E_SERVER_ERROR;
}

static int compareMsgContent(IOTHUB_MESSAGE_HANDLE *msg, const char *key, const char *val)
{
	char jsonVal[64];

	if (!parseJsonStr(msg, key, jsonVal, sizeof(jsonVal)))
	{
		if (!strncmp(val, jsonVal, strlen(val)))
		{
			return ABBA_E_OK;
		}
	}
	return -ABBA_E_SERVER_ERROR;
}

static int parseGwRegEvtJson(ABBA_Data * This, IOTHUB_MESSAGE_HANDLE *recvmsg)
{
	int ret = ABBA_E_OK;
	const char *data = NULL;

	MAP_HANDLE mHandle = IoTHubMessage_Properties(*recvmsg);

	//"ability-messagetype"	== "platformEvent"
	if (compareMsgHeader(mHandle, ABBA_KEY_MSG_TYPE, ABBA_VAL_MSG_PLATFROM_EVT))
	{
		return -ABBA_E_SERVER_ERROR;
	}
	//"model" == "abb.ability.device"
	if (compareMsgHeader(mHandle, ABBA_KEY_MODEL, ABBA_VAL_MODEL_DEV))
	{
		return -ABBA_E_SERVER_ERROR;
	}
	//check panel type
	if (compareMsgContent(recvmsg, ABBA_KEY_TYPE, ABBA_VAL_GW_PANEL_TYPE))
	{
		return -ABBA_E_SERVER_ERROR;
	}
	//check panel sn
	if (compareMsgContent(recvmsg, ABBA_JSON_KEY_GW_SN, This->gwSerialNumber))
	{
		return -ABBA_E_SERVER_ERROR;
	}

	data = Map_GetValueFromKey(mHandle, ABBA_KEY_OBJ_ID);
	strncpy(This->ABB_GW_Id, data, sizeof(This->ABB_GW_Id));
	This->ABB_GW_Id[sizeof(This->ABB_GW_Id) - 1] = '\0';
	return ret;
}

static int parseGwCfgEvtJson(ABBA_Data * This, IOTHUB_MESSAGE_HANDLE *recvmsg)
{
	int ret = ABBA_E_OK;
	MAP_HANDLE mHandle = IoTHubMessage_Properties(*recvmsg);
	
	if (!This->ABB_GW_Id[0])
	{
		return -ABBA_E_SERVER_ERROR;
	}
	//"model" == "abb.ability.configuration"
	if (compareMsgHeader(mHandle, ABBA_KEY_MODEL, ABBA_VAL_MODEL_CFG))
	{
		return -ABBA_E_SERVER_ERROR;
	}
	//"ability-messagetype"	== "platformEvent"
	if (compareMsgHeader(mHandle, ABBA_KEY_MSG_TYPE, ABBA_VAL_MSG_PLATFROM_EVT))
	{
		return -ABBA_E_SERVER_ERROR;
	}
	//"objectid" == "xxxxxxx"
	if (compareMsgHeader(mHandle, ABBA_KEY_OBJ_ID, This->ABB_GW_Id))
	{
		return -ABBA_E_SERVER_ERROR;
	}
	//check panel sm
	if (compareMsgContent(recvmsg, ABBA_JSON_KEY_GW_SN, This->gwSerialNumber))
	{
		return -ABBA_E_SERVER_ERROR;
	}
		
	return ret;
}

static int parseDrvRegEvtJson(ABBA_Data * This, IOTHUB_MESSAGE_HANDLE *recvmsg)
{
	int ret = ABBA_E_OK;
	const char *data = NULL;
	MAP_HANDLE mHandle = IoTHubMessage_Properties(*recvmsg);

	//"ability-messagetype"	== "platformEvent"
	if (compareMsgHeader(mHandle, ABBA_KEY_MSG_TYPE, ABBA_VAL_MSG_PLATFROM_EVT))
	{
		return -ABBA_E_SERVER_ERROR;
	}
	//"model" == "abb.ability.device"
	if (compareMsgHeader(mHandle, ABBA_KEY_MODEL, ABBA_VAL_MODEL_DEV))
	{
		return -ABBA_E_SERVER_ERROR;
	}
	//check drive type 
	if (compareMsgContent(recvmsg, ABBA_KEY_TYPE, ABBA_VAL_DRV_ACS_880_TYPE))
	{
		return -ABBA_E_SERVER_ERROR;
	}
	//check drive SN
	if (compareMsgContent(recvmsg, ABBA_JSON_KEY_DRV_SN, This->driveSerialNumbers[0]))
	{
		return -ABBA_E_SERVER_ERROR;
	}
	//"objectid" == "xxxxxxx"
	data = Map_GetValueFromKey(mHandle, ABBA_KEY_OBJ_ID);
	strncpy(This->ABB_Drive_Id[0], data, sizeof(This->ABB_Drive_Id[0]));
	This->ABB_Drive_Id[0][sizeof(This->ABB_Drive_Id[0]) - 1] = '\0';

	return ret;
}

static int parseDrvCfgEvtJson(ABBA_Data * This, IOTHUB_MESSAGE_HANDLE *recvmsg)
{
	int ret = ABBA_E_OK;
	MAP_HANDLE mHandle = IoTHubMessage_Properties(*recvmsg);

	if (!This->ABB_Drive_Id[0][0])
	{
		return -ABBA_E_SERVER_ERROR;
	}

	//"model" == "abb.ability.configuration"
	if (compareMsgHeader(mHandle, ABBA_KEY_MODEL, ABBA_VAL_MODEL_CFG))
	{
		return -ABBA_E_SERVER_ERROR;
	}
	//"ability-messagetype"	== "platformEvent"
	if (compareMsgHeader(mHandle, ABBA_KEY_MSG_TYPE, ABBA_VAL_MSG_PLATFROM_EVT))
	{
		return -ABBA_E_SERVER_ERROR;
	}
	//"objectid" == "xxxxxxx"
	if (compareMsgHeader(mHandle, ABBA_KEY_OBJ_ID, This->ABB_Drive_Id[0]))
	{
		return -ABBA_E_SERVER_ERROR;
	}
	//check drive sn
	if (compareMsgContent(recvmsg, ABBA_JSON_KEY_DRV_SN, This->driveSerialNumbers[0]))
	{
		ret = -ABBA_E_SERVER_ERROR;
	}
	return ret;
}

static int getDrvRegJson(ABBA_Data * This, char * buffer, int size)
{
	return  snprintf(
		buffer, 
		size,
		"\"" ABBA_KEY_REF "\": {\n"
		"\t\t\"" ABBA_KEY_DRV "\": [\n"
		"\t\t{\n"
		"\t\t\t\"" ABBA_KEY_MODEL "\": \"" ABBA_VAL_MODEL_DEV "\",\n"
		"\t\t\t\"" ABBA_KEY_TYPE "\" : \"" ABBA_VAL_DRV_ACS_880_TYPE "\",\n"
		"\t\t\t\"" ABBA_KEY_PROP "\" : {\n"
		"\t\t\t\t\"" ABBA_KEY_DRV_SN "\" : {\"" ABBA_KEY_VAL "\": \"%s\"},\n"
		"\t\t\t\t\"" ABBA_KEY_DRV_FWVER "\" : {\"" ABBA_KEY_VAL "\": \"%s\"},\n"
		"\t\t\t\t\"" ABBA_KEY_DRV_DEVTYPE "\" : {\"" ABBA_KEY_VAL "\": \"%s\"},\n"
		"\t\t\t\t\"" ABBA_KEY_DRV_VERION "\" : {\"" ABBA_KEY_VAL "\": \"%s\"}\n"
		"\t\t\t},\n"
		"\t\t\t\"" ABBA_KEY_CREAT_IF_MISS "\": true\n"
		"\t\t}\n"
		"\t]\n"
		"}\n"
		""
		,
		This->driveSerialNumbers[0],
		This->drvFwVer,
		This->drvDevType,
		This->drvDevType
		);
}

static int getGwRegJson(ABBA_Data * This, char * buffer, int size)
{
	const char *netOperater = NULL;
	const char *sigName = NULL;
	char iccid[32];

	ABBA_get_network_operator_and_signal(&netOperater, &sigName);
	ABBA_get_panelICCID(iccid, sizeof(iccid));

	return snprintf(buffer, size,
		"\"" ABBA_KEY_TYPE "\": \"" ABBA_VAL_GW_PANEL_TYPE "\",\n"
		"\"" ABBA_KEY_PROP "\": {\n"
		"\t\"" ABBA_KEY_GW_DEVID "\":{ \"" ABBA_KEY_VAL "\": \"%s\" },\n" /* GW IOT hub id From DPS */
		"\t\"" ABBA_KEY_GW_HUB_NAME "\":{ \"" ABBA_KEY_VAL "\": \"%s\" },\n" /*Hub name from DPS*/
		"\t\"" ABBA_KEY_GW_SN "\": { \"" ABBA_KEY_VAL "\": \"%s\" },\n"/*IoT Panel serial number*/
		"\t\"" ABBA_KEY_GW_FWVER "\": { \"" ABBA_KEY_VAL "\": \"%s\" },\n"/*IoT Panel fw version*/
		"\t\"" ABBA_KEY_GW_ICCID "\": { \"" ABBA_KEY_VAL "\": \"%s\" },\n"/*IoT Panel iccid*/
		"\t\"" ABBA_KEY_GW_NET_OPERATOR "\": { \"" ABBA_KEY_VAL "\": \"%s\" },\n"/*IoT Panel network operator*/
		"\t\"" ABBA_KEY_GW_SIGNAL "\": { \"" ABBA_KEY_VAL "\": \"%s\" }\n"/*IoT Panel signal strength*/
		"}",
		This->hubId,
		This->hubName,
		This->gwSerialNumber,
		This->gwFwVer,
		iccid,
		netOperater,
		sigName
	);
}

static int updateGwRegJson(ABBA_Data * This, char * buffer, int size)
{
	return snprintf(buffer, size,
		"\"" ABBA_KEY_TYPE "\": \"" ABBA_VAL_GW_PANEL_TYPE "\",\n"
		"\"" ABBA_KEY_OBJ_ID "\": \"%s\",\n"
		"\"" ABBA_KEY_MODEL "\": \"" ABBA_VAL_MODEL_DEV "\",\n"
		"\"" ABBA_KEY_PROP "\": {\n"
		"\t\"" ABBA_KEY_GW_DEVID "\":{ \"" ABBA_KEY_VAL "\": \"%s\" },\n" /* GW IOT hub id From DPS */
		"\t\"" ABBA_KEY_GW_HUB_NAME "\":{ \"" ABBA_KEY_VAL "\": \"%s\" },\n" /*Hub name from DPS*/
		"\t\"" ABBA_KEY_GW_SN "\": { \"" ABBA_KEY_VAL "\": \"%s\" },\n"/*IoT Panel serial number*/
		"\t\"" ABBA_KEY_GW_FWVER "\": { \"" ABBA_KEY_VAL "\": \"%s\" }\n"/*IoT Panel fw version*/
		"}",
		This->ABB_GW_Id,
		This->hubId,
		This->hubName,
		This->gwSerialNumber,
		This->gwFwVer
	);
}

static int getDrvSyncJson(char * buffer, size_t size)
{
	int ret = ABBA_E_OK;
	JSON_Value* root_value = NULL;
	JSON_Object* root_object = NULL;
	JSON_Value* ref_array_val = NULL;
	JSON_Array* ref_json_array = NULL;
	JSON_Value* related_array_val = NULL;
	JSON_Array* related_json_array = NULL;
	char* serialized_string;

	if ((root_value = json_value_init_object()) == NULL)
	{
		ret = ABBA_E_SERVER_ERROR;
	}
	else if ((root_object = json_value_get_object(root_value)) == NULL)
	{
		ret = ABBA_E_SERVER_ERROR;
	}
	else 
	{
		if (json_object_set_boolean(root_object, ABBA_KEY_RECURSIVE, false) != JSONSuccess)
		{
			ret = ABBA_E_SERVER_ERROR;
		}

		if (json_object_set_boolean(root_object, ABBA_KEY_CONTAINED, false) != JSONSuccess)
		{
			ret = ABBA_E_SERVER_ERROR;
		}
		//add references array
		if ((ref_array_val = json_value_init_array()) == NULL)
		{
			ret = ABBA_E_SERVER_ERROR;
		}
		else
		{
			if ((ref_json_array = json_value_get_array(ref_array_val)) == NULL)
			{
				ret = ABBA_E_SERVER_ERROR;
			}
			else
			{
				if ((json_array_append_null(ref_json_array)) != JSONSuccess)
				{
					ret = ABBA_E_SERVER_ERROR;
				}
				else
				{
					if (json_object_set_value(root_object, ABBA_KEY_REF, ref_array_val) != JSONSuccess)
					{
						ret = ABBA_E_SERVER_ERROR;
					}
				}
			}
		}
		//add related array 
		if ((related_array_val = json_value_init_array()) == NULL)
		{
			ret = ABBA_E_SERVER_ERROR;
		}
		else
		{
			if ((related_json_array = json_value_get_array(related_array_val)) == NULL)
			{
				ret = ABBA_E_SERVER_ERROR;
			}
			else
			{
				if ((json_array_append_string(related_json_array, ABBA_VAL_MODEL_CFG)) != JSONSuccess)
				{
					ret = ABBA_E_SERVER_ERROR;
				}
				else
				{
					if (json_object_set_value(root_object, ABBA_KEY_RELATED, related_array_val) != JSONSuccess)
					{
						ret = ABBA_E_SERVER_ERROR;
					}
				}
			}
		}
	}
	if (!ret) 
	{

		if ((serialized_string = json_serialize_to_string(root_value)) == NULL)
		{
			ret = ABBA_E_SERVER_ERROR;
		}
		else 
		{
			strncpy(buffer, serialized_string, size);
			buffer[size-1] = '\0';
		}
	}
	json_value_free(root_value);
	root_value = NULL;
	return ret;
}

static int registerGwInfoModel(ABBA_Data * This, IOTHUB_MESSAGE_HANDLE *out)
{
	char outData[1024];
	char *pData = outData;

	if(!This->gwSerialNumber)
		return -ABBA_E_MISSING_SERIAL_NUMBER;

	if(!This->hubId)
		return -ABBA_E_MISSING_GW_ID;

	if(!This->hubName)
		return -ABBA_E_MISSING_HUB_NAME;

	pData += snprintf(pData, sizeof(outData) - (pData - outData), "{\n");
	pData += getGwRegJson(This, pData, sizeof(outData) - (pData - outData));
	snprintf(pData, sizeof(outData) - (pData - outData), "\n}\n");

	*out = createFromString(This, outData);

	MAP_HANDLE mHandle = IoTHubMessage_Properties(*out);

	Map_Add(mHandle, ABBA_KEY_MSG_TYPE , ABBA_VAL_MSG_PLATFROM_EVT);
	Map_Add(mHandle, ABBA_KEY_MODEL, ABBA_VAL_MODEL_DEV);
	Map_Add(mHandle, ABBA_KEY_EVT_TYPE, ABBA_KEY_EVT_DEV_CREAT);
	return ABBA_E_OK;
}

static int registerDrvInfoModel(ABBA_Data * This, IOTHUB_MESSAGE_HANDLE *out)
{
	char outData[1024];
	char *pData = outData;

	if (!This->ABB_GW_Id[0])
		return -ABBA_E_MISSING_ABBA_GW_ID;

	if (!This->gwSerialNumber)
		return -ABBA_E_MISSING_SERIAL_NUMBER;

	if (!This->hubId)
		return -ABBA_E_MISSING_GW_ID;

	if (!This->hubName)
		return -ABBA_E_MISSING_HUB_NAME;
	
	pData += snprintf(pData, sizeof(outData) - (pData - outData), "{\n");
	pData += updateGwRegJson(This, pData, sizeof(outData) - (pData - outData));
	pData += snprintf(pData, sizeof(outData) - (pData - outData), ",\n");
	pData += getDrvRegJson(This, pData, sizeof(outData) - (pData - outData));
	snprintf(pData, sizeof(outData) - (pData - outData), "\n}\n");

	*out = createFromString(This, outData);

	MAP_HANDLE mHandle = IoTHubMessage_Properties(*out);

	Map_Add(mHandle, ABBA_KEY_MSG_TYPE, ABBA_VAL_MSG_PLATFROM_EVT);
	Map_Add(mHandle, ABBA_KEY_EVT_TYPE, ABBA_KEY_EVT_DEV_UPDATE);
	Map_Add(mHandle, ABBA_KEY_OBJ_ID, (const char *)This->ABB_GW_Id);
	Map_Add(mHandle, ABBA_KEY_MODEL, ABBA_VAL_MODEL_DEV);
	return ABBA_E_OK;
}

static int syncDrvInfoModel(ABBA_Data * This, IOTHUB_MESSAGE_HANDLE *out)
{
	char outData[1024];

	if (!This->ABB_GW_Id[0])
		return -ABBA_E_MISSING_ABBA_GW_ID;

	if (getDrvSyncJson(outData, sizeof(outData)))
		return -ABBA_E_SERVER_ERROR;

	*out = createFromString(This, outData);

	MAP_HANDLE mHandle = IoTHubMessage_Properties(*out);

	Map_Add(mHandle, ABBA_KEY_MSG_TYPE, ABBA_VAL_MSG_PLATFROM_EVT);
	Map_Add(mHandle, ABBA_KEY_EVT_TYPE, ABBA_KEY_EVT_DEV_SYNC);
	Map_Add(mHandle, ABBA_KEY_OBJ_ID, (const char *)This->ABB_Drive_Id[0]);
	Map_Add(mHandle, ABBA_KEY_MODEL, ABBA_VAL_MODEL_DEV);

	return ABBA_E_OK;
}

int ABBA_Register_StateMachine(ABBA_Data * This, IOTHUB_MESSAGE_HANDLE *out, IOTHUB_MESSAGE_HANDLE *in)
{
	int ret = 0;
	switch (This->registerState)
	{
		case ABBA_INFO_MODEL_INIT:
		{
			break;
		}
		case ABBA_INFO_MODEL_GW_REGISTERING:
		{
			if (!registerGwInfoModel(This, out))
			{
				ret = 1;
				This->registerState = ABBA_INFO_MODEL_GW_REGISTERED;
			}
			break;
		}
		case ABBA_INFO_MODEL_GW_REGISTERED:
		{
			if (in != NULL && *in)
			{
				if (!parseGwRegEvtJson(This, in))
				{
					This->registerState = ABBA_INFO_MODEL_GW_CONFIGURED;
				}
			}
			break;
		}
		case ABBA_INFO_MODEL_GW_CONFIGURED:
		{
			if (in != NULL && *in)
			{
				if (!parseGwCfgEvtJson(This, in))
				{
					This->registerState = ABBA_INFO_MODEL_DRIVE_REGISTERING;
				}
			}
			break;
		}
		case ABBA_INFO_MODEL_DRIVE_REGISTERING:
		{
			if (!registerDrvInfoModel(This, out))
			{
				ret = 1;
				This->registerState = ABBA_INFO_MODEL_DRIVE_REGISTERED;
			}
			break;
		}
		case ABBA_INFO_MODEL_DRIVE_REGISTERED:
		{
			if (in != NULL && *in)
			{
				if (!parseDrvRegEvtJson(This, in))
				{
					This->registerState = ABBA_INFO_MODEL_DRIVE_CONFIGURED;
				}
			}
			break;
		}
		case ABBA_INFO_MODEL_DRIVE_CONFIGURED:
		{
			//recv drive config
			if (in != NULL && *in)
			{
				if (!parseDrvCfgEvtJson(This, in))
				{
					This->registerState = ABBA_INFO_MODEL_SYNC_CONFIGURING;
				}
			}
			break;
		}
		case ABBA_INFO_MODEL_SYNC_CONFIGURING:
		{
			//request drive config
			if (!syncDrvInfoModel(This, out))
			{
				ret = 1;
				This->registerState = ABBA_INFO_MODEL_SYNC_CONFIGURED;
			}
			break;
		}
		case ABBA_INFO_MODEL_SYNC_CONFIGURED:
		{
			//sync drive config
			if (in != NULL && *in)
			{
				ABBA_ParseConf(This, *in);
				This->registerState = ABBA_INFO_MODEL_DONE;
			}
			break;
		}
		case ABBA_INFO_MODEL_ERROR: 
		{
			break;
		}
		case ABBA_INFO_MODEL_DONE:
		{
			break;
		}
	}
	return ret;
}

int ABBA_Init(
	ABBA_Data  * This, 
	const char * gwSerialNumber, 
	const char * gwObjectId,
	const char * driveSerialNumber,
	const char * driveObjectId,
	const char * hubName,
	const char * hubId,
	const char * gwFwVer, 
	const char * driveFwVer,
	const char * driveDevType,
	const char * driveDevMode
	)
{
	This->registerState = ABBA_INFO_MODEL_INIT;

	This->gwSerialNumber = gwSerialNumber;

	This->driverSerialNumberAmount = 1;

	This->driveSerialNumbers[0] = driveSerialNumber;

	This->ABB_Drive_Id[0][0] = 0;
	This->ABB_GW_Id[0] = 0;

	if(driveObjectId)
	{ 
		snprintf(This->ABB_Drive_Id[0], sizeof(This->ABB_Drive_Id[0]),"%s", driveObjectId);
	}

	if(gwObjectId)
	{
		snprintf(This->ABB_GW_Id, sizeof(This->ABB_GW_Id), "%s", gwObjectId);
	}

	This->hubName = hubName;

	This->hubId = hubId;

	This->drvDevMode = driveDevMode;
	This->drvDevType = driveDevType;
	This->drvFwVer = driveFwVer;
	This->gwFwVer = gwFwVer;

	return 0;
}

int parseDeviceTwinsJson(ABBA_Data * This, const char *recvmsg)
{
	int ret = ABBA_E_OK;
	int i;
	JSON_Value* root_value;
	JSON_Object* root_object;
	JSON_Value* refVal;
	JSON_Object* refObj;
	const char *gwSN;
	const char *driveObjId;
	const char *gwObjId;
	const char *drvSN;
	char drvSnKey[128];
	int drvFound = 0;

	if ((root_value = json_parse_string(recvmsg)) == NULL)
	{
		ret = -ABBA_E_SERVER_ERROR;
	}
	else if ((root_object = json_value_get_object(root_value)) == NULL)
	{
		json_value_free(root_value);
		ret = -ABBA_E_SERVER_ERROR;
	}
	else
	{
		if (json_object_has_value(root_object, ABBA_KEY_DESIRED))
		{
			gwSN = json_object_dotget_string(root_object, ABBA_JSON_KEY_GW_DESIRED_SN);
			if (gwSN == NULL)
			{
				This->registerState = ABBA_INFO_MODEL_GW_REGISTERING;
			}
			else
			{
				if (strncmp(This->gwSerialNumber, gwSN, strlen(This->gwSerialNumber)))
				{
					This->registerState = ABBA_INFO_MODEL_GW_REGISTERING;
				}
				else
				{
					gwObjId = json_object_dotget_string(root_object, ABBA_JSON_KEY_GW_OBJID);
					if (gwObjId == NULL)
					{
						This->registerState = ABBA_INFO_MODEL_GW_REGISTERING;
					}
					else
					{
						strncpy(This->ABB_GW_Id, gwObjId, sizeof(This->ABB_GW_Id));
						This->ABB_GW_Id[sizeof(This->ABB_GW_Id)-1] = '\0';
					}
					//get drive info
					if ((refVal = json_object_dotget_value(root_object, ABBA_JSON_KEY_GW_REF)) == NULL)
					{
						json_value_free(refVal);
						This->registerState = ABBA_INFO_MODEL_DRIVE_REGISTERING;
					}
					else
					{
						if ((refObj = json_value_get_object(refVal)) == NULL)
						{
							json_object_clear(refObj);
							This->registerState = ABBA_INFO_MODEL_DRIVE_REGISTERING;
						}
						else
						{
							for (i=0; i<json_object_get_count(refObj); i++)
							{
								driveObjId = json_object_get_name(refObj, i);
								if (driveObjId == NULL)
								{
									json_object_clear(refObj);
									This->registerState = ABBA_INFO_MODEL_DRIVE_REGISTERING;
								}
								else
								{
									snprintf(drvSnKey, sizeof(drvSnKey), "%s.%s.%s", ABBA_JSON_KEY_GW_REF, driveObjId, ABBA_JSON_KEY_DRV_SN);
									drvSN = json_object_dotget_string(root_object, drvSnKey);
									if (!strncmp(This->driveSerialNumbers[0], drvSN, sizeof(This->ABB_Drive_Id)))
									{
										drvFound = 1;
										strncpy(This->ABB_Drive_Id[0], driveObjId, sizeof(This->ABB_Drive_Id[0]));
										This->ABB_Drive_Id[0][sizeof(This->ABB_Drive_Id[0]) - 1] = '\0';
										This->registerState = ABBA_INFO_MODEL_SYNC_CONFIGURING;
									}
								}
							}
							if (!drvFound)
							{
								This->registerState = ABBA_INFO_MODEL_DRIVE_REGISTERING;
							}
						}
					}
				}
			}
		}
		else 
		{
			This->registerState = ABBA_INFO_MODEL_GW_REGISTERING;
		}
		json_object_clear(root_object);
		json_value_free(root_value);
	}
	return ret;
}

int ABBA_RegisterIsDone(ABBA_Data * This)
{
	return This->registerState == ABBA_INFO_MODEL_DONE;
}

//only for unit test
int ABBA_RegisterSet(
	ABBA_Data * This,
	const char * gwSerialNumber,
	int DriveSerialAmount,
	const char * DriveSerials[],
	const char * gwId,
	const char * hubName,
	const char * gwFwVer,
	const char * driveFwVer,
	const char * driveDevType,
	const char * driveDevMode
)
{
	memset(This, 0, sizeof(*This));

	This->registerState = ABBA_INFO_MODEL_INIT;
	This->gwSerialNumber = "";

	if (DriveSerialAmount == 1)
	{
		This->driverSerialNumberAmount = 1;
		This->driveSerialNumbers[0] = DriveSerials[0];
	}
	else
	{
		return ABBA_E_MISSING_DRIVE_SERIAL_NUMBER;
	}

	This->hubId = gwId;
	This->hubName = hubName;
	This->gwSerialNumber = gwSerialNumber;
	This->gwFwVer = gwFwVer;
	This->drvFwVer = driveFwVer;
		This->drvDevMode = driveDevMode;
		This->drvDevType = driveDevMode;

	return ABBA_E_OK;
}

