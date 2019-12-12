#ifdef MIA_LIB_CONFIGURATION
//#error Do not include this file from source files. Let the enviroment do including
#endif
#define MIA_LIB_CONFIGURATION


#include "build_configuration_os.h" // Specials for for Keil and VS

#define OSAL_OS

#ifdef WIN32
// Azure full logging
#else // target
#ifdef MIA_DEBUG
#define NO_LOGGING
#define MINIMAL_LOGERROR
#endif
#endif

//#define NO_HAVE_RTC

// Azure conf
#define DONT_USE_UPLOADTOBLOB
#define USE_PROV_MODULE
#define HSM_TYPE_X509
#define HSM_AUTH_TYPE_CUSTOM

// CPM+ conf
#define G_LONG_STRING_LEN 256
#define CURRENT_VALUE_PUSHER_RECONNECT_NO_TIMEOUT
#define NO_UNORDERED_MAP
#define OSAL_OS
#define NO_EXCEPTIONS
#define G_M_K_S_DEFAULT_BUFFER_SIZE (2048 + 16)
#define WS_PING_TIMEOUT  (180)
#define WS_PING_INTERVAL (60)
#define PING_ONLY_WHEN_SENDING_DATA

// Modem conf
#define USE_HUAEWEI_EM909 0
#define USE_QUACTELEC20_DRIVER 0
#define USE_QUACTEL_BG36_DRIVER 0
#define USE_QUACTEL_BC26_DRIVER 1
#define USE_QUACTEL_BC26_DRIVER_OLD 0

//#define BAND_SELECTION "0,10,80" // China Mobile
#define BAND_SELECTION "0,10,10" // China telecom

#if USE_QUACTEL_BC26_DRIVER || USE_QUACTEL_BG36_DRIVER || USE_QUACTEL_BC26_DRIVER_OLD

#if USE_QUACTEL_BC26_DRIVER || USE_QUACTEL_BC26_DRIVER_OLD

#define SIGNAL_QUALITY_CMD "CESQ"
#define RX_SIGNAL_MAX 62

#endif

#define NO_LAZY_MODEM 

#define CGREG_CMD "CEREG"
#define USE_WOLF_SSL 1
#endif

#define USE_EMBEDDED_SDK_CPM 0
#define USE_CLOUDLIB_AZURE 1

#define WOLF_SSL_MODEM_TIMEOUT 40

#ifdef NO_LAZY_MODEM

#define WEBSOCKET_POLL_TIME 100 // poll 10 times per second

#else
#define WEBSOCKET_POLL_TIME 333 // poll 3 times per second
#define WS_WEBSOCET_LAZY_WRITE
#define WS_WEBSOCET_LAZY_READ 6 // poll 6 seconds after write
#define WS_WEBSOCET_LAZY_READ_SECOND_TRY 50
#endif

#define DO_NOT_WAIT_TIMEOUT_IN_ERROR
#define DEVICE_DIE_TIMEOUT (60 * 1000)
#define DRIVER_DIE_TIMEOUT (60 * 1000)

#ifndef MIA_DEBUG
#define NDEBUG
#else
#define MODEM_DEBUG 1
#define WEBSOCKET_DEBUG 1
#define DEBUG_WOLFSSL 1
#endif

#define MODEM_DEBUG_ERR 1
#define MODEM_DEBUG_LIGHT 1

#define WOLFSSL_USER_SETTINGS 1

#ifdef __cplusplus

#include <sstream>
#include <ostream> 
//#include <iostream>

namespace ABB
{
namespace Mia
{

class C_OsalPrinter : public std::ostream {
private:
	class C_OsalPrinterBuf : public std::stringbuf {
	private:
		bool *m_quiet;
	public:
		C_OsalPrinterBuf(bool *quiet) : m_quiet(quiet){ }
		virtual ~C_OsalPrinterBuf() {  pubsync(); }
		virtual int sync();
	};

	public:
		C_OsalPrinter() : m_quiet(false), std::ostream(new C_OsalPrinterBuf(&m_quiet)) {}
		virtual ~C_OsalPrinter() { delete rdbuf(); }
		bool m_quiet;
};

extern C_OsalPrinter G_OsalPrinter;
extern C_OsalPrinter G_OsalPrinterDebug;
extern C_OsalPrinter G_OsalPrinterInfo;
extern C_OsalPrinter G_OsalPrinterWebsocket;
}
}

// In embedded devices this streams takes lot of heap. Try to optimise heap usage when not printing to avoiding evalution of stram using this if(quiet) {} else stream macro

#define MIA_OUT_WARNING          if(G_OsalPrinter.m_quiet) {}          else G_OsalPrinter
#define MIA_OUT_WARNING_STREAM   G_OsalPrinter
#define MIA_OUT_DEBUG            if(G_OsalPrinterDebug.m_quiet) {}     else G_OsalPrinterDebug
#define MIA_OUT_DEBUG_WEBSOCKET  if(G_OsalPrinterWebsocket.m_quiet) {} else G_OsalPrinterWebsocket
#define MIA_OUT_INFO             if(G_OsalPrinterInfo.m_quiet) {}      else G_OsalPrinterInfo

#endif // __cplusplus
