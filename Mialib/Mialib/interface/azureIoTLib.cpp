/******************************************************************************
*                                                                             *
*                   COPYRIGHT (C) 2018    ABB OY, DRIVES                      *
*                                                                             *
*******************************************************************************/
#include "miaLib.h"
#include "modem_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include "build_configuration.h"
#if USE_CLOUDLIB_AZURE

#ifdef MIA_LIB_DEBUG
#include <assert.h>
#define ASSERT(a) assert(a)
#define MIA_LIB_DEBUG_PRINT 1
#else
#define ASSERT(a)
#define MIA_LIB_DEBUG_PRINT 1
#endif

#define MIA_DEBUG_PRINT(...) do{ if(MIA_LIB_DEBUG_PRINT && g_mia_interface_output) printf(__VA_ARGS__); }while(0)

extern "C"
{
	#include "CL_Manager.h"
	#include "CL_ABBAbilitySerialiser.h"
	#include "AzureConnection.h"
	#include "ABBALib.h"
	#include "ABBA_EventMsg.h"
	#include "MIA_Certificate.h"

	int CL_UnitTestDo(void);
	int ABBALIB_UnitTestDo(void);

	void WS_Socket_Osal_Init(uint8 modemId);
	int G_Socket_get_network(const char ** network, uint16 * signalPercent);
	int G_Socket_get_modem_state(const char ** error);
	int G_Socket_get_transmission_amount(uint32 *tx, uint32 *rx);
	int G_Socket_get_ccid(char * buffer, unsigned int buffer_len);
	int G_Socket_set_operator_net_params(const char * apn, const char * username, const char * password);
	void G_Socket_set_pin_cb(void(*cb)(int action, void * param));
	int G_Socket_set_certs(const char *cacert, const char * publicKey, const char * privateKey);
	void G_Socket_Power(int powerOn);
	int G_Socket_get_cgmr(char * buffer, unsigned int buffer_len);
	void G_Socket_clear_diagnose_error(void);
	int G_Socket_get_imei(char * buffer, unsigned int buffer_len);

	int custom_hsm_set_certs(const char * commonName, const char * certificate, const char * privateKey, const char * symmetricKey, const char * registrationName);

	#define Azure_Socket_Osal_Init                   WS_Socket_Osal_Init
	#define Azure_Socket_get_network                 G_Socket_get_network
	#define Azure_Socket_get_ccid                    G_Socket_get_ccid
	#define Azure_Socket_get_modem_state             G_Socket_get_modem_state
	#define Azure_Socket_get_transmission_amount     G_Socket_get_transmission_amount
	#define Azure_Socket_set_pin_cb                  G_Socket_set_pin_cb
	#define Azure_Socket_get_modem_version           G_Socket_get_cgmr
	#define Azure_Socket_get_modem_imei              G_Socket_get_imei

	int G_Socket_get_time(unsigned int *utc_time, int *timezone);

	extern uint8 g_modem_output;
	extern char g_WS_WebsocketDebugEnable;
	extern uint8 g_socket_debug;
	extern char g_Azure_debug_level;

	static int recvAbbaEvt(AzureConnectConf * azure, IOTHUB_MESSAGE_HANDLE message);
	static int recvAbbaStat(AzureConnectConf * azure, const char * message);
	static int recvAbbaMethod(AzureConnectConf * azure, const char* method_name, const unsigned char* payLoad, size_t size, unsigned char** response, size_t* resp_size);
	static void connectAbba(AzureConnectConf * azure);
	static void disconnectAbba(AzureConnectConf * azure);
	static void ABBAparameterCB(struct ABBA_Data * This, ABBA_Data_CB_Parameter par, ABBA_Data_CB_Parameter_Data * data);

}

static uint8 g_mia_interface_output;
static MiaConnectionInfo s_info;
static AzureConnectConf conf;
extern char g_debug_output_c_thread;
static ABBA_Data abbLib;
static Mia_CB mia_cb = NULL;
static void * mia_cb_usr = NULL;

#define DO_CB(reason, params) do{if(mia_cb){mia_cb(mia_cb_usr, reason, params);}}while(0)

void MIA_LibInit(const MiaInitData *init, Mia_CB cb, void * usr)
{
#ifdef WIN32 // Do unit tests only in simulator
	CL_UnitTestDo();
	ABBALIB_UnitTestDo();
#endif

	Azure_Socket_Osal_Init(init->modemUartId);

	mia_cb     = cb;
	mia_cb_usr = usr;

	conf.connected    = connectAbba;
	conf.disconnected = disconnectAbba;
	conf.statRecv = recvAbbaStat;
	conf.evtRecv = recvAbbaEvt;
	conf.methodRecv = recvAbbaMethod;

	conf.keepalive = 60 * 10;
	conf.logtrace = true;
}

