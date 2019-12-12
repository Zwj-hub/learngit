#include "miaLib.h"
#include "Mia_Base.h"
#include "Mia_Driver.h"
#include "Mia_Device.h"
#include "Mia_Exception.h"
#include "modem_driver.h"
#include <stdio.h>
#include <stdlib.h>

#if USE_EMBEDDED_SDK_CPM

#ifdef MIA_LIB_DEBUG
#include <assert.h>
#define ASSERT(a) assert(a)
#define MIA_LIB_DEBUG_PRINT 1
#else
#define ASSERT(a)
#define MIA_LIB_DEBUG_PRINT 1
#endif

#define MIA_DEBUG_PRINT(...) do{ if(MIA_LIB_DEBUG_PRINT && g_mia_interface_output) printf(__VA_ARGS__); }while(0)
	
static uint8 g_mia_interface_output;
static MiaConnectionInfo s_info;

static Mia_CB mia_cb = NULL;
static void * mia_cb_usr = NULL;

extern "C"
{
	void WS_Socket_Osal_Init(uint8 modemId);
	int G_Socket_get_network(const char ** network, uint16 * signalPercent);
	int G_Socket_get_modem_state(const char ** error);
	void G_Socket_clear_diagnose_error(void);
	int G_Socket_get_transmission_amount(uint32 *tx, uint32 *rx);
	int G_Socket_get_ccid(char * buffer, unsigned int buffer_len);
	int G_Socket_get_cgmr(char * buffer, unsigned int buffer_len);
	int G_Socket_get_imei(char * buffer, unsigned int buffer_len);
	int G_Socket_get_time(unsigned int *utc_time, int *timezone);
	int G_Socket_set_operator_net_params(const char * apn, const char * username, const char * password);
	void G_Socket_set_pin_cb(void(*cb)(int action, void * param));
	void G_Socket_Power(int powerOn);

	extern uint8 g_modem_output;
	extern char g_WS_WebsocketDebugEnable;
	extern uint8 g_socket_debug;
}

extern char g_debug_output_c_thread;

void Mia_Base_Osal_Init(const uint8 * EnoughTaskIdsForMia);

#define DO_CB(reason, params) do{if(mia_cb){mia_cb(mia_cb_usr, reason, params);}}while(0)

void MIA_LibInit(const MiaInitData *init, Mia_CB cb, void* usr)
{
	Mia_Base_Osal_Init(init->priorities);
	WS_Socket_Osal_Init(init->modemUartId);
	
	mia_cb     = cb;
	mia_cb_usr = usr;
}

uint32 MIA_CheckVersion(void)
{
	return _MIA_VERSION_PRIVATE_
#ifdef MIA_DEBUG
	| MIA_LIB_DEBUG_ON_FLAG
#endif
	;
}

static ConnectionParamas conPar;
static DeviceParamas devPar;

static ABB::Mia::C_Device    *m_device = NULL;
static ABB::Mia::C_MiaDriver *m_driver = NULL;

static ABB::Mia::C_SharedPointer<ABB::Mia::C_EventSubscription> m_subcription;

void MIA_ConnectionsParams(const ConnectionParamas *params)
{
	conPar = *params;
}

// Set Panel
void MIA_SetPanel(const DeviceParamas *params) // TODO
{
	if (params->GUID)
	{
		devPar = *params;
	}
}

void MIA_SetDrive(const DeviceParamas *params)
{
	if (params->GUID)
	{
		devPar = *params;
	}
}

void deviceInitialized()
{
	if(!m_device)
	{
		m_device = new ABB::Mia::C_Device(devPar.name, devPar.model);
		m_device->M_SetId(ABB::Mia::C_Guid(devPar.GUID));
		m_subcription = m_device->M_AddEventSubscription("Drive events");
	}
}

