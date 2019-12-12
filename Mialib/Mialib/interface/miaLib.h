#ifndef MIALIB_H_
#define MIALIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "OSAL.h" // uint8
#include <time.h> // struct tm
#define MIA_VERSION_IS_1_0_14 1000014

#define _MIA_VERSION_PRIVATE_ MIA_VERSION_IS_1_0_14

#define MIA_LIB_DEBUG_ON_FLAG    0x80000000
#define MIA_LIB_VERSION_CMP(a,b) (MIA_LIB_GET_VERSION(a) != MIA_LIB_GET_VERSION(b)) // return 0 if same
#define MIA_LIB_IS_DEBUG(a)      (MIA_LIB_DEBUG_ON_FLAG & a)
#define MIA_LIB_GET_VERSION(a)   (~MIA_LIB_DEBUG_ON_FLAG & a)

#define CPM_SERVER_NOT_INITALISED -13200

#define CPM_DIAGNOSE_OK                 0x0000u
#define CPM_DIAGNOSE_SIM                0x1101u
#define CPM_DIAGNOSE_REGISTER_DENIED    0x2101u
#define CPM_DIAGNOSE_REGISTER_TIMEOUT   0x2102u
#define CPM_DIAGNOSE_UNKNOWN            0x5000u
  
// Structure for initialising Mia library.
typedef struct MiaInitData
{
	uint8 priorities[4]; // Mia library requires 4 tasks running and here priorities for those tasks are given.
	uint8 modemUartId;   // Modem uart port ID

} MiaInitData;

typedef struct MiaConnectionInfo
{
	uint32 sendParameters;
	uint32 sendEvents;
	uint16 starts;
	uint16 parameterNotUpdated;
	uint16 timeUpdated;

} MiaConnectionInfo;

typedef enum Mia_CB_Action{
	MIA_CB_CONNECTED_ABBA,
	MIA_CB_DISCONNECTED_ABBA,
	MIA_CB_ERROR_STATE,
	MIA_CB_NEW_OBEJECT_ID,

	MIA_CB_PARAMETERS_INIT,
	MIA_CB_PARAMETERS_ADD,
	MIA_CB_PARAMETERS_FINALISE,
	MIA_CB_DIAGNOSE_ERROR,

} Mia_CB_Action;

typedef struct MIA_cbNewObjectId
{
	int         isDrive;
	const char *objectId;
	const char *serialNumber;
	const char *hubId;
	const char *hubUrl;

} MIA_cbNewObjectId;

typedef struct MIA_cbNewParameter
{
	int interval;
	int group;
	int index;
	int sendOnlyChange;

} MIA_cbNewParameter;

typedef struct MIA_cbDiagnoseParameter
{
	uint32 diagnoseError;
} MIA_cbDiagnoseParameter;


typedef int(*Mia_CB)(void * usr, Mia_CB_Action action, const void *args);


#define CREATE_MiaInitData(name, pr1, pr2, pr3, pr4, _modemUartId)\
	const MiaInitData name = { {pr1, pr2, pr3, pr4}, _modemUartId }

void MIA_LibInit(const MiaInitData *init, Mia_CB cb, void* usr); // Initialise library

typedef struct ConnectionParamas
{
	const char * host;
	const char * username;
	const char * password;

} ConnectionParamas;

typedef struct DeviceParamas
{
	const char * GUID;
	const char * name;
	const char * model;
	const char * typeSN;
	const char *fwVer;
} DeviceParamas;

typedef char Guid[39];

// Verify that people remember update submodules
uint32 MIA_CheckVersion(void);

// Set connecttion settings. This will remove device parameters
void MIA_ConnectionsParams(const ConnectionParamas *params);

// Set Drive
void MIA_SetDrive(const DeviceParamas *params);

// Set Panel
void MIA_SetPanel(const DeviceParamas *params);

