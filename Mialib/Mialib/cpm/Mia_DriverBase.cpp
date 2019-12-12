/*
 * Mia_MiaDriverBase.cpp
 *
 *  Created on: Mar 18, 2015
 *      Author: Tuan Vu
 */
#include <stdlib.h>
#include <iostream>
extern "C"
{
	#include <WS_Util.h>
	#include <WS_Websocket.h>
}

#include "Mia_DriverBase.h"
#include "Mia_Exception.h"

#ifndef DRIVER_DIE_TIMEOUT
#define DRIVER_DIE_TIMEOUT 2000000000
#endif

#ifndef WEBSOCKET_POLL_TIME
#define WEBSOCKET_POLL_TIME 50
#endif

namespace ABB
{
	namespace Mia
	{
		class C_MiaDriverBase::C_WebsocketWrapper
		{
			/**
			* @brief A handler class that will be used by the C_Websocket to handle messages
			*
			*/
			public: class C_WebsocketHandler
			{
				public: virtual ~C_WebsocketHandler(){};
				public: virtual void M_OnMessage(C_WebsocketWrapper*, const char* message, int length) = 0;
				public: virtual void M_OnConnected(C_WebsocketWrapper*) = 0;
				public: virtual void M_OnDisconnected(C_WebsocketWrapper*) = 0;
				public: virtual void M_OnError(C_WebsocketWrapper*, const char* errorMessage) = 0;
			};

			public: C_WebsocketWrapper(C_WebsocketHandler *handler);

			public: ~C_WebsocketWrapper();
			/**
			* @brief Connect the client to the connection string, using the given handler to handle return messages and
			* optionally authenticate the connection with the given authentication
			*
			* The method starts an internal thread to connect to the given connectionString. It assumes that the client is in the
			* g_INITIAL or g_DISCONNECTED state.
			*
			* @param connectionString The string used to identify the server
			* @param handler The C_WebsocketHandler implementation to handle messages
			* @param authentication Optional authentication
			*
			* @return true if the method succeeds
			*/
			public: bool M_Connect(const std::string &connectionString, C_WebsocketHandler* handler, const std::string &username = "", const std::string &password = "");

			/**
			* @brief Send the message to the server. This method does not block since it queues to internal buffer.
			*
			* @param message The message to be sent, limited to only 4KB length message size.
			* @param length Optional parameter to indicate the length, otherwise, strlen will be used.
			*
			* @return true if the method succeeds
			*/
			public: bool M_Send(const char* message, int length = 0);

			/**
			* @brief Send the message to the server. This method does not block since it queues to internal buffer.
			*
			* @param message The message to be sent, limited to only 4KB length message size.
			*
			* @return true if the method succeeds
			*/
			public: bool M_Send(const std::string &message);

			/**
			* @brief Send the message to the server. This method does not block since it queues to internal buffer.
			*
			* @param message The message to be sent, limited to only 4KB length message size.
			*
			* @return true if the method succeeds
			*/
			public: bool M_Send(const t_Message &message);

			/**
			* @brief Disconnect the client
			*
			* @return true if the method succeeds
			*/
			public: bool M_Disconnect();

			/**
			* @brief Check websocket buffer state
			*
			* @return Return websocket buffer item count
			*/
			public: int32 M_BufferCnt();

			public: C_WebsocketHandler *M_GetHandler() { return m_Handler;}

			class C_WebsocketWrapperPrivate;
			friend class C_WebsocketWrapperPrivate;
			class C_WebsocketStaticHandler;
			friend class C_WebsocketStaticHandler;
			private: C_WebsocketHandler 			*m_Handler;
			private: C_WebsocketWrapperPrivate 	*m_Private;
		};

		class C_MiaDriverBase::C_WebsocketWrapper::C_WebsocketWrapperPrivate : private C_Thread
		{
			class C_WebsocketStaticHandler
			{
				public: static int M_S_OnMessage(t_Websocket websocket, const char* message, int length)
				{
					C_WebsocketWrapper* wrapper = (C_WebsocketWrapper*)websocket->m_Info->m_CallbackData;
					if (!wrapper) return -1;
					C_WebsocketWrapper::C_WebsocketHandler *handler = (C_WebsocketWrapper::C_WebsocketHandler*) wrapper->M_GetHandler();
					if (!handler) return -1;
					handler->M_OnMessage(wrapper, message, length);
					return 0;
				}

