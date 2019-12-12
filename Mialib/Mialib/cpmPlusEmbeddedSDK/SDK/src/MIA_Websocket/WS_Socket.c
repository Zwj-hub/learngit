/*
 * WS_Socket.c
 *
 *  Created on: Sep 29, 2014
 *      Author: Tuan Vu
 */

// This file handles OS dependencies to Windows and Linux systems. Do not add new OS here. Just create new OS file like WS_Socket_Osal.c
// Perfect world would have WS_Socket_Linux.c and WS_Socket_Windows.c. Christer is lazy and does not refactor original ifdefs to separate files.


#include "WS_Socket.h"
#include "WS_Error.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>

#ifdef __linux__
	#include <pthread.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <fcntl.h>
	#include <sys/select.h>
	#include <signal.h>
#elif _WIN32
	#include <time.h>
	#include <winsock2.h>
	#include <mstcpip.h>
	#include <ws2tcpip.h>
	#include <Windows.h>
	#pragma comment(lib, "ws2_32.lib")
	#if WINVER < _WIN32_WINNT_VISTA 
		
	#else
		#include <Inaddr.h>
	#endif
#endif

#ifdef __OPENSSL__
	#include <openssl/ssl.h>
	#include <openssl/rsa.h>
	#include <openssl/x509.h>
	#include <openssl/evp.h>
   #include <openssl/err.h>
   #ifdef TPM_ENGINE
      #include <openssl/engine.h>
   #endif
#elif _WIN32
	#include <rpc.h>
	#include <ntdsapi.h>
	#include <tchar.h>
	#pragma comment(lib, "Fwpuclnt.lib")
	#pragma comment(lib, "ntdsapi.lib")
#endif

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#define NI_MAXSERV 32
#define NI_NUMERICHOST 0
#define NI_NUMERICSERV 0
#endif

#ifdef __linux__
	// We should define here also lot of more like close/closesocket and strncpy/strncpy_s but Christer is lazy and does not do it.
	#define MIA_SLEEP_MS(a) usleep(1000 * a);
#elif _WIN32
	#define MIA_SLEEP_MS(a) Sleep(a);
#endif
/*****************************************************************************************************/
/*********************                    Socket                      ********************************/
/*****************************************************************************************************/

#ifdef WEBSOCKET_DEBUG
	#define C_THREAD_DEBUG 1
#else
	#define C_THREAD_DEBUG 0
#endif

#define MIA_DEBUG_PRINT(...) if (C_THREAD_DEBUG) { printf("[Mia_Websocket]: "); do { printf(__VA_ARGS__); } while(0); }

const int g_IP_LENGTH = 32;

typedef struct _Socket
{
	#ifdef __linux__
		int		m_SocketFd;
	#else
		SOCKET	m_SocketFd;
	#endif
		struct sockaddr_in m_ServerAddress;
		fd_set	m_ReadFlag;
		fd_set	m_WriteFlag;
		char		*m_Host;
		int 		m_Port;
} *t_Socket;

t_Socket G_Socket_New(char* hostName, int port);

int G_Socket_Connect(t_Socket socket);

int G_Socket_Write(t_Socket socket, const char* message, size_t bufferLength);

int G_Socket_Read(t_Socket socket, char* buffer, size_t size);

void G_Socket_Free(t_Socket *socket);

int G_ResolveHostName(char * hostname , char* ip)
{
#ifdef __linux__
	struct hostent *he;
	struct in_addr **addr_list;
	int i;

	if ((he = gethostbyname(hostname)) == NULL)
	{
		/* get the host info */
		perror("G_ResolveHostName: Error");
		return 1;
	}

	addr_list = (struct in_addr **) he->h_addr_list;

	for (i = 0; addr_list[i] != NULL; i++)
	{
		/* Return the first one; */
		strcpy(ip, inet_ntoa(*addr_list[i]));
		return 0;
	}

	return 1;
#elif _WIN32
	int result;
	struct addrinfo hints, *res = NULL;
	struct in_addr addr;

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = 0;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_TCP;
	
	if ((result = getaddrinfo(hostname, NULL, &hints, &res)) != 0)
	{
		MIA_DEBUG_PRINT("G_ResolveHostName: Error %d\n", result);
		return 1;
	}

	addr.S_un = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.S_un;
	strcpy_s(ip, g_IP_LENGTH, inet_ntoa(addr));
	return 0;
#endif
}