// Add device parameter
// name is parameter name in equipment model
// bits is 8,16,32 or 64
// signedVal is parameter signes. int32 vs uint32
// floating is true if value is float or double
void MIA_DeviceAddParam(const char *name, uint8 bits, boolean signedVal, boolean floating);

// name is parameter name in equipment model
void MIA_DeviceAddParamStr(const char *name);

// Start Mia library
// 4 threads starts running.
// return 0 on success
int MIA_Start(void);

// Stops Mia library
// 4 threads suspended
// power offs modem
// return 0 on success
int MIA_Stop(void);

//telemetry data
int16 MIA_Sync(void);

// Get UTC time from server. Seconds from 1970 in epoch
// return 0 when time and timezone valid. 
// return 1 when time valid
// return <0 when nothing valid
int MIA_getTime(uint32 * epoch, int32 *tz);

// How many data item Mia is have in buffer waiting for transmitting
int16 MIA_GetDataBufferCnt(void);

// Update device parameter
#define DECLARE_updater(type)\
	void MIA_Update_##type(const char *name, type val, uint32 ts)

DECLARE_updater(boolean);
DECLARE_updater(int8);
DECLARE_updater(uint8);
DECLARE_updater(int16);
DECLARE_updater(uint16);
DECLARE_updater(int32);
DECLARE_updater(uint32);
DECLARE_updater(int64);
DECLARE_updater(uint64);
DECLARE_updater(real32);
DECLARE_updater(real64);
void MIA_Update_string(const char *name, const char * val, uint32 ts);

// Event interface
typedef enum {
	EVL_FAULT,    // Fault event
	EVL_WARNING,  // Warning event
	EVL_EVENT     // Event event
} Event_level;

typedef enum {
	EVT_NONE,    // Event does not have parameter
	EVT_UINT32,  // Event parameter type is uint32
	EVT_INT32,   // Event parameter type is int32
	EVT_FLOAT,   // Event parameter type is float
	EVT_STR,     // Event parameter type is string
} Event_val_type;

typedef enum {
	EVT_DRIVE_FAULT,
	EVT_DRIVE_WARNING,
	EVT_DRIVE_EVENT,
	EVT_PANEL_EVENT
} Event_name;

// Make device event
void MIA_SendEvent(Event_name name, Event_level level, Event_val_type type, const void * value, const struct tm *t);

void MIA_ConnectionState(uint16 * signalPercent, const char ** network, int *connection_error, const char ** error_str);

void MIA_ConnectionActivity(uint32 *txCnt, uint32 *rxCnt);

void MIA_ModemPower(int powerOn);

int MIA_GetConnectionId(char *buffer, unsigned int buffer_len);

int MIA_GetModemFwVersion(char *buffer, unsigned int buffer_len);

int MIA_GetModemImei(char *buffer, unsigned int buffer_len);

void MIA_VerboseLevel(uint8 miaLevel, uint8 modemLevel);

void MIA_GetConnectionInfo(MiaConnectionInfo *info);

void MIA_ConnectionSetSettings(const char * apn, const char * username, const char * password);

int MIA_SetAzureParameters(
	const char * idScope,
	const char * globalProvUrl,
	const char * name,
	const char * serverSertificate,
	const char * publicKey,
	const char * privateKey
);

typedef enum {
	MIA_PIN_GET          = 0,
	MIA_PIN_OK           = 1,
	MIA_PIN_BAD          = 2,
	MIA_PIN_PUK_REQUIRED = 3,
	MIA_PIN_NOT_LOCKED   = 4

} Mia_pin_action;

typedef void(*MiaPinCBFunc)(int action, void * param);

void MIA_ConnectionSetPinCB(MiaPinCBFunc cb);


// Get GUID by instance name in [New equipment] folder.
int MIA_GetGUIDbyInsName(const char * ins_name, Guid guid);

void MIA_CheckDiagnoseInfo(void);

#ifdef __cplusplus
}
#endif

#endif

