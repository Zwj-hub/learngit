#include "build_configuration.h"    //zwj
#if USE_WOLF_SSL
#include "wolfssl/ssl.h"
#include "wolfssl/error-ssl.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "GENERIC_defines.h"
#include "OSAL_Semaphore.h"
#include "OSAL_Time.h"

#include "WS_Error.h"
#include "WS_Socket.h"
#include "WS_Websocket.h"


#include "modem_driver.h"

#include "HAL_modem.h"


#ifdef WS_SOCKECT_DEBUG
#include <assert.h>
#define ASSERT(a) assert(a)
#else
#define ASSERT(a) ((void)(a))
#endif

#define MAX_HOSTNAME 70
#define MAX_IP       17

#define MODEM_CLOSE_LIMIT 6

#ifndef WOLF_SSL_SLOW_POLL_DELAY
#define WOLF_SSL_SLOW_POLL_DELAY  950
#define WOLF_SSL_TIGHT_POLL_DELAY 100
#endif

static uint8 modemUartId = 3;
#define WS_SOCKET_UART_NR modemUartId

void WS_Socket_Osal_Init(uint8 modemId)
{
	modemUartId = modemId;
}

uint8 g_socket_debug; // Contol socket debugging

// Private function. Depends OS 
// Declared here because this are private to G_ThreadLock_Free functions
void * G_ThreadLock_New_OS(void);
void G_ThreadLock_Free_OS(void * l);
int G_ThreadLock_Lock_OS(void * l);
int G_ThreadLock_Unlock_OS(void * l);

static char only_one_socket_supported = 0;

static uint32 s_tx_amount;
static uint32 s_rx_amount;

typedef struct _Socket
{
	char *host;
	int port;
	char uart_initialised;
	char modem_initialised;
	void *lock;
	char noBlocking;
	const char *cacertificate;
	const char *x509certificate;
	const char *x509privatekey;
#if USE_WOLF_SSL
	WOLFSSL_CTX* wolfSslCtx;
	WOLFSSL* wolfSsl;
#endif
} *t_Socket;


static struct _Socket s_socket;
static char s_modem_closeCnt;

static int dataAvailable(void)
{
	int available;
	HAL_E_Status status = HAL_eModemIOCTRL(WS_SOCKET_UART_NR, MODEM_IS_DATA_AVAILABLE, &available);

	ASSERT(status == ER_OK);

	return available;
}

static int uartInit(void)
{
	HAL_E_Status stat = HAL_eModemOpen(WS_SOCKET_UART_NR);

	if(s_socket.uart_initialised && (ERBUSY == stat))
	{
		return 0; // Modem was initialised and open
	}

	if(ERBUSY == stat)
	{
		stat = ER_OK;
	}

	s_modem_closeCnt = 0;

	if(ER_OK != stat)
	{
		HAL_eModemClose(WS_SOCKET_UART_NR);
		stat = HAL_eModemOpen(WS_SOCKET_UART_NR);
		ASSERT(stat == ER_OK);
	}

	s_socket.uart_initialised = 1;
	s_socket.modem_initialised = 0;

	return stat != ER_OK;
}

static void uartClose(void)
{
	HAL_E_Status stat = HAL_eModemClose(WS_SOCKET_UART_NR);

	ASSERT(stat == ER_OK);

	s_socket.uart_initialised = 0;
	s_socket.modem_initialised = 0;
}

static int writeUart(const char * data, unsigned int len)
{
	HAL_E_Status stat = HAL_eModemWrite(WS_SOCKET_UART_NR, data, len);

	ASSERT(stat == ER_OK);

	if(stat == ER_OK)
		return len;
	else
		return -stat;
}

static int readUart(char * data, unsigned int len)
{
	uint16 ulen = len;
	HAL_E_Status stat = HAL_eModemRead(WS_SOCKET_UART_NR, data, &ulen);
	
	ASSERT(stat == ER_OK);
	
	if(stat == ER_OK)
		return ulen;
	else
		return -stat;
}

static unsigned int getCACertificate(char *buffer, unsigned int bufferLen, unsigned int offset)
{
	return 0;
}

/*****************************************************************************************************/
/*********************                    Socket                      ********************************/
/*****************************************************************************************************/

t_Socket G_Socket_New(char* hostName, int port);

int G_Socket_Connect(t_Socket socket);

int G_Socket_Write(t_Socket socket, const char* message, size_t bufferLength);

