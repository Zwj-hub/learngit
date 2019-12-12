/*
 * Mia_MiaDriverBase.h
 *
 *  Created on: Mar 18, 2015
 *      Author: Tuan Vu
 */

#ifndef _MIA_DRIVERBASE_H_
#define _MIA_DRIVERBASE_H_

#include <string>
#include <list>
#include <iostream>
#include <functional>
#include "Mia_Base.h"

extern "C"
{
	#include <WS_Websocket.h>
}

namespace ABB
{
	namespace Mia
	{

		class Mia_EXPORT C_IReconnectHandler
		{
			class RunThread : public C_Thread
			{
				public: RunThread(C_IReconnectHandler * handler) : C_Thread(std::string("C_IReconnectHandler")), m_handler(handler)
				{

				}

				virtual void M_Routine() {
					m_handler->M_OnReconnect();
				}

				private: C_IReconnectHandler * m_handler;
			};

			public: C_IReconnectHandler() : runThread(this) {};
			public: virtual ~C_IReconnectHandler() {}
			public: virtual void M_OnReconnect() = 0;
			public: void M_OnReconnectThread()     { runThread.M_Start(); }

			private: RunThread runThread;

		};

		/**
		 * @brief C_MiaDriverBase class is used to interface with RTDB History server in a asynchronous manner
		 * The driver implements secure websocket connections together with
		 * basic authentication.
		 *
		 * For further instructions on each commands, please query for further
		 * document on the server application.
		 * \ingroup MiaDriver
		 */
		class Mia_EXPORT C_MiaDriverBase
		{
			protected: class C_WebsocketWrapper;
			protected: class C_MiaDriverBasePrivate;

			public:	typedef void (*F_Callback)(void*, t_CallId callId, void *result);

			public:	enum t_CallType
			{
				g_CALL, g_SUBSCRIBE
			};

			public:	enum t_State
			{
				g_INITIAL, g_CONNECTING, g_READY, g_ERROR, g_DISCONNECTED
			};

			/**
			 * @brief Getter for driver state
			 *
			 * @return driver state
			 */
			public:	t_State M_GetState() { return m_State; };
			/**
			 * @brief Constructor
			 * @param connectionString: The connectionString to the MiaServer
			 */

			public:	C_MiaDriverBase(const std::string &connectionString);

			public: virtual ~C_MiaDriverBase();

			/**
			 * @brief Change driver mode to enable/disable auto reconnect of disconnection
			 * @param reconnect
			 */
			public:	void M_SetAutoReconnect(bool reconnect = true);

			/**
			 * @brief Connect to the connection string
			 * @param result The callback when the connection is completed or failed.
			 * 				The callback receives result as the new state of the connection
			 *
			 */
			public:	bool M_Connect(F_Callback *result = 0);

			/**
			 * @brief Connect to the connection string using username and password
			 * 			Authentication shall be handled by http basic authentication
			 * @param username The username used for authenticate the connection
			 * @param password The password used for authenticate the connection
			 * @param onResult The callback when the connection is completed or failed.
			 * 						The callback receives result as the new state of the connection
			 * @return true if the connection can be established
			 * @throw C_SSLException If server encounters a problem with SSL connection
			 * @throw C_SocketException If server encounters a problem with the socket connection
			 * @throw C_TimeoutException If server is unable to establish a connection within the defined Timeout
			 * @throw C_BadConnectionStringException If there is a problem with the connectionString formatl
			 */
			public:	bool M_Connect(const std::string &username, const std::string &password, F_Callback *onResult = 0);

			/**
			 * @brief Reconnect to the connection, should only be called when the connection is disconnected
			 *
			 * @return true if the connection can be established
			 */
			public: bool M_Reconnect();

			/**
			 * @brief Send the message to the server. The message must be in a good format.
			 * The driver will queue the message into its internal buffer and return immediately.
			 *
			 * @param message The message to be sent. It's preferable to create the message with
			 * the C_WebsocketStream class.
			 *
			 * @return true If the message is able to be queued.
			 * @return false If the message cannot be queued.
			 *
			 * @throw C_ServerException If server is unable to handle the request.
			 */
			public: bool M_Send(const std::string &message);
			/**
			* @brief Send the message to the server. The message must be in a good format.
			* The driver will queue the message into its internal buffer and return immediately.
			*
			* @param message The message to be sent. 
			*
			* @return true If the message is able to be queued.
			* @return false If the message cannot be queued.
			*
			* @throw C_ServerException If server is unable to handle the request.
			*/
			public: bool M_Send(const C_MemoryBuffer &message);

			/**
			* @brief Send the message to the server. The message must be in a good format.
			* The driver will queue the message into its internal buffer and return immediately.
			*
			* @param message The message to be sent.
			* @param length The length of the message to be sent.
			*
			* @return true If the message is able to be queued.
			* @return false If the message cannot be queued.
			*
			* @throw C_ServerException If server is unable to handle the request.
			*/
			public: bool M_Send(const char *message, const uint length);

			/**
			* @brief Send the message to the server. The message must be in a good format.
			* The driver will queue the message into its internal buffer and return immediately.
			*
			* @param message The message to be sent.
			*
			* @return true If the message is able to be queued.
			* @return false If the message cannot be queued.
			*
			* @throw C_ServerException If server is unable to handle the request.
			*/
			public: bool M_Send(const t_Message& message);