void MIA_DeviceAddParam(const char *name, uint8 bits, boolean signedVal, boolean floating)
{
	deviceInitialized();
	
	using namespace ABB::Mia;

	t_PropertyType ptype;

	if(floating)
	{
		if(bits > 32)
		{
			ptype = g_PT_DOUBLE;
		}
		else
		{
			ptype = g_PT_FLOAT;
		}
	}
	else
	{
		switch(bits)
		{
			case 1:
				ptype = g_PT_BOOLEAN;
				break;
			case 8:
				ptype = signedVal ? g_PT_BYTE : g_PT_CHAR;
				break;
			case 16:
				ptype = signedVal ? g_PT_INT16 : g_PT_UINT16;
				break;
			default:
			case 32:
				ptype = signedVal ? g_PT_INT : g_PT_UINT;
				break;
			case 64:
				ptype = signedVal ? g_PT_INT64 : g_PT_UINT64;
				break;
		}
	}

	m_device->M_AddProperty(name, ptype);
}

void MIA_DeviceAddParamStr(const char *name)
{
	deviceInitialized();
	
	using namespace ABB::Mia;

	m_device->M_AddProperty(name, ABB::Mia::g_PT_STRING);
}


int MIA_getTime(uint32 * epoch, int32 *tz)
{
	using ABB::Mia::C_MiaDriverBase;

#if 0
	return G_Socket_get_time(epoch, tz);
#else

	if(!m_driver || !epoch || !tz)
	{
		ASSERT(epoch);
		ASSERT(tz);
		return CPM_SERVER_NOT_INITALISED;
	}

	*epoch = 0;
	*tz = 0;

	if(m_driver->M_GetStatus() != ABB::Mia::C_MiaDriverBase::g_READY)
	{
		MIA_DEBUG_PRINT("MIA_getTime: driver not connected\n");
		return -4;
	}

	ABB::Mia::C_DateTime myTime = m_driver->M_GetServerTimeUtc();

	if(myTime.M_GetYear() < 2018 || myTime.M_GetYear() > 2181)
	{
		MIA_DEBUG_PRINT("Getting UTC time failed\n");
		return -1;
	}
	else
	{
		unsigned int utc_time;
		int timezone;

		MIA_DEBUG_PRINT("Received UTC time %d.%02d\n", myTime.M_GetHour(), myTime.M_GetMinute());
		s_info.timeUpdated++;
		*epoch = myTime.M_GetTotalSeconds();

		if(G_Socket_get_time(&utc_time, &timezone) == 0)
		{
			MIA_DEBUG_PRINT("Received TZ %+04d\n", timezone * 100 / 3600);
			*tz = timezone;
			return 0;
		}

		return 1; // TZ mssing
	}
#endif
}

int16 MIA_Sync(void)
{
	MIA_CheckDiagnoseInfo();
	return 0;
}

int MIA_Start(void)
{
	MIA_DEBUG_PRINT("Mia starting\n");

	if(!m_device)
	{
		MIA_DEBUG_PRINT("Device parameters were not added\n");
		ASSERT(m_device);
		return -2;
	}
	
	if(m_driver)
	{
		MIA_DEBUG_PRINT("Mia was already running\n");
		ASSERT(!m_driver);
		return -3;
	}

	m_driver = new ABB::Mia::C_MiaDriver(conPar.host);

	if(m_driver && m_driver->M_Connect(conPar.username, conPar.password))
	{
		MIA_CheckDiagnoseInfo();
		if(m_driver->M_SubscribeEquipmentSession(m_device))
		{
			m_device->M_StartProduction();
			s_info.starts++;
			MIA_DEBUG_PRINT("\nDevice production started\n");
			return 0;
		}
		else
		{
			MIA_DEBUG_PRINT("\nFailed to subscripe equipments\n");
			MIA_Stop();
			return -2;
		}
	}
	else
	{
		MIA_CheckDiagnoseInfo();
		MIA_DEBUG_PRINT("\nConnection to server failed\n");
		MIA_Stop();
		return -1;
	}
}

int MIA_Stop(void)
{
	if(m_device)
	{
		m_device->M_StopProduction();
	}

	if(m_driver)
	{
		delete m_driver;
	}

	if(m_device)
	{
		delete m_device;
	}

	m_device = NULL;
	m_driver = NULL;
	m_subcription = NULL;

	MIA_DEBUG_PRINT("MIA stopped\n");

	return 0;
}