				public: static int M_S_OnError(t_Websocket websocket, int errorCode, const char* additionalInformation)
				{
					C_WebsocketWrapper* wrapper = (C_WebsocketWrapper*)websocket->m_Info->m_CallbackData;
					//std::cout<<"Wrapper" <<wrapper <<std::endl;
					if (!wrapper) return -1;
					C_WebsocketWrapper::C_WebsocketHandler *handler = (C_WebsocketWrapper::C_WebsocketHandler*) wrapper->M_GetHandler();
					//std::cout<<"Handler" <<handler <<std::endl;
					if (!handler) return -1;
					handler->M_OnError(wrapper, additionalInformation);

					return 0;
				}

				public: static int M_S_OnDisconnect(t_Websocket websocket, int reasonCode)
				{
					C_WebsocketWrapper* wrapper = (C_WebsocketWrapper*)websocket->m_Info->m_CallbackData;
					if (!wrapper) return -1;
					C_WebsocketWrapper::C_WebsocketHandler *handler = (C_WebsocketWrapper::C_WebsocketHandler*) wrapper->M_GetHandler();
					if (!handler) return -1;
					handler->M_OnDisconnected(wrapper);
					return 0;
				}

				public: static int M_S_OnConnected(t_Websocket websocket)
				{
					C_WebsocketWrapper* wrapper = (C_WebsocketWrapper*)websocket->m_Info->m_CallbackData;
					if (!wrapper) return -1;
					C_WebsocketWrapper::C_WebsocketHandler *handler = (C_WebsocketWrapper::C_WebsocketHandler*) wrapper->M_GetHandler();
					if (!handler) return -1;
					handler->M_OnConnected(wrapper);
					return 0;
				}

				public: static int M_S_OnAdditionalRequest(t_Websocket websocket, const char* message, int length)
				{
					return M_S_OnError(websocket, -1, message);
				}
				friend class C_WebsocketWrapper;
			};

			public: C_WebsocketWrapperPrivate(C_WebsocketWrapper* wrapper): C_Thread(std::string("C_WebsocketWrapperPrivate")),
					m_Public(wrapper),
					m_WebsocketInfo(0),
					m_Websocket(0),
					m_SleepTime(WEBSOCKET_POLL_TIME)
			{
			}

			virtual void M_Routine()
			{
				if(m_Websocket)
				{
					G_Websocket_Service(m_Websocket);
				}
			}

			private: C_WebsocketWrapper 	*m_Public;
			private: t_WebsocketInfo 		m_WebsocketInfo;
			private: t_Websocket 			m_Websocket;
			private: int 						m_SleepTime;

			friend class C_WebsocketWrapper;
		};

		C_MiaDriverBase::C_WebsocketWrapper::C_WebsocketWrapper(C_WebsocketHandler *handler) : m_Handler(handler)
		{
			m_Private = DBG_NEW C_WebsocketWrapper::C_WebsocketWrapperPrivate(this);
		}


		C_MiaDriverBase::C_WebsocketWrapper::~C_WebsocketWrapper()
		{
			G_Websocket_Free(&(this->m_Private->m_Websocket));
			delete m_Private;
		}
		bool C_MiaDriverBase::C_WebsocketWrapper::M_Connect(const std::string &connectionString, C_WebsocketHandler* handler, const std::string &username, const std::string &password)
		{
			static char sThisRoutine[] = "C_WebsocketWrapper::M_Connect";
			this->m_Handler = handler;
			t_WebsocketInfo info = G_WebsocketInfo_New();
			info->m_ConnectionString = G_String_NewRaw(connectionString.c_str());
			info->m_Username = G_String_NewRaw(username.c_str());
			info->m_Password = G_String_NewRaw(password.c_str());
			info->m_CallbackData = this;

			info->M_OnError = C_WebsocketWrapperPrivate::C_WebsocketStaticHandler::M_S_OnError;
			info->M_OnConnected = C_WebsocketWrapperPrivate::C_WebsocketStaticHandler::M_S_OnConnected;
			info->M_OnDisconnect = C_WebsocketWrapperPrivate::C_WebsocketStaticHandler::M_S_OnDisconnect;
			info->M_OnMessage = C_WebsocketWrapperPrivate::C_WebsocketStaticHandler::M_S_OnMessage;

			m_Private->m_WebsocketInfo = info;
			m_Private->m_Websocket = G_Websocket_New(info);

			if(!m_Private->m_Websocket)
			{
				G_WebsocketInfo_Free(&(m_Private->m_WebsocketInfo));
				C_Exception::M_S_HandleException(Mia_THIS_LOCATION, G_GetLastError(), connectionString, (int64) handler, username);
				return false;
			}

			int ret = G_Websocket_Connect(m_Private->m_Websocket);
			if (ret < 0)
			{
				G_Websocket_Free(&m_Private->m_Websocket);
				C_Exception::M_S_HandleException(Mia_THIS_LOCATION, G_GetLastError(), connectionString, (int64) handler, username);
				return false;
			}
			else
			{
				return m_Private->M_Loop(m_Private->m_SleepTime);
			}
		}

