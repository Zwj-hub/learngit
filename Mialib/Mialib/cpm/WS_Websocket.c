#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "WS_Error.h"
#include "WS_Socket.h"
#include "WS_Util.h"
#include "WS_Websocket.h"

#include "assert.h"
#define ASSERT(a) assert(a)

#include <string.h>

#ifndef WEBSOCKET_AUTO_RECONNECT
#define WEBSOCKET_AUTO_RECONNECT 1
#endif

static const char m_k_Encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			     "abcdefghijklmnopqrstuvwxyz0123456789+/";
			
#ifndef G_M_K_S_DEFAULT_BUFFER_SIZE
	#define  G_M_K_S_DEFAULT_BUFFER_SIZE 8096
#endif

static const int m_k_s_DEFAULT_BUFFER_SIZE = G_M_K_S_DEFAULT_BUFFER_SIZE;
static const int m_k_s_DEFAULT_QUEUE_SIZE = 256;
static const int m_k_s_DEFAULT_SOCKET_TIMEOUT = 30;
#ifdef _WIN64
	#define strcasecmp(s1, s2) strcmpi(s1, s2) 
	#define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n) 
#endif 

#undef G_M_K_S_DEFAULT_BUFFER_SIZE

#ifndef G_LONG_STRING_LEN
	#define G_LONG_STRING_LEN 256
#endif

#ifndef WS_PING_TIMEOUT
	#define WS_PING_TIMEOUT 50
#endif

#ifndef WS_PING_INTERVAL
	#define WS_PING_INTERVAL 50
#endif

#ifndef TEMP_BUFFER_LEN
	#define TEMP_BUFFER_LEN 1024
	#define TEMP_BUFFER_MAX_ELEMENTS 50
#endif
#ifdef WEBSOCKET_DEBUG
	#define C_THREAD_DEBUG 1
#else
	#define C_THREAD_DEBUG 0
#endif

char g_WS_WebsocketDebugEnable = 1;
#define MIA_DEBUG_PRINT(...) if (C_THREAD_DEBUG && g_WS_WebsocketDebugEnable) do{ printf("[Mia_Websocket]: "); { printf(__VA_ARGS__); } } while(0)

static char * duplicate(char * dst, const char * src, unsigned int len)
{
	dst = (char*) realloc(dst, len + 1);
	
	if(!dst)
	{
		ASSERT(0);
		return NULL;
	}
	
	strncpy(dst, src, len);
	dst[len] = 0;
	
	return dst;
}

char * strdup_os(const char *org){
	return duplicate(NULL, org, strlen(org));
}


int G_Encode(const char *inputString, int inputLength, char *outputString, int outputLength)
{
	unsigned char triple[3];
	int i;
	int len;
	int line = 0;
	int done = 0;

	while (inputLength)
	{
		len = 0;
		for (i = 0; i < 3; i++) {
			if (inputLength) {
				triple[i] = *inputString++;
				len++;
				inputLength--;
			} else
				triple[i] = 0;
		}
		if (!len)
			continue;

		if (done + 4 >= outputLength)
			return -1;

		*outputString++ = m_k_Encode[triple[0] >> 2];
		*outputString++ = m_k_Encode[((triple[0] & 0x03) << 4) |
					     ((triple[1] & 0xf0) >> 4)];
		*outputString++ = (len > 1 ? m_k_Encode[((triple[1] & 0x0f) << 2) |
					     ((triple[2] & 0xc0) >> 6)] : '=');
		*outputString++ = (len > 2 ? m_k_Encode[triple[2] & 0x3f] : '=');

		done += 4;
		line += 4;
	}

	if (done + 1 >= outputLength)
		return -1;

	*outputString++ = '\0';

	return done;
}


/*****************************************************************************************************/
/*********************                 Websocket                      ********************************/
/*****************************************************************************************************/

typedef struct _Header
{
	/* Actualy header bytes:
	enum t_Websocket_OpCode m_OpCode : 4;
	unsigned char 		m_RSV3 : 1;
	unsigned char 		m_RSV2 : 1;
	unsigned char 		m_RSV1 : 1;
	unsigned char 		m_FIN : 1;
	unsigned char 		m_PayLoad : 7;
	unsigned char 		m_MASK : 1;*/
	unsigned char		m_Header[2];
} t_Header;

void G_Header_SetOpCode(t_Header *header, enum t_Websocket_OpCode code)
{
	header->m_Header[0] = (header->m_Header[0] & 0xF0) | (code & 0x0F);
};

enum t_Websocket_OpCode G_Header_GetOpCode(t_Header *header)
{
	enum t_Websocket_OpCode code = (enum t_Websocket_OpCode) ((header->m_Header[0]) & 0x0F);
	
	return code;
};

void G_Header_SetFin(t_Header *header, unsigned char fin)
{
	if (fin) header->m_Header[0] = header->m_Header[0] | (1 << 7);
	else header->m_Header[0] = header->m_Header[0] & 0x7F;
}

unsigned char G_Header_GetFin(t_Header *header)
{
	return (header->m_Header[0] >> 7) & 0x01;
}

void G_Header_SetPayload(t_Header *header, unsigned char payload)
{
	header->m_Header[1] = (header->m_Header[1] & 0x80) | (payload & 0x7F);
}

unsigned char G_Header_GetPayload(t_Header *header)
{
	return (header->m_Header[1] & 0x7F);
}

void G_Header_SetMask(t_Header *header, unsigned char mask)
{
	if (mask) header->m_Header[1] = header->m_Header[1] | (1 << 7);
	else header->m_Header[1] = header->m_Header[1] & 0x7F;
}

unsigned char G_Header_GetMask(t_Header *header)
{
	return (header->m_Header[1] >> 7) & 0x01;
}
struct _MessageDataFrame
	{
		t_Header m_Header;

		unsigned char 		m_Data[1];
	};
typedef struct _MessagePrivate
{
	unsigned long long int 	m_Length;
	int 	m_Sent;
	int 	m_DataIndex;
	 struct _MessageDataFrame *m_DataFrame;

} *t_MessagePrivate;

typedef char _test__MessageDataFrame[ sizeof(struct _MessageDataFrame) == 3 ? 1 : -1 ]; // Test that _MessageDataFrame size is correct. This struct is used at binary transfer thus padding is not accepted 


enum t_Websocket_OpCode G_Message_GetType(t_Message message)
{
	return G_Header_GetOpCode(&message->m_Private->m_DataFrame->m_Header);
}

t_Message G_Message_NewPing(char length)
{
	t_Message t = G_Message_New(length);
	if (t)
	{
		t_Header *header = &t->m_Private->m_DataFrame->m_Header;
		G_Header_SetOpCode(header, g_WEBSOCKET_PING);
		G_Header_SetFin(header, 1);
		G_Header_SetPayload(header, length);
		G_Header_SetMask(header, 1);
		return t;
	} else return 0;
}

