/*
 * WS_Websocket.h
 *
 *  Created on: Sep 25, 2014
 *      Author: Tuan Vu
 */

#ifndef WS_WEBSOCKET_H_
#define WS_WEBSOCKET_H_

#include "WS_Socket.h"
#include "WS_Util.h"

struct _WebsocketPrivate;
struct _WebsocketInfo;

/*****************************************************************************************************/
/*********************                 Websocket                      ********************************/
/*****************************************************************************************************/

/**
 * \defgroup MIA_LibWebsocket cpmPlus Embedded SDK - Websocket C Library
 */

/**
 * @brief t_Websocket Pointer to Websocket connection struct
 * \ingroup MIA_LibWebsocket
 */
typedef struct _Websocket
{
	struct _WebsocketPrivate *m_Private;
	struct _WebsocketInfo* m_Info;
} *t_Websocket;

typedef struct _WebsocketInfo
{
	t_String m_ConnectionString;
	char *m_Host;
	char *m_CustomHandshake;
	int  	m_BufferSize;
	int 	m_TimeOut;
	int 	m_AutoReconnect;
	int 	m_MaxQueueSize;
	int 	m_WebsocketVersion;
	void* m_CallbackData;
	t_String m_Username;
	t_String m_Password;

	/* Socket implementation, otherwise will use the default*/
	t_ISocket m_Socket;

	/* Callbacks*/
	int (*M_OnMessage)           (t_Websocket websocket, const char* message, int length);
	int (*M_OnError)             (t_Websocket websocket, int errorCode, const char* additionalInformation);
	int (*M_OnDisconnect)        (t_Websocket websocket, int reasonCode);
	int (*M_OnConnected)         (t_Websocket websocket);
	int (*M_OnAdditionalRequest) (t_Websocket websocket, const char* message, int length);

#ifdef __OPENSSL__
	char*	m_SSLPrivateCertificate;
	char*	m_SSLPublicCertificate;
#endif
} * t_WebsocketInfo;

struct _MessagePrivate;

typedef struct _Message
{
	struct _MessagePrivate *m_Private;
} *t_Message;

enum t_Websocket_OpCode
{
	g_WEBSOCKET_CONTINUATION = 0,
	g_WEBSOCKET_TEXT = 1,
	g_WEBSOCKET_BINARY = 2,
	g_WEBSOCKET_CLOSE = 8,
	g_WEBSOCKET_PING = 0x09,
	g_WEBSOCKET_PONG = 0x0A
};

enum t_WebsocketState
{
	g_WS_INITIAL,
	g_WS_CONNECTING,
	g_WS_CONNECTED,
	g_WS_HANDSHAKING,
	g_WS_ADDITONALREQUEST,
	g_WS_READY,
	g_WS_DISCONNECTING,
	g_WS_DISCONNECTED,
	g_WS_ERROR
};
/**
 * @brief Create new message with length
 * @param length Reserved length of the message
 * \ingroup MIA_LibWebsocket
 */
t_Message G_Message_New(size_t length);

/**
 * @brief Create new Ping message
 * \ingroup MIA_LibWebsocket
 */
t_Message G_Message_NewPing(char length);

/**
 * @brief Create new Pong
 * \ingroup MIA_LibWebsocket
 */
t_Message G_Message_NewPong(int length);

/**
 * @brief Create new text message
 * \ingroup MIA_LibWebsocket
 */
t_Message G_Message_NewText(long int length);

/**
 * @brief Create new binary message
 * \ingroup MIA_LibWebsocket
 */
t_Message G_Message_NewBinary(size_t length);

/**
 * @brief Get data pointer of the message
 * \ingroup MIA_LibWebsocket
 */
char*		G_Message_Data(t_Message message);

/**
 * @brief Get size of the message
 * \ingroup MIA_LibWebsocket
 */
int		G_Message_Size(t_Message message);

/**
 * @brief Write to the message buffer
 * \ingroup MIA_LibWebsocket
 */
int		G_Message_Write(t_Message message, const char* data, int index, int length);

/**
 * @brief Append to the message buffer
 * \ingroup MIA_LibWebsocket
 */
int		G_Message_Append(t_Message message, const char* data, int length);

/**
 * @brief Get type of the message
 * \ingroup MIA_LibWebsocket
 */
enum t_Websocket_OpCode G_Message_GetType(t_Message message);

/**
 * @brief Free the message
 * \ingroup MIA_LibWebsocket
 */
void G_Message_Free(t_Message* message);

/**
 * @brief Create new t_WebsocketInfo
 * \ingroup MIA_LibWebsocket
 */
t_WebsocketInfo G_WebsocketInfo_New(void);

/**
* @brief Create new t_WebsocketInfo
* \ingroup MIA_LibWebsocket
*/
void G_WebsocketInfo_Free(t_WebsocketInfo*);

/**
 * @brief Create new t_Websocket
 * \ingroup MIA_LibWebsocket
 */
t_Websocket G_Websocket_New(t_WebsocketInfo info);

/**
* @brief Get Websocket state
* \ingroup MIA_LibWebsocket
*/
enum t_WebsocketState G_Websocket_GetState(t_Websocket websocket);

/**
 * @brief Count items in websocket buffer or <0 if websocket is not ready not send
 * \ingroup MIA_LibWebsocket
 */
int G_Websocket_BufferCnt(t_Websocket websocket);

/**
 * @brief Service the websocket. Supposed to be called in an interval to handle
 * internal operations of the socket
 * \ingroup MIA_LibWebsocket
 */
void G_Websocket_Service(t_Websocket websocket);

/**
 * @brief Connect the websocket
 * \ingroup MIA_LibWebsocket
 */
int G_Websocket_Connect(t_Websocket websocket);

/**
 * @brief Send message via the socket
 * @param message The message
 * @param length Length of the message
 * \ingroup MIA_LibWebsocket
 */
int G_Websocket_Send(t_Websocket websocket, const char* message, int length);

/**
 * @brief Send binary message via the socket
 * @param message The message
 * @param length The length of the message
 * \ingroup MIA_LibWebsocket
 */
int G_Websocket_SendBinary(t_Websocket websocket, const char *message, int length);

/**
* @brief Send raw message via the socket
* @param message The raw message
* \ingroup MIA_LibWebsocket
*/
int G_Websocket_SendRaw(t_Websocket websocket, t_Message message);

/**
 * @brief Send the message
 * @param message The message
 * \ingroup MIA_LibWebsocket
 */
int G_Websocket_SendMessage(t_Websocket websocket, t_Message message);

/**
 * @brief Free the websocket
 * \ingroup MIA_LibWebsocket
 */
void G_Websocket_Free(t_Websocket* websocket);

/**
 * @brief Seconds from fixed point history. 
 * 
 */
unsigned int G_Monotonic_Time(void);

#ifdef WIN32
	#define strncasecmp(x,y,z) _strnicmp(x,y,z)
	#ifdef USE_DLOG_WEBSOCKET
		void DLOG_vPrintF (char * format, ...);
		#define printf DLOG_vPrintF
	#endif
	#define INLINE __inline
#else
	#ifndef INLINE
		#define INLINE inline
	#endif
#endif

char * strdup_os(const char *org);


#endif /* RTDB_WEBSOCKET_H_ */
