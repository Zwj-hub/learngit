#ifndef ABBALIB_H_
#define ABBALIB_H_

#include <stdint.h>
#include "iothub_message.h"

#define ABBALIB_MAX_DRIVES 1

#define ABB_GW_ID_SIZE 64

#define ABBA_KEY_OBJ_ID "objectId"
#define ABBA_KEY_TYPE "type"
#define ABBA_KEY_PROP "properties"
#define ABBA_KEY_MODEL "model"
#define ABBA_KEY_REF "references"
#define ABBA_KEY_VAL "value"
#define ABBA_KEY_CREAT_IF_MISS "createIfMissing"
#define ABBA_KEY_MSG_TYPE "ability-messagetype"
#define ABBA_KEY_EVT_TYPE "eventType"
#define ABBA_KEY_DESIRED "desired"
#define ABBA_KEY_RELATED "related"
#define ABBA_KEY_RECURSIVE "recursive"
#define ABBA_KEY_CONTAINED "contained"
#define ABBA_KEY_TIMESTAMP "timestamp"
#define ABBA_KEY_EVENT "event"

#define ABBA_KEY_EVT_DEV_CREAT "Abb.Ability.Device.Created"
#define ABBA_KEY_EVT_DEV_UPDATE "Abb.Ability.Device.Updated"
#define ABBA_KEY_EVT_DEV_SYNC "Abb.Ability.Device.SyncModel"

#define ABBA_KEY_INFO_UPDATE "Abb.Ability.InformationModel.ObjectModelUpdated"
#define ABBA_KEY_INFO_CREATE "Abb.Ability.InformationModel.ObjectModelCreated"

#define ABBA_KEY_GW_DEVID "deviceId"
#define ABBA_KEY_GW_HUB_NAME "hubName"
#define ABBA_KEY_GW_SN "serialNumberIotPanel"
#define ABBA_KEY_GW_FWVER "firmwareVersion"
#define ABBA_KEY_GW_ICCID "simIccid"
#define ABBA_KEY_GW_NET_OPERATOR "networkOperator"
#define ABBA_KEY_GW_SIGNAL "signalStrength"
#define ABBA_KEY_GW_LAT "latitude"
#define ABBA_KEY_GW_LONG "longitude"

#define ABBA_KEY_DRV "Drive"
#define ABBA_KEY_DRV_SN "serialNumberDrive"
#define ABBA_KEY_DRV_FWVER "firmwareVersion"
#define ABBA_KEY_DRV_DEVTYPE "deviceType"
#define ABBA_KEY_DRV_VERION "deviceVersion"

#define ABBA_KEY_EVENTVAL_ID "eventId"
#define ABBA_KEY_EVENTVAL_MSG "message"
#define ABBA_KEY_EVENTVAL_SRC "source"
#define ABBA_KEY_EVENTVAL_SEVERITY "severity"
#define ABBA_KEY_EVENTVAL_DEVTIMESTAMP "deviceTimeStamp"

#define ABBA_VAL_MSG_PLATFROM_EVT "platformEvent"
#define ABBA_VAL_MSG_TIMESERIE "timeSeries"
#define ABBA_VAL_MSG_EVT "event"
#define ABBA_VAL_MSG_ALARM "alarm"

#define ABBA_VAL_MODEL_DEV "abb.ability.device"
#define ABBA_VAL_MODEL_CFG "abb.ability.configuration"

#define ABBA_VAL_GW_PANEL_TYPE "abb.drives.iotpanel@1"
#define ABBA_VAL_GW_PANEL_CONF "abb.drives.iotpanel.config@1"

#define ABBA_VAL_DRV_ACS_880_TYPE "abb.drives.acs880.inu@1"
#define ABBA_VAL_DRV_ACS_880_CONF "abb.drives.acs880.inu.config@1"

#define ABBA_VAL_EVT_DEVEVT "deviceEvent"

#define ABBA_JSON_KEY_DRV_SN ABBA_KEY_PROP "." ABBA_KEY_DRV_SN "." ABBA_KEY_VAL
#define ABBA_JSON_KEY_GW_SN ABBA_KEY_PROP "." ABBA_KEY_GW_SN "." ABBA_KEY_VAL
#define ABBA_JSON_KEY_GW_DESIRED_SN ABBA_KEY_DESIRED "." ABBA_KEY_PROP "." ABBA_KEY_GW_SN "." ABBA_KEY_VAL
#define ABBA_JSON_KEY_GW_OBJID ABBA_KEY_DESIRED "." ABBA_KEY_OBJ_ID
#define ABBA_JSON_KEY_GW_REF ABBA_KEY_DESIRED "." ABBA_KEY_REF