		bool C_MiaDriverBase::C_WebsocketWrapper::M_Send(const char* message, int length)
		{
			int ret = G_Websocket_SendBinary(m_Private->m_Websocket, message, length);
			return (ret != -1);
		}

		bool C_MiaDriverBase::C_WebsocketWrapper::M_Send(const std::string &message)
		{
			int ret = G_Websocket_Send(m_Private->m_Websocket, message.c_str(), (int)message.length());
			return (ret != -1);
		}

		bool  C_MiaDriverBase::C_WebsocketWrapper::M_Send(const t_Message &message)
		{
			return G_Websocket_SendRaw(m_Private->m_Websocket, message) != -1;
		}

		int32 C_MiaDriverBase::C_WebsocketWrapper::M_BufferCnt()
		{
			return G_Websocket_BufferCnt(m_Private->m_Websocket);
		}
		bool C_MiaDriverBase::C_WebsocketWrapper::M_Disconnect()
		{
			return m_Private->M_Stop(true, DRIVER_DIE_TIMEOUT);
		}

		t_CallId C_MiaDriverBase::s_CallId = 100;

		class C_MiaDriverBase::C_MiaDriverBasePrivate: public C_WebsocketWrapper::C_WebsocketHandler, private C_Thread
		{
			friend class C_MiaDriver;

			public: C_MiaDriverBasePrivate(C_MiaDriverBase *driver);
			public: virtual ~C_MiaDriverBasePrivate();

			public:	virtual void M_OnMessage(C_WebsocketWrapper* ws, const char* message, int length);
			public:	virtual void M_OnConnected(C_WebsocketWrapper* ws);
			public:	static void* M_S_ReconnectRoutine(void *d);
			public: 	static void* M_S_ReconnectHandlerDispatcherRoutine(void* h) { ((C_IReconnectHandler*)h)->M_OnReconnect(); return 0;};
			public:	virtual void M_OnDisconnected(C_WebsocketWrapper* ws);
			public:	virtual void M_OnError(C_WebsocketWrapper* ws, const char* errorMessage);
			public:	void			 M_RegisterCall(t_CallId callid, C_MiaDriverBase::F_Callback onResult, t_CallType type = g_CALL, void* callbackData = 0);
			public:  static void  M_OnEventResult(void* driver, t_CallId callId, void *result);

			public:	C_MiaDriverBase::t_MapElement* M_RegisterEventCall(t_CallId callId, t_CallType type = g_CALL, void* callbackData = 0);
			public:	t_MapElement*	M_RemoveCall(t_CallId callid);
			public:	t_MapElement*	M_GetCall(t_CallId callid);
			public:	t_CallId		M_GetCallId(const char* message, int length);
			public:	void				M_ClearCalls();
			public: void				M_OnReconnect();
			private: virtual void	M_Routine();

			private:	C_MiaDriverBase	*m_Driver;
			private:	C_ThreadLock		m_MapLock;
			protected:	std::string		m_Username;
			protected:	std::string		m_Password;
			protected: bool				m_Connected;
			protected: C_ThreadLock		m_ReconnectLock;
			private:	C_MiaDriverBase::F_Callback *m_OnConnectionChanged;
			private:	unordered_map<t_CallId, t_MapElement*> m_Map;
			protected: std::set<C_IReconnectHandler*> m_ReconnectHandler;
			friend class C_MiaDriverBase;
		};

		C_MiaDriverBase::t_MapElement* C_MiaDriverBase::M_RegisterEventCall(t_CallId callId, t_CallType type)
		{
			return m_Private->M_RegisterEventCall(callId, type);
		}