int G_Socket_Read(t_Socket socket, char* buffer, size_t size);

void G_Socket_Free(t_Socket *socket);

int G_Socket_IsReady(t_Socket s);

int G_ResolveHostName(char * hostname , char* ip)
{
	return -1;
}

int G_ResolveHostAndIP(char **hostname, char **hostIp, char* host)
{
	int a = -1,b = -1,c = -1, d = -1;

	*hostname = (char*) malloc(MAX_HOSTNAME);
	if(!*hostname)
	{
		return -1;
	}

	*hostIp = (char*) malloc(MAX_IP);
	if(!*hostIp)
	{
		free(*hostname);
		return -1;
	}
	*hostname[0] = 0;
	*hostIp[0]   = 0;

	sscanf(host,"%d.%d.%d.%d", &a, &b, &c, &d);

	if(a < 0 || b < 0 || c < 0 || d < 0)
	{
		// Host is not IP
		if(G_ResolveHostName(host, *hostIp))
		{
			// Hostname cannot be resolved to IP. Set to default
			strncpy(*hostIp, "0.0.0.0", MAX_IP);
		}

		strncpy(*hostname, host, MAX_HOSTNAME);

		hostIp[MAX_IP - 1] = 0;
		hostname[MAX_HOSTNAME - 1] = 0;
	}
	else
	{
		// Host is IP addr
		strncpy(*hostIp, host, MAX_IP);

		// Host is IP but host cannot be resolved to name. Set IP to host name
		strncpy(*hostname, host, MAX_HOSTNAME);

		hostIp[MAX_IP - 1] = 0;
		hostname[MAX_HOSTNAME - 1] = 0;
	}

	// Free over reservation
	*hostname = (char*) realloc(*hostname, strlen(*hostname) + 1);
	*hostIp   = (char*) realloc(*hostIp,   strlen(*hostIp) + 1);

	return 0;
}


CREATE_network_modem(s_modem, "", "", "", 1, "", getCACertificate);

t_Socket G_Socket_Secure_New(char* hostName, int port)
{
	t_Socket s = G_Socket_New(hostName, port);
	s_modem.crypted = 1;
	return s;
}

t_Socket G_Socket_New(char* hostName, int port)
{
	t_Socket socket = &s_socket;

	if(!hostName || port < 1)
	{
		G_SetLastError(G_ERROR_INVALID_CONNECTION_STRING);
		return NULL;
	}

	if(only_one_socket_supported)
	{
		G_SetLastError(G_ERROR_UNABLE_OPEN_SOCKET);
		return NULL;
	}

	socket->host = strdup_os(hostName);
	socket->port = port;
	socket->lock = G_ThreadLock_New_OS();
	socket->noBlocking = FALSE;

	if(!socket->host || !socket->lock)
	{
		free(socket->host);
		G_ThreadLock_Free_OS(socket->lock);
		G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}

	only_one_socket_supported = 1;
	s_modem.crypted = 0;

	return socket;
}

int G_Socket_get_network(const char ** network, uint16 * signalPercent)
{
	*network       = s_modem.network;
	*signalPercent = s_modem.signal_quality;
	return s_modem.registed_network;
}

int G_Socket_get_modem_state(const char ** error)
{
	*error = s_modem.error_str;
	return s_modem.diagnose_error;
}

void G_Socket_clear_diagnose_error(void)
{
	s_modem.diagnose_error = ERROR_OK;
}

int G_Socket_get_transmission_amount(uint32 *tx, uint32 *rx)
{
	*tx = s_tx_amount;
	*rx = s_rx_amount;
	return 0;
}

void G_Socket_Power(int powerOn)
{
	if(powerOn)
	{
		// Initialise UART if needed
		if(uartInit()) {
			return;
		}
		if (!s_socket.modem_initialised) {
			modem_init(&s_modem, writeUart, readUart);
			s_socket.modem_initialised = 1;
		}
	}
	else
	{
		if(s_socket.uart_initialised)
		{
			modem_cmd(&s_modem, CMD_MODEM_POWER_CHANGE, NULL);
			uartClose();
		}

	}

}

int G_Socket_get_time(unsigned int *utc_time, int *timezone)
{
	if(!s_socket.modem_initialised)
	{
		return -5;
	}

	int tzret = modem_cmd(&s_modem, CMD_MODEM_TIME_TZ, timezone);

	int ret   = modem_cmd(&s_modem, CMD_MODEM_TIME_UTC, utc_time);

	if(!ret)
	{
		if(!tzret)
		{
			return 0; // all valid
		}
		else
		{
			return 1; // Only time valid
		}
	}
	else
	{
		return -1; // Nothing valid
	}
}