int G_ResolveHostAndIP(char **hostname, char **hostIp, char* host)
{
	// Do allocation here for making possible to save heap in other OS implementations
	*hostname = malloc(NI_MAXHOST);
	if (!*hostname)
	{
		return -1;
	}

	*hostIp = malloc(INET6_ADDRSTRLEN);
	if (!*hostIp)
	{
		free(*hostname);
		return -1;
	}
	*hostname[0] = 0;
	*hostIp[0]   = 0;

	if (-1 == inet_addr(host))
	{
		// Host is not IP
		if(G_ResolveHostName(host, *hostIp))
		{
			// Hostname cannot be resolved to IP. Set to default
			strncpy(*hostIp, "0.0.0.0", INET6_ADDRSTRLEN);
		}

		strncpy(*hostname, host, NI_MAXHOST);
	}
	else
	{
		// Host is IP addr
		struct sockaddr_in serv;

		memset(&serv, 0, sizeof(serv));
		serv.sin_family      = AF_INET;
		serv.sin_addr.s_addr = inet_addr(host);
		serv.sin_port        = htons(80); // Port or service is not interesting. Just fill something

		strncpy(*hostIp, host, INET6_ADDRSTRLEN);

		if ( getnameinfo((struct sockaddr *) &serv, sizeof(serv), *hostname, NI_MAXHOST, NULL, 0, NI_NUMERICHOST | NI_NUMERICSERV) != 0)
		{
			// Host is IP but host cannot be resolved to name. Set IP to host name
			strncpy(*hostname, host, NI_MAXHOST);
		}
	}

	// Free over reservation
	*hostname = realloc(*hostname, strlen(*hostname) + 1);
	*hostIp   = realloc(*hostIp,   strlen(*hostIp) + 1);

	return 0;
}

t_Socket G_Socket_New(char* hostName, int port)
{
	if (hostName)
	{
		int length = (int) strlen(hostName);
		if (length > 5)
		{
			t_Socket socket = malloc(sizeof(struct _Socket));
			if (socket)
			{
				socket->m_Host = malloc(length+1);
				if (socket->m_Host)
				{
#ifdef __linux__
					strncpy(socket->m_Host, hostName, length);
#elif _WIN32
					strcpy_s(socket->m_Host, length + 1, hostName);
#endif
					socket->m_Host[length] = '\0';
					socket->m_Port = port;
					return socket;
				}
			}
			G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		}
	}
	return 0;
}

/* return 0 if connected
// return 1 if in progress,
// return -1 if failed */
int G_Socket_Connect(t_Socket s)
{
	u_long flags, err;

	if (s && s->m_Host)
	{
		s->m_SocketFd = socket(AF_INET, SOCK_STREAM, 0);
		if (s->m_SocketFd >= 0)
		{
			memset(&(s->m_ServerAddress), 0, sizeof(s->m_ServerAddress));
			s->m_ServerAddress.sin_addr.s_addr = inet_addr(s->m_Host);
			s->m_ServerAddress.sin_family = AF_INET;
			s->m_ServerAddress.sin_port = htons(s->m_Port);

#ifdef _WIN32
			flags = 1;
			ioctlsocket(s->m_SocketFd, FIONBIO, &flags);
#elif __linux
			flags = fcntl(s->m_SocketFd, F_GETFL, 0);
			assert(flags != -1);
			fcntl(s->m_SocketFd, F_SETFL, flags | O_NONBLOCK);
#endif
			FD_ZERO(&(s->m_ReadFlag));
			FD_ZERO(&(s->m_WriteFlag));

			if (connect(s->m_SocketFd, (struct sockaddr*) &(s->m_ServerAddress), sizeof(s->m_ServerAddress)) >= 0)
			{
				return 0;
			}
			err = errno;
			if (errno == EINPROGRESS)
			{
				return 1;
			}
			else
			{
				if (err == ECONNREFUSED || err == ENETUNREACH) G_SetLastErrorExact(G_ERROR_CONNECTION_REFUSED, err);
				else if (err == ETIMEDOUT) G_SetLastErrorExact(G_ERROR_TIME_OUT, err);
				else G_SetLastErrorExact(G_ERROR_UNABLE_CONNECT, err);
				return -1;
			}
		}
		G_SetLastErrorExact(G_ERROR_UNABLE_OPEN_SOCKET, errno);
		return -1;
	}
	G_SetLastError(G_ERROR_UNINITIALIZED);
	return -1;
}