			/**
			 * @brief Call the given methodName with given parameters and call onResult
			 * callback upon receiving data
			 *
			 * @param methodName 	The name of the method
			 * @param parameters 	Parameters of the method
			 * @param onResult 	The callback which shall be called when the method returns.
			 * 							The result is a string buffer of type char*. User is
			 * 							responsible for freeing it after
			 *
			 * @return Call Id of the call
			 */
			public:	t_CallId M_Call(const std::string &methodName, const std::string &parameters, F_Callback onResult);
			public: void M_RegisterCall(t_CallId callid, F_Callback onResult, void* callbackData = 0);

			/**
			 * @brief Fetch properties data from the given className using the limitation given by
			 * fetchMax, where and whereData. On result, the callback onResult will be called
			 *
			 * @param className 	The name of the class to fetch data
			 * @param properties 	Properties of the class
			 * @param fetchMax 	The maximum number of records return
			 * @param where			The filtering string
			 * @param whereData	The filtering data
			 * @param onResult 	The callback which shall be called when the method returns
			 * @return Call Id of the call
			 */
			public:	t_CallId M_FetchClassData(const std::string &className, const std::string &properties, int fetchMax,
					const std::string &where, const std::string &whereData, F_Callback onResult);

			/**
			 * @brief Update/insert properties data to the given className using the with the values
			 * given in newValues
			 *
			 * @param className 	The name of the class to fetch data
			 * @param properties 	Properties of the class
			 * @param instanceId 	Id of the instance in case of updating, should be left as null string if inserting
			 * @param newValues	The values to be updated/inserted
			 * @param oldValues 	Only update if the value matches
			 * @param onResult 	The callback which shall be called when the method returns
			 * @return Call Id of the call
			 */
			public: t_CallId M_CommitClassData(const std::string &className, const std::string &properties,
					const std::string &instanceId, const std::string &newValues, const std::string &oldValues,
					F_Callback onResult);
			/**
			 * @brief Add reconnect event handler
			 * @param func Pointer to the handler
			 */
			public: void M_AddOnReconnect(C_IReconnectHandler *handler);

			/**
			 * @brief Remove reconnect event handler
			 * @param func Pointer to the handler
			 */
			public: void M_RemoveOnReconnect(C_IReconnectHandler *handler);

			/**
			 * @brief Count items in send buffer
			 * @return items in buffer
			 */
			public: int32 M_BufferCnt();

			/**
			 * @brief The connection string which used to connect to MiaServer
			 */
			protected: std::string m_ConnectionString;
			/**
			 * @brief The websocket connection, represent the connection to MiaServer
			 * @see C_Websocket
			 */
				//protected: 	C_Websocket 	*m_Websocket;
			protected:	C_WebsocketWrapper *m_Websocket;
			/**
			 * @brief Current state of the driver
			 * Can have the following values:
			 * 		+ g_INITIAL,
			 *			+ g_CONNECTING,
			 *			+ g_READY,
			 *			+ g_ERROR,
			 *			+ g_DISCONNECTED
			 */
			protected:	t_State m_State;
			/**
			 * @brief A flag indicating that should the client retry the connection when error or disconnect occurred
			 */
			protected:	bool m_AutoReconnect;
			/**
			 * @brief Timeout for each retry
			 */
			protected:	int m_ReconnectTimeout;

			/**
			 * @brief Retry time increase as the number of failures increase up to the m_MaxRetryTime
			 * @see m_MaxRetryTime
			 */
			protected:	int m_CurrentRetryTime;
			/**
			 * @brief Maximum time between two connection retry
			 */
			protected:	static int m_MaxRetryTime;
			protected:	static t_CallId s_CallId;

			public:	static t_CallId M_S_GetCallId();

			/**
			* @brief Authentication token which used to authenticate the connection.
			* Using the basic http authentication
			* @see C_Authentication
			*/
			//protected: 	C_Websocket::C_Authentication *m_Authentication;
			protected:	static int m_s_TimeOut;

			/* Private implementation sections*/
			protected:	class t_MapElement
			{
				public: t_CallId 		m_CallId;
				public: C_MiaDriverBase::F_Callback m_OnResult;
				public: void			*m_CallbackData;
				public: t_CallType 	m_CallType;
				public: C_DateTime 	m_CreatedTime;
				public: C_Event * 	m_Event;
				public: char* 			m_Message;
				public: int 			m_Length;

				public:	t_MapElement(t_CallId callId, C_MiaDriverBase::F_Callback onResult, t_CallType callType, void* callbackData = 0) :
								m_CallId(callId), m_OnResult(onResult),m_CallbackData(callbackData), m_CallType(callType), m_Event(0), m_Message(0), m_Length(0) 
				{
				}
				public: ~t_MapElement()
				{
					if (m_Event) delete m_Event;
					if (m_Message) delete [] m_Message;
				}
			};

			protected: t_MapElement* M_RegisterEventCall(t_CallId callId, t_CallType type = g_CALL);
			protected: bool M_RemoveEventCall(t_CallId callid);
			protected: friend class C_MiaDriverBasePrivate;
			protected: C_MiaDriverBasePrivate* m_Private;

			friend class C_MiaDriver;
		};

	} /* namespace Mia */
} /* namespace ABB */

#endif /* _MIA_DRIVERBASE_H_ */
