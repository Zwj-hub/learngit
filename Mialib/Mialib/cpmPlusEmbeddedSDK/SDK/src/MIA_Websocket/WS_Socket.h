/*
 * WS_Socket.h
 *
 *  Created on: Sep 29, 2014
 *      Author: Tuan Vu
 */

#ifndef WS_SOCKET_H_
#define WS_SOCKET_H_

#include <string.h>

/**
 * @brief General socket interface
 */
typedef struct _ISocket
{
	void					(*M_Socket_Init)(void);
	void 					(*M_Socket_Finalize)(void);
	/**
	 * @brief Create a new socket instance.
	 * @return A pointer to the initiated instance.
	 */
	void* 				(*M_Socket_New)(char* hostName, int port);

	/**
	 * @brief Establish a socket connection.
	 * @return  0 if the socket connection.
	 * 			positive value if the connection is being established on a non-blocking io.
	 * 			negative if the connection is not able to be established.
	 */
	int 					(*M_Socket_Connect)(void* socket);

	/**
	 * @brief Write data to the socket. This method should be non-blocking.
	 * @return	positive_value The number of byte which was written. Writing can be resumed at a later time.
	 * 			negative If the socket is not able to be written. Using G_SetLastError and G_GetLastError to set/retrieve error code.
	 */
	int					(*M_Socket_Write)(void* socket, const char* message, size_t bufferLength);

	/**
	 * @brief Read data to the buffer.  This method should be non-blocking.
	 * @return	The number of byte read. If the result is negative, the socket is not able to read.
	 * 			Using G_GetLastError and G_SetLastError to retrieve the exact error code.
	 */
	int					(*M_Socket_Read)(void* socket, char *buffer, size_t size);

	/**
	 * @brief Check if the socket is ready for read/write.
	 * @return 	0 if socket is unable to read/write.
	 * 			1 if the socket is available for reading.
	 * 			2 if the socket is available for writing.
	 * 			3 if the socket is available for both reading/writing.
	 */
	int					(*M_Socket_IsReady)(void* socket);

	/**
	 * @brief Release the socket associated resource
	 */
	int					(*M_Socket_Free)(void* socket);
} *t_ISocket;


/**
 * @brief Translate host to host name and IP address
 *
 * If translation fails then host is copied to hostname and IP address
 *
 * @return       0 success
 *              -1 memory allocation fails
 */
int G_ResolveHostAndIP(char **hostname, char **hostIp, char* host);

t_ISocket G_Socket_CreateUnsecure(void);

t_ISocket G_Socket_CreateSecure(void);


#endif /* WS_SOCKET_H_ */