		bool C_MiaDriverBase::M_RemoveEventCall(t_CallId callid)
		{
            return m_Private->M_RemoveCall(callid) > (void*)0;    //zwj
		}

		C_MiaDriverBase::C_MiaDriverBasePrivate::C_MiaDriverBasePrivate(C_MiaDriverBase *driver) : C_Thread(std::string("C_MiaDriverBasePrivate")), m_Driver(driver), m_OnConnectionChanged(0), m_Connected(false)
		{	}

		void C_MiaDriverBase::C_MiaDriverBasePrivate::M_OnReconnect()
		{

			MIA_OUT_DEBUG<<"C_MiaDriverBase::M_Reconnect handler called" <<std::endl;

			C_Locker locker(&m_ReconnectLock);
			for (std::set<C_IReconnectHandler*>::iterator iter = m_ReconnectHandler.begin(); iter != m_ReconnectHandler.end(); iter++)
			{
				C_IReconnectHandler *h = *iter;
				h->M_OnReconnect();
			}
		}

		C_MiaDriverBase::C_MiaDriverBasePrivate::~C_MiaDriverBasePrivate()
		{
			MIA_OUT_WARNING << "C_MiaDriverBasePrivate::Clearing" << std::endl;
			M_ClearCalls();
		}

		t_CallId C_MiaDriverBase::C_MiaDriverBasePrivate::M_GetCallId(const char* message, int length)		{
			t_CallId callid = 1;
			if (message[1] == '\"')
			{
				callid = std::atoi(message + 2);
			} else
			{
				callid = std::atoi(message + 1);
			}
			return callid;
		}

		void C_MiaDriverBase::C_MiaDriverBasePrivate::M_ClearCalls()
		{
			C_MiaDriverBase::t_MapElement* ret;
			
			while (m_Map.size())
			{
				C_Locker locker(&m_MapLock);
				ret = m_Map.begin()->second;
				m_Map.erase(m_Map.begin());
				delete ret;
			}
		}

		void C_MiaDriverBase::C_MiaDriverBasePrivate::M_OnMessage(C_WebsocketWrapper* ws, const char* message, int length)
		{
			Mia_THIS_ROUTINE("C_MiaDriverBase::C_MiaDriverBasePrivate::M_OnMessage");
			t_CallId callid = M_GetCallId(message, length);

			t_MapElement *e = M_GetCall(std::abs(callid));
			if (!e) 
			{
				callid = std::atoi(message+2);
				e = M_GetCall(std::abs(callid));
			}
			
			if (e)
			{
				e->m_Message = DBG_NEW char[length + 1];
				strncpy(e->m_Message, message, length);
				e->m_Message[length] = 0;
				e->m_Length = length;
				if (e->m_OnResult) (e->m_OnResult)(this->m_Driver, callid, (void*) e->m_Message);
			}
		}

		void C_MiaDriverBase::C_MiaDriverBasePrivate::M_OnConnected(C_WebsocketWrapper* ws)
		{
			MIA_OUT_DEBUG_WEBSOCKET << "[RTDB_MiaDriver:M_OnConnected]: Connection ready"<< std::endl;

			m_Driver->m_State = g_READY;
			if (m_Connected)
			{
				MIA_OUT_DEBUG <<"C_MiaDriverBase::M_Reconnect handler called" <<std::endl;

				C_Locker locker(&m_ReconnectLock);

				for (std::set<C_IReconnectHandler*>::iterator iter = m_ReconnectHandler.begin(); iter != m_ReconnectHandler.end(); iter++)
				{
					C_IReconnectHandler *h = *iter;
					h->M_OnReconnectThread();
				}
			} else
			{
				m_Connected = true;
			}

			if (m_OnConnectionChanged)
			{
				(*m_OnConnectionChanged)(m_Driver, 0, &m_Driver->m_State);
				m_OnConnectionChanged = 0;
			}
		}