uint32 MIA_CheckVersion(void)
{
	return _MIA_VERSION_PRIVATE_
#ifdef MIA_DEBUG
	| MIA_LIB_DEBUG_ON_FLAG
#endif
	;
}

static DeviceParamas devParDrive;
static DeviceParamas devParPanel;

CREATE_CL_Manager(cloudLibMgr, 1, 40, "IoTPanel", "");

static CL_Device CLoudLibDevice;
CREATE_CL_ParameterList(CLoudLibDeviceParaList, 60);

static int s_devicesAdded = 0;

void MIA_ConnectionsParams(const ConnectionParamas *params)
{
	if (params->host!= NULL && params->host[0])
	{
		mallocAndStrcpy_s(&conf.iothub_uri, params->host);
	}
	if (params->username != NULL && params->username[0])
	{
		mallocAndStrcpy_s(&conf.device_id, params->username);
	}
}

// Set Panel
void MIA_SetPanel(const DeviceParamas *params)
{
	devParPanel = *params;
	devParPanel.model = "abb.ability.device";
}

void MIA_SetDrive(const DeviceParamas *params)
{
	devParDrive = *params;
	devParPanel.model = "abb.ability.device";
}

static uint16_t makeId(const char *str)
{
	typedef uint16_t crc;
	const crc WIDTH      = (8 * sizeof(crc));
	const crc TOPBIT     = (1 << (WIDTH - 1));
	const crc POLYNOMIAL = 0xD8;

	crc  remainder = 0;

	for (int byte = 0; str[byte]; ++byte)
	{
		remainder ^= (str[byte] << (WIDTH - 8));
		for (uint8_t bit = 8; bit > 0; --bit)
		{
			if (remainder & TOPBIT)
			{
				remainder = (remainder << 1) ^ POLYNOMIAL;
			}
			else
			{
				remainder = (remainder << 1);
			}
		}
	}
	return (remainder);
}

static void ABBAparameterCB(struct ABBA_Data * This, ABBA_Data_CB_Parameter par, ABBA_Data_CB_Parameter_Data * data)
{
	switch(par)
	{
	case ABBA_PAR_FIRST:
		DO_CB(MIA_CB_PARAMETERS_INIT, NULL);
		break;
	case ABBA_PAR_ADD:
	{
		MIA_cbNewParameter param;
		param.interval = data->interval;
		sscanf(data->parameterName,"%d_%d", &param.group, &param.index);
		param.sendOnlyChange = data->onlyOnChange;

		DO_CB(MIA_CB_PARAMETERS_ADD, &param);
		break;
	}
	case ABBA_PAR_LAST:
		DO_CB(MIA_CB_PARAMETERS_FINALISE, NULL);
		break;
	}
}

void MIA_DeviceAddParam(const char *name, uint8 bits, boolean signedVal, boolean floating)
{
	CL_Parameter parameter;
	CL_ParameterMeasType parType;

	if(floating)
	{
		if(bits > 32)
		{
			parType = CL_REAL64;
		}
		else
		{
			parType = CL_REAL32;
		}
	}
	else
	{
		switch(bits)
		{
			case 1:
				parType = CL_BOOL;
				break;
			case 8:
				parType = signedVal ? CL_INT8 : CL_UINT8;
				break;
			case 16:
				parType = signedVal ? CL_INT16 : CL_UINT16;
				break;
			default:
			case 32:
				parType = signedVal ? CL_INT32 : CL_UINT32;
				break;
			case 64:
				parType = signedVal ? CL_INT64 : CL_UINT64;
				break;
		}
	}


	if (!CL_ParameterInit(&parameter, name, makeId(name), parType))
	{
		CL_ParameterListAddParam(&CLoudLibDeviceParaList, &parameter);
	}
	else
	{
		MIA_DEBUG_PRINT("Parameter init failed\n");
	}

}

void MIA_DeviceAddParamStr(const char *name)
{
	return;
}

int MIA_getTime(uint32 * epoch, int32 *tz)
{
	int ret = G_Socket_get_time(epoch, tz);

	if( ret >= 0)
	{
		s_info.timeUpdated++;
		MIA_DEBUG_PRINT("Received time %d %u TZ %+04d\n", ret, *epoch, (*tz) * 100 / 3600);
		return ret;
	}
	else
	{
		MIA_DEBUG_PRINT("Time reading failed\n");
		return ret;
	}
}