int G_Socket_Write(t_Socket s, const char* message, size_t bufferLength)
{
	if (message && bufferLength > 0)
	{
		int sent = send(s->m_SocketFd, message, (int) bufferLength, 0);
		if (sent < 0)
		{
			if (errno == ENOBUFS) G_SetLastErrorExact(G_ERROR_BUFFER_FULL, errno);
			else G_SetLastErrorExact(G_ERROR_UNABLE_SEND, errno);
		}
		return sent;
	} else
	{
		G_SetLastError(G_ERROR_UNINITIALIZED);
	}
	return -1;
}

int G_Socket_Read(t_Socket s, char* buffer, size_t size)
{
	if (buffer && size > 0)
	{
		int re = recv(s->m_SocketFd, buffer, (int) size, 0);
		if (re >= 0)
		{
			return re;
		}
		else
		{
			if (EAGAIN == errno)
			{
				return 0;
			}
			G_SetLastErrorExact(G_ERROR_UNABLE_RECEIVE, errno);
		}
	} else
	{
		G_SetLastError(G_ERROR_UNINITIALIZED);
	}

	return -1;
}

int G_Socket_IsReady(t_Socket socket)
{
	t_Socket s = socket;
	int sel;
	int readready;
	int writeready;

	static struct timeval waittime = { 0, 0};
	waittime.tv_sec = 1;

	FD_SET(s->m_SocketFd, &(s->m_ReadFlag));
	FD_SET(s->m_SocketFd, &(s->m_WriteFlag));

	sel = select((int) socket->m_SocketFd+1, &(s->m_ReadFlag), &(socket->m_WriteFlag), 0, &waittime);
	if (sel < 0) return sel;

	readready = FD_ISSET(s->m_SocketFd, &(s->m_ReadFlag));
	writeready = FD_ISSET(s->m_SocketFd, &(s->m_WriteFlag));

	FD_CLR(s->m_SocketFd, &(s->m_ReadFlag));
	FD_CLR(s->m_SocketFd, &(s->m_WriteFlag));

	if (readready && writeready) return 3;
	if (writeready) return 2;
	if (readready) return 1;
	return 0;
}

void G_Socket_Free(t_Socket *s)
{
	if (s && *s)
	{
#ifdef __linux__
		close((*s)->m_SocketFd);
#elif _WIN32
		closesocket((*s)->m_SocketFd);
#endif
		if ((*s)->m_Host) free((*s)->m_Host);
		free (*s);
		*s = 0;
	}
}

typedef struct _SecureSocket
{
	int		m_Socket;
	struct sockaddr_in m_ServerAddress;
#ifdef __OPENSSL__
	int		m_SocketFd;
	SSL_CTX *m_SSLContext;
	SSL		*m_SSL;
#ifdef TPM_ENGINE
	char 		*m_KeyId;
	char 		*m_ClientCertificateId;
#endif
#elif _WIN32
	SOCKET	m_SocketFd;
#endif
	fd_set	m_ReadFlag;
	fd_set	m_WriteFlag;
	char		*m_Host;
	int 		m_Port;
	char		*m_PublicKey;
	char		*m_PrivateKey;

} *t_SecureSocket;

int G_SecureSocket_IsReady(t_SecureSocket socket);