		void* C_MiaDriverBase::C_MiaDriverBasePrivate::M_S_ReconnectRoutine(void *d)
		{
			C_MiaDriverBase *driver = (C_MiaDriverBase*) d;
			driver->m_CurrentRetryTime = 5000;
			C_Thread::M_S_Sleep(driver->m_CurrentRetryTime);
			while (true)
			{
				if (driver->M_GetState() == g_DISCONNECTED) break;
				if (driver->M_GetState() == g_CONNECTING) C_Thread::M_S_Sleep(1000);
				if (driver->M_GetState() == g_READY) return 0;
				C_Thread::M_S_Sleep(100);
			}
			while (driver->M_GetState() != g_READY)
			{
				MIA_OUT_DEBUG_WEBSOCKET << std::endl << std::endl
						<< "[C_MiaDriver::M_Reconnect]: Reconnect within "
						<< driver->m_CurrentRetryTime << std::endl;

				if (driver->M_Reconnect()) break;
				driver->m_CurrentRetryTime = driver->m_CurrentRetryTime * 2;
				if (driver->m_CurrentRetryTime > driver->m_MaxRetryTime)	driver->m_CurrentRetryTime = driver->m_MaxRetryTime;

				C_Thread::M_S_Sleep(driver->m_CurrentRetryTime);
			}

			return 0;
		}

		void C_MiaDriverBase::C_MiaDriverBasePrivate::M_Routine() 
		{
			M_S_ReconnectRoutine(&m_Driver);
		}

		void C_MiaDriverBase::C_MiaDriverBasePrivate::M_OnDisconnected(C_WebsocketWrapper* ws)
		{
			if (m_Driver->m_State == g_CONNECTING || m_Driver->m_State == g_DISCONNECTED) return;

			MIA_OUT_DEBUG_WEBSOCKET << "[RTDB_MiaDriver:M_OnDisconnected]: Disconnected"	<< std::endl;

			m_Driver->m_State = g_DISCONNECTED;
			if (m_OnConnectionChanged)
			{
				(*m_OnConnectionChanged)(m_Driver, 0, &m_Driver->m_State);
				m_OnConnectionChanged = 0;
			}

			// Start the reconnect thread
			if (m_Driver->m_AutoReconnect)
			{
				M_Start();
			}
		}

		void C_MiaDriverBase::C_MiaDriverBasePrivate::M_OnError(C_WebsocketWrapper* ws, const char* errorMessage)
		{
			Mia_THIS_ROUTINE("C_MiaDriverBase::C_MiaDriverBasePrivate::M_OnError");
			MIA_OUT_WARNING << "[" << sThisRoutine << "]: " << errorMessage << std::endl;

			m_Driver->m_State = g_ERROR;
			if (m_OnConnectionChanged)
			{
				(*m_OnConnectionChanged)(m_Driver, 0, &m_Driver->m_State);
				m_OnConnectionChanged = 0;
			}
		}

		void C_MiaDriverBase::C_MiaDriverBasePrivate::M_RegisterCall(t_CallId callid,C_MiaDriverBase::F_Callback onResult, t_CallType type, void* callbackData)
		{
			t_MapElement *element = DBG_NEW t_MapElement(callid, onResult, type, callbackData);
			C_Locker locker(&m_MapLock);
			m_Map[callid] = element;
		}

		void C_MiaDriverBase::C_MiaDriverBasePrivate::M_OnEventResult(void* driverBase, t_CallId callId, void *result)
		{
			C_MiaDriverBase *driver = (C_MiaDriverBase*)driverBase;
			t_MapElement *e = driver->m_Private->M_GetCall(abs(callId));
			e->m_Event->M_Set();
		}

		C_MiaDriverBase::t_MapElement* C_MiaDriverBase::C_MiaDriverBasePrivate::M_RegisterEventCall(t_CallId callId, t_CallType type, void* callbackData)
		{
			C_Event* e = DBG_NEW C_Event();
			C_MiaDriverBase::t_MapElement *element = DBG_NEW C_MiaDriverBase::t_MapElement(callId, M_OnEventResult, type, callbackData);
			element->m_Event = e;
			C_Locker locker(&m_MapLock);
			m_Map[callId] = element;
			return element;
		}

		C_MiaDriverBase::t_MapElement* C_MiaDriverBase::C_MiaDriverBasePrivate::M_RemoveCall(t_CallId callid)
		{
#ifdef WEBSOCKET_DEBUG
//			MIA_OUT_DEBUG_WEBSOCKET << "[C_MiaDriverBase::C_MiaDriverBasePrivate::M_RemoveCall]: Removed "	<< callid << std::endl;
#endif
			C_MiaDriverBase::t_MapElement* ret;
			C_Locker locker(&m_MapLock);
			ret = m_Map[callid];
			m_Map[callid] = 0;
			m_Map.erase(callid);
			return ret;
		}