int MIA_Start(void)
{
	int ret = AzureConnectInit(&conf);

	MIA_CheckDiagnoseInfo();

	if ( ret < 0) {
		AzureConnectDeinit(&conf);
		return ret;
	}

	CL_DeviceInit(&CLoudLibDevice, devParDrive.GUID, devParDrive.model, devParDrive.name, devParDrive.typeSN);

	CL_DeviceSetParameters(&CLoudLibDevice, &CLoudLibDeviceParaList);

	CL_MgrSetDevice(&cloudLibMgr, &CLoudLibDevice, 0);

	AzureClientRefresh(&conf);

	s_info.starts++;

	return 0;
}

static int azureSend(IOTHUB_MESSAGE_HANDLE msg)
{
	ABBA_MsgDebug(msg, ABBA_TX_MSG);
	AzureClientSendMessageHandle(&conf, msg);
	return 0;
}

static int recvAbbaStat(AzureConnectConf * azure, const char *recvmsg)
{
	IOTHUB_MESSAGE_HANDLE out;
	int ret = parseDeviceTwinsJson(&abbLib, recvmsg);
	AzureClinetDeviceTwinsDisable(azure);

	if (!ret)
	{
		if (AzureClientConnectStat(&conf))
		{
			if (ABBA_Register_StateMachine(&abbLib, &out, NULL))
			{
				azureSend(out);
			}
		}
	}
	return 0;
}

static int recvAbbaEvt(AzureConnectConf * azure, IOTHUB_MESSAGE_HANDLE message)
{
	IOTHUB_MESSAGE_HANDLE out;
	int sending = ABBA_Register_StateMachine(&abbLib, &out, &message);

	ABBA_MsgDebug(message, ABBA_RX_MSG);

	if(AzureClientConnectStat(&conf))
	{
		if(sending)
		{
			azureSend(out);
		}
	}
	return 0;
}

static int recvAbbaMethod(AzureConnectConf * azure, const char* method_name, const unsigned char* payLoad, size_t size, unsigned char** response, size_t* resp_size)
{
	MIA_DEBUG_PRINT("receive abb device method\n");
	return 0;
}

static void connectAbba(AzureConnectConf * azure)
{
	ABBA_Init(
		&abbLib,
		devParPanel.typeSN,
		devParPanel.GUID,
		devParDrive.typeSN,
		devParDrive.GUID,
		azure->iothub_uri,
		azure->device_id,
		devParPanel.fwVer,
		devParDrive.fwVer,
		devParDrive.name,
		devParDrive.model
	);

	abbLib.cb.parameter = ABBAparameterCB; // XXX add setter

	// Mia sync will handle starting of registration
}

static void disconnectAbba(AzureConnectConf * azure)
{
	DO_CB(MIA_CB_DISCONNECTED_ABBA, NULL);
	// Oh shit!
}

static void pushData(void)
{
	int len;
	const int outbufferSize = (10 * 1024);
	static char *AzureMeasDataOutputBuffer;
	uint16_t free, meas;

	int measSuccess = CL_MgrMeasCount(&cloudLibMgr, &meas, &free);

	if(
		!AzureClientConnectStat(&conf) ||
		measSuccess ||
		!ABBA_RegisterIsDone(&abbLib)
	)
	{
		return;
	}

	if(!AzureMeasDataOutputBuffer)
	{
		AzureMeasDataOutputBuffer = (char*)malloc(outbufferSize);
		if (!AzureMeasDataOutputBuffer)
		{
			return;
		}
	}

	len = CL_MgrPush(&cloudLibMgr, AzureMeasDataOutputBuffer, outbufferSize, CL_ABB_ABILITY_CREATE_TELEMETRICS);

	if (len > 1)
	{
		IOTHUB_MESSAGE_HANDLE msg = ABBA_MsgCreateTelemetrics(AzureMeasDataOutputBuffer);

		// Panel or drive registeration to ABBA server must be done.
		azureSend(msg);
		s_info.sendParameters += meas;
	}
	else
	{
		s_info.parameterNotUpdated += meas;
	}

	CL_MgrDelMeas(&cloudLibMgr);
}

int MIA_Stop(void)
{
	pushData();
	AzureConnectDeinit(&conf);
	s_devicesAdded = 0;
	return 0;
}