t_SecureSocket G_SecureSocket_New(char* hostName, int port)
{
	static int sslinit = 0;

#ifdef __OPENSSL__
	const SSL_METHOD *method = 0;
	static SSL_CTX *context = 0;
#endif
	//signal(SIGPIPE, SIG_IGN);
	if (hostName)
	{
		int length = (int) strlen(hostName);
		if (length > 5)
		{
			t_SecureSocket s = malloc(sizeof(struct _SecureSocket));
			memset(s, 0 , sizeof(struct _SecureSocket));
			if (s)
			{
				s->m_Host = malloc(g_IP_LENGTH);

				if (s->m_Host)
				{
					if (G_ResolveHostName(hostName, s->m_Host))
					{
#ifdef __linux__
						strncpy(s->m_Host, hostName, length);
#elif _WIN32
						strncpy_s(s->m_Host, length + 1, hostName, length);
#endif
						s->m_Host[length] = '\0';
					}

					s->m_Port = port;

#ifdef __OPENSSL__
					if (!sslinit)
					{
						SSL_library_init();
						SSL_load_error_strings();
						OpenSSL_add_all_algorithms();
						sslinit = 1;
					}

					if (!context)
					{
						method = SSLv23_client_method();
						context = SSL_CTX_new(method);
					}
					s->m_SSLContext = context;

					if (!s->m_SSLContext)
					{
						MIA_DEBUG_PRINT("This context is null\n");
						/* Fail this conection */
						free(s->m_Host);
						free(s);
						return 0;
					}				
#endif
					return s;
				}
			}
			G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		}
	}

	return 0;
}