		C_MiaDriverBase::t_MapElement* C_MiaDriverBase::C_MiaDriverBasePrivate::M_GetCall(t_CallId callid)
		{
			C_Locker locker(&m_MapLock);
			unordered_map<t_CallId, t_MapElement*>::iterator iter = m_Map.find(callid);
			if (iter != m_Map.end()) return iter->second;
			
			return NULL;
		}

		C_MiaDriverBase::C_MiaDriverBase(const std::string &connectionString) :
				m_ConnectionString(connectionString), m_Websocket(0), m_State(g_INITIAL),
				m_AutoReconnect(false), m_ReconnectTimeout(1000), m_CurrentRetryTime(0), m_Private(0)
		{
			m_Private = DBG_NEW C_MiaDriverBasePrivate(this);
			m_Websocket = DBG_NEW C_WebsocketWrapper((C_WebsocketWrapper::C_WebsocketHandler*) this);
		}

		C_MiaDriverBase::~C_MiaDriverBase()
		{
			m_Private->M_Stop(true);

			if(m_Websocket)
			{
				m_Websocket->M_Disconnect();
				delete m_Websocket;
			}
			
			delete m_Private;
		}

		void C_MiaDriverBase::M_SetAutoReconnect(bool reconnect)
		{
			m_AutoReconnect = reconnect;
		}

		t_CallId C_MiaDriverBase::M_Call(const std::string &methodName, const std::string &parameters, F_Callback onResult)
		{
			Mia_THIS_ROUTINE("C_MiaDriverBase::M_Call");
			if (m_State == g_READY)
			{
				t_CallId callid = C_MiaDriverBase::M_S_GetCallId();

				C_WebsocketStream stream;
				stream.M_BeginArray();
				stream << callid;
				stream.M_NextDirective();
				stream << methodName;
				stream.M_NextDirective();
				stream << parameters;
				stream.M_EndArray();

				std::string message = stream.M_GetString();

				MIA_OUT_DEBUG_WEBSOCKET << "[C_MiaDriver::M_Call] Sending: " << message << std::endl;

				m_Private->M_RegisterCall(callid, onResult);
				if (m_Websocket->M_Send(message))
				{
					return callid;
				} else
				{
					C_Exception::M_S_HandleException(Mia_THIS_LOCATION, G_GetLastError(), methodName, parameters, (int64) (&onResult));
				}
			}
			std::stringstream ss;
			ss << "State = " << m_State;
			THROW(C_InvalidStateException(Mia_THIS_LOCATION, ss.str(), "State == g_READY", methodName, parameters, (int64) (&onResult)));
			NO_THROW(return -1;);
		}

		void C_MiaDriverBase::M_RegisterCall(t_CallId callid, F_Callback onResult, void* callbackData)
		{
			m_Private->M_RegisterCall(callid, onResult, g_CALL, callbackData);
		}

		bool C_MiaDriverBase::M_Connect(F_Callback *onResult)
		{
			Mia_THIS_ROUTINE("C_MiaDriverBase::M_Connect");
			m_State = g_CONNECTING;
			if (onResult)
			{
				m_Private->m_OnConnectionChanged = onResult;
				return m_Websocket->M_Connect(m_ConnectionString, m_Private);
			} else
			{
				int timeout = 0;
				m_Websocket->M_Connect(m_ConnectionString, m_Private);
				while (m_State == g_CONNECTING && timeout++ < C_MiaDriverBase::m_s_TimeOut)
				{
					ABB::Mia::C_Thread::M_S_Sleep(1000);
				}

				if (timeout >= C_MiaDriverBase::m_s_TimeOut)
				{
					m_State = g_DISCONNECTED;
					THROW(C_TimeoutException(Mia_THIS_LOCATION, (int64) ((void*)onResult)));
				}

				if (m_State == g_ERROR)
				{
					C_Exception::M_S_HandleException(Mia_THIS_LOCATION, G_GetLastError(), "***username***(missing)", "****password****(missing)", (int64) ((void*)onResult));
				}

				return m_State == g_READY;
			}
		}