int G_Socket_get_ccid(char * buffer, unsigned int buffer_len)
{
	strncpy(buffer, s_modem.iccd, buffer_len);
	buffer[buffer_len - 1] = 0;

	return 0;
}

int G_Socket_get_cgmr(char * buffer, unsigned int buffer_len)
{
	strncpy(buffer, s_modem.cgmr, buffer_len);
	buffer[buffer_len - 1] = 0;

	return 0;
}

int G_Socket_get_imei(char * buffer, unsigned int buffer_len)
{
	strncpy(buffer, s_modem.imei, buffer_len);
	buffer[buffer_len - 1] = 0;

	return 0;
}

int G_Socket_set_operator_net_params(const char * apn, const char * username, const char * password)
{
	if(!apn)
	{
		apn = "";
	}

	if(!username)
	{
		username = "";
	}

	if(!password)
	{
		password = "";
	}

	s_modem.user_name = username;
	s_modem.user_password = password;
	s_modem.user_apn = apn;

	return 0;
}

/* return 0 if connected
// return 1 if in progress,
// return -1 if failed */
int G_Socket_Connect(t_Socket s)
{
	char readBuf;

	if(!s) {
		G_SetLastError(G_ERROR_UNINITIALIZED);
		return -1;
	}

	// Initialise UART if needed
	if(uartInit()) {
		return -4;
	}

	s_modem.server_addr = s->host;
	s_modem.server_port = s->port;

	// Initialise Modem if needed
	if(!s->modem_initialised) {
		modem_init(&s_modem, writeUart, readUart);
		s->modem_initialised = 1;
	}

	AT_handler_init(s_modem.at_handler);

	modem_cmd(&s_modem, CMD_MODEM_CONNECT, 0);

	if(G_Socket_IsReady(s) != 3)
	{
		s_modem_closeCnt = MODEM_CLOSE_LIMIT;
		return -1;
	}

	// Update signal quality
	modem_read(&s_modem, &readBuf, 0);

	// Check state
	if(s_modem.error) {
		G_SetLastErrorExact(G_ERROR_UNABLE_OPEN_SOCKET, s_modem.error);
		s_modem_closeCnt = MODEM_CLOSE_LIMIT;
		return -1;
	}

	// Use signal quolity to check connection
	if(!s_modem.signal_quality) {
		s_modem_closeCnt = MODEM_CLOSE_LIMIT;
		return -1;
	}

	return 0;
}


int G_Socket_Write(t_Socket s, const char* message, size_t bufferLength)
{
	int ret = -1;

	if(!s || !message || bufferLength <= 0)
	{
		G_SetLastError(G_ERROR_UNINITIALIZED);
		return -1;
	}

	if(1)
	{
		G_ThreadLock_Lock_OS(s->lock);
		ret = modem_write(&s_modem, message, bufferLength);
		if(ret > 0)
		{
			s_tx_amount += ret;
		}
		G_ThreadLock_Unlock_OS(s->lock);
	}

	if(ret < 0)
	{
		G_SetLastError(G_ERROR_UNABLE_SEND);
		return -1;
	}

	return ret;
}

int G_Socket_Read(t_Socket s, char* buffer, size_t size)
{
	int ret = 0;

	if(!s || !buffer || size <= 0)
	{
		G_SetLastError(G_ERROR_UNINITIALIZED);
		return -1;
	}

	if(dataAvailable())
	{
		G_ThreadLock_Lock_OS(s->lock);
		ret = modem_read(&s_modem, buffer, size);
		if(ret > 0)
		{
			s_rx_amount += ret;
		}
		G_ThreadLock_Unlock_OS(s->lock);
	}

	if(ret < 0)
	{
		G_SetLastError(G_ERROR_UNABLE_RECEIVE);
		return -1;
	}

	return ret;
}

int G_Socket_IsReady(t_Socket s)
{
	char readBuf;
	int ret = 4;

	if(s)
	{
		G_ThreadLock_Lock_OS(s->lock);
		
		if(modem_read(&s_modem, &readBuf, 0) == 0)
		{
			ret = 3;
		}
		else
		{
			s_modem_closeCnt = MODEM_CLOSE_LIMIT;
		}
		G_ThreadLock_Unlock_OS(s->lock);
	}

	return ret;
}