int G_SecureSocket_Connect(t_SecureSocket s)
{
	int err;
	int set = 0;
	if (s && s->m_Host)
	{
		#ifdef __linux__
			if (s->m_SocketFd) close(s->m_SocketFd);
		#elif _WIN32
			if (s->m_SocketFd) closesocket(s->m_SocketFd);
		#endif

		#ifdef __OPENSSL__

			// Using certificates for authentication
			if (s->m_PublicKey)
			{
				if (SSL_CTX_use_certificate_file(s->m_SSLContext, s->m_PublicKey, SSL_FILETYPE_PEM) <= 0)
				{
					MIA_DEBUG_PRINT("Unable to load SSL certificate.\n");
					G_SetLastError(G_ERROR_SSL_UNABLE_LOAD_CERTIFICATE);
					return -1;
				}
			}

			if (s->m_PrivateKey)
			{
				if (SSL_CTX_use_PrivateKey_file(s->m_SSLContext, s->m_PrivateKey, SSL_FILETYPE_PEM) <= 0)
				{
					MIA_DEBUG_PRINT("Unable to load SSL certificate.\n");
					G_SetLastError(G_ERROR_SSL_UNABLE_LOAD_CERTIFICATE);
					return -1;
				}
			}

#ifdef TPM_ENGINE
			if (s->m_KeyId)
			{
				ENGINE_load_builtin_engines();
				ENGINE* engine = ENGINE_by_id("tpm");
				ENGINE_init(engine);

				UI_METHOD* ui_method = UI_OpenSSL();
				EVP_PKEY* private_key = ENGINE_load_private_key(engine, s->m_KeyId, ui_method, NULL);
				
				if (SSL_CTX_use_PrivateKey(s->m_SSLContext, private_key) <= 0)
				{
					MIA_DEBUG_PRINT("Unable to load private key from TPM.\n");
					G_SetLastError(G_ERROR_SSL_UNABLE_LOAD_CERTIFICATE);
					return -1;
				}

				char cert_name[128];
				X509 *x509 = NULL;
				ENGINE_ctrl_cmd(engine, "CERT_ID", strlen(cert_name), cert_name, NULL, 0);
				EVP_PKEY *prikey = NULL;
				if (ENGINE_load_ssl_client_cert(engine, s->m_SSL, SSL_get_client_CA_list(s->m_SSL), &x509, &prikey, NULL, ui_method, NULL))
				{
					MIA_DEBUG_PRINT("Unable to load client certificate from TPM.\n");
					G_SetLastError(G_ERROR_SSL_UNABLE_LOAD_CERTIFICATE);
					return -1;
				}
			}
#endif
			s->m_SocketFd = (int) socket(AF_INET, SOCK_STREAM, 0);
			if (s->m_SocketFd >= 0)
			{
				memset(&(s->m_ServerAddress), 0, sizeof (s->m_ServerAddress));
				s->m_ServerAddress.sin_addr.s_addr = inet_addr(s->m_Host);
				s->m_ServerAddress.sin_family = AF_INET;
				s->m_ServerAddress.sin_port = htons(s->m_Port);

				#ifdef __linux__
					int flag;
					flag = fcntl(s->m_SocketFd, F_GETFL);
					if (flag == -1)
					{
						MIA_DEBUG_PRINT("Unable to control ssl socket.\n");
						G_SetLastError(G_ERROR_SSL_UNABLE_CONNECT_SOCKET);
						return -1;
					}
				#endif

				if (connect(s->m_SocketFd, (struct sockaddr*) &(s->m_ServerAddress), sizeof(s->m_ServerAddress))>=0)
				{
					#ifdef __linux__
						fcntl(s->m_SocketFd, F_SETFL, flag | O_NONBLOCK); /* Nonblocking socket-io*/
					#elif _WIN32
						u_long flags = 1;
						ioctlsocket(s->m_SocketFd, FIONBIO, &flags);
					#endif
					set = 1;

					s->m_SSL = SSL_new(s->m_SSLContext);

					FD_ZERO(&s->m_ReadFlag);
					FD_ZERO(&s->m_WriteFlag);

					if (!s->m_SSL || !SSL_set_fd(s->m_SSL, s->m_SocketFd))
					{
						MIA_DEBUG_PRINT("Unable to connect SSL.\n");
						G_SetLastError(G_ERROR_SSL_UNABLE_CONNECT_SOCKET);
						return -1;
					}

					while (!G_SecureSocket_IsReady(s))
					{
						MIA_SLEEP_MS(1);
					};

				while(1)
				{
					int retcode = SSL_connect(s->m_SSL);
					if (retcode != 1)
					{
						int code = SSL_get_error(s->m_SSL, retcode);
						if(code == SSL_ERROR_WANT_WRITE || code == SSL_ERROR_WANT_READ)
						{
							MIA_SLEEP_MS(1);
							continue;
						}

						G_SetLastErrorExact(G_ERROR_SSL_UNABLE_CONNECT_SOCKET, code);
						return -1;
					}
					return 0;
				}
			}
			MIA_DEBUG_PRINT("Unable to make connection.\n");
			err = errno;
			if (err == ECONNREFUSED || err == ENETUNREACH) G_SetLastErrorExact(G_ERROR_CONNECTION_REFUSED, err);
			else if (err == ETIMEDOUT) G_SetLastErrorExact(G_ERROR_TIME_OUT, err);
			else G_SetLastErrorExact(G_ERROR_UNABLE_CONNECT, err);
		} else
		{
			MIA_DEBUG_PRINT("Unable to open socket.\n");
			G_SetLastErrorExact(G_ERROR_UNABLE_OPEN_SOCKET, errno);
			return -1;
		}
	}
	#elif _WIN32
			DWORD sockErr;
			memset(&(s->m_ServerAddress), 0, sizeof(s->m_ServerAddress));
			s->m_ServerAddress.sin_addr.s_addr = inet_addr(s->m_Host);
			s->m_ServerAddress.sin_family = AF_INET;
			s->m_ServerAddress.sin_port = htons(s->m_Port);

			// WSASocket
			s->m_SocketFd = socket(AF_INET, SOCK_STREAM, 0);//WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

			if (s->m_SocketFd == INVALID_SOCKET)
			{
				int code = WSAGetLastError();
				MIA_DEBUG_PRINT("WSASocket returned error %ld\n", code);
				G_SetLastErrorExact(G_ERROR_SSL_UNABLE_CONNECT_SOCKET, code);
				return -1;
			}

			//-----------------------------------------
			// Specify the server SPN
			size_t newsize = strlen(s->m_Host) + 1;
			wchar_t serverSPN[MAX_PATH] = { 0 };
			DWORD serverSPNLen = MAX_PATH;
			wchar_t * hostname = (wchar_t *)LocalAlloc(LPTR, sizeof(wchar_t) * newsize);

			struct addrinfo	aiHints = { 0 };
			struct addrinfo*	aiList = NULL;

			// Convert char* string to a wchar_t* string.
			ZeroMemory(&aiHints, sizeof(aiHints));
			aiHints.ai_family = AF_INET;
			aiHints.ai_socktype = SOCK_STREAM;
			aiHints.ai_protocol = IPPROTO_TCP;

			size_t convertedChars = 0;
			mbstowcs_s(&convertedChars, hostname, newsize, s->m_Host, _TRUNCATE);
			wchar_t port[16];
			swprintf(port, 16, L"%d", s->m_Port);
			sockErr = getaddrinfo(s->m_Host, port, &aiHints, &aiList);
			struct sockaddr* serverAddr = aiList->ai_addr;
			ULONG serverAddrLen = (ULONG)aiList->ai_addrlen;
		
			//-----------------------------------------
			// Connect to the server

			SOCKADDR_STORAGE LocalAddr = { 0 };
			SOCKADDR_STORAGE RemoteAddr = { 0 };
			DWORD dwLocalAddr = sizeof(LocalAddr);
			DWORD dwRemoteAddr = sizeof(RemoteAddr);
			TCHAR PortName[10] = { 0 };
			
			_itot_s(s->m_Port, PortName, _countof(PortName), 10);

			BOOL bSuccess = WSAConnectByName(s->m_SocketFd, hostname,
				port, &dwLocalAddr,
				(SOCKADDR*)&LocalAddr,
				&dwRemoteAddr,
				(SOCKADDR*)&RemoteAddr,
				NULL,
				NULL);

			if (!bSuccess) 
			{
				closesocket(s->m_SocketFd);
				return -1;
			}
			sockErr = setsockopt(s->m_SocketFd, SOL_SOCKET,
				0x7010 /*SO_UPDATE_CONNECT_CONTEXT*/, NULL, 0);
			if (sockErr == SOCKET_ERROR) {
				
				closesocket(s->m_SocketFd);
				return -1;
			}
			return 0;
		}
		#endif
	return -1;
}
/*
void print_hex_memory(void *mem, int length) 
{
	int i;
	unsigned char *p = (unsigned char *)mem;
	for (i = 0; i<length; i++)
	{
		printf("0x%02x ", p[i]);
		if ((i % 16 == 0) && i)
			printf("\n");
	}
	printf("\n");
}*/