static int16 registering()
{
	IOTHUB_MESSAGE_HANDLE out;
	int sending = ABBA_Register_StateMachine(&abbLib, &out, NULL); // Handle panel / drive registering messages

	if(sending)
	{
		azureSend(out);
		return 1; // Do only one message at time.
	}

	if(!ABBA_RegisterIsDone(&abbLib))
	{
		// Panel or drive have not registered to ABBA server. No reason to continue
		return 1;
	}

	if(!s_devicesAdded)
	{
		s_devicesAdded = 1;

		// Drive GUID
		CL_DeviceSetGUID(CL_MgrGetDevice(&cloudLibMgr, 0), abbLib.ABB_Drive_Id[0]); // XXX create getter for ids

		if(!devParPanel.GUID || !devParPanel.GUID[0])
		{
			MIA_cbNewObjectId cbDataPanel = { 0, abbLib.ABB_GW_Id,       abbLib.gwSerialNumber,        abbLib.hubId, abbLib.hubName};
			DO_CB(MIA_CB_NEW_OBEJECT_ID, &cbDataPanel);
		}

		if(!devParDrive.GUID || !devParDrive.GUID[0])
		{
			MIA_cbNewObjectId cbDataDrive = {1, abbLib.ABB_Drive_Id[0], abbLib.driveSerialNumbers[0], abbLib.hubId, abbLib.hubName};
			DO_CB(MIA_CB_NEW_OBEJECT_ID, &cbDataDrive);
		}

		DO_CB(MIA_CB_CONNECTED_ABBA, NULL);
	}

	return 0;
}

int16 MIA_Sync(void)
{
	AzureClientRefresh(&conf);
	int16 ret;

	MIA_CheckDiagnoseInfo();

	if(!AzureClientConnectStat(&conf))
	{
		// Not connecteted. No reason to continue
		return 0;
	}

	ret = registering();

	if(ret > 0)
	{
		return 0; // actions done. Quit
	}
	else if(ret)
	{
		return ret; // Return error
	}

	pushData(); // Push telemetry

	return 0;
}

int16 MIA_GetDataBufferCnt(void)
{
	uint16_t free, meas;

	if (!AzureClientConnectStat(&conf))
	{
		return -1;
	}

	int ret = CL_MgrMeasCount(&cloudLibMgr, &meas, &free);

	if (ret)
		return ret;

	return meas;
}

#define CREATE_updater(type) void MIA_Update_##type(const char *name, type val, uint32 ts) {CL_MgrAddMeas(&cloudLibMgr, makeId(name), &val, ts); }

CREATE_updater(boolean);
CREATE_updater(int8);
CREATE_updater(uint8);
CREATE_updater(int16);
CREATE_updater(uint16);
CREATE_updater(int32);
CREATE_updater(uint32);
CREATE_updater(int64);
CREATE_updater(uint64);
CREATE_updater(real32);
CREATE_updater(real64);

void MIA_Update_string(const char *name, const char * val, uint32 ts)
{
}

void MIA_SendEvent(Event_name name, Event_level level, Event_val_type type, const void * value, const struct tm *t)
{
	IOTHUB_MESSAGE_HANDLE out;
	const char *severity = NULL;
	const char *srcMsg = NULL;

	//only handle evt str
	if (type != EVT_STR) return;
	//if (!ABBA_RegisterIsDone(&abbLib)) return;                     //zwj

	switch (name)
	{
		case EVT_DRIVE_FAULT:
			srcMsg = "Drive Fault";
			break;
		case EVT_DRIVE_WARNING:
			srcMsg = "Drive Warning";
			break;
		case EVT_DRIVE_EVENT:
			srcMsg = "Drive Event";
			break;
		case EVT_PANEL_EVENT:
			srcMsg = "Panel Event";
			break;
		default:
			srcMsg = "Drive Notification";
	}

	switch (level)
	{
		case EVL_FAULT:
			severity = "Fault";
			break;
		case EVL_WARNING:
			severity = "Warning";
			break;
		case EVL_EVENT:
			severity = "Severe";
			break;
		default:
			severity = "Notification";
	}

	if (!ABBA_EventMsg(&abbLib, &out, (const char *)value, srcMsg, severity, t))
	{
		azureSend(out);
		s_info.sendEvents++;
	}

	MIA_Sync();
}

void MIA_ModemPower(int powerOn)
{
	G_Socket_Power(powerOn);
}

int MIA_GetConnectionId(char *buffer, unsigned int buffer_len)
{
	return Azure_Socket_get_ccid(buffer, buffer_len);
}

 // Panel SW can call MIA_ConnectionState to check status of modem by this func
 void MIA_ConnectionState(uint16 * signalPercent, const char ** network, int *connection_error, const char ** error_str)
{
	int state = Azure_Socket_get_network(network, signalPercent);
	*connection_error = Azure_Socket_get_modem_state(error_str);
}

void MIA_ConnectionActivity(uint32 *txCnt, uint32 *rxCnt)
{
	Azure_Socket_get_transmission_amount(txCnt, rxCnt);
}