void G_Socket_Free(t_Socket *s)
{
	if (s && *s)
	{
		G_ThreadLock_Lock_OS((*s)->lock);
		
		if(s_modem_closeCnt++ >= MODEM_CLOSE_LIMIT) // Do not shutdown modem everytime
		{
			modem_cmd(&s_modem, CMD_MODEM_CONNECT, (void*)1); // Disconnect
			uartClose();
		}
		G_ThreadLock_Free_OS((*s)->lock);
		only_one_socket_supported = 0;
		free((*s)->host);
		*s = 0;
	}
}

#if USE_WOLF_SSL

static void wolfSSL_Logging_cbmia(const int logLevel, const char *const logMessage)
{
	CONSOLE_PRINTER("WOLFSSL:(%d): %s\n",logLevel, logMessage);
}

static int VerifyCallbackMia(int preverify, WOLFSSL_X509_STORE_CTX* store)
{
	return 0;
}

static int osalCryptReceive(WOLFSSL *ssl, char *buf, int sz, void *ctx)
{
	t_Socket sc = (t_Socket)ctx;
	int retry = 0;
	const int retryLimit = WOLF_SSL_MODEM_TIMEOUT + 5;

	while(1)
	{
		int ret = G_Socket_Read(sc, buf, sz);

		if(!ret)
		{
			if(sc->noBlocking)
			{
				return WOLFSSL_CBIO_ERR_WANT_READ; // No blockking
			}
			else if(retry++ < retryLimit) // Block until timeout
			{
				if(retry <= 5)
				{
					OSAL_eTimeDelayMS(WOLF_SSL_TIGHT_POLL_DELAY);
				}
				else
				{
					OSAL_eTimeDelayMS(WOLF_SSL_SLOW_POLL_DELAY);
				}
				continue;
			}
			else
			{
				return WOLFSSL_CBIO_ERR_TIMEOUT;
			}
		}
		else if(ret < 0)
		{
			return WOLFSSL_CBIO_ERR_CONN_RST;
		}
		else
		{
			sc->noBlocking = FALSE; // If any data received then start blockking for full frame
			return ret;
		}
	}
}

static int osalCryptSend(WOLFSSL* ssl, char *buf, int sz, void *ctx)
{
	t_Socket sc = (t_Socket)ctx;
	return G_Socket_Write(sc, buf, sz);
}

int myRngFunc(unsigned char* output, unsigned int sz)
{
	while(sz--)
		*output++ = G_Random();

	return 0;
}

static t_Socket G_Socket_WolfSsl_Secure_New(char* hostName, int port)
{
	static char s_wolfSslInited;
	
	t_Socket s = G_Socket_New(hostName, port);
	s_modem.crypted = 0;
	
	if(!s_wolfSslInited)
	{
		s_wolfSslInited = 1;
		wolfSSL_Init();
	}

	if(s->wolfSsl)
	{
		wolfSSL_free(s->wolfSsl);
	}

	if(s->wolfSslCtx)
	{
		wolfSSL_CTX_free(s->wolfSslCtx);
	}

	s->wolfSslCtx = wolfSSL_CTX_new(wolfSSLv23_client_method());
	
	if (s->cacertificate == NULL ) 
	{	
		wolfSSL_CTX_set_verify(s->wolfSslCtx, 0, VerifyCallbackMia);
	}
	else 
	{
		if (wolfSSL_CTX_load_verify_buffer(s->wolfSslCtx, (const unsigned char*)s->cacertificate, strlen(s->cacertificate), SSL_FILETYPE_PEM) != SSL_SUCCESS)
		{
			CONSOLE_PRINTER("wolfSSL_CTX_load_verify_buffer failed");
			if(s->wolfSslCtx)
			{
				wolfSSL_CTX_free(s->wolfSslCtx);
			}
			return NULL;
		}
	}

	wolfSSL_SetIORecv(s->wolfSslCtx, osalCryptReceive);
	wolfSSL_SetIOSend(s->wolfSslCtx, osalCryptSend);
	
	s->wolfSsl = wolfSSL_new(s->wolfSslCtx);

	wolfSSL_SetIOReadCtx(s->wolfSsl, s);
	wolfSSL_SetIOWriteCtx(s->wolfSsl, s);

	if (s->x509certificate) 
	{
		if (wolfSSL_use_certificate_chain_buffer(s->wolfSsl, (unsigned char*)s->x509certificate, strlen(s->x509certificate)) != SSL_SUCCESS)
		{
			CONSOLE_PRINTER("unable to load x509 client certificate");
			if(s->wolfSsl)
			{
				wolfSSL_free(s->wolfSsl);
			}
			if(s->wolfSslCtx)
			{
				wolfSSL_CTX_free(s->wolfSslCtx);
			}
			return NULL;
		}
	}

	if (s->x509privatekey) 
	{
		//private key
		if (wolfSSL_use_PrivateKey_buffer(s->wolfSsl, (unsigned char*)s->x509privatekey, strlen(s->x509privatekey), SSL_FILETYPE_PEM) != SSL_SUCCESS)
		{
			CONSOLE_PRINTER("unable to load x509 client private key");
			if(s->wolfSsl)
			{
				wolfSSL_free(s->wolfSsl);
			}
			if(s->wolfSslCtx)
			{
				wolfSSL_CTX_free(s->wolfSslCtx);
			}
			return NULL;
		}
	}

	if(g_socket_debug)
	{
		wolfSSL_SetLoggingCb(wolfSSL_Logging_cbmia);
		wolfSSL_Debugging_ON();

#ifdef WIN32
		char buffer[4048];
		wolfSSL_get_ciphers(buffer, sizeof(buffer));
		CONSOLE_PRINTER("WOLFSSL: Cipher Suites %s\n", buffer);
#endif

	}
	else
	{
		wolfSSL_Debugging_OFF();
	}
	return s;
}