int G_SecureSocket_Write(t_SecureSocket s, const char* message, size_t bufferLength)
{
	#ifdef __OPENSSL__
		if ((message && bufferLength > 0) && s && (s->m_SSL))
		{
			int sent = SSL_write(s->m_SSL, message, (int) bufferLength);
			if (sent < 0)
			{
				int error = SSL_get_error(s->m_SSL, (int) sent);
				if (error == 5)
				{
					error = ERR_get_error();
					G_SetLastErrorExact(G_ERROR_UNABLE_SEND, error);
				} else if (error == SSL_ERROR_SSL)
				{
					error = ERR_get_error();
					G_SetLastErrorExact(G_ERROR_UNABLE_SEND, error);
				}
				else if (error == SSL_ERROR_WANT_WRITE || error == SSL_ERROR_WANT_READ)
				{
					return 0;
				}
				else if (error != SSL_ERROR_WANT_WRITE && error != SSL_ERROR_WANT_READ)
				{
					G_SetLastErrorExact(G_ERROR_UNABLE_SEND, error);
				}
			}
			else if (sent != bufferLength)
			{
				MIA_DEBUG_PRINT("Potential error. \n");
			}
			return sent;
		} else
		{
			G_SetLastError(G_ERROR_UNINITIALIZED);
		}
		return -1;
	#elif _WIN32
		if ((message && bufferLength > 0) && s && (s->m_SocketFd))
		{
			WSABUF buffer = { 0 };
			buffer.len = (ULONG)bufferLength;
			buffer.buf = message;
			DWORD sent;
			DWORD sockErr = WSASend(s->m_SocketFd, &buffer, 1, &sent, 0, NULL, NULL);
			if (sockErr == SOCKET_ERROR)
			{
				sockErr = WSAGetLastError();
				MIA_DEBUG_PRINT("WSASend returned error %ld\n", sockErr);
				G_SetLastErrorExact(G_ERROR_UNABLE_SEND, sockErr);
				return -1;
			}

			return sent;
		}
		else
		{
			G_SetLastError(G_ERROR_UNINITIALIZED);
		}
		return -1;
	#endif
}