static std::string char_to_key(const char*key)
{
	std::string skey(key);
	G_MIA_TRANSFORM_TO_LOWER(skey);
	return skey;
}

int16 MIA_GetDataBufferCnt(void)
{
	if(!m_driver || !m_device)
		return -1;
	
	if(m_driver->M_GetStatus() != ABB::Mia::C_MiaDriver::g_READY)
		return -2;

	if(!m_device->M_IsSubscripted())
		return -3;

	return (int16)m_driver->M_BufferCnt();
}

static ABB::Mia::C_DeviceProperty* getProperty(const char *name)
{
	using namespace ABB::Mia;

	if(m_device && m_driver){
		C_SharedPointer<C_DeviceProperty> prop = m_device->M_GetProperty(char_to_key(name));
		if(!prop)
		{
			ASSERT(0);
			s_info.parameterNotUpdated++;
			return NULL;
		}
		
		if(prop->M_IsSubscribed()) 
		{
			s_info.sendParameters++;
			return prop.get();
		}
	}

	s_info.parameterNotUpdated++;
	return NULL;
}

#define CREATE_updater(type)\
	void MIA_Update_##type(const char *name, type val, uint32 ts)\
		{	ABB::Mia::C_DeviceProperty * prop = getProperty(name);\
		if(prop) {m_device->M_PropertyValueChanged(prop, val, ABB::Mia::C_DateTime(ts, 0), ABB::Mia::g_VS_OK);}\
	}

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

	ABB::Mia::C_DeviceProperty * prop = getProperty(name);
	if(prop) 
	{
		m_device->M_PropertyValueChanged(prop, std::string(val), ABB::Mia::C_DateTime(ts, 0), ABB::Mia::g_VS_OK);
	}
}