		bool C_MiaDriverBase::M_Connect(const std::string &username, const std::string &password, F_Callback *onResult)
		{
			Mia_THIS_ROUTINE("C_MiaDriverBase::M_Connect");
			m_State = g_CONNECTING;
			if (onResult)
			{
				m_Private->m_OnConnectionChanged = onResult;
				bool ret = m_Websocket->M_Connect(m_ConnectionString, m_Private, username, password);
				return ret;
			} else
			{
				int timeout = 0;
				if(!m_Websocket->M_Connect(m_ConnectionString, m_Private, username, password))
				{
					m_State = g_ERROR;
				}

				while (m_State == g_CONNECTING && timeout++ < C_MiaDriverBase::m_s_TimeOut)
				{
					ABB::Mia::C_Thread::M_S_Sleep(1000);
				}

				if (timeout >= C_MiaDriverBase::m_s_TimeOut)
				{
					m_State = g_DISCONNECTED;
					THROW(C_TimeoutException(Mia_THIS_LOCATION, username, password, (int64) ((void*)onResult)));
				}

				if (m_State == g_ERROR)
				{
					C_Exception::M_S_HandleException(Mia_THIS_LOCATION, G_GetLastError(), username, "****password****", (int64) ((void*)onResult));
				}

				return m_State == g_READY;
			}
		}

		bool C_MiaDriverBase::M_Reconnect()
		{
			Mia_THIS_ROUTINE("C_MiaDriverBase::M_Reconnect");
			MIA_OUT_WARNING << sThisRoutine <<std::endl;
			if (m_State == g_DISCONNECTED)
			{
				m_State = g_DISCONNECTED;
				int timeout = 0;
				m_Websocket->M_Connect(m_ConnectionString, m_Private,	m_Private->m_Username, m_Private->m_Password);
				while (m_State == g_CONNECTING && timeout++ < C_MiaDriverBase::m_s_TimeOut)
				{
					ABB::Mia::C_Thread::M_S_Sleep(1000);
				}

				if (timeout >= C_MiaDriverBase::m_s_TimeOut)
				{
					m_State = g_DISCONNECTED;
					THROW(C_TimeoutException(Mia_THIS_LOCATION));
				}

				if (m_State == g_ERROR)
				{
					C_Exception::M_S_HandleException(Mia_THIS_LOCATION, G_GetLastError());
				}

				m_Private->M_OnReconnect();

				return m_State == g_READY;
			}
			std::stringstream ss;
			ss << "State == " << m_State;
			THROW(C_InvalidStateException(Mia_THIS_LOCATION, ss.str(), "State == g_DISCONNECTED"));
			NO_THROW(return false;);
		}