int G_SecureSocket_Read(t_SecureSocket socket, char* buffer, size_t size)
{
#ifdef __OPENSSL__
	while (1)
	{
		int ret = SSL_read(socket->m_SSL, buffer, (int) size);

		if (ret < 0)
		{
			int code = SSL_get_error(socket->m_SSL, ret);
			if (code != SSL_ERROR_WANT_WRITE && code != SSL_ERROR_WANT_READ)
			{
				G_SetLastErrorExact(G_ERROR_UNABLE_RECEIVE, SSL_get_error(socket->m_SSL, ret));
				return ret;
			}
			MIA_SLEEP_MS(1); // give IO time

			continue; // try again
		}
		return ret;
	}
	#elif _WIN32
		DWORD recevived = 0;
		WSABUF wbuffer = { 0 };
		wbuffer.len = (ULONG)size;
		wbuffer.buf = buffer;
		DWORD flags = MSG_PARTIAL;

		DWORD ret = WSARecv(socket->m_SocketFd, &wbuffer, 1, &recevived, &flags,NULL,NULL);
		if (ret == SOCKET_ERROR)
		{
			ret = WSAGetLastError();
			MIA_DEBUG_PRINT("WSARecv returned error %ld\n", ret);
			G_SetLastErrorExact(G_ERROR_UNABLE_RECEIVE, ret);
			return ret;
		}
	#endif
}

void G_SecureSocket_Free(t_SecureSocket *socket)
{
	if (*socket)
	{
		t_SecureSocket s = *socket;

#ifdef __OPENSSL__
		if (s->m_SSL)
		{
			int repeat = 1;
			while (repeat)
			{
				int ret = SSL_shutdown(s->m_SSL);
				if (ret == 1 || ret <0)
				{
					repeat = 0;
				}
			}
			SSL_free(s->m_SSL);
		}

#endif
		if (s->m_SocketFd)
		{
#ifdef __linux__
			close(s->m_SocketFd);
#elif _WIN32
			closesocket(s->m_SocketFd);
#endif
			s->m_SocketFd = 0;
		}

		free(s->m_Host);
		s->m_Host = 0;

#ifdef __OPENSSL__
		s->m_SSLContext = 0;
		s->m_SSL = 0;
#endif
		free(s);
		*socket = 0;
	}
}

int G_SecureSocket_IsReady(t_SecureSocket socket)
{
	t_SecureSocket s=socket;
	int sel;
	int readready;
	int writeready;
	static struct timeval waittime = { 1, 0};

	FD_SET(s->m_SocketFd, &(s->m_ReadFlag));
	FD_SET(s->m_SocketFd, &(s->m_WriteFlag));

	sel = select((int)socket->m_SocketFd+1, &(s->m_ReadFlag), &(socket->m_WriteFlag), 0, &waittime);
	if (sel < 0) return sel;

	readready = FD_ISSET(s->m_SocketFd, &(s->m_ReadFlag));
	writeready = FD_ISSET(s->m_SocketFd, &(s->m_WriteFlag));

	FD_CLR(s->m_SocketFd, &(s->m_ReadFlag));
	FD_CLR(s->m_SocketFd, &(s->m_WriteFlag));

	if (readready && writeready) return 3;
	if (writeready) return 2;
	if (readready) return 1;
	return 0;
}