t_Message G_Message_NewPong(int length)
{
	t_Message t = G_Message_New(length);
	if (t)
	{
		t_Header *header = &t->m_Private->m_DataFrame->m_Header;
		G_Header_SetOpCode(header, g_WEBSOCKET_PONG);
		G_Header_SetFin(header, 1);
		G_Header_SetPayload(header, length);
		return t;
	} else return 0;
}

t_Message G_Message_NewText(long int length)
{
	t_Message t = G_Message_New(length);
	if (t && length > 0)
	{
		t_Header *header = &t->m_Private->m_DataFrame->m_Header;
		G_Header_SetOpCode(header, g_WEBSOCKET_TEXT);
		G_Header_SetFin(header, 1);

		if (length < 126)
		{
			G_Header_SetPayload(header, (unsigned char)length);		} else if (length < 0xFFFF)
		{
			t->m_Private->m_DataFrame->m_Data[0] = ((length >> 8) & 0xFF);
			t->m_Private->m_DataFrame->m_Data[1] = length & 0xFF;
			/*((unsigned short*)&(t->m_Private->m_DataFrame->m_Data[0])) = length;*/
			G_Header_SetPayload(header, 126);
		} else
		{
			t->m_Private->m_DataFrame->m_Data[0] = (length >> 24) & 0xFF;
			t->m_Private->m_DataFrame->m_Data[1] = (length >> 16) & 0xFF;
			t->m_Private->m_DataFrame->m_Data[2] = (length >> 8) & 0xFF;
			t->m_Private->m_DataFrame->m_Data[3] = length & 0xFF;
			/*((unsigned short*)&(t->m_Private->m_DataFrame->m_Data[0])) = length;*/
			G_Header_SetPayload(header, 127);
		}
		return t;
	} else return 0;
}

t_Message G_Message_NewBinary(size_t length)
{
	t_Message t = G_Message_New(length);
	if (t)
	{
		t_Header *header = &t->m_Private->m_DataFrame->m_Header;
		//G_Header_SetOpCode(header, g_WEBSOCKET_TEXT);
		G_Header_SetOpCode(header, g_WEBSOCKET_BINARY);
		G_Header_SetFin(header, 1);
		if (length < 126)
		{
			G_Header_SetPayload(header, (unsigned char) length);
		} else if (length < 0xFFFF)
		{
			t->m_Private->m_DataFrame->m_Data[0] = ((length >> 8) & 0xFF);
			t->m_Private->m_DataFrame->m_Data[1] = length & 0xFF;
			G_Header_SetPayload(header, 126);
		} else
		{
			t->m_Private->m_DataFrame->m_Data[0] = ((uint64_t)length >> 56) & 0xFF;
			t->m_Private->m_DataFrame->m_Data[1] = ((uint64_t)length >> 48) & 0xFF;
			t->m_Private->m_DataFrame->m_Data[2] = ((uint64_t)length >> 40) & 0xFF;
			t->m_Private->m_DataFrame->m_Data[3] = ((uint64_t)length >> 32) & 0xFF;
			t->m_Private->m_DataFrame->m_Data[4] = (length >> 24) & 0xFF;
			t->m_Private->m_DataFrame->m_Data[5] = (length >> 16) & 0xFF;
			t->m_Private->m_DataFrame->m_Data[6] = (length >> 8) & 0xFF;
			t->m_Private->m_DataFrame->m_Data[7] = length & 0xFF;
			G_Header_SetPayload(header, 127);
		}
		return t;
	} else return 0;
}

void G_Message_SetMask(t_Message m, unsigned int mask)
{
	t_MessagePrivate mp = m->m_Private;
	if (mp->m_Length < 126)
	{
		*((unsigned int*) &(mp->m_DataFrame->m_Data[0])) = mask;
	} else if (mp->m_Length < 0xFFFF)
	{
		*((unsigned int*) &(mp->m_DataFrame->m_Data[2])) = mask;
	} else
	{
		*((unsigned int*) &(mp->m_DataFrame->m_Data[4])) = mask;
	}
}

t_Message G_Message_FromBinary(char* data)
{
	t_Message m = (t_Message)malloc(sizeof(struct _Message));
	t_MessagePrivate mp;
	unsigned char * dataptr;
	t_Header *header;

	if (!m)
	{
		G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		return 0;
	}

	mp = (t_MessagePrivate)malloc(sizeof (struct _MessagePrivate));
	if (!mp)
	{
		free(m);
		G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		return 0;
	}

	mp->m_DataFrame = (struct _MessageDataFrame*) data;
	header = &mp->m_DataFrame->m_Header;

	if (G_Header_GetPayload(header) < 126) mp->m_Length = G_Header_GetPayload(header);
	else if (G_Header_GetPayload(header) ==126)
	{
		dataptr = (unsigned char *) (G_Header_GetMask(header) ? &(mp->m_DataFrame->m_Data[4]) : &(mp->m_DataFrame->m_Data[0]));
		mp->m_Length = dataptr[0] << 8 | dataptr[1];
	} else
	{
		dataptr = (unsigned char *) ((G_Header_GetMask(header) ? &(mp->m_DataFrame->m_Data[10]) : &(mp->m_DataFrame->m_Data[0])));
		mp->m_Length = (
				((uint64_t) dataptr[0] << (8 * 7)) |
				((uint64_t) dataptr[1] << (8 * 6)) |
				((uint64_t) dataptr[2] << (8 * 5)) |
				((uint64_t) dataptr[3] << (8 * 4)) |
				((uint64_t) dataptr[4] << (8 * 3)) |
				((uint64_t) dataptr[5] << (8 * 2)) |
				((uint64_t) dataptr[6] << (8 * 1)) |
				((uint64_t) dataptr[7] << (8 * 0)) );
	}

	mp->m_DataIndex = (int) mp->m_Length+1;
	m->m_Private = mp;
	return m;
}