static int G_Socket_Connect_wolffSsl(t_Socket s)
{
	int ret = G_Socket_Connect(s);
	if(!ret)
	{
		ret = wolfSSL_connect(s->wolfSsl); // Initiate SSL connection
        if (WOLFSSL_SUCCESS == ret)
		{
			return 0;
		}
		return ret;
	}
	else
	{
		return ret;
	}
}
static int G_Socket_Read_Wolfssl(t_Socket s, char* buffer, size_t size)
{
	int ret;
	char block = s->noBlocking;
	s->noBlocking = TRUE;
	ret = wolfSSL_read(s->wolfSsl, buffer, size);
	s->noBlocking = block;

	if(wolfSSL_get_error(s->wolfSsl, ret) == WOLFSSL_ERROR_WANT_READ)
	{
		if(G_Socket_IsReady(s) == 3)
		{
			return 0;
		}
	}

	return ret;
}

static int G_Socket_Write_Wolfssl(t_Socket s, const char* message, size_t bufferLength)
{
	return wolfSSL_write(s->wolfSsl, message, bufferLength);
}

static void G_Socket_Free_wolfssl(t_Socket *s)
{
	if (s && *s)
	{
		if((*s)->wolfSslCtx)
		{
			wolfSSL_CTX_free((*s)->wolfSslCtx);
			(*s)->wolfSslCtx = 0;
		}
	
		if((*s)->wolfSsl)
		{
			wolfSSL_free((*s)->wolfSsl);
			(*s)->wolfSsl = 0;
		}
	}
	G_Socket_Free(s);
}

static struct _ISocket secure_socket_wolfssl =
{
	NULL,
	NULL,
	(void* (*) (char*, int)) G_Socket_WolfSsl_Secure_New,
	(int (*) (void*)) G_Socket_Connect_wolffSsl,
	(int (*) (void*, const char*, size_t)) G_Socket_Write_Wolfssl,
	(int (*) (void*, char *, size_t)) G_Socket_Read_Wolfssl,
	(int (*) (void* socket)) G_Socket_IsReady,
	(int (*) (void* socket)) G_Socket_Free_wolfssl
/*
		.M_Socket_New = (void* (*) (char*, int)) G_Socket_New,
		.M_Socket_Connect 	= (int (*) (void*)) G_Socket_Connect,
		.M_Socket_Write 	= (int (*) (void*, const char*, size_t)) G_Socket_Write,
		.M_Socket_Read		= (int (*) (void*, char *, size_t)) G_Socket_Read,
		.M_Socket_Free		= (int (*) (void* socket)) G_Socket_Free,
		.M_Socket_IsReady 	= (int (*) (void* socket)) G_Socket_IsReady
*/
};

#endif