t_ISocket G_Socket_CreateUnsecure()
{
	t_ISocket s = malloc(sizeof(struct _ISocket));
	memset(s, 0, sizeof(struct _ISocket));

	s->M_Socket_New 		= (void* (*) (char*, int)) G_Socket_New;
	s->M_Socket_Connect 	= (int (*) (void*)) G_Socket_Connect;
	s->M_Socket_Write 	= (int (*) (void*, const char*, size_t)) G_Socket_Write;
	s->M_Socket_Read		= (int (*) (void*, char *, size_t)) G_Socket_Read;
	s->M_Socket_Free		= (int (*) (void* socket)) G_Socket_Free;
	s->M_Socket_IsReady 	= (int (*) (void* socket)) G_Socket_IsReady;

	return s;
}

t_ISocket G_Socket_CreateSecure()
{
#if __OPENSSL__ | _WIN32
	t_ISocket s = malloc(sizeof(struct _ISocket));
	memset(s, 0, sizeof(struct _ISocket));

	s->M_Socket_New 		= (void* (*) (char*, int)) G_SecureSocket_New;
	s->M_Socket_Connect 	= (int (*) (void*)) G_SecureSocket_Connect;
	s->M_Socket_Write 	= (int (*) (void*, const char*, size_t)) G_SecureSocket_Write;
	s->M_Socket_Read		= (int (*) (void*, char *, size_t)) G_SecureSocket_Read;
	s->M_Socket_Free		= (int (*) (void* socket)) G_SecureSocket_Free;
	s->M_Socket_IsReady 	= (int (*) (void* socket)) G_SecureSocket_IsReady;
	return s;
#else
	assert(0);
	return 0;
#endif
}


unsigned long long G_Monotonic_Time(void)
{
#ifdef __linux__
	struct timespec monotime;
	clock_gettime(CLOCK_MONOTONIC, &monotime);

	return (unsigned int)monotime.tv_sec;
#else
	LARGE_INTEGER performance_count;
	LARGE_INTEGER frequency;

	if (!QueryPerformanceCounter(&performance_count))
	{
		return time(NULL);
	}

	if (!QueryPerformanceFrequency(&frequency))
	{
		return time(NULL);
	}

	return performance_count.QuadPart / frequency.QuadPart;

#endif
}

#ifdef __linux__

void *G_ThreadLock_New_OS(void)
{
	int ret;
	pthread_mutex_t * mutex = malloc(sizeof(pthread_mutex_t));

	if (!mutex)	return NULL; // Malloc failed

	ret = pthread_mutex_init(mutex, NULL);

	if (ret)
	{   // Initialization failed
		free(mutex);
		return NULL;
	}

	return mutex;
}

int G_ThreadLock_Lock_OS(void * mutex)
{
	return pthread_mutex_lock((pthread_mutex_t*)mutex);
}

int G_ThreadLock_Unlock_OS(void * mutex)
{
	return pthread_mutex_unlock((pthread_mutex_t*)mutex);
}

void G_ThreadLock_Free_OS(void * mutex)
{
	if(mutex) pthread_mutex_destroy((pthread_mutex_t*)mutex);
}

#elif _WIN32

void * G_ThreadLock_New_OS(void)
{
	return  CreateMutex(NULL, FALSE, NULL);;
}

int G_ThreadLock_Lock_OS(void * mutex)
{
	DWORD result = WaitForSingleObject((HANDLE)mutex, INFINITE);
	if (result != WAIT_OBJECT_0)
	{
		G_SetLastErrorExact(G_ERROR_UNKOWN, errno);
		return -1;
	}
	return 0;
}

int G_ThreadLock_Unlock_OS(void * mutex)
{
	return ReleaseMutex((HANDLE)mutex);
}

void G_ThreadLock_Free_OS(void * mutex)
{
	if(mutex) CloseHandle((HANDLE)mutex);
}

#else
	#error OS not regonised
#endif