typedef enum {
	ABBA_INFO_MODEL_INIT,
	ABBA_INFO_MODEL_GW_REGISTERING,
	ABBA_INFO_MODEL_GW_REGISTERED,
	ABBA_INFO_MODEL_GW_CONFIGURED,
	ABBA_INFO_MODEL_DRIVE_REGISTERING,
	ABBA_INFO_MODEL_DRIVE_REGISTERED,
	ABBA_INFO_MODEL_DRIVE_CONFIGURED,
	ABBA_INFO_MODEL_SYNC_CONFIGURING,
	ABBA_INFO_MODEL_SYNC_CONFIGURED,
	ABBA_INFO_MODEL_DONE,
	ABBA_INFO_MODEL_ERROR
} ABBA_RegisterState;

typedef enum {
	ABBA_E_OK = 0,
	ABBA_E_MISSING_SERIAL_NUMBER,
	ABBA_E_MISSING_GW_ID,
	ABBA_E_MISSING_HUB_NAME,
	ABBA_E_MISSING_DRIVE_SERIAL_NUMBER,
	ABBA_E_SERVER_ERROR,
	ABBA_E_MISSING_ABBA_GW_ID,
	ABBA_E_WRONG_HANDLER,
	ABBA_E_MISSING_CONFIGURATION,
} ABBA_Errors;

typedef enum {

	ABBA_PAR_FIRST,
	ABBA_PAR_ADD,
	ABBA_PAR_LAST,

} ABBA_Data_CB_Parameter;

struct ABBA_Data;

typedef struct ABBA_Data_CB_Parameter_Data
{
	const char * cycleType;
	const char * parameterName;
	int          interval;
	int          onlyOnChange;

} ABBA_Data_CB_Parameter_Data;

typedef struct ABBA_Data_CB
{
	void (*parameter)(struct ABBA_Data * This, ABBA_Data_CB_Parameter par, ABBA_Data_CB_Parameter_Data * data);
} ABBA_Data_CB;

typedef struct ABBA_Data
{
	ABBA_RegisterState registerState;
	
	const char * gwSerialNumber;
	const char * gwFwVer;

	uint16_t     driverSerialNumberAmount;
	const char * driveSerialNumbers[ABBALIB_MAX_DRIVES];
	const char * drvFwVer;
	const char * drvDevType;
	const char * drvDevMode;

	const char * hubId;    // GW id in HUN
	const char * hubName;  // IOT hub url
	
	char        ABB_GW_Id[ABB_GW_ID_SIZE]; // Abba ID for GW
	char        ABB_Drive_Id[ABBALIB_MAX_DRIVES][ABB_GW_ID_SIZE]; // Abba id for drives

	ABBA_Data_CB cb; // Call backs

} ABBA_Data;

int ABBA_RegisterSet(
	ABBA_Data * This,
	const char * gwSerialNumber,
	int DriveSerialAmount,
	const char * DriveSerials[],
	const char * hubId,  
	const char * hubName ,
	const char * gwFwVer,
	const char * driveFwVer,
	const char * driveDevType,
	const char * driveDevMode
	);

int parseDeviceTwinsJson(ABBA_Data * This, const char *recvmsg);

int ABBA_Register_StateMachine(ABBA_Data * This, IOTHUB_MESSAGE_HANDLE *out, IOTHUB_MESSAGE_HANDLE *in);

int ABBA_RegisterIsDone(ABBA_Data * This); // return !0 if panel and drive are registered succesfully

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
);

IOTHUB_MESSAGE_HANDLE ABBA_MsgCreateTelemetrics(const char *jsonPayload);

typedef enum ABBA_Debug_Control
{
	ABBA_RX_MSG,
	ABBA_TX_MSG,
	ABBA_CTRL_MSG,

} ABBA_Debug_Control;

int  ABBA_MsgDebug(IOTHUB_MESSAGE_HANDLE msg, ABBA_Debug_Control control);

#endif