static struct _ISocket unsecure_socket =
{
	NULL,
	NULL,
	(void* (*) (char*, int)) G_Socket_New,
	(int (*) (void*)) G_Socket_Connect,
	(int (*) (void*, const char*, size_t)) G_Socket_Write,
	(int (*) (void*, char *, size_t)) G_Socket_Read,
	(int (*) (void* socket)) G_Socket_IsReady,
	(int (*) (void* socket)) G_Socket_Free
/*
		.M_Socket_New = (void* (*) (char*, int)) G_Socket_New,
		.M_Socket_Connect 	= (int (*) (void*)) G_Socket_Connect,
		.M_Socket_Write 	= (int (*) (void*, const char*, size_t)) G_Socket_Write,
		.M_Socket_Read		= (int (*) (void*, char *, size_t)) G_Socket_Read,
		.M_Socket_Free		= (int (*) (void* socket)) G_Socket_Free,
		.M_Socket_IsReady 	= (int (*) (void* socket)) G_Socket_IsReady
*/
};

static struct _ISocket secure_socket =
{
	NULL,
	NULL,
	(void* (*) (char*, int)) G_Socket_Secure_New,
	(int (*) (void*)) G_Socket_Connect,
	(int (*) (void*, const char*, size_t)) G_Socket_Write,
	(int (*) (void*, char *, size_t)) G_Socket_Read,
	(int (*) (void* socket)) G_Socket_IsReady,
	(int (*) (void* socket)) G_Socket_Free
/*
		.M_Socket_New = (void* (*) (char*, int)) G_Socket_New,
		.M_Socket_Connect 	= (int (*) (void*)) G_Socket_Connect,
		.M_Socket_Write 	= (int (*) (void*, const char*, size_t)) G_Socket_Write,
		.M_Socket_Read		= (int (*) (void*, char *, size_t)) G_Socket_Read,
		.M_Socket_Free		= (int (*) (void* socket)) G_Socket_Free,
		.M_Socket_IsReady 	= (int (*) (void* socket)) G_Socket_IsReady
*/
};


t_ISocket G_Socket_CreateUnsecure()
{
	return &unsecure_socket;
}

t_ISocket G_Socket_CreateSecure()
{
	(void)secure_socket;
	
	#if USE_WOLF_SSL
	return &secure_socket_wolfssl;
	#else
	return &secure_socket;
	#endif
}

unsigned int G_Monotonic_Time(void)
{
	static uint32 lastVal;
	static uint16 loops;

	OSAL_T_TimeCounter counter; 
	
	OSAL_eTimeGetCounter(&counter);
		
	if(counter.u32MilliSeconds < lastVal) // timer overflow
		loops++;

	lastVal = counter.u32MilliSeconds;

	return (counter.u32MilliSeconds / 1000) + (loops * ((UINT32_MAX / 1000) + 1));
}

void * G_ThreadLock_New_OS(void)
{
	OSAL_T_Semaphore * lock = (OSAL_T_Semaphore*) malloc(sizeof(OSAL_T_Semaphore));	

	OSAL_vSemCreate(lock, 1);

	return lock;
}

int G_ThreadLock_Lock_OS(void * mutex)
{
	if(!mutex)
	{
#ifndef NDEBUG
		CONSOLE_PRINTER("G_ThreadLock_Lock_OS: bad mutex\n");
#endif
		return 0;
	}
#ifdef NDEBUG
	OSAL_vSemPend((OSAL_T_Semaphore*)mutex);
	return 0;
#else
	while(1)
	{
		
		OSAL_E_Status status = OSAL_eSemPendTimeOut((OSAL_T_Semaphore*)mutex, 5000);

		if(status == OS_ERR_NONE)
		{
			return 0;
		}
		else
		{
			CONSOLE_PRINTER("G_ThreadLock_Lock_OS: timeout %x\n", (uint32) mutex);
		}
	}
#endif
}

int G_ThreadLock_Unlock_OS(void * mutex)
{
	OSAL_vSemPost((OSAL_T_Semaphore*)mutex);
	return 0;
}

void G_ThreadLock_Free_OS(void * mutex)
{
	if(mutex) {
		OSAL_vSemDelete((OSAL_T_Semaphore*)mutex);
		free(mutex);
	}
}

void G_Socket_set_pin_cb(void(*cb)(int action, void * param))
{
	s_modem.pinCb = cb;
}

int G_Socket_set_certs(const char *cacert, const char * publicKey, const char * privateKey)
{
	s_socket.cacertificate = cacert;
	s_socket.x509privatekey = privateKey;
	s_socket.x509certificate = publicKey;
	return 0;
}