void MIA_SendEvent(Event_name name, Event_level level, Event_val_type type, const void * value, const struct tm *t)
{
	using namespace ABB::Mia;

	const C_DateTime eventTime = C_DateTime(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

	const char *nameStr;
	
	C_Variant data(g_OBJECT);
	
	if(!m_subcription)
	{
		MIA_DEBUG_PRINT("Subsription was not created\n");
		ASSERT(m_subcription);
		return;
	}
	
	switch(name)
	{
		case EVT_DRIVE_FAULT:
			nameStr = "Drive fault";
			break;
		case EVT_DRIVE_WARNING:
			nameStr = "Drive warning";
			break;
		case EVT_DRIVE_EVENT:
			nameStr = "Drive event";
			break;
		case EVT_PANEL_EVENT:
			nameStr = "Panel Event";
			break;
		default:
			return;
	}
	
	switch(level)
	{
		case EVL_FAULT:
			data["Severity"] = "Fault";
			break;
		case EVL_WARNING:
			data["Severity"] = "Warning";
			break;
		case EVL_EVENT:
			data["Severity"] = "Event";
			break;

		default:
			return;
	}

	switch(type)
	{
		case EVT_NONE:
			break;
		case EVT_UINT32:
		{
			uint32 valUint32 = value ? *(uint32*)value : 0;
			data["DataProduced"] = valUint32;
			break;
		}
		case EVT_INT32:
		{
			int32  valInt32  = value ? *(int32*)value  : 0;
			data["DataProduced"] = valInt32;
			break;
		}
		case EVT_FLOAT:
		{
			float  valFloat  = value ? *(float*)value  : 0;
			data["DataProduced"] = valFloat;
			break;
		}
		case EVT_STR:
		{
			std::string valStr  =  std::string( (const char*) (value ? value : "") );
			data["DataProduced"] = valStr;
			break;
		}

		default:
			return;
	}


	C_SharedPointer<C_DeviceEvent> event2 = m_subcription->M_CreateEvent(nameStr, data, eventTime);
	m_subcription->M_SendEvent(event2);
	s_info.sendEvents++;
	
}

void MIA_ModemPower(int powerOn)
{
	G_Socket_Power(powerOn);
}

int MIA_GetConnectionId(char *buffer, unsigned int buffer_len)
{
	return G_Socket_get_ccid(buffer, buffer_len);
}

int MIA_GetModemFwVersion(char *buffer, unsigned int buffer_len)
{
	return G_Socket_get_cgmr(buffer, buffer_len);
}

int MIA_GetModemImei(char *buffer, unsigned int buffer_len)
{
	return G_Socket_get_imei(buffer, buffer_len);
}

 // Panel SW can call MIA_ConnectionState to check status of modem by this func
 void MIA_ConnectionState(uint16 * signalPercent, const char ** network, int *connection_error, const char ** error_str)
{
	int state = G_Socket_get_network(network, signalPercent);
	*connection_error = G_Socket_get_modem_state(error_str);
}

void MIA_ConnectionActivity(uint32 *txCnt, uint32 *rxCnt)
{
	G_Socket_get_transmission_amount(txCnt, rxCnt);
}

void MIA_GetConnectionInfo(MiaConnectionInfo *info)
{
	*info = s_info;
}

void MIA_VerboseLevel(uint8 miaLevel, uint8 modemLevel)
{
	using namespace ABB::Mia;

	G_OsalPrinter.m_quiet          = true;
	G_OsalPrinterDebug.m_quiet     = true;
	G_OsalPrinterWebsocket.m_quiet = true;
	G_OsalPrinterInfo.m_quiet      = true;
	g_WS_WebsocketDebugEnable      = 0;
	g_mia_interface_output         = 0;
	g_debug_output_c_thread        = 0;
	g_socket_debug                 = 0;
	
	if (miaLevel > 0)
	{
		G_OsalPrinter.m_quiet = false;

		if (miaLevel > 1)
		{
			G_OsalPrinterInfo.m_quiet = false;
			g_mia_interface_output = 1;
			
			if (miaLevel > 2)
			{
				G_OsalPrinterWebsocket.m_quiet = false;

				if(miaLevel > 3)
				{
					G_OsalPrinterDebug.m_quiet = false;

					if(miaLevel > 4)
					{
						g_WS_WebsocketDebugEnable  = 1;
						g_debug_output_c_thread    = 1;
					}
				}
			}
		}
	}
	
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

	// Make some initialisation and remove buffering
	G_OsalPrinter <<""<< std::unitbuf;
	G_OsalPrinterDebug <<""<< std::unitbuf;
	G_OsalPrinterWebsocket <<""<< std::unitbuf;
	G_OsalPrinterInfo <<""<< std::unitbuf;
}

void MIA_ConnectionSetSettings(const char * apn, const char * username, const char * password)
{
	int ret = G_Socket_set_operator_net_params(apn, username, password);
	ASSERT(!ret);
}

int MIA_GetGUIDbyInsName(const char * ins_name, Guid guid)
{
	ASSERT(conPar.host);
	ASSERT(conPar.username);
	ASSERT(conPar.password);
	ASSERT(m_driver);

	if(!conPar.host || !conPar.username || !conPar.password || !m_driver)
	{
		return -4;
	}

	if(!ins_name)
	{
		MIA_DEBUG_PRINT("Instance name should not be NULL.\r\n");
		ASSERT(ins_name);
		return -1;
	}

	ABB::Mia::C_DbClass cl = m_driver->M_GetClass("Path");

	ABB::Mia::C_DbClassInstances instances;

	if (cl && cl->M_GetInstanceSet(&instances, 1, "DisplayName = ?", ins_name))
	{
		ABB::Mia::C_DbClassInstance ii = instances.front();
		strncpy(guid, ii->M_GetId().M_ToString().c_str(), sizeof(Guid)-1);
	}
	else
	{
		MIA_DEBUG_PRINT("M_GetInstanceSet return false.");
		return -2;
	}

	return 0;
}

void MIA_ConnectionSetPinCB(MiaPinCBFunc cb)
{
	G_Socket_set_pin_cb(cb);
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
	return 0;
}

uint32 modemErrorToEventParam(int modemError)
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