		bool C_MiaDriverBase::M_Send(const std::string &message)
		{
			Mia_THIS_ROUTINE("C_MiaDriverBase::M_Send");
			if (m_State != g_READY)
			{
				std::stringstream ss;
				ss << "State == " << m_State;
				THROW(C_InvalidStateException(Mia_THIS_LOCATION, ss.str(), "State == g_READY"));
			}
			TRY
			{
				return m_Websocket->M_Send(message);
			} 
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION);
				return false;
			})
		}
		bool C_MiaDriverBase::M_Send(const C_MemoryBuffer &message)
		{
			Mia_THIS_ROUTINE("C_MiaDriverBase::M_Send");
			if (m_State != g_READY)
			{
				std::stringstream ss;
				ss << "State == " << m_State;
				THROW(C_InvalidStateException(Mia_THIS_LOCATION, ss.str(), "State == g_READY"));
			}
			TRY
			{
				return m_Websocket->M_Send(message.M_Data(), message.M_GetSize());
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION);
				return false;
			})
		}
		bool C_MiaDriverBase::M_Send(const char *message, const uint length)
		{
			Mia_THIS_ROUTINE("C_MiaDriverBase::M_Send");
			#ifdef WEBSOCKET_DEBUG
				std::cout << "[" << sThisRoutine << "]: ";
				std::cout.write(message, length); // NOTE: this is done to also print messages that don't end with '\0' character (i.e. binaries)
				std::cout << std::endl;
			#endif
			if (m_State != g_READY)
			{
				std::stringstream ss;
				ss << "State == " << m_State;
				THROW(C_InvalidStateException(Mia_THIS_LOCATION, ss.str(), "State == g_READY"));
			}
			TRY
			{
				return m_Websocket->M_Send(message, length);
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION);
				return false;
			})
		}

		bool C_MiaDriverBase::M_Send(const t_Message &message)
		{
			Mia_THIS_ROUTINE("C_MiaDriverBase::M_Send");
			#ifdef WEBSOCKET_DEBUG
				std::cout <<"["<< sThisRoutine <<"]: " << message << std::endl;
			#endif
			if (m_State != g_READY)
			{
				std::stringstream ss;
				ss << "State == " << m_State;
				THROW(C_InvalidStateException(Mia_THIS_LOCATION, ss.str(), "State == g_READY"));
			}
			TRY
			{
				return m_Websocket->M_Send(message);
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION);
				return false;
			})
		}
		t_CallId C_MiaDriverBase::M_FetchClassData(const std::string &className,
				const std::string &properties, int fetchMax,
				const std::string &where, const std::string &whereData,
				F_Callback onResult)
		{
			Mia_THIS_ROUTINE("C_MiaDriverBase::M_FetchClassData");
			TRY
			{
				// building the call
				t_CallId callid;
				char *p;
				C_WebsocketStream stream;
				std::string parameters;

				stream.M_BeginArray();
				stream << className;

				stream.M_NextDirective();
				stream.M_BeginArray();
				// tokenizing the properties
				char *t = DBG_NEW char[properties.length() + 1];
				strcpy(t, properties.c_str());
				p = strtok(t, ", ");
				while (p)
				{
					std::string pp = p;
					stream << pp;
					p = strtok(NULL, ", ");
					if (p) stream.M_NextDirective();
				}
				delete t;
				stream.M_EndArray();

				stream.M_NextDirective();
				stream << fetchMax;

				stream.M_NextDirective();
				stream << where;

				stream.M_NextDirective();
				stream.M_BeginArray();
				stream << whereData;
				stream.M_EndArray();

				stream.M_EndArray();

				callid = M_Call("FetchClassData", parameters, onResult);
				return callid;
			} 
			CATCH(C_ExceptionBase &ex,
			{
				ex.M_AddTraceback(Mia_THIS_LOCATION, className, properties, fetchMax, where, whereData);
				THROW(;);
			})
		}

		t_CallId C_MiaDriverBase::M_CommitClassData(const std::string &className,
				const std::string &properties, const std::string &instanceId,
				const std::string &newValues, const std::string &oldValues,
				F_Callback onResult)
		{
			Mia_THIS_ROUTINE("C_MiaDriverBase::M_CommitClassData");
			TRY
			{
				C_WebsocketStream stream;
				std::string parameters;

				stream.M_BeginArray();
				stream << className;

				stream.M_NextDirective();
				stream.M_BeginArray();
				// tokenizing the properties
				char *t = DBG_NEW char[properties.length() + 1];
				strcpy(t, properties.c_str());
				char *p = strtok(t, ", ");
				while (p)
				{
					std::string pp = p;
					stream << pp;
					p = strtok(NULL, ", ");
					if (p) stream.M_NextDirective();
				}
				delete[] t;
				stream.M_EndArray();

				stream.M_NextDirective();
				stream << instanceId;

				stream.M_NextDirective();
				stream.M_BeginArray();
				stream << newValues;
				stream.M_EndArray();

				stream.M_NextDirective();
				stream.M_BeginArray();
				stream << oldValues;
				stream.M_EndArray();

				stream.M_EndArray();

				return M_Call("CommitClassData", parameters, onResult);
			}
			CATCH(C_ExceptionBase &ex,
			{
				ex.M_AddTraceback(Mia_THIS_LOCATION,className, properties, instanceId,newValues, oldValues);
				THROW(;);
			})
		}

		void C_MiaDriverBase::M_AddOnReconnect(C_IReconnectHandler* handler)
		{
			C_Locker locker(&m_Private->m_ReconnectLock);
			m_Private->m_ReconnectHandler.insert(handler);
		}

		void C_MiaDriverBase::M_RemoveOnReconnect(C_IReconnectHandler* handler)
		{
			C_Locker locker(&m_Private->m_ReconnectLock);
			m_Private->m_ReconnectHandler.erase(handler);
		}

		t_CallId C_MiaDriverBase::M_S_GetCallId()
		{
			return s_CallId++;
		}

		int32 C_MiaDriverBase::M_BufferCnt()
		{
			return  m_Websocket->M_BufferCnt();
		}

	} /* namespace Mia */
} /* namespace ABB */