void MIA_GetConnectionInfo(MiaConnectionInfo *info)
{
	*info = s_info;
}

void MIA_VerboseLevel(uint8 miaLevel, uint8 modemLevel)
{
	g_WS_WebsocketDebugEnable = 0;
	g_mia_interface_output = 0;
	g_debug_output_c_thread = 0;
	g_socket_debug = 0;

	g_modem_output = 0;

	if (modemLevel > 0)
	{
		g_modem_output |= 4;

		if (modemLevel > 1)
		{
			g_modem_output |= 2;

			if (modemLevel > 2)
			{
				g_modem_output |= 1;

				if (modemLevel > 3)
				{
					g_socket_debug = 1;
				}
			}
		}
	}

	g_Azure_debug_level = miaLevel;
	ABBA_MsgDebug((IOTHUB_MESSAGE_HANDLE) (miaLevel > 3), ABBA_CTRL_MSG);
}

void MIA_ConnectionSetSettings(const char * apn, const char * username, const char * password)
{
	int ret = G_Socket_set_operator_net_params(apn, username, password);
}

int MIA_GetGUIDbyInsName(const char * ins_name, Guid guid)
{
	strncpy(guid, ins_name, sizeof(Guid));
	guid[sizeof(Guid) - 1] = 0;

	return 0;
}

void MIA_ConnectionSetPinCB(MiaPinCBFunc cb)
{
	Azure_Socket_set_pin_cb(cb);
}


int MIA_SetAzureParameters(
	const char * idScope,
	const char * globalProvUrl,
	const char * name,
	const char * serverCert,
	const char * publicKey,
	const char * privateKey
)
{
//#ifdef WIN32
  //  #ifndef WIN32
	conf.idScope       = idScope;
	conf.globalProvUrl = globalProvUrl;
	custom_hsm_set_certs(name, "", "", "", "");

	return G_Socket_set_certs(serverCert, publicKey, privateKey);
/*#else
    printf("1.1..........\n");
	Mia_CertificateInit();
    printf("1.2..........\n");
	conf.idScope = Mia_CertificateGetIdScope();
	conf.globalProvUrl = Mia_CertificateGetUrl();
printf("1.3..........\n");
	custom_hsm_set_certs(Mia_CertificateGetName(), "", "", "", "");

	return G_Socket_set_certs(Mia_CertificateGetServerCert(), Mia_CertificateGetPubKey(), Mia_CertificateGetPrivateKey());
#endif*/
}

int MIA_GetModemFwVersion(char *buffer, unsigned int buffer_len)
{
	return Azure_Socket_get_modem_version(buffer, buffer_len);
}

int MIA_GetModemImei(char *buffer, unsigned int buffer_len)
{
	return Azure_Socket_get_modem_imei(buffer, buffer_len);
}

static uint32 modemErrorToEventParam(int modemError)
{
	int eventParam;
	switch (modemError)
	{
	case ERROR_OK:
		eventParam = CPM_DIAGNOSE_OK;
		break;
	case ERROR_SIM:
		eventParam = CPM_DIAGNOSE_SIM;
		break;
	case ERROR_CREG_REGISTERING_FAILED:
		eventParam = CPM_DIAGNOSE_REGISTER_DENIED;
		break;
	case ERROR_CREG_REGISTERING_TIMEOUT:
		eventParam = CPM_DIAGNOSE_REGISTER_TIMEOUT;
		break;
	default:
		eventParam = CPM_DIAGNOSE_UNKNOWN;
		break;
	}
	return eventParam;
}

void MIA_CheckDiagnoseInfo(void)
{
	static int s_diagnose = ERROR_OK;
	static boolean reported_error = FALSE;

	if (MIA_GetDataBufferCnt() == 0)
	{
		if (reported_error)
		{
			MIA_cbDiagnoseParameter param;
			reported_error = FALSE;
			s_diagnose = ERROR_OK;
			G_Socket_clear_diagnose_error();
			param.diagnoseError = modemErrorToEventParam(s_diagnose);
			DO_CB(MIA_CB_DIAGNOSE_ERROR, &param);
		}
	}
	else
	{
		MIA_cbDiagnoseParameter param;
		const char * error_str;
		int modemError = G_Socket_get_modem_state(&error_str);
		G_Socket_clear_diagnose_error();
		if (modemError != ERROR_OK)
		{
			reported_error = TRUE;
			param.diagnoseError = modemErrorToEventParam(modemError);
			if (s_diagnose != param.diagnoseError)
			{
				s_diagnose = param.diagnoseError;
				DO_CB(MIA_CB_DIAGNOSE_ERROR, &param);
			}
		}
	}
}

#endif