t_Message G_Message_New(size_t length)
{
	t_Message m = (t_Message)malloc(sizeof(struct _Message));
	t_MessagePrivate mp;
	const int basesize = 2 + 4; // header + mask key
	size_t size;

	if (!m)
	{
		G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		return 0;
	}


	if (length < 126)
	{
		size = length + basesize;
	} else if (length < 0xFFFF)
	{
		size = length + basesize + 2;  // header + mask key + ext 16 bit length
	} else
	{
		size = length + basesize + 8;// header + mask key + ext 64 bit length
	}

	mp = (t_MessagePrivate)malloc(sizeof (struct _MessagePrivate));
	if (!mp)
	{
		free(m);
		G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		return 0;
	}

	memset(mp, 0, sizeof (struct _MessagePrivate));
	mp->m_Length = length;
	mp->m_DataFrame = (struct _MessageDataFrame *)malloc(size);

	if (!mp->m_DataFrame)
	{
		free(mp);
		free(m);
		G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	
	memset(mp->m_DataFrame, 0, size);

	G_Header_SetMask(&mp->m_DataFrame->m_Header, 1);
	m->m_Private = mp;
	G_Message_SetMask(m, G_Random());

	return m;
}

char* G_Message_Data(t_Message message)
{
	int length = (int) message->m_Private->m_Length;
	int mask = G_Header_GetMask(&message->m_Private->m_DataFrame->m_Header);
	if (mask)
	{
		if (length < 126)
		{
			return (char*)&(message->m_Private->m_DataFrame->m_Data[4]);
		} else if (length < 0xFFFF)
		{
			return (char*)&(message->m_Private->m_DataFrame->m_Data[6]);
		} else
		{
			return (char*)&(message->m_Private->m_DataFrame->m_Data[12]);
		}
	} else
	{
		if (length < 126)
		{
			return (char*)&(message->m_Private->m_DataFrame->m_Data[0]);
		} else if (length <= 0xFFFF)
		{
			return (char*)&(message->m_Private->m_DataFrame->m_Data[2]);
		} else
		{
			return (char*) &(message->m_Private->m_DataFrame->m_Data[8]);
		}
	}
}

char* G_Message_Frame(t_Message message)
{
	return (char*)(message->m_Private->m_DataFrame);
}

int G_Message_FrameLength(t_Message message)
{
	int length = (int) message->m_Private->m_Length;
	int ret = 2 + (length < 126 ? 0 : (length <= 0xFFFF ? 2 : 8)) + (G_Header_GetMask(&message->m_Private->m_DataFrame->m_Header) ? 4 : 0);
	return ret+length;
}

int G_Message_Size(t_Message message)
{
	return (int) message->m_Private->m_Length;
}

int G_Message_Mask(t_Message message)
{
	int length = (int) message->m_Private->m_Length;
	t_MessagePrivate p = message->m_Private;
	return *((int*)(length < 126 ? &p->m_DataFrame->m_Data[0] :
			(length < 0xFFFF ? &p->m_DataFrame->m_Data[2] : &p->m_DataFrame->m_Data[4])));
}

int	G_Message_Write(t_Message message, const char* data, int index, int length)
{
	char* dataptr = G_Message_Data(message);

	if (message->m_Private->m_Length - index >= length)
	{
		message->m_Private->m_DataIndex = index+length+1;
		memcpy(&dataptr[index], data, length);

		/* apply masking*/
		if (G_Header_GetMask(&message->m_Private->m_DataFrame->m_Header))
		{
			int i;
			int count = index + length;
			int mask = G_Message_Mask(message);
			char *c = (char*)&mask;
			for (i = index; i < count; i++)
			{
				dataptr[i] = dataptr[i] ^ c[i%4];
			}
		}
		return 0;
	} else
	{
		G_SetLastError(G_ERROR_BUFFER_FULL);
		MIA_DEBUG_PRINT("\n[Mia_Websocket]: Printing dataptr %d\\%llu\n", length, message->m_Private->m_Length);
		return -1;
	}
}

int	G_Message_Append(t_Message message, const char* data, int length)
{
	char* dataptr = G_Message_Data(message);
	int index = message->m_Private->m_DataIndex;

	if (message->m_Private->m_Length - index >= length)
	{
		message->m_Private->m_DataIndex = index+length+1;
		memcpy(&dataptr[index], data, length);
		return 0;
	} else
	{
		G_SetLastError(G_ERROR_BUFFER_FULL);
		return -1;
	}
}

void G_Message_Free(t_Message* message)
{
	if (*message)
	{
		/* Empty the queue*/
		free((*message)->m_Private->m_DataFrame);
		free((*message)->m_Private);
		free(*message);
		*message = 0;
	}
}

typedef struct _WebsocketPrivate
{
	char 		*m_HostName;    // Hostname is domain name or ip addr
	char		*m_HostAddress; // Host add is IP address of host or 0.0.0.0
	int 		m_Port;
	char		*m_Path;
	int 		m_SSL;

	t_ISocket m_Socket;
	void*		 m_SocketImp;
	char 		*m_Buffer;
	int 		m_BufferIndex;
	char		m_InitialHandshakeSHA[128];

	unsigned int m_LastModified;
	unsigned int m_LastPingTime;
	unsigned int m_LastPongTime;
	unsigned int m_LastSendTime;

	enum t_WebsocketState m_State;
	int 		m_ToRead;
	char*		m_ExternalBuffer;
	t_List	m_MessageQueue;

} *t_WebsocketPrivate;

static void Websocket_PrivateFree(t_WebsocketPrivate *p);

int G_Websocket_SendHandshake(t_Websocket websocket);


void G_Websocket_TimeoutUpdate(t_Websocket websocket)
{
	websocket->m_Private->m_LastModified = G_Monotonic_Time();
}
void G_Websocket_TimeoutUpdatePong(t_Websocket websocket)
{
	websocket->m_Private->m_LastPongTime = G_Monotonic_Time();
}

void G_Websocket_ChangeState(t_Websocket websocket, enum t_WebsocketState state)
{
	websocket->m_Private->m_State = state;
	G_Websocket_TimeoutUpdate(websocket);
}


static void ChangeStateError(t_Websocket websocket, int errorCode, const char * reason)
{
	G_Websocket_ChangeState(websocket, g_WS_ERROR);
	websocket->m_Info->M_OnError(websocket, errorCode, reason);
}


t_WebsocketPrivate G_WebsocketPrivate_FromString(t_Websocket ws, const char* connectionString)
{
	int index;
	int nextindex;
	int length = (int) strlen(connectionString);
	t_WebsocketPrivate priv;

	if(!ws->m_Private)
	{
		priv = (t_WebsocketPrivate)malloc(sizeof(struct _WebsocketPrivate));
		if(priv)
		{
			memset(priv, 0, sizeof(struct _WebsocketPrivate));
		}
	}
	else
	{
		priv = ws->m_Private;
	}

	if (priv)
	{
		priv->m_Buffer = (char *)realloc(priv->m_Buffer, m_k_s_DEFAULT_BUFFER_SIZE);
		if(!priv->m_Buffer)
		{
			free(priv);
			return 0;
		}

		if(!priv->m_MessageQueue) priv->m_MessageQueue = G_List_New();
		if(!priv->m_MessageQueue)
		{
			free(priv->m_Buffer);
			free(priv);
			return 0;
		}
	}
	else 
		return 0;

	/* parse connection string*/
	ws->m_Private = priv;
	index = 0;
	G_Websocket_ChangeState(ws, g_WS_INITIAL);

	if (strncmp(connectionString, "wss", 3) == 0)
	{
		priv->m_SSL = 1;
		priv->m_Port = 443;
		index = 6;
	} else if (strncmp(connectionString, "wst", 3) == 0)
	{
		priv->m_SSL = 2;
		priv->m_Port = 443;
		index = 6;
	} else
	{
		priv->m_Port = 80;
		index = 5;
	}

	/* getting the host name / address*/
	nextindex = index;
	while (nextindex < length)
	{
		char c = connectionString[nextindex++];
		if (c == ':')
		{
			int port = strtol(&(connectionString[nextindex]), NULL, 0);
			int hostlength;
			
			if (port) 
				priv->m_Port = port;
			
			hostlength = nextindex - index;
			priv->m_HostName = duplicate(priv->m_HostName, &(connectionString[index]), hostlength-1);

			while (connectionString[nextindex] != '/' && nextindex < length) 
				nextindex++;
			
			nextindex++;
			break;
		}

		if (c == '/')
		{
			int hostlength = nextindex - index;
			priv->m_HostName = duplicate(priv->m_HostName, &(connectionString[index]), hostlength-1);
			break;
		}
	}

	/* copy the path*/
	if (nextindex < length)
	{
		int copylength = length - nextindex;
		priv->m_Path = duplicate(priv->m_Path, &(connectionString[nextindex-1]), copylength+1);
	} else
	{
		priv->m_Path = duplicate(priv->m_Path, "/", 1);
	}

	return priv;
}

t_Websocket G_Websocket_New_FromString(const char* connectionString)
{
	t_Websocket ws;
	t_WebsocketPrivate priv;
	int length = (int)strlen(connectionString);
	if (length <= 3) { G_SetLastError(G_ERROR_UNINITIALIZED); return 0; }

	ws = (struct _Websocket*) malloc(sizeof(struct _Websocket));
	memset(ws, 0, sizeof(struct _Websocket));

	if (ws)
	{
		priv = G_WebsocketPrivate_FromString(ws, connectionString);
		if (priv)
		{
			return ws;
		}
		else
		{
			free (ws);
			G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		}
	} else
	{
		G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
	}
	return 0;
}

t_Websocket G_Websocket_New(t_WebsocketInfo info)
{
	t_Websocket ws = G_Websocket_New_FromString(G_String_Data(info->m_ConnectionString));

	if(!ws)
	{
		return NULL;
	}

	ws->m_Info = info;

	if (!(info->m_Socket))
	{
		/* Using default socket implementation here*/
		if (ws->m_Private->m_SSL)
		{
			/* Using secure websocket*/
			ws->m_Private->m_Socket = G_Socket_CreateSecure();
		} else
		{
			/* Using non-secure version*/
			ws->m_Private->m_Socket = G_Socket_CreateUnsecure();
		}
	} else
	{
		ws->m_Private->m_Socket = info->m_Socket;
	}
	return ws;
}

int G_Websocket_Timeout(t_Websocket websocket)
{
	/* check timeout*/
if ((unsigned int)(G_Monotonic_Time() - websocket->m_Private->m_LastModified) > (unsigned int)(websocket->m_Info->m_TimeOut + WS_PING_TIMEOUT))	{
		ChangeStateError(websocket, G_ERROR_TIME_OUT, "Websocket request had time-out");
		return 1;
	}
	return 0;
}

int G_Websocket_Timeout_Ping(t_Websocket websocket)
{
	/* check timeout*/
	if((int)(websocket->m_Private->m_LastPongTime - websocket->m_Private->m_LastPingTime) >= 0)
	{
		return 0; // Pong received after ping
	}

	if ((unsigned int)(G_Monotonic_Time() - websocket->m_Private->m_LastPingTime) > (unsigned int)(websocket->m_Info->m_TimeOut + WS_PING_TIMEOUT))
	{
		ChangeStateError(websocket, G_ERROR_TIME_OUT, "Websocket Ping/pong had time-out");
		return 1;
	}
	return 0;
}


/* This method parse the handshake and check for error*/
/* Return 0 if everything is OK*/
int G_Websocket_ParseHandshake(t_Websocket websocket)
{
	/* buffer received package to p*/
	int length = (int)strlen(websocket->m_Private->m_Buffer);
	char* fr;
	int i = 0;
	char* p = (char*) malloc(length + 1);
	if(!p)
		return -1;
	fr = p;
	strncpy(p, websocket->m_Private->m_Buffer, length);
	p[length] = '\0';

	G_Websocket_ChangeState(websocket, g_WS_READY);
	while (p[i])
	{
		p[i] = tolower(p[i]);
		i++;
	}

	if (strncasecmp(p, "HTTP/1.1", 8) != 0)
	{
		char* buffer = (char*) malloc(length + 100);
		G_SetLastError(G_ERROR_INVALID_HTTP_RESPONSE);
		if(buffer)
		{
			sprintf(buffer, "The client expects HTTP header response to be HTTP/1.1 but received %s", websocket->m_Private->m_Buffer);
		}
		ChangeStateError(websocket, G_ERROR_INVALID_HTTP_RESPONSE, buffer ? buffer : "NA");
		free(buffer);
		return -1;
	} else
	{
		int response = 0;
		p+=9;

		/* Get response code*/
		response = strtol(p, 0, 0);

		if (response == 404)
		{
			G_SetLastError(G_ERROR_INVALID_CONNECTION_STRING);
			ChangeStateError(websocket, G_ERROR_INVALID_CONNECTION_STRING, "The provided connection string is invalid, please check.");
		} else if (response == 401)
		{
			G_SetLastError(G_ERROR_UNAUTHORIZED);
			ChangeStateError(websocket, G_ERROR_UNAUTHORIZED, "The provided authentication is invalid, please check.");
		} else if (response == 101)
		{
			/* Check if there is an upgrade*/
			char *u = strstr(p, "upgrade: websocket");
			if (!u)
			{
				G_SetLastError(G_ERROR_INVALID_HTTP_RESPONSE);
				ChangeStateError(websocket, G_ERROR_INVALID_HTTP_RESPONSE, "The client expects HTTP header response to contains the Upgrade : websocket metadata");
				return -1;
			}

			{
				int length;
				char *iu;
				/* Validate the Sec-Websocket-Accept*/
				u = strstr(p, "sec-websocket-accept:");
				u+= (strlen("Sec-WebSocket-Accept:") + 1);

				length = G_Strnlen(websocket->m_Private->m_InitialHandshakeSHA, 127);
				while (*u==' ') u++;

				iu = websocket->m_Private->m_InitialHandshakeSHA;
				while (*iu==' ') {iu++; length--;}

				if (strncasecmp(u, iu, length) != 0)
				{
					G_SetLastError(G_ERROR_INVALID_HTTP_RESPONSE);
					ChangeStateError(websocket, G_ERROR_INVALID_HTTP_RESPONSE, "The client expects HTTP header response to contains the correct SHA1 return of the Security Key");
					return -1;
				}
			}

			G_Websocket_ChangeState(websocket, g_WS_READY);
			websocket->m_Info->M_OnConnected(websocket);

		} else if (response < 0)
		{
			G_SetLastError(G_ERROR_INVALID_HTTP_RESPONSE);
			ChangeStateError(websocket, G_ERROR_INVALID_HTTP_RESPONSE, "Invalid response code received");
			return -1;
		} else if (websocket->m_Info->M_OnAdditionalRequest)
		{
			G_Websocket_ChangeState(websocket, g_WS_ADDITONALREQUEST);
			websocket->m_Info->M_OnAdditionalRequest(websocket, websocket->m_Private->m_Buffer, length);
		}
	}
	free(fr);
	return 0;
}

int G_Websocket_ReadHandshake(t_Websocket websocket)
{
	int r;
	if(websocket->m_Info->m_BufferSize > websocket->m_Private->m_BufferIndex)
	{
		r = websocket->m_Private->m_Socket->M_Socket_Read(websocket->m_Private->m_SocketImp, &(websocket->m_Private->m_Buffer[websocket->m_Private->m_BufferIndex]), websocket->m_Info->m_BufferSize - websocket->m_Private->m_BufferIndex);
	}
	else
	{
		r = -5;
	}

	if (r > 0)
	{
		char *c;
		websocket->m_Private->m_BufferIndex += r;
		
		if(websocket->m_Private->m_BufferIndex >= websocket->m_Info->m_BufferSize)
			return -1;

		websocket->m_Private->m_Buffer[websocket->m_Private->m_BufferIndex] = '\0';
		c = strstr(websocket->m_Private->m_Buffer, "\r\n\r\n");
		if (c)
		{
			int ret;
			ret = websocket->m_Private->m_BufferIndex;
			websocket->m_Private->m_BufferIndex = 0;
			return ret;
		}
		return 0;
	} else if (r == 0)
	{
		return 0;
	}
	return -1;
}

void G_Websocket_ProcessMessage(t_Websocket websocket, char* data, int length, enum t_Websocket_OpCode opcode)
{
	switch (opcode)
	{
		case g_WEBSOCKET_PING:
		{
			/* Received a PING message, prepare to send back a PONG*/
			MIA_DEBUG_PRINT("[PING]: %.*s\n", length, data ? data : "");
		}
		break;

		case g_WEBSOCKET_PONG:
		{
			/* Received a PONG message, update the latency?*/
			MIA_DEBUG_PRINT("[PONG]: %.*s\n", length, data ? data : "");
			G_Websocket_TimeoutUpdatePong(websocket);
			G_Websocket_TimeoutUpdate(websocket);
		}
		break;

		case g_WEBSOCKET_TEXT:
		{
			/* Received a text message, process the text message in case of fragment*/
			MIA_DEBUG_PRINT("[WS::TEXT len:%d]: %.*s\n", length, length, data ? data : "");
			websocket->m_Info->M_OnMessage(websocket, data, length);
		}
		break;

		case g_WEBSOCKET_BINARY:
		{
			/* Received a text message, process the text message in case of fragment*/
			MIA_DEBUG_PRINT("[BIN]: %.*s\n", length, data ? data : "");
			websocket->m_Info->M_OnMessage(websocket, data, length);
		}
		break;

		case g_WEBSOCKET_CONTINUATION:
		{
			MIA_DEBUG_PRINT("[WS::CONT]: *********%.*s\n", length, data ? data : "");
		}
		break;

		case g_WEBSOCKET_CLOSE:
		{
			MIA_DEBUG_PRINT("[WS::CLOS]: *********\n");
			websocket->m_Info->M_OnDisconnect(websocket, G_GetLastError());
			G_Websocket_ChangeState(websocket, g_WS_DISCONNECTING);
		}
		break;
		default:
		{
			MIA_DEBUG_PRINT("[WS::ERRO]: %.*s\n", length, data ? data : "");
			websocket->m_Info->M_OnError(websocket, G_GetLastError(), data);
		}
		break;
	}
}

/* Return not important?*/
int G_Websocket_ReadFullBuffer(t_Websocket websocket, char* buffer, int byteToRead)
{
	t_WebsocketPrivate p = websocket->m_Private;
	int byteread = 0, r;
	for (;;)
	{
		r = p->m_Socket->M_Socket_Read(p->m_SocketImp, &(buffer[byteread]), byteToRead - byteread);
		if (r >= 0)
		{
			byteread += r;
		}
		else
		{
			return r;
		}
		if (byteread == byteToRead) return byteread;
	}
}

int G_Websocket_ReadExternalBuffer(t_Websocket websocket)
{
	t_WebsocketPrivate p = websocket->m_Private;
	if (p->m_ExternalBuffer && p->m_ToRead)
	{
		int r;

		if(p->m_BufferIndex +  p->m_ToRead >=  websocket->m_Info->m_BufferSize)
		{
			return -1;
		}

		r = p->m_Socket->M_Socket_Read(p->m_SocketImp, &(p->m_ExternalBuffer[p->m_BufferIndex]), p->m_ToRead);

		if (r < 0)
		{
			ChangeStateError(websocket,  G_ERROR_UNABLE_RECEIVE, "Unable to read from socket"); 
			return -1;
		}
		if (r == p->m_ToRead)
		{
			t_Message m = G_Message_FromBinary(p->m_ExternalBuffer);
			/* Already get the message*/
			G_Websocket_ProcessMessage(websocket, G_Message_Data(m), G_Message_Size(m), g_WEBSOCKET_TEXT);
			free(p->m_ExternalBuffer);
			p->m_ExternalBuffer = 0;
			p->m_ToRead = 0;
			p->m_BufferIndex = 0;
			memset(p->m_Buffer, 0, websocket->m_Info->m_BufferSize);
			free(m->m_Private);
			free(m);
			G_Websocket_TimeoutUpdate(websocket);
		} else
		{
			/* Continue to append to message*/
			p->m_LastPingTime = G_Monotonic_Time();
			p->m_BufferIndex += r;
			p->m_ToRead -= r;
			G_Websocket_TimeoutUpdate(websocket);
		}
		return r;
	}
	return 0;
}

int G_Websocket_ReadMessage(t_Websocket websocket)
{
	int ret = G_Websocket_ReadExternalBuffer(websocket);

	if (ret == 0)
	{
		int r;
		t_WebsocketPrivate p = websocket->m_Private;

		if(websocket->m_Info->m_BufferSize > p->m_BufferIndex + 1 )
		{
			r = p->m_Socket->M_Socket_Read(p->m_SocketImp,
							&(p->m_Buffer[p->m_BufferIndex]),
							websocket->m_Info->m_BufferSize - p->m_BufferIndex-1);
		}
		else
		{
			r = -5;
		}

		if (r > 0)
		{
			int dataindex = 0;

			p->m_BufferIndex += r;

			for (;;)
			{
				t_Message m;
				int availabledata = p->m_BufferIndex - dataindex;

				if (availabledata < 2) 
					return 0;

				m = G_Message_FromBinary(&p->m_Buffer[dataindex]);
				if ((G_Header_GetPayload(&m->m_Private->m_DataFrame->m_Header) < 126 && (availabledata >= G_Message_FrameLength(m)))
					|| ((G_Header_GetPayload(&m->m_Private->m_DataFrame->m_Header) == 126) && (availabledata > 4) && (availabledata >= G_Message_FrameLength(m)))
					|| ((G_Header_GetPayload(&m->m_Private->m_DataFrame->m_Header) == 127) && (availabledata > 10) && (availabledata >= G_Message_FrameLength(m))))
				{
					/* Enough data, just populate it*/
					G_Websocket_ProcessMessage(websocket, G_Message_Data(m), (int)m->m_Private->m_Length, G_Header_GetOpCode(&m->m_Private->m_DataFrame->m_Header));
					dataindex += G_Message_FrameLength(m);
					free(m->m_Private);
					free(m);
					if (p->m_BufferIndex == dataindex) { p->m_BufferIndex = 0; return 0;}
					continue;
				} else
				{
					if (((G_Header_GetPayload(&m->m_Private->m_DataFrame->m_Header) == 126) && (availabledata > 4))
							|| ((G_Header_GetPayload(&m->m_Private->m_DataFrame->m_Header) == 127) && (availabledata > 10)))
					{
						/* Lenngth is determined*/
						int framelength = G_Message_FrameLength(m);
						if (framelength >= (websocket->m_Info->m_BufferSize - dataindex))
						{
							/* Move everything into external buffer*/
							p->m_ExternalBuffer = (char *)malloc(framelength+1);
							p->m_ToRead = framelength - availabledata;

							memcpy(p->m_ExternalBuffer, &p->m_Buffer[dataindex], availabledata);
							p->m_BufferIndex = availabledata;

							free(m->m_Private);
							free(m);
							return 0;
						}
					}
					if (dataindex != 0)
					{
						memmove(p->m_Buffer, &(p->m_Buffer[dataindex]), availabledata);
						p->m_BufferIndex = availabledata;
					}

					p->m_ToRead = 0;
				}
				free(m->m_Private);
				free(m);
				return 0;
			}
		}
		else if(r < 0)
		{
			ChangeStateError(websocket,  G_ERROR_UNABLE_RECEIVE, "Unable to read from socket");
			return r;
		}
	}
	return ret;
}

int G_Websocket_Ping(t_Websocket websocket)
{
	t_WebsocketPrivate p = websocket->m_Private;
	unsigned int currenttime = G_Monotonic_Time();
	int write, write_len;
	t_Message m;
	if (currenttime - p->m_LastPingTime > WS_PING_INTERVAL)
	{
		p->m_LastPingTime = currenttime;
		m = G_Message_NewPing(4);
		G_Message_Write(m, "PING", 0, 4);
		write_len = G_Message_FrameLength(m);
		MIA_DEBUG_PRINT("[Mia_Websocket]: Send ping\n");

		write = websocket->m_Private->m_Socket->M_Socket_Write(websocket->m_Private->m_SocketImp, G_Message_Frame(m), write_len);
		G_Message_Free(&m);

		if (write != write_len)
		{
			ChangeStateError(websocket, G_ERROR_UNABLE_SEND, "Write mismatch");
			return -1;
		}
	}
	
	return 0;
}

enum t_WebsocketState G_Websocket_GetState(t_Websocket websocket)
{
	return websocket->m_Private->m_State;

}

void G_Websocket_Service(t_Websocket websocket)
{
	/*printf("Websocket state: %d\n", websocket->m_Private->m_State);*/
	int ret;
	int code;
	switch (websocket->m_Private->m_State)
	{
		case g_WS_INITIAL:
		/* Start the connection*/
		{
			MIA_DEBUG_PRINT("[Mia_Websocket]: Initializing connection\n");

			websocket->m_Private = G_WebsocketPrivate_FromString(websocket, websocket->m_Info->m_ConnectionString->m_Data);
			ret = G_Websocket_Connect(websocket);

			if (ret == 0)
			{
				G_Websocket_ChangeState(websocket, g_WS_CONNECTED);

			} else if (ret < 0)
			{
				char message[200] = "";
#ifndef NDEBUG
				snprintf(message, sizeof(message), "[Mia_Websocket]: Unable to establish a connection to the indicated %s %d", 
					websocket->m_Private->m_HostName ? websocket->m_Private->m_HostName : "<unknown>", 
					websocket->m_Private->m_Port);
				
				message[sizeof(message) - 1] = 0;
				MIA_DEBUG_PRINT("Websocket error! %d \n%s\n", ret, message);
#endif
				ChangeStateError(websocket, G_GetLastError(), message);
			}
		}
		break;

		case g_WS_CONNECTING:
			MIA_DEBUG_PRINT("[Mia_Websocket]: Connecting\n");
			assert(websocket->m_Private->m_Socket->M_Socket_IsReady);
			if (websocket->m_Private->m_Socket->M_Socket_IsReady(websocket->m_Private->m_SocketImp))
			{
				G_Websocket_ChangeState(websocket, g_WS_CONNECTED);
			} else
			{
				G_Websocket_Timeout(websocket);
			}
		break;

		case g_WS_CONNECTED:
			MIA_DEBUG_PRINT("[Mia_Websocket]: Connected\n");

			websocket->m_Private->m_BufferIndex = 0;
			G_Websocket_ChangeState(websocket, g_WS_HANDSHAKING);
			code = G_Websocket_SendHandshake(websocket);
			if (code < 0)
			{
				ChangeStateError(websocket, G_GetLastError(), "Unable to send handshake to websocket server");
			}
		break;

		case g_WS_HANDSHAKING:
		{
			int r = G_Websocket_ReadHandshake(websocket);
			if (r > 0)
			{
				G_Websocket_ParseHandshake(websocket);
			} else if (r < 0)
			{
				ChangeStateError(websocket, G_GetExactCode(), "Unable to read from socket");
			}
		}
		break;

		case g_WS_ADDITONALREQUEST:
			/*Check for timeout mainly, do not do anything particularly*/
			G_Websocket_Timeout(websocket);
		break;

		case g_WS_READY:
			/* Do send and receive here*/
			/* First send*/
			{
				t_List msgQ = websocket->m_Private->m_MessageQueue;
				t_ISocket socket = websocket->m_Private->m_Socket;

#ifndef PING_ONLY_WHEN_SENDING_DATA
				G_Websocket_Ping(websocket);
				G_Websocket_Timeout(websocket);
#endif

				/* check if it is ready to write*/
				if (msgQ->m_Size && (socket->M_Socket_IsReady(websocket->m_Private->m_SocketImp) & 0x04))
				{
					ChangeStateError(websocket, 42, "Socket closed unexpectedly before write");
					break;
				}
				else if (msgQ->m_Size && (socket->M_Socket_IsReady(websocket->m_Private->m_SocketImp) & 0x02))
				{
#ifdef WS_WEBSOCET_LAZY_WRITE
					if
#else
					while 
#endif
					(msgQ->m_Size && (websocket->m_Private->m_State == g_WS_READY))
					{
						t_Message removeList[TEMP_BUFFER_MAX_ELEMENTS];
						unsigned int removeListLen = 0;
						char buffer_data[TEMP_BUFFER_LEN];
						char *buffer = buffer_data;
						unsigned int bufferLen = 0;
						
						removeList[removeListLen] = (t_Message) G_List_RemoveFront(msgQ);
						
						if(removeList[removeListLen])
						{
							if(G_Message_FrameLength(removeList[removeListLen]) >= TEMP_BUFFER_LEN) // If one very big buffer
							{
								buffer = G_Message_Frame(removeList[removeListLen]);
								bufferLen = G_Message_FrameLength(removeList[removeListLen]);
								removeListLen++;
							}
							else
							{
								while(1)
								{
									if(!removeList[removeListLen])
										break;

									if(TEMP_BUFFER_LEN < bufferLen + G_Message_FrameLength(removeList[removeListLen]))
									{
										G_List_AppendFront(msgQ, removeList[removeListLen]); // message does not fit puffer. Put back 
										break;
									}
									memcpy(&buffer[bufferLen], G_Message_Frame(removeList[removeListLen]), G_Message_FrameLength(removeList[removeListLen]));
									bufferLen += G_Message_FrameLength(removeList[removeListLen]);
									removeListLen++;
									if(removeListLen < TEMP_BUFFER_MAX_ELEMENTS)
									{
										removeList[removeListLen] = (t_Message) G_List_RemoveFront(msgQ);
									}
									else
									{
										break;
									}
									
								}
							}
						}

						if (removeListLen) 
						{
							int sent = 0;
							
							while (sent < (int)bufferLen) 
							{
								int r = socket->M_Socket_Write(websocket->m_Private->m_SocketImp, &buffer[sent], bufferLen - sent);

								if (r < 0) 
								{
									int i;
									for( i = 0; i < removeListLen; i++)
										G_List_AppendFront(msgQ, removeList[removeListLen - i - 1]); // Put message back to queue for resending

									ChangeStateError(websocket, G_GetLastError(), "Message was not sent successfully");
									sent = -1;
									break; // while
								}
								else
								{
									sent += r;
									G_Websocket_TimeoutUpdate(websocket);
									websocket->m_Private->m_LastSendTime =  G_Monotonic_Time();
								}
							}
							if(sent >= 0)
							{
								int i;
								for( i = 0; i < removeListLen; i++)
									G_Message_Free(&removeList[i]);
								
#ifdef PING_ONLY_WHEN_SENDING_DATA
								if(!msgQ->m_Size && sent)
								{
									G_Websocket_Ping(websocket);
									G_Websocket_Timeout_Ping(websocket);
								}
#endif
							}
							else
							{
								break; // case g_WS_READY:
							}
						}
						else
						{
							break;
						}
					}
				}

#ifdef WS_WEBSOCET_LAZY_READ
				if(websocket->m_Private->m_LastSendTime + WS_WEBSOCET_LAZY_READ >= G_Monotonic_Time() || websocket->m_Private->m_LastSendTime + WS_WEBSOCET_LAZY_READ_SECOND_TRY == G_Monotonic_Time() ) // Read only if data send before
#endif
				if ((socket->M_Socket_IsReady(websocket->m_Private->m_SocketImp) & 0x1)) 
				{
					G_Websocket_ReadMessage(websocket);
				}
				else if ((socket->M_Socket_IsReady(websocket->m_Private->m_SocketImp) & 0x4)) 
				{
					ChangeStateError(websocket, 48, "Socket closed unexpectedly before read");
				}

				break;
			}
		default:

		case g_WS_DISCONNECTING:
		{
			G_Websocket_ChangeState(websocket, g_WS_DISCONNECTED);
			break;
		}

		case g_WS_DISCONNECTED:
		{
			websocket->m_Info->M_OnDisconnect(websocket, G_GetLastError());
		}

		case g_WS_ERROR:
			/* Reach the time out and then try to clean the*/
#ifndef DO_NOT_WAIT_TIMEOUT_IN_ERROR
			if (G_Websocket_Timeout(websocket))
#endif
			if(websocket->m_Info->m_AutoReconnect)
			{
				MIA_DEBUG_PRINT("Is being destroyed\n");

				Websocket_PrivateFree(&websocket->m_Private); // Free buffers
				websocket->m_Private = G_WebsocketPrivate_FromString(websocket, websocket->m_Info->m_ConnectionString->m_Data);

				/* Reconnect to the server*/
				G_Websocket_ChangeState(websocket, g_WS_INITIAL);
			}

		break;
	}
}

int G_Websocket_CreateHandshake(t_Websocket websocket)
{
	t_WebsocketPrivate priv = websocket->m_Private;
	char* p = priv->m_Buffer;
	int n;

	p += sprintf(p, "GET %s HTTP/1.1\x0d\x0a", priv->m_Path);
	/* basic authentication*/
	if (websocket->m_Info->m_Username && websocket->m_Info->m_Password)
	{
		if (G_String_Length(websocket->m_Info->m_Username) && G_String_Length(websocket->m_Info->m_Password))
		{
			char password[G_LONG_STRING_LEN];
			char password_enc[G_LONG_STRING_LEN];

			snprintf(password,
					sizeof(password),"%s:%s",
					G_String_Data(websocket->m_Info->m_Username),
					G_String_Data(websocket->m_Info->m_Password));
			password[sizeof(password) - 1] = 0; // Force terminate because snprintf does not do it.

			G_Encode(password, (int)strlen(password), password_enc, sizeof(password_enc));
			p += sprintf(p, "Authorization: Basic %s\x0d\x0a", password_enc);
		}
	}


	if (1) // Resolve hostname
	{
		char *hostname;
		char *hostIp;

		if(!G_ResolveHostAndIP(&hostname, &hostIp, priv->m_HostName))
		{
			free(priv->m_HostName);
			free(priv->m_HostAddress);
			priv->m_HostAddress = hostIp; // m_HostAddress is IP address if resolving success otherwise it is 0.0.0.0
			priv->m_HostName = hostname;  //m_HostName is host name if resolving success. Otherwise it is IP
			
			MIA_DEBUG_PRINT("Hostname is %s. Host IP address is %s\n", priv->m_HostName, priv->m_HostAddress);
		
		}
		else {
			// Memory allocation failed. Just go on because private->m_HostName is valid
			priv->m_HostAddress = strdup_os("0.0.0.0");
		}
	}

	p += sprintf(p, "Host: %s\x0d\x0a", priv->m_HostName);
	/* no cache*/
	p += sprintf(p, "Pragma: no-cache\x0d\x0a""Cache-Control: no-cache\x0d\x0a");

	p += sprintf(p, "Origin: http://%s\x0d\x0a", priv->m_HostName);
	p += sprintf(p, "Sec-WebSocket-Extensions: \x0d\x0a""Upgrade: websocket\x0d\x0a""Connection: keep-alive, Upgrade\x0d\x0a""Sec-WebSocket-Key: ");

	if(1) /* create random key*/
	{
		char temp[G_LONG_STRING_LEN];
		char temp2[G_LONG_STRING_LEN];
		char hash[20];

		G_Random_String(hash, 16);
		G_Encode(hash, 16, temp, sizeof(temp));

		/*temp holds the Base64 encoded random string*/
		/* strcpy(temp, "dGhlIHNhbXBsZSBub25jZQ==");*/
		strncpy(p, temp, sizeof(temp));
		p += strlen(temp);
		p += sprintf(p, "\x0d\x0a");
		p += sprintf(p, "Sec-WebSocket-Version: %d\x0d\x0a", websocket->m_Info->m_WebsocketVersion);

		temp[39] = '\0';  /* enforce composed length below buf sizeof*/
		n = sprintf(temp2, "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", temp);

		/* temp2 holds the appended guid, which will be stored into hash*/
		G_SHA1((unsigned char *)temp2, n, (unsigned char *)hash);

		/* hash will be encoded to SHA key and stored*/
		G_Encode(hash, 20, priv->m_InitialHandshakeSHA, sizeof(priv->m_InitialHandshakeSHA));
		p += sprintf(p, "\x0d\x0a");
		return 0;
	}
}

int G_Websocket_SendHandshake(t_Websocket websocket)
{
	/* Create the header*/
	char *hs;
	int length;
	int sent;
	if (!websocket->m_Info->m_CustomHandshake)
	{
		G_Websocket_CreateHandshake(websocket);
		hs = websocket->m_Private->m_Buffer;
	} else
	{
		hs = websocket->m_Private->m_InitialHandshakeSHA;
	}

	MIA_DEBUG_PRINT("Sending %s.\n", hs);

	length = (int)strlen(hs);
	sent = websocket->m_Private->m_Socket->M_Socket_Write(websocket->m_Private->m_SocketImp, hs, length);
	MIA_DEBUG_PRINT("Sent %d/%d\n", sent, length);
	return sent;
}

int G_Websocket_Connect(t_Websocket websocket)
{
	t_WebsocketPrivate priv;
	G_Websocket_ChangeState(websocket, g_WS_CONNECTING);

	priv = (t_WebsocketPrivate) websocket->m_Private;
	/* Using default socket implementation here*/
	if (!websocket->m_Private->m_Socket)
	{
		if (websocket->m_Private->m_SSL)
		{
			/* Using secure websocket*/
			websocket->m_Private->m_Socket = G_Socket_CreateSecure();
		} else
		{
			/* Using non-secure version*/
			websocket->m_Private->m_Socket = G_Socket_CreateUnsecure();
		}
	}

	if(!websocket->m_Private->m_Socket) 
	{
		return -2;
	}

	if(!priv->m_SocketImp) 
	{
		priv->m_SocketImp = priv->m_Socket->M_Socket_New(priv->m_HostName, priv->m_Port);
	}

	if (!priv->m_SocketImp)
	{
		return -1;
	}

	return priv->m_Socket->M_Socket_Connect(priv->m_SocketImp);
}

int G_Websocket_Send(t_Websocket websocket, const char* message, int length)
{
	t_Message m;
	if (!length) length = (int)strlen(message);

	if ( websocket->m_Private->m_MessageQueue->m_Size > websocket->m_Info->m_MaxQueueSize)
	{
		G_List_RemoveFront(websocket->m_Private->m_MessageQueue);
	}

	m = G_Message_NewText(length);
	if (!m) return -1;

	G_Message_Write(m, message, 0, length);

	return G_List_AppendBack(websocket->m_Private->m_MessageQueue, m);
}

int G_Websocket_SendBinary(t_Websocket websocket, const char *message, int length)
{
	t_Message m;
	if ( websocket->m_Private->m_MessageQueue->m_Size > websocket->m_Info->m_MaxQueueSize)
	{
		G_List_RemoveFront(websocket->m_Private->m_MessageQueue);
	}

	m = G_Message_NewBinary(length);
	if (!m) return -1;

	G_Message_Write(m, message, 0, length);

	return G_List_AppendBack(websocket->m_Private->m_MessageQueue, m);
}

int G_Websocket_SendRaw(t_Websocket websocket, t_Message message)
{
	if (websocket->m_Private->m_MessageQueue->m_Size > websocket->m_Info->m_MaxQueueSize)
	{
		G_List_RemoveFront(websocket->m_Private->m_MessageQueue);
	}

	return G_List_AppendBack(websocket->m_Private->m_MessageQueue, message);
}

int G_Websocket_SendMessage(t_Websocket websocket, t_Message message)
{
	if ( websocket->m_Private->m_MessageQueue->m_Size > websocket->m_Info->m_MaxQueueSize)
	{
		G_List_RemoveFront(websocket->m_Private->m_MessageQueue);
	}

	return G_List_AppendBack(websocket->m_Private->m_MessageQueue, message);
}

static void Websocket_PrivateFree(t_WebsocketPrivate * pp)
{
	t_WebsocketPrivate p;

	if(!pp || !*pp)
		return;
	 
	p = *pp;

	while(p->m_MessageQueue)
	{
		t_Message data = (t_Message)G_List_RemoveFront(p->m_MessageQueue);
		if (data)
		{
			G_Message_Free(&data);
		} else
		{
			break;
		}
	}
	
	G_List_Free(&(p->m_MessageQueue));

	free(p->m_HostName);
	free(p->m_HostAddress);
	free(p->m_Path);
	if(p->m_Socket)
	{
		p->m_Socket->M_Socket_Free(&(p->m_SocketImp));
	}
	free(p->m_Buffer);
	free(p->m_ExternalBuffer);

#ifndef NDEBUG
	memset(p, 0xce, sizeof(*p)); // make accessing more easy to see
#endif

	free(p);

	*pp = NULL;
}

void G_Websocket_Free(t_Websocket* websocket)
{
	t_Websocket w = *websocket;
	if (w)
	{
		Websocket_PrivateFree(&w->m_Private);
		
		G_WebsocketInfo_Free(&(w->m_Info));
#ifndef NDEBUG
		memset(w, 0xce, sizeof(*w)); // make accessing to released buffer more easy to see
#endif
		free(w);
		*websocket = 0;
	}
}

void G_WebsocketInfo_Free(t_WebsocketInfo* info)
{
	t_WebsocketInfo i = *info;
	if (i)
	{
		if (i->m_ConnectionString) G_String_Free(&(i->m_ConnectionString));
		if (i->m_Username) G_String_Free(&(i->m_Username));
		if (i->m_Password) G_String_Free(&(i->m_Password));
		if (i->m_Host) free(i->m_Host);

#ifndef NDEBUG
		memset(i, 0xce, sizeof(*i)); // make accessing to released buffer more easy to see
#endif
		free(i);

		*info = 0;
	}
}

t_WebsocketInfo G_WebsocketInfo_New()
{
	t_WebsocketInfo info = (t_WebsocketInfo) malloc(sizeof(struct _WebsocketInfo));
	memset(info, 0, sizeof(*info));
	info->m_BufferSize = m_k_s_DEFAULT_BUFFER_SIZE;
	info->m_MaxQueueSize = m_k_s_DEFAULT_QUEUE_SIZE;
	info->m_TimeOut = m_k_s_DEFAULT_SOCKET_TIMEOUT;
	info->m_WebsocketVersion = 13;
	info->m_AutoReconnect = WEBSOCKET_AUTO_RECONNECT;
	info->m_Socket = 0;

	return info;
}

int  G_Websocket_BufferCnt(t_Websocket websocket)
{
	if(!websocket->m_Private || !websocket->m_Private->m_Socket)
		return -10;

	if(websocket->m_Private->m_State != g_WS_READY)
		return -11;
	
	if(websocket->m_Private->m_Socket->M_Socket_IsReady(websocket->m_Private->m_SocketImp) & 0x4)
	{
		ChangeStateError(websocket, 41, "Socket closed unexpectedly before buffer count");
		return -12;
	}
	return G_List_GetSize(websocket->m_Private->m_MessageQueue);
}
