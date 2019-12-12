/*
 * Mia_MiaDriver.cpp
 *
 *  Created on: Jun 12, 2014
 *      Author: Tuan Vu
 */

#include <sstream>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <climits>
#include <algorithm>
#include <ctype.h>

extern "C"
{
	#include <WS_Error.h>
	#include <WS_Websocket.h>
}

#include "Mia_Driver.h"
#include "Mia_Base.h"
#include "Mia_Device.h"
#include "Mia_Equipment.h"
#include "build_configuration.h"    //zwj

namespace ABB
{
	namespace Mia
	{

#ifdef ENABLE_EXCEPTION
#define G_SET_PROPERTY(propertyName, value) \
	if (!m_Updating) throw C_InvalidStateException(Mia_THIS_LOCATION, "m_Updating = true", "m_Updating = false", propertyName); \
	C_Variant* v = M_GetProperty(propertyName); \
	if (v) \
	{ \
		C_Locker locker(&m_ModifiedLock);\
		C_Variant* mv = M_GetModifiedProperty(propertyName); \
		if (mv) mv->M_SetValue(value); \
		else \
		{ \
			std::string pn = G_StringToLower(propertyName); \
			mv = DBG_NEW C_Variant(value); \
			m_ModifiedPropertyList[pn] = C_SharedPointer<C_Variant> (mv); \
		} \
	} else \
	{ \
		THROW(C_PropertyNotFoundException(Mia_THIS_LOCATION, propertyName)); \
	}

#define Mia_GET_PROPERTY(propertyName, value) \
	C_Variant* v = M_GetProperty(propertyName); \
	if (v) \
	{ \
		if (m_Private->m_Updating)\
		{\
			C_Locker locker(&m_Private->m_ModifiedLock); \
			C_Variant* mv = M_GetModifiedProperty(propertyName); \
			if (mv && mv->M_IsValid()) { mv->M_GetValue(value);  return; } \
		}\
		if (v->M_IsValid()) v->M_GetValue(value);\
	} else \
	{ \
		THROW(C_PropertyNotFoundException(Mia_THIS_LOCATION, propertyName)); \
	}

#else
#define G_SET_PROPERTY(propertyName, value) \
	if (m_Updating) \
	{ \
		C_Variant* v = M_GetProperty(propertyName); \
		if (v) \
		{ \
			C_Locker locker(&m_ModifiedLock);\
			C_Variant* mv = M_GetModifiedProperty(propertyName); \
			if (mv) mv->M_SetValue(value); \
			else \
			{ \
				std::string pn = G_StringToLower(propertyName); \
				mv = DBG_NEW C_Variant(value); \
				m_ModifiedPropertyList[pn] = C_SharedPointer<C_Variant> (mv); \
			} \
		} \
	}

#define Mia_GET_PROPERTY(propertyName, value) \
	C_Variant* v = M_GetProperty(propertyName); \
	if (v) \
	{ \
		C_Locker locker(&m_Private->m_ModifiedLock);\
		C_Variant* mv = M_GetModifiedProperty(propertyName); \
		if (mv && mv->M_IsValid()) mv->M_GetValue(value); \
		if (v->M_IsValid()) v->M_GetValue(value);\
	} else \
	{ \
		THROW(C_PropertyNotFoundException(Mia_THIS_LOCATION, propertyName)); \
	}
#endif

#ifndef G_SET_EQUIPMENT_PROPERTY
#define G_SET_EQUIPMENT_PROPERTY \
	if (this->m_PropertyInfo->M_IsHistorized()) \
	{ \
		C_PropertyValuePusher pusher = M_GetPusher(); \
		m_CurrentValue = value; \
		m_LastUpdateTime = C_DateTime::M_S_Now(); \
		if (time && status != g_VS_OK) pusher->M_Push(value, *time,	status); \
		else if (time) pusher->M_Push(value, *time); \
		else pusher->M_Push(value); \
	} else \
	{ \
		m_CurrentValue = value; \
		m_LastUpdateTime = C_DateTime::M_S_Now(); \
		M_Save(); \
	}
#endif

		int ABB::Mia::C_MiaDriverBase::m_s_TimeOut = 60;
		int ABB::Mia::C_MiaDriverBase::m_MaxRetryTime = 60000;

		class C_DbPropertyInfoClass::C_DbPropertyInfoPrivate
		{
			private: C_DbPropertyInfoPrivate(C_DbClass parent, const std::string &propname,
				const std::string & category, const std::string & displayname, const std::string & defaultval,
				const std::string & description, const std::string & index, const std::string & size, const std::string & type,
				const std::string & rawtype, bool nullable, bool numeric, bool interned, bool readonly, bool unique,
				bool isvirtual, bool visibletouser, bool maskrequired, const std::string & referencetarget) :

					m_Class(parent), m_PropertyName(propname), m_Category(category), m_DisplayName(displayname), m_Description(
					description), m_Size(0), m_RawType(type), m_IsNullable(nullable), m_IsNumeric(numeric), m_IsInterned(interned), m_IsReadonly(
					readonly), m_IsUnique(unique), m_IsVirtual(isvirtual), m_IsVisibleToUser(visibletouser), m_MaskRequired(maskrequired),
					m_ReferenceTarget(referencetarget)
			{
				m_Index = atoi(index.c_str());
				m_Size = atoi(size.c_str());
			};

			private: C_DbClass m_Class;
			private:	std::string m_PropertyName;
			private:	std::string m_Category;
			private:	std::string m_DisplayName;
			private:	std::string m_DefaultValue;
			private:	std::string m_Description;
			private:	uint32 m_Index;
			private:	uint32 m_Size;
			private:	t_DataType m_Type;
			private:	std::string m_TypeName;
			private:	std::string m_RawType;
			private:	bool m_IsNullable;
			private:	bool m_IsNumeric;
			private:	bool m_IsInterned;
			private:	bool m_IsReadonly;
			private:	bool m_IsUnique;
			private:	bool m_IsVirtual;
			private:	bool m_IsVisibleToUser;
			private:	bool m_MaskRequired;
			private:	std::string m_ReferenceTarget;

			friend class C_MiaDriver;
			friend class C_DbClassClass;
			friend class C_DbPropertyInfoClass;
		};


		/* C_MiaDriver */
		class C_MiaDriver::C_MiaDriverPrivate
		{
			~C_MiaDriverPrivate()
			{
				C_Locker locker(&m_ClassLock);
				//m_Classes.clear();
				while (m_Classes.size())
				{
					C_DbClass c = m_Classes.begin()->second;
					c->M_ClearProperties();
					int count = c.use_count();
					MIA_OUT_DEBUG << "[" << c->M_GetClassName() << "] " << count  << std::endl;
					m_Classes.begin()->second.reset();
					m_Classes.erase(m_Classes.begin());
				}
			}
			public: struct t_Element
			{
					C_Event	*m_Event;
					t_CallId	m_CallId;
					char		*m_Message;
					int		m_Length;
			};

			public: static void M_S_OnEventCallback(C_MiaDriver* driver, t_CallId callId, void *result)
			{
				t_Element *e = driver->m_BlockingPrivate->M_GetElement(callId);
				e->m_Message = DBG_NEW char[strlen((char*) result) + 1];
				strcpy(e->m_Message, (char*) result);
				e->m_Event->M_Set();
			}

			public: static void M_S_OnSubscriptionCallback(void* data, t_CallId callId, void *result)
			{
				// Find the driver and device and send the update
				C_MiaDriver* driver = (C_MiaDriver*)data;
				C_Device* device = driver->m_BlockingPrivate->M_GetSubscription(callId);
				if (device)
				{
					device->M_OnDeviceUpdate(result);
				}
			}

			public: t_Element* M_GetElement(t_CallId callId)
			{
				C_Locker locker(&m_MapLock);
				t_Element *e = m_Map[callId];
				m_Map[callId] = 0;
				return e;
			}

			public: t_Element* M_PutElement(t_CallId callId)
			{
				t_Element *e = DBG_NEW t_Element();
				e->m_CallId = callId;
				e->m_Event = DBG_NEW C_Event();
				C_Locker locker(&m_MapLock);
				m_Map[callId] = e;
				return e;
			}

			public: C_Device* M_GetSubscription(t_CallId callId)
			{
				C_Locker locker(&m_SubcriptionLock);
				C_Device *e = m_Subscriptions[callId];
				return e;
			}

			public: C_Device* M_PutSubscription(C_Device* device, t_CallId callId)
			{
				C_Locker locker(&m_SubcriptionLock);
				m_Subscriptions[callId] = device;
				return device;
			}

			public: C_Device* M_PopSubscription(t_CallId callId)
			{
				C_Locker locker(&m_SubcriptionLock);
				C_Device *e = m_Subscriptions[callId];
				m_Subscriptions[callId] = 0;
				return e;
			}

			public:	C_MiaDriverPrivate(C_MiaDriver *driver) :	m_Driver(driver) {}

			private:	unordered_map<t_CallId, t_Element*>		m_Map;
			private:	unordered_map<t_CallId, C_Device*>		m_Subscriptions;
			private:	unordered_map<std::string, C_DbClass>	m_Classes;
			private:	C_ThreadLock	m_MapLock;
			private:	C_ThreadLock	m_SubcriptionLock;
			private:	C_ThreadLock	m_ClassLock;
			private:	C_MiaDriver		*m_Driver;

			friend class C_MiaDriver;
		};

		C_MiaDriver::C_MiaDriver(const std::string &connectionString) :
			C_MiaDriverBase(connectionString), m_BlockingPrivate(0)
		{
			MIA_OUT_INFO<<"*******************************************************"<<std::endl;
			MIA_OUT_INFO<<"*******************************************************"<<std::endl;
			MIA_OUT_INFO<<"********** CPMPLUS EMBEDDED VERSION 2.0 ***************"<<std::endl;
			MIA_OUT_INFO<<"*******************************************************"<<std::endl;
			m_BlockingPrivate = DBG_NEW C_MiaDriver::C_MiaDriverPrivate(this);
		}

		C_MiaDriver::~C_MiaDriver()
		{
			delete m_BlockingPrivate;
		}

		C_DbClass C_MiaDriver::M_GetClass(const std::string &className)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_GetClass");
			C_Locker locker(&m_BlockingPrivate->m_ClassLock);
			unordered_map<std::string, C_DbClass>::iterator iter = m_BlockingPrivate->m_Classes.find(className);
			if (iter != m_BlockingPrivate->m_Classes.end())
			{
				return iter->second;
			} else
			{
				TRY
				{
					// Fetch class data
					std::string classinfo = M_FetchClassData("Class", C_DbClassClass::m_k_s_ClassProperties, -1, "Id = ? ", className);
					if (classinfo.size())
					{
						C_Variant json(classinfo, g_ARRAY);
						MIA_OUT_DEBUG<<json<<std::endl;

						if (json.M_IsValid() && json.M_GetArraySize()>4)
						{
							C_DbClassClass* cl = new C_DbClassClass(this, className, json[1].M_ToString(), json[2].M_ToBool(), json[3].M_ToString(), json[4].M_ToString());
							C_DbClass c = C_SharedPointer<C_DbClassClass>(cl);
							m_BlockingPrivate->m_Classes[className] = c;
							c->M_FetchAndUpdateProperties(&c);
							return c;
						}
					}
					THROW(C_ClassNotFoundException(Mia_THIS_LOCATION, className));
				} 
				CATCH(C_ExceptionBase &e,
				{
					e.M_RethrowTraceback(Mia_THIS_LOCATION, className);
				})
			}

			return C_DbClass();
		}

		C_DbClass C_MiaDriver::operator[](const std::string &className)
		{
			return M_GetClass(className);
		}

		bool C_MiaDriver::M_Connect(const std::string &username, const std::string &password)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_Connect");
			TRY
			{
				return C_MiaDriverBase::M_Connect(username, password);
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION);
			})
		}

		C_MiaDriverBase::t_State C_MiaDriver::M_GetStatus()
		{
			return m_State;
		}

		bool C_MiaDriver::M_IsReady()
		{
			return m_State == g_READY;
		}

		std::string C_MiaDriver::M_Call(const std::string &methodName, const std::string &parameters, t_CallId *callId, C_Variant* retJson)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_Call");
			if (m_State == g_READY)
			{
				t_CallId callid = C_MiaDriver::M_S_GetCallId();
				if (callId) *callId = callid;

				C_WebsocketStream stream;
				stream.M_BeginArray();
				stream << callid;
				stream.M_NextDirective();
				stream << methodName;
				stream.M_NextDirective();
				stream.M_AppendString(parameters);
				stream.M_EndArray();
				std::string message = stream.M_GetString();

				MIA_OUT_DEBUG_WEBSOCKET << "[" << sThisRoutine << "]: " << message << std::endl;

				C_AutoPointer<C_MiaDriverBase::t_MapElement> e = M_RegisterEventCall(callid);

				TRY
				{
					M_Send(message);
					e->m_Event->M_Wait(m_s_TimeOut * 1000);

					M_RemoveEventCall(callid);
				}
					CATCH(C_Exception &ex,
					{
						M_RemoveEventCall(callid);
						ex.M_RethrowTraceback(Mia_THIS_LOCATION, methodName, parameters);
					})

					if (e->m_CallId < 0)
					{

						MIA_OUT_DEBUG_WEBSOCKET << "[" << sThisRoutine << "]: " << " Failed to call " << methodName << " " << std::endl;

						std::string data = "";
						if (e->m_Message) data = std::string(e->m_Message, e->m_Length);
						THROW(C_ServerException(Mia_THIS_LOCATION, data, methodName, parameters));
					}

					if (!e->m_Message) return "";
					std::string data(e->m_Message, e->m_Length);
					if (retJson)
					{
						if (retJson->M_FromString(data, g_ARRAY))	return "";

						std::string emessage = retJson->M_ToString();
						THROW(C_ServerException(Mia_THIS_LOCATION, emessage, methodName, parameters));
					}
					else
					{
						C_Variant json(data, g_ARRAY);
						if (json.M_IsValid() && json.M_GetArraySize() > 1)
						{
							C_Variant retjson;
							json.M_GetValue(1, retjson);

							MIA_OUT_DEBUG_WEBSOCKET << "[" << sThisRoutine << "]: " << "Received \"" << retjson << "\"" << std::endl;

							if (methodName == "SubscribeEquipmentSession" || 
								methodName == "GetServerTimeInUTCFromServer")
							{
								return retjson.M_ToString();
							}
							else
							{
								if (retjson.M_GetType() == g_OBJECT)
								{
									C_Variant ret = retjson["Data"];
									if (ret.M_IsValid())
									{
										return ret.M_ToString();
									}
								}
								else if (retjson.M_GetType() == g_ARRAY)
								{
									C_Variant ret;
									retjson.M_GetValue(0, ret);
									if (ret.M_GetType() == g_NULL) return "";
									if (ret.M_GetType() == g_ARRAY || ret.M_GetType() == g_STRING)
									{
										return ret.M_ToString();
									}
								}

								std::string emessage = json.M_ToString();
								THROW(C_ServerException(Mia_THIS_LOCATION, emessage, methodName, parameters));
							}
						}
						else
						{
							return "";
						}
					}
				}

			MIA_OUT_DEBUG_WEBSOCKET << "[" << sThisRoutine << "]: " << "MiaDriver in invalid state: " << m_State << " expected " << g_READY << std::endl;

			THROW(C_InvalidStateException(Mia_THIS_LOCATION, "State != g_READY", "State == g_READY", methodName, parameters));
			NO_THROW(return "";);
		}

		bool C_MiaDriver::M_Subscribe(const std::string &methodName, const std::string &parameters, C_Variant *result, F_Callback callback, t_CallId* callId)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_Subscribe");
			if (m_State == g_READY)
			{
				t_CallId callid = C_MiaDriver::M_S_GetCallId();
				if (callId) *callId = callid;

				C_WebsocketStream stream;
				stream.M_BeginArray();
				stream << callid;
				stream.M_NextDirective();
				stream << methodName;
				stream.M_NextDirective();
				stream.M_AppendString(parameters);
				stream.M_EndArray();
				std::string message = stream.M_GetString();

				MIA_OUT_DEBUG_WEBSOCKET << "[" << sThisRoutine << "]: " << message << std::endl;

				C_MiaDriverBase::t_MapElement* e = M_RegisterEventCall(callid);

				TRY
				{
					M_Send(message);
					e->m_Event->M_Wait(m_s_TimeOut * 1000);
				}
				CATCH(C_Exception &ex,
				{
					M_RemoveEventCall(callid);
					ex.M_RethrowTraceback(Mia_THIS_LOCATION, methodName, parameters);
				})

				if (e->m_CallId < 0)
				{
					MIA_OUT_DEBUG_WEBSOCKET << "[" << sThisRoutine << "]: " << " Failed to call " << methodName << " " << std::endl;

					std::string data = "";
					if (e->m_Message) data = std::string(e->m_Message, e->m_Length);
					delete[] e->m_Message;
					e->m_Message = 0;
					M_RemoveEventCall(callid);
					THROW(C_ServerException(Mia_THIS_LOCATION, data, methodName, parameters));
				}

				if (!e->m_Message) return "";
				std::string data(e->m_Message, e->m_Length);
				delete[] e->m_Message;
				e->m_Message = 0;
				*result = C_Variant(data, g_ARRAY);
				if (result->M_IsValid() && result->M_GetArraySize() >= 1)
				{
					e->m_OnResult = callback;
					return true;
				}
				else
				{
					M_RemoveEventCall(callid);
					return false;
				}
			}

			MIA_OUT_DEBUG_WEBSOCKET << "[" << sThisRoutine << "]: " << "MiaDriver in invalid state: " << m_State << " expected " << g_READY << std::endl;

			THROW(C_InvalidStateException(Mia_THIS_LOCATION, "State != g_READY", "State == g_READY", methodName, parameters));
			NO_THROW(return "";);
		}

		bool C_MiaDriver::M_UnSubscribe(int64 callId)
		{
			return M_RemoveEventCall(callId);
		}

		C_DateTime Mia::C_MiaDriver::M_GetServerTimeUtc()
		{
			C_DateTime now = C_DateTime::M_S_Now();

			Mia_THIS_ROUTINE("C_MiaDriver::M_GetServerTimeUtc");
			
			// building the call
			TRY
			{
				C_WebsocketStream stream;
				std::string parameters;

				stream.M_BeginArray();
				stream.M_EndArray();
				parameters = stream.M_GetString();

				C_Variant json;
				M_Call("GetServerTimeInUTCFromServer", parameters, 0, &json);

				int64 time = G_MIA_STRTOLL(json[1].M_ToString().c_str());
				int64 deltatick = (C_DateTime::M_S_Now().M_GetTicks() - now.M_GetTicks())/2;
				return C_DateTime::M_S_FromTicks(time + deltatick);
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION);
			})
		}

		std::string C_MiaDriver::M_FetchClassData(const std::string &className, const std::string &properties,
		   int fetchMax, const std::string &where, const C_Variant& data1, const C_Variant& data2, const C_Variant& data3,
		   const C_Variant& data4, const C_Variant& data5, const C_Variant& data6, const C_Variant& data7,
		   const C_Variant& data8)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_FetchClassData");
			// building the call

			TRY
			{
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
					p = strtok(NULL, ",");
					while (p && p[0] == ' ') p++;
					if (p) stream.M_NextDirective();
				}
				delete[] t;
				stream.M_EndArray();

				stream.M_NextDirective();
				stream << fetchMax;

				stream.M_NextDirective();
				stream << where;

				stream.M_NextDirective();
				stream.M_BeginArray();
				if (data1.M_IsValid()) stream << data1;
				if (data2.M_IsValid())
				{
					stream.M_NextDirective();
					stream << data2;
				}
				if (data3.M_IsValid())
				{
					stream.M_NextDirective();
					stream << data3;
				}
				if (data4.M_IsValid())
				{
					stream.M_NextDirective();
					stream << data4;
				}
				if (data5.M_IsValid())
				{
					stream.M_NextDirective();
					stream << data5;
				}
				if (data6.M_IsValid())
				{
					stream.M_NextDirective();
					stream << data6;
				}
				if (data7.M_IsValid())
				{
					stream.M_NextDirective();
					stream << data7;
				}
				if (data8.M_IsValid())
				{
					stream.M_NextDirective();
					stream << data8;
				}
				stream.M_EndArray();

				stream.M_EndArray();

				parameters = stream.M_GetString();

				return M_Call("FetchClassData", parameters);
			} 
			CATCH(C_Exception &ex, 
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, className, properties, fetchMax, where, data1, data2, data3, data4);
			}

			return "";
			)
		}

		void C_MiaDriver::M_SubscribeEquipmentSession(const C_Variant &value, C_Variant *returnValue)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_SubscribeEquipmentSession");
			std::string result;

			TRY
			{
				M_SubscribeEquipmentSession(value.M_ToString(), &result);
			} 
			CATCH(C_Exception &e,
			{
				e.M_RethrowTraceback(Mia_THIS_LOCATION, value.M_ToString(), *returnValue);
			})

			*returnValue = C_Variant::M_S_FromJSON(result);
			if (!returnValue->M_IsValid())
			{				
				THROW(C_ServerException(Mia_THIS_LOCATION, result, value.M_ToString(), *returnValue));
			}
		}

		void C_MiaDriver::M_SubscribeEquipmentSession(const std::string &value, std::string *returnValue)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_SubscribeEquipmentSession");
			*returnValue = "";
			TRY
			{
				*returnValue = M_Call("SubscribeEquipmentSession", "[" + value + "]");
			} 
			CATCH(C_Exception &e,
			{
				e.M_RethrowTraceback(Mia_THIS_LOCATION, value, *returnValue);
			})
		}

		/**
		* @brief Retrieve JSON array of element in C_Variant type
		* @param in is JSON string
		* @param element is JSON element name
		*
		* @return Array of values
		*
		*/
		C_Variant C_MiaDriver::M_GetJsonArray (const std::string &in, const std::string &element)
		{
			std::string search_element = std::string("\"") + element + std::string("\""); // set " around element

			int index_start = (int)in.find(search_element); // Search element
			if(index_start < 0) 
				return C_Variant(); // Not found

			index_start += (int)search_element.length() + 1; // Array starts after ":"

			int index_end = (int)in.find("]", index_start); // Array ends to first ]. Hopefully array data does not have any ].
			if (index_end < 0)  return C_Variant(); // No end found

			return C_Variant::M_S_FromJSON(in.substr(index_start, index_end - index_start + 1));
		}

		bool C_MiaDriver::M_SubscribeEquipmentSession(C_Device *device)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_SubscribeEquipmentSession");
			std::string dj = device->M_ToJson();
			device->M_SetDriver(this);
			device->m_IsSubscriped = false;
			C_Variant out;
			t_CallId callid;
			TRY
			{
				if (M_Subscribe("SubscribeEquipmentSession", "[" + dj + "]", &out, C_MiaDriverPrivate::M_S_OnSubscriptionCallback, &callid))
				{
					if (!out.M_IsValid())
					{
						MIA_OUT_DEBUG_WEBSOCKET << sThisRoutine << ": Subscription failed " << out << std::endl;
						if (callid > 0) M_RemoveEventCall(callid);
						return false;
					}
				}

				M_ParseDeviceSubscription(device, &out);

				m_BlockingPrivate->M_PutSubscription(device, callid);

				MIA_OUT_DEBUG_WEBSOCKET <<C_DateTime::M_S_Now()<<"[C_MiaDriver::M_SubscribeEquipmentSession]: " << out << std::endl;
			} 
			CATCH(C_Exception &ex,
			{
				if (callid > 0) M_RemoveEventCall(callid);
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, device->M_GetEquipmentName());
			})	


			#ifndef NDEBUG
				#define EXCEPTION_OUT  MIA_OUT_WARNING << "Exception: " << ex.what() << std::endl;
			#else
				#define EXCEPTION_OUT
			#endif
			CATCH(std::exception &ex,
			{
				EXCEPTION_OUT
				throw;
			})

			device->m_IsSubscriped = true;
			return true;
		}

		void C_MiaDriver::M_ParseDeviceSubscription(C_Device* device, C_Variant* out)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_ParseDeviceSubscription");
			TRY
			{
				if ((*out).M_GetType() == g_ARRAY && out->M_GetArraySize() && (*out)[0].M_GetType() == g_ARRAY)
				{
					const C_Variant& arrjson = (*out);
					const C_Variant& retjson = (*out)[arrjson.M_GetArraySize()-1][1];
					device->M_OnDeviceUpdate(retjson);
				}
				else
				{
					const C_Variant& retjson = (*out)[1];
					device->M_OnDeviceUpdate(retjson);
				}
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, device->M_GetEquipmentName());
			})
		}

		std::string C_MiaDriver::M_CommitClassData(const std::string &className, const std::string &properties,
		   const std::string &instanceId, const C_Variant& data1, const C_Variant& data2, const C_Variant& data3,
		   const C_Variant& data4, const C_Variant& data5, const C_Variant& data6, const C_Variant& data7,
		   const C_Variant& data8)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_FetchClassData");
			// building the call
			TRY
			{
				char *p;
				C_WebsocketStream stream;

				stream.M_BeginArray();
				stream << className;

				stream.M_NextDirective();
				stream.M_BeginArray();
				int propertycount = 0;

				// tokenizing the properties
				char *t = DBG_NEW char[properties.length() + 1];
				strcpy(t, properties.c_str());
				p = strtok(t, ",");
				while (p)
				{
					propertycount++;
					std::string pp = p;
					stream << pp;
					p = strtok(NULL, ",");
					if (p) stream.M_NextDirective();
				}
				delete[] t;
				stream.M_EndArray();

				stream.M_NextDirective();
				stream.M_BeginArray();
				if (instanceId.size())
				{
					stream << instanceId;
				} else
				{
					stream.M_AppendNull();
				}

				stream.M_EndArray();

				stream.M_NextDirective();

				stream.M_BeginArray();
				stream.M_BeginArray();
				// new values
				if (data1.M_IsValid())
				{
					stream << data1;
				}
				if (data2.M_IsValid() && propertycount > 1)
				{
					stream.M_NextDirective();
					stream << data2;
				}
				if (data3.M_IsValid() && propertycount > 2)
				{
					stream.M_NextDirective();
					stream << data3;
				}
				if (data4.M_IsValid() && propertycount > 3)
				{
					stream.M_NextDirective();
					stream << data4;
				}

				if (propertycount > 4)
				{
					if (data5.M_IsValid())
					{
						stream.M_NextDirective();
						stream << data5;
					}
					if (data6.M_IsValid())
					{
						stream.M_NextDirective();
						stream << data6;
					}
					if (data7.M_IsValid())
					{
						stream.M_NextDirective();
						stream << data7;
					}
					if (data8.M_IsValid())
					{
						stream.M_NextDirective();
						stream << data8;
					}
				}

				stream.M_EndArray();
				stream.M_EndArray();

				stream.M_NextDirective();
				stream.M_BeginArray();
				stream.M_BeginArray();

				if (propertycount > 4)
				{
					stream.M_AppendNull();
				} else
				{
					if (propertycount == 1)
					{
						if (data2.M_IsValid())
						{
							stream << data2;
						} else stream.M_AppendNull();
					} else if (propertycount == 2)
					{
						if (data3.M_IsValid())
						{
							stream << data3;
						} else stream.M_AppendNull();
						if (data4.M_IsValid())
						{
							stream.M_NextDirective();
							stream << data4;
						} else
						{
							stream.M_NextDirective();
							stream.M_AppendNull();
						}
					} else if (propertycount == 3)
					{
						if (data4.M_IsValid())
						{
							stream << data4;
						} else stream.M_AppendNull();
						if (data5.M_IsValid())
						{
							stream.M_NextDirective();
							stream << data5;
						} else
						{
							stream.M_NextDirective();
							stream.M_AppendNull();
						}
						if (data6.M_IsValid())
						{
							stream.M_NextDirective();
							stream << data6;
						} else
						{
							stream.M_NextDirective();
							stream.M_AppendNull();
						}
					} else if (propertycount == 4)
					{
						if (data5.M_IsValid())
						{
							stream << data5;
						} else stream.M_AppendNull();
						if (data6.M_IsValid())
						{
							stream.M_NextDirective();
							stream << data6;
						} else
						{
							stream.M_NextDirective();
							stream.M_AppendNull();
						}
						if (data7.M_IsValid())
						{
							stream.M_NextDirective();
							stream << data7;
						} else
						{
							stream.M_NextDirective();
							stream.M_AppendNull();
						}
						if (data8.M_IsValid())
						{
							stream.M_NextDirective();
							stream << data8;
						} else
						{
							stream.M_NextDirective();
							stream.M_AppendNull();
						}
					} else
					{
						stream.M_AppendNull();
					}
				}

				stream.M_EndArray();
				stream.M_EndArray();

				stream.M_EndArray();

				return M_Call("CommitClassData", stream.M_GetString());
			}
			CATCH(C_Exception &ex, 
				{
					ex.M_RethrowTraceback(Mia_THIS_LOCATION, className, properties, instanceId, data1, data2, data3, data4);
				}
				return "";
			)
		}

		std::string C_MiaDriver::M_CommitClassDataRaw(const std::string &className, const std::string &properties,
		   const std::string &instanceId, const std::string &newValues, const std::string &oldValues)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_CommitClassDataRaw");
			// building the call
			TRY
			{
				char *p;
				C_WebsocketStream stream;

				stream.M_BeginArray();
				stream << className;

				stream.M_NextDirective();
				stream.M_BeginArray();
				int propertycount = 0;
				// tokenizing the properties
				char *t = DBG_NEW char[properties.length() + 1];
				strcpy(t, properties.c_str());
				p = strtok(t, ",");
				while (p)
				{
					propertycount++;
					std::string pp = p;
					stream << pp;
					p = strtok(NULL, ",");
					if (p) stream.M_NextDirective();
				}
				delete[] t;
				stream.M_EndArray();

				stream.M_NextDirective();
				stream.M_BeginArray();
				if (instanceId.size())
				{
					stream << instanceId;
				} else
				{
					stream.M_AppendNull();
				}
				stream.M_EndArray();

				stream.M_NextDirective();

				stream.M_BeginArray();

				stream.M_BeginArray();
				stream.M_AppendRaw(newValues);
				stream.M_EndArray();
				stream.M_EndArray();

				stream.M_NextDirective();
				stream.M_BeginArray();

				if (oldValues.size())
				{
					stream.M_BeginArray();
					stream.M_AppendRaw(oldValues);
					stream.M_EndArray();
				} else stream.M_AppendNull();

				stream.M_EndArray();

				stream.M_EndArray();

				return M_Call("CommitClassData", stream.M_GetString());
			} 
			CATCH(C_Exception &ex,
				{
					ex.M_RethrowTraceback(Mia_THIS_LOCATION, className, properties, instanceId, newValues, oldValues);
				}
				return "";
			)
		}
		std::string C_MiaDriver::M_CommitClassDataComposite(const std::string &className, const std::string &properties,
			const C_Variant& compositeId, const std::string &newValues, const std::string &oldValues)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_CommitClassDataRaw");
			// building the call
			TRY
			{
				char *p;
				C_WebsocketStream stream;

				stream.M_BeginArray();
				stream << className;

				stream.M_NextDirective();
				stream.M_BeginArray();
				int propertycount = 0;
				// tokenizing the properties
				char *t = DBG_NEW char[properties.length() + 1];
				strcpy(t, properties.c_str());
				p = strtok(t, ",");
				while (p)
				{
					propertycount++;
					std::string pp = p;
					stream << pp;
					p = strtok(NULL, ",");
					if (p) stream.M_NextDirective();
				}
				delete[] t;
				stream.M_EndArray();

				stream.M_NextDirective();
				stream.M_BeginArray();
				if (compositeId.M_IsValid())
				{
				//stream.M_A
					stream.M_AppendRaw(compositeId.M_ToString());
				}
				else
				{
					stream.M_AppendNull();
				}
				stream.M_EndArray();

				stream.M_NextDirective();

				stream.M_BeginArray();

				stream.M_BeginArray();
				stream.M_AppendRaw(newValues);
				stream.M_EndArray();
				stream.M_EndArray();

				stream.M_NextDirective();
				stream.M_BeginArray();

				if (oldValues.size())
				{
					stream.M_BeginArray();
					stream.M_AppendRaw(oldValues);
					stream.M_EndArray();
				}
				else stream.M_AppendNull();

				stream.M_EndArray();

				stream.M_EndArray();

				return M_Call("CommitClassData", stream.M_GetString());
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, className, properties, compositeId, newValues, oldValues);
			}
			return "";
			)
		}
		C_Equipment C_MiaDriver::M_GetEquipment(const std::string &equipmentName)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_GetEquipment");
			TRY
			{
				C_DbClass eclass = this->M_GetClass("Equipment");
				C_DbClass cpclass = this->M_GetClass("EquipmentPropertyInfo");
				if (eclass && cpclass)
				{
					C_DbClassInstances instances;
					if (eclass->M_GetInstanceSet(&instances, 1, "Name = ?", equipmentName))
					{
						C_EquipmentClass* ec = DBG_NEW C_EquipmentClass(instances.front());
						C_Equipment e(ec);
						e->M_UpdateProperties(e);
						return e;
					}
				}
				return C_Equipment();
			} 
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, equipmentName);
				throw;
			})
		}

		bool C_MiaDriver::M_GetEquipment(C_Equipments *equipments, const std::string &filter,
		   const std::string &filterData, int fetchMax)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_GetEquipment");
			TRY
			{
				C_DbClass eclass = this->M_GetClass("Equipment");
				C_DbClass cpclass = this->M_GetClass("EquipmentPropertyInfo");
				if (eclass && cpclass)
				{
					C_DbClassInstances instances;
					if (eclass->M_GetInstanceSet(&instances, fetchMax, filter, filterData))
					{
						MIA_OUT_DEBUG << sThisRoutine << ": Found " << instances.size() << std::endl;

						for (C_DbClassInstances::iterator iter = instances.begin(); iter != instances.end(); iter++)
						{
							C_EquipmentClass* ec = DBG_NEW C_EquipmentClass(*iter);
							C_Equipment e(ec);
							e->M_UpdateProperties(e);
							equipments->push_back(e);
						}
						return instances.size() > 0;
					}
					else
					{
						MIA_OUT_DEBUG << sThisRoutine << ": Fetching instances failed " << std::endl;
					}
				}
				return false;
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, filter);
				throw;
			})
		}

		bool C_MiaDriver::M_GetEquipment(C_Equipments *equipments, const std::string &filter,
		   const C_ArgumentList &filterData, int fetchMax)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_GetEquipment");
			TRY
			{
				C_DbClass eclass = this->M_GetClass("Equipment");
				C_DbClass cpclass = this->M_GetClass("EquipmentPropertyInfo");
				if (eclass && cpclass)
				{
					C_DbClassInstances instances;
					if (eclass->M_GetInstanceSet(&instances, fetchMax, filter, filterData))
					{
						for (C_DbClassInstances::iterator iter = instances.begin(); iter != instances.end(); iter++)
						{
							C_EquipmentClass* ec = DBG_NEW C_EquipmentClass(*iter);
							C_Equipment e(ec);
							e->M_UpdateProperties(e);
							equipments->push_back(e);
						}
						return instances.size() > 0;
					}
				}
				return false;
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, filter);
				throw;
			})
		}

		t_CallId C_MiaDriver::M_CreatePusher(const std::string &className, const std::string &instanceId,
		   const std::string &properties, const int &operationMode)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_CreatePusher");
			TRY
			{
				t_CallId pusherid = C_MiaDriver::M_S_GetCallId();

				C_WebsocketStream stream;
				stream.M_BeginArray();
				stream << pusherid;
				stream.M_NextDirective();
				stream << "SubscribePushReceiver";
				stream.M_NextDirective();
				// Id

				// parameters

				stream.M_BeginArray();
				stream << className;
				stream.M_NextDirective();
				if (instanceId.size() == 0)
				{
					stream.M_AppendNull();
				} else
				{
					stream << instanceId;
				}

				stream.M_NextDirective();
				stream.M_BeginArray();
				// tokenizing the properties
				char *t = DBG_NEW char[properties.length() + 1], *p;
				strcpy(t, properties.c_str());
				p = strtok(t, ",");
				while (p)
				{
					std::string pp = p;
					stream << pp;
					p = strtok(NULL, ",");
					if (p) stream.M_NextDirective();
				}
				delete[] t;
				stream.M_EndArray();

				stream.M_NextDirective();
				stream << operationMode;

				stream.M_EndArray();
				stream.M_EndArray();

				C_AutoPointer<C_MiaDriverBase::t_MapElement> e = M_RegisterEventCall(pusherid);
				M_Send(stream.M_GetString());

				if (!e->m_Event->M_Wait(m_s_TimeOut*1000) || e->m_CallId < 0)
				{
					M_RemoveEventCall(pusherid);
					THROW(C_ServerException(Mia_THIS_LOCATION, className, instanceId, properties, operationMode));
					NO_THROW(return -1;);
				}
				M_RemoveEventCall(pusherid);
				return pusherid;
			} 
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, className, instanceId, properties, operationMode);
				throw;
			})
		}

		C_CurrentValuePusher C_MiaDriver::M_CreateValuePusher(const std::string &className)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_CreateValuePusher");
			TRY
			{
				t_CallId pushid = M_CreatePusher(className, "", "Value", 3);
				C_CurrentValuePusherClass *pusher = DBG_NEW C_CurrentValuePusherClass(this, pushid, className);
				this->M_AddOnReconnect(pusher);
				return C_CurrentValuePusher(pusher);
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, className);
				throw;
			})
		}

		C_PropertyValuePusher C_MiaDriver::M_CreatePropertyValuePusher(const std::string &instanceId,
		   const std::string &property)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_CreatePropertyValuePusher");
			TRY
			{
				t_CallId pushid = M_CreatePusher("Path", instanceId, property, 3);
				if (pushid == 0) return C_PropertyValuePusher();
				C_PropertyValuePusherClass *pusher = DBG_NEW C_PropertyValuePusherClass(this, pushid);
				return C_PropertyValuePusher(pusher);
			} 
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, instanceId, property);
				throw;
			})
		}

		C_PropertyValuePusher C_MiaDriver::M_CreatePropertyValuePusher(const C_Guid &instanceId,
		   const std::string &property)
		{
			Mia_THIS_ROUTINE("C_MiaDriver::M_CreatePropertyValuePusher");
			TRY
			{
				t_CallId pushid = M_CreatePusher("Path", instanceId.M_ToString(), property, 3);
				if (pushid == 0) return C_PropertyValuePusher();
				C_PropertyValuePusherClass *pusher = DBG_NEW C_PropertyValuePusherClass(this, pushid);
				return C_PropertyValuePusher(pusher);
			} 
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, instanceId, property);
				throw;
			})
		}

		C_CurrentValuePusherClass::C_CurrentValuePusherClass(C_MiaDriver *driver, t_CallId pusherid, const std::string &className) :
			m_PusherId(pusherid), m_ClassName(className), m_Driver(driver)	{}
		C_CurrentValuePusherClass::~C_CurrentValuePusherClass()
		{
			m_Driver->M_RemoveOnReconnect(this);
		}
		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, const std::string &value)
		{
			M_PushJson(M_ResolveVariableId(variableName), value);
		}

		void C_CurrentValuePusherClass::M_PushBinary(const int & variableId, const C_Variant & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			Mia_THIS_ROUTINE("C_CurrentValuePusherClass::M_PushJson");
			TRY
			{
				if (variableId > 0)
				{
					C_WebsocketStreamBinary stream;
					stream.M_BeginArray();
					stream << m_PusherId;
					stream.M_BeginArray();
					stream << variableId;
					stream << value;
					if (time != NULL && time.M_GetTicks())
					{
						stream << time;
					}
					if (status != g_VS_OK)
					{
						stream << (int)status;
					}
					stream.M_EndArray();
					stream.M_EndArray();

					m_Driver->M_Send(stream.M_GetBuffer(), stream.M_GetSize());
					}
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, variableId, value, time, (int)status);
				throw;
			})
		}

		void C_CurrentValuePusherClass::M_PushJson(const int &variableId, const std::string &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			Mia_THIS_ROUTINE("C_CurrentValuePusherClass::M_PushJson");
			TRY
			{
				if (variableId > 0)
				{
					C_WebsocketStream stream;
					stream.M_BeginArray();
					stream << m_PusherId;
					stream.M_NextDirective();
					stream.M_BeginArray();
					stream << variableId;
					stream.M_NextDirective();
					stream << value;
					if (time != NULL && time.M_GetTicks())
					{
						stream.M_NextDirective();
						stream << time;
					}
					if (status != g_VS_OK)
					{
						stream.M_NextDirective();
						stream << (int)status;
					}
					stream.M_EndArray();
					stream.M_EndArray();
					m_Driver->M_Send(stream.M_GetString());
				}
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, variableId, value, time, (int)status);
				throw;
			})
		}

		void C_CurrentValuePusherClass::M_Push(const std::string & variableName, const char & value)
		{
			M_Push(M_ResolveVariableId(variableName), value);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string & variableName, const byte & value)
		{
			M_Push(M_ResolveVariableId(variableName), value);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string & variableName, const int16 & value)
		{
			M_Push(M_ResolveVariableId(variableName), value);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, const int32 &value)
		{
			M_Push(M_ResolveVariableId(variableName), value);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string & variableName, const int64 & value)
		{
			M_Push(M_ResolveVariableId(variableName), value);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string & variableName, const uint16 & value)
		{
			M_Push(M_ResolveVariableId(variableName), value);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string & variableName, const uint32 & value)
		{
			M_Push(M_ResolveVariableId(variableName), value);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string & variableName, const uint64 & value)
		{
			M_Push(M_ResolveVariableId(variableName), value);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string & variableName, const float & value)
		{
			M_Push(M_ResolveVariableId(variableName), value);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, const double &value)
		{
			M_Push(M_ResolveVariableId(variableName), value);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string & variableName, const bool & value)
		{
			M_Push(M_ResolveVariableId(variableName), value);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string & variableName, const C_Variant & value)
		{
			M_Push(M_ResolveVariableId(variableName), value);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const int64 & value)
		{
			M_PushBinary<int64>(variableId, value);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const uint16 & value)
		{
			M_PushBinary<uint16>(variableId, value);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const uint32 & value)
		{
			M_PushBinary<uint32>(variableId, value);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const uint64 & value)
		{
			M_PushBinary<uint64>(variableId, value);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const float & value)
		{
			M_PushBinary<float>(variableId, value);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const C_Variant & value)
		{
			if (value.M_GetType() == g_STRING)
			{
				M_PushJson(variableId, value.M_ToString());
			}
			else
			{
				M_PushBinary(variableId, value);
			}
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const int32 & value)
		{
			M_PushBinary<int32>(variableId, value);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const char & value)
		{
			M_PushBinary<char>(variableId, value);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const byte & value)
		{
			M_PushBinary<byte>(variableId, value);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const int16 & value)
		{
			M_PushBinary<int16>(variableId, value);
		}

		void C_CurrentValuePusherClass::M_Push(const int &variableId, const std::string &value)
		{
			M_PushJson(variableId, value);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const C_Variant & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			if (value.M_GetType() == g_STRING)
			{
				M_PushJson(variableId, value.M_ToString(), time, status);
			}
			else
			{
				M_PushBinary(variableId, value, time, status);
			}
		}

		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, std::string &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_Push(M_ResolveVariableId(variableName), value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, const char &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_Push(M_ResolveVariableId(variableName), value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, const byte &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_Push(M_ResolveVariableId(variableName), value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, const int16 &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_Push(M_ResolveVariableId(variableName), value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, const int32 &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_Push(M_ResolveVariableId(variableName), value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, const int64 &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_Push(M_ResolveVariableId(variableName), value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, const uint16 &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_Push(M_ResolveVariableId(variableName), value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, const uint32 &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_Push(M_ResolveVariableId(variableName), value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, const uint64 &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_Push(M_ResolveVariableId(variableName), value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, const float &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_Push(M_ResolveVariableId(variableName), value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, const double &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_Push(M_ResolveVariableId(variableName), value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, const bool &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_Push(M_ResolveVariableId(variableName), value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const std::string &variableName, const C_Variant &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_Push(M_ResolveVariableId(variableName), value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, std::string & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			M_PushJson(variableId, value);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const char & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			M_PushBinary<char>(variableId, value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const byte & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			M_PushBinary<byte>(variableId, value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const int16 & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			M_PushBinary<int16>(variableId, value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const int32 & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			M_PushBinary<int32>(variableId, value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const int64 & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			M_PushBinary<int64>(variableId, value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const uint16 & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			M_PushBinary<uint16>(variableId, value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const uint32 & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			M_PushBinary<uint32>(variableId, value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const uint64 & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			M_PushBinary<uint64>(variableId, value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const float & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			M_PushBinary<float>(variableId, value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const double & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			M_PushBinary<double>(variableId, value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const bool & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			M_PushBinary<bool>(variableId, value, time, status);
		}

		void C_CurrentValuePusherClass::M_Push(const int &variableId, const double &value)
		{
			M_PushBinary<double>(variableId, value);
		}

		void C_CurrentValuePusherClass::M_Push(const int & variableId, const bool & value)
		{
			M_PushBinary<bool>(variableId, value);
		}

		void C_CurrentValuePusherClass::M_OnReconnect()
		{
			M_ReconnectHandler();
		}

		void C_CurrentValuePusherClass::M_ReconnectHandler()
		{
			C_Locker locker(&m_Lock);
			
#ifdef CURRENT_VALUE_PUSHER_RECONNECT_NO_TIMEOUT
			while (!m_Driver->M_IsReady()) ABB::Mia::C_Thread::M_S_Sleep(100);
#else
			THIS_ROUTINE("C_CurrentValuePusherClass::M_ReconnectHandler");
			
			int timeout = 0;
			while (!m_Driver->M_IsReady())
			{
				ABB::Mia::C_Thread::M_S_Sleep(10);
				if (timeout++ > 3000){

					MIA_OUT_WARNING << sThisRoutine << ": timeout " << std::endl;
					return;
				}
			}
#endif
			
			m_PusherId = m_Driver->M_CreatePusher(m_ClassName, "", "Value", 3);
			m_Variables.clear();
		}

		int C_CurrentValuePusherClass::M_ResolveVariableId(const std::string &variableName)
		{
			int id = -1;

			unordered_map<std::string, int>::iterator iter;
			{
				C_Locker locker(&m_Lock);
				iter = m_Variables.find(variableName);
			}

			if (iter == m_Variables.end())
			{
				std::string ret = m_Driver->M_FetchClassData("Variable", "RAW:Id", 1, "Name = ?", variableName);

				// insert into the map
				if (ret.size() > 2)
				{
					unsigned long variableid;
					variableid = strtoul(ret.c_str() + 1, NULL, 0);
					MIA_OUT_DEBUG << " Variable Id = " << variableid << " from " << ret << std::endl;
					C_Locker locker(&m_Lock);
					if (variableid > 0 && variableid < ULONG_MAX)
					{
						m_Variables[variableName] = variableid;
						return variableid;
					}
				}
			} else
			{
				id = iter->second;
			}
			return id;
		}

		C_PropertyValuePusherClass::C_PropertyValuePusherClass(C_MiaDriver *driver, t_CallId pusherid) :
			m_Driver(driver), m_PusherId(pusherid)	{}

		void C_PropertyValuePusherClass::M_Push(const std::string &value)
		{
			M_PushJson(value);
		}

		void C_PropertyValuePusherClass::M_Push(const char &value)
		{
			M_PushBinary<char>(value);
		}

		void C_PropertyValuePusherClass::M_Push(const byte &value)
		{
			M_PushBinary<byte>(value);
		}

		void C_PropertyValuePusherClass::M_Push(const int16 &value)
		{
			M_PushBinary<int16>(value);
		}

		void C_PropertyValuePusherClass::M_Push(const int32 &value)
		{
			M_PushBinary<int32>(value);
		}

		void C_PropertyValuePusherClass::M_Push(const int64 &value)
		{
			M_PushBinary<int64>(value);
		}

		void C_PropertyValuePusherClass::M_Push(const uint16 &value)
		{
			M_PushBinary<uint16>(value);
		}

		void C_PropertyValuePusherClass::M_Push(const uint32 &value)
		{
			M_PushBinary<uint32>(value);
		}

		void C_PropertyValuePusherClass::M_Push(const uint64 &value)
		{
			M_PushBinary<uint64>(value);
		}

		void C_PropertyValuePusherClass::M_Push(const float &value)
		{
			M_PushBinary<float>(value);
		}

		void C_PropertyValuePusherClass::M_Push(const double &value)
		{
			M_PushBinary<double>(value);
		}

		void ABB::Mia::C_PropertyValuePusherClass::M_Push(const int64 * value, int & size)
		{
			M_PushBinary<int64>(value, size);
		}

		void C_PropertyValuePusherClass::M_Push(const bool &value)
		{
			M_PushBinary<bool>(value);
		}
		
		void C_PropertyValuePusherClass::M_Push(const C_Variant &value)
		{
			if (value.M_GetType() == g_STRING || value.M_GetType() == g_OBJECT)
			{
				M_PushJson(value.M_ToString());
			}
			else
			{
				M_PushBinary(value);
			}
		}

		void C_PropertyValuePusherClass::M_Push(const std::string &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_PushJson(value, time, status);
		}

		void C_PropertyValuePusherClass::M_Push(const char &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_PushBinary<char>(value, time, status);
		}

		void C_PropertyValuePusherClass::M_Push(const byte &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_PushBinary<byte>(value, time, status);
		}

		void C_PropertyValuePusherClass::M_Push(const int16 &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_PushBinary<int16>(value, time, status);
		}

		void C_PropertyValuePusherClass::M_Push(const int32 &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_PushBinary<int32>(value, time, status);
		}
		
		void C_PropertyValuePusherClass::M_Push(const int64 &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_PushBinary<int64>(value, time, status);
		}
		
		void C_PropertyValuePusherClass::M_Push(const uint16 &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_PushBinary<uint16>(value, time, status);
		}

		void C_PropertyValuePusherClass::M_Push(const uint32 &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_PushBinary<uint32>(value, time, status);
		}

		void C_PropertyValuePusherClass::M_Push(const uint64 &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_PushBinary<uint64>(value, time, status);
		}

		void C_PropertyValuePusherClass::M_Push(const float &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_PushBinary<float>(value, time, status);
		}
		
		void C_PropertyValuePusherClass::M_Push(const double &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_PushBinary<double>(value, time, status);
		}
		
		void C_PropertyValuePusherClass::M_Push(const bool &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			M_PushBinary<bool>(value, time, status);
		}
		
		void C_PropertyValuePusherClass::M_Push(const C_Variant &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			if (value.M_GetType() == g_STRING)
			{
				M_PushJson(value.M_ToString(), time, status);
			}
			else
			{
				M_PushBinary(value, time, status);
			}
		}
		void C_PropertyValuePusherClass::M_PushBinary(const C_Variant & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			Mia_THIS_ROUTINE("C_PropertyValuePusherClass::M_PushBinary");
			TRY
			{
				C_WebsocketStreamBinary stream;
				size_t buffersize = value.M_GetEstimatedSize();
				//if (buffersize > 32) stream.M_Reserve(buffersize + 32);
				stream.M_BeginArray();
				stream << m_PusherId;
				stream.M_BeginArray();
				stream << value;
				if (time != NULL && time.M_GetTicks())
				{
					stream << time;
				}
				if (status != g_VS_OK)
				{
					stream << (int)status;
				}
				stream.M_EndArray();
				stream.M_EndArray();

				m_Driver->M_Send(stream.M_GetBuffer(), stream.M_GetSize());
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, value, time, (int)status);
				throw;
			})
		}

		void C_PropertyValuePusherClass::M_PushJson(const std::string &value, const C_DateTime &time, const t_ValueStatus &status)
		{
			Mia_THIS_ROUTINE("C_PropertyValuePusherClass::M_PushJson");
			TRY
			{
				C_WebsocketStream stream;
				stream.M_BeginArray();
				stream << m_PusherId;
				stream.M_NextDirective();
				stream.M_BeginArray();
				stream << value;
				if (time != NULL && time.M_GetTicks())
				{
					stream.M_NextDirective();
					stream << time;
				}
				if (status != g_VS_OK)
				{
					stream.M_NextDirective();
					stream << (int)status;
				}
				stream.M_EndArray();
				stream.M_EndArray();
				m_Driver->M_Send(stream.M_GetString());
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, value, time, (int)status);
				throw;
			})
		}

		const char* C_DbClassClass::m_k_s_ClassProperties = "RAW:Id, RAW:DisplayName, NonCacheable, BaseTables, ClassType";
		const char* C_DbPropertyInfoClass::m_k_s_ClassProperties =
		   "ClassName, RAW:Name, RAW:Category, RAW:DisplayName, DefaultValue, RAW:Description, Index, Size, Type, RawType, IsNullable, IsNumeric, IsInterned, IsReadOnly, IsUnique, IsVirtual, IsVisibleToUserByDefault, MaskRequired, ReferenceTarget";


		class C_DbClassClass::C_DbClassPrivate
		{
			private: C_DbClassPrivate(): m_Driver(0), m_NonCacheable(false), m_ClassType(g_CLASSTYPE_TABLE) {}

			private: C_DbClassPrivate(C_MiaDriver* driver, const std::string &className) : m_Driver(driver), m_ClassName(className), m_NonCacheable(false)
			{
				m_ClassType = g_CLASSTYPE_TABLE;
			}

			private: C_DbClassPrivate(C_MiaDriver* driver, const std::string &className, const std::string &displayName,
				bool nonCacheable, const std::string &baseTable, const std::string &classType) :
				m_Driver(driver), m_ClassName(className), m_NonCacheable(nonCacheable), m_BaseTable(baseTable)
			{
				m_ClassType = (classType == "Table" ? g_CLASSTYPE_TABLE : g_CLASSTYPE_COMMAND);
			}

			protected: unordered_map<std::string, C_DbPropertyInfo> m_PropertyInfos;

			public: ~C_DbClassPrivate()
			{
				m_PropertyInfos.clear();
			}


			public: std::string M_GetPropertiesString()
			{
				std::string properties;
				{
					C_Locker locker(&m_PropertyLock);
					for (unordered_map<std::string, C_DbPropertyInfo>::iterator iter = m_PropertyInfos.begin();
						iter != m_PropertyInfos.end(); iter++)
					{
						C_DbPropertyInfo pi = iter->second;

						if (properties.length() != 0) properties += ",";
						
						properties += ("RAW:" + pi->M_GetName());
					}
				}

				return properties;
			}
			private: C_MiaDriver *m_Driver;
			private: std::string m_ClassName;
			private: std::string m_DisplayName;
			private: bool			m_NonCacheable;
			private: std::string m_BaseTable;
			private: t_ClassType m_ClassType;
			private: C_ThreadLock m_PropertyLock;
			private: std::string m_PropertyString;

			friend class C_DbClassClass;
		};

		C_DbClassClass::C_DbClassClass()
		{
			m_Private = DBG_NEW C_DbClassClass::C_DbClassPrivate();
		}

		C_DbClassClass::C_DbClassClass(C_MiaDriver* driver, const std::string &className)
		{
			m_Private = DBG_NEW C_DbClassPrivate(driver, className);
		}

		C_DbClassClass::~C_DbClassClass(){}

		C_MiaDriver * C_DbClassClass::M_GetDriver() { return m_Private->m_Driver;}

		const std::string &C_DbClassClass::M_GetClassName() const
		{
			return m_Private->m_ClassName;
		}

		const std::string &C_DbClassClass::M_GetDisplayName() const
		{
			return m_Private->m_DisplayName;
		}

		bool C_DbClassClass::M_IsCacheable() const
		{
			return !m_Private->m_NonCacheable;
		}

		bool C_DbClassClass::M_IsNonCacheable() const
		{
			return m_Private->m_NonCacheable;
		}

		const std::string &C_DbClassClass::M_GetBaseTable() const
		{
			return m_Private->m_BaseTable;
		}

		const t_ClassType &C_DbClassClass::M_GetClassType() const
		{
			return m_Private->m_ClassType;
		}

		C_DbClassClass::C_DbClassClass(C_MiaDriver* driver, const std::string &className, const std::string &displayName,
		   bool nonCacheable, const std::string &baseTable, const std::string &classType)
		{
			m_Private = DBG_NEW C_DbClassPrivate(driver, className, displayName, nonCacheable, baseTable, classType);
			m_Private->m_ClassType = (classType == "Table" ? g_CLASSTYPE_TABLE : g_CLASSTYPE_COMMAND);
			MIA_OUT_DEBUG << "C_DbClass::Init" << className << std::endl;
		}

		bool C_DbClassClass::M_Remove(C_DbClassInstance &instance)
		{
			return instance->M_Remove();
		}

		void C_DbClassClass::M_GetProperties(C_DbPropertyInfos *properties, bool refresh)
		{
			if (refresh || !m_Private->m_PropertyInfos.size())
			{
				M_FetchAndUpdateProperties();
			}

			properties->clear();
			C_Locker locker(&m_Private->m_PropertyLock);
			unordered_map<std::string, C_DbPropertyInfo>::iterator piter;
			for (piter = m_Private->m_PropertyInfos.begin(); piter != m_Private->m_PropertyInfos.end(); piter++)
			{
				properties->push_back(piter->second);
			}
		}

		const C_DbPropertyInfo& C_DbClassClass::operator[](const std::string &propertyInfo)
		{
			C_Locker locker(&m_Private->m_PropertyLock);
			std::string propname = propertyInfo;
			G_MIA_TRANSFORM_TO_LOWER(propname);
			return M_GetProperty(propname);
		}

		const C_DbPropertyInfo& C_DbClassClass::M_GetProperty(const std::string &propertyName)
		{
			Mia_THIS_ROUTINE("C_DbClassClass::M_GetProperty");
			C_Locker locker(&m_Private->m_PropertyLock);
			static C_DbPropertyInfo invalid;
			
			if (!m_Private->m_PropertyInfos.size())
			{
				M_FetchAndUpdateProperties();
			};

			std::string propname = propertyName;
			G_MIA_TRANSFORM_TO_LOWER(propname);
			unordered_map<std::string, C_DbPropertyInfo>::iterator piter = m_Private->m_PropertyInfos.find(propname);
			if (piter != m_Private->m_PropertyInfos.end())
			{
				return (piter->second);
			}
			THROW(C_PropertyNotFoundException(Mia_THIS_LOCATION, propertyName));
			NO_THROW(return invalid;);
		}

		void ABB::Mia::C_DbClassClass::M_ClearProperties()
		{
			m_Private->m_PropertyInfos.clear();
		}

		void C_DbClassClass::M_FetchAndUpdateProperties(C_DbClass *c)
		{
			Mia_THIS_ROUTINE("C_DbClassClass::M_FetchAndUpdateProperties");
			std::string propertyinfos = m_Private->m_Driver->M_FetchClassData("ClassProperty",
				C_DbPropertyInfoClass::m_k_s_ClassProperties, -1, "ClassName=?", m_Private->m_ClassName);

			C_WebsocketStream s(propertyinfos);
			C_Variant infos(propertyinfos, g_ARRAY);
			std::string temp;

			if (infos.M_IsValid())
			{
				int i = 0;
				for (int j = 0;; j++)
				{
					std::string temp, propname, category, displayname, defaultval, description, index, size, type, rawtype,
						nullable, numeric, interned, readonly, unique, isvirtual, visibletouser, maskrequired, referencetarget;
					TRY
					{
						C_Variant info;
						infos.M_GetValue(i++, info);
						temp = info.M_ToString();

						if (!temp.length() || temp != m_Private->m_ClassName) return; // TODO: Throw exception or just return ?

						infos.M_GetValue(i++, info);
						propname = info.M_ToString();

						infos.M_GetValue(i++, info);
						category = info.M_ToString();

						infos.M_GetValue(i++, info);
						displayname = info.M_ToString();

						infos.M_GetValue(i++, info);
						defaultval = info.M_ToString();

						infos.M_GetValue(i++, info);
						description = info.M_ToString();

						infos.M_GetValue(i++, info);
						index = info.M_ToString();

						infos.M_GetValue(i++, info);
						size = info.M_ToString();

						infos.M_GetValue(i++, info);
						type = info.M_ToString();

						infos.M_GetValue(i++, info);
						rawtype = info.M_ToString();

						infos.M_GetValue(i++, info);
						nullable = info.M_ToString();

						infos.M_GetValue(i++, info);
						numeric = info.M_ToString();

						infos.M_GetValue(i++, info);
						interned = info.M_ToString();

						infos.M_GetValue(i++, info);
						readonly = info.M_ToString();

						infos.M_GetValue(i++, info);
						unique = info.M_ToString();

						infos.M_GetValue(i++, info);
						isvirtual = info.M_ToString();

						infos.M_GetValue(i++, info);
						visibletouser = info.M_ToString();

						infos.M_GetValue(i++, info);
						maskrequired = info.M_ToString();

						infos.M_GetValue(i++, info);
						referencetarget = info.M_ToString();

						if (propname.size())
						{
							C_Locker locker(&m_Private->m_PropertyLock);
							unordered_map<std::string, C_DbPropertyInfo>::iterator piiter = m_Private->m_PropertyInfos.find(propname);
							if (piiter != m_Private->m_PropertyInfos.end())
							{
								C_DbPropertyInfoClass::C_DbPropertyInfoPrivate* pi = m_Private->m_PropertyInfos[propname]->m_Private;
								if (pi->m_Category != category) pi->m_Category = category;
								if (pi->m_DisplayName != displayname) pi->m_DisplayName = displayname;
								if (pi->m_DefaultValue != defaultval) pi->m_DefaultValue = defaultval;
								if (pi->m_Description != description) pi->m_Description = description;
								if (pi->m_Index != (uint32)atoi(index.c_str())) pi->m_Index = atoi(index.c_str());
								if (pi->m_Size != (uint32)atoi(size.c_str())) pi->m_Size = atoi(size.c_str());
								m_Private->m_PropertyInfos[propname]->M_SetType(type);
								if (pi->m_RawType != rawtype) pi->m_RawType = rawtype;
								if (pi->m_ReferenceTarget != referencetarget) pi->m_ReferenceTarget = referencetarget;
							}
							else
							{
								if (!c) *c = m_Private->m_Driver->M_GetClass(m_Private->m_ClassName);

								C_DbPropertyInfoClass *pi = DBG_NEW C_DbPropertyInfoClass(*c, propname, category, displayname,
									defaultval, description, index, size, type, rawtype, nullable == "True", numeric == "True",
									interned == "True", readonly == "True", unique == "True", isvirtual == "True",
									visibletouser == "True", maskrequired == "True", referencetarget);
								G_MIA_TRANSFORM_TO_LOWER(propname);
								m_Private->m_PropertyInfos[propname] = C_DbPropertyInfo(pi);
							}
						}
					}
					CATCH(C_Exception &ex,
					{
						ex.M_RethrowTraceback(Mia_THIS_LOCATION, (uint64)c );
					})
				}
			}
		}

		std::string C_DbClassClass::M_GetPropertySerializedString()
		{
			if (m_Private->m_PropertyString.size()) return m_Private->m_PropertyString;
			std::string properties;
			C_Locker locker(&m_Private->m_PropertyLock);
			for (unordered_map<std::string, C_DbPropertyInfo>::iterator iter = m_Private->m_PropertyInfos.begin(); iter != m_Private->m_PropertyInfos.end(); iter++)
			{
				if (properties.length() != 0) properties += ",";
				properties += iter->second->M_GetName();
			}
			m_Private->m_PropertyString = properties;
			return properties;
		}

		C_DbPropertyInfoClass::C_DbPropertyInfoClass(C_DbClass parent, const std::string &propname,
		   const std::string & category, const std::string & displayname, const std::string & defaultval,
		   const std::string & description, const std::string & index, const std::string & size, const std::string & type,
		   const std::string & rawtype, bool nullable, bool numeric, bool interned, bool readonly, bool unique,
		   bool isvirtual, bool visibletouser, bool maskrequired, const std::string & referencetarget)
		{
			m_Private = DBG_NEW C_DbPropertyInfoPrivate(parent, propname, category, displayname, defaultval,
			description, index, size, type,
			rawtype, nullable, numeric, interned, readonly, unique,
			isvirtual, visibletouser, maskrequired, referencetarget);

			M_SetType(type);
		}

		ABB::Mia::C_DbPropertyInfoClass::~C_DbPropertyInfoClass()
		{
		}

		/**
		 * @brief Get name of the property
		 * */
		const std::string &C_DbPropertyInfoClass::M_GetName() const
		{
			return m_Private->m_PropertyName;
		}

		/**
		 * @brief Get category
		 */
		const std::string &C_DbPropertyInfoClass::M_GetCategory() const
		{
			return m_Private->m_Category;
		}

		/**
		 * @brief Get display name
		 */
		const std::string &C_DbPropertyInfoClass::M_GetDisplayName() const
		{
			return m_Private->m_DisplayName;
		}

		/**
		 * @brief Get default value
		 */
		const std::string &C_DbPropertyInfoClass::M_GetDefaulValue() const
		{
			return m_Private->m_DefaultValue;
		}

		/**
		 * @brief Get description
		 */
		const std::string &C_DbPropertyInfoClass::M_GetDescription() const
		{
			return m_Private->m_Description;
		}

		/**
		 * @brief Get index
		 */
		const uint32 &C_DbPropertyInfoClass::M_GetIndex() const
		{
			return m_Private->m_Index;
		}

		/**
		 * @brief Get size of the property
		 */
		const uint32 &C_DbPropertyInfoClass::M_GetSize() const
		{
			return m_Private->m_Size;
		}

		/**
		 * @brief Get type of the property
		 */
		const t_DataType &C_DbPropertyInfoClass::M_GetType() const
		{
			return m_Private->m_Type;
		}

		/**
		 * @brief Get type of the property
		 */
		const std::string &C_DbPropertyInfoClass::M_GetTypeName() const
		{
			return m_Private->m_TypeName;
		}

		/**
		 * @brief Get raw type of the property
		 */
		const std::string &C_DbPropertyInfoClass::M_GetRawType() const
		{
			return m_Private->m_RawType;
		}

		/**
		 * @brief Get reference target
		 */
		const std::string &C_DbPropertyInfoClass::M_GetReferenceTarget() const
		{
			return m_Private->m_ReferenceTarget;
		}

		/**
		 * @brief Get class of this property
		 */
		C_DbClass C_DbPropertyInfoClass::M_GetClass()
		{
			return m_Private->m_Class;
		}

		/**
		 * @brief Check if the property is numeric
		 */
		bool C_DbPropertyInfoClass::M_IsNumeric()
		{
			return m_Private->m_IsNumeric;
		}

		/**
		 * @brief Check if the property is nullable
		 */
		bool C_DbPropertyInfoClass::M_IsNullable()
		{
			return m_Private->m_IsNullable;
		}

		/**
		 * @brief Check if the property is interned
		 */
		bool C_DbPropertyInfoClass::M_IsInterned()
		{
			return m_Private->m_IsInterned;
		}

		/**
		 * @brief Check if the property is readonly
		 */
		bool C_DbPropertyInfoClass::M_IsReadonly()
		{
			return m_Private->m_IsReadonly;
		}

		/**
		 * @brief Check if the property is unique
		 */
		bool C_DbPropertyInfoClass::M_IsUnique()
		{
			return m_Private->m_IsUnique;
		}

		/**
		 * @brief Check if the property is virtual
		 */
		bool C_DbPropertyInfoClass::M_IsVirtual()
		{
			return m_Private->m_IsVirtual;
		}

		/**
		 * @brief Check if the property is visible to other user
		 */
		bool C_DbPropertyInfoClass::M_IsVisibleToUser()
		{
			return m_Private->m_IsVisibleToUser;
		}

		/**
		 * @brief Check if the property required masking
		 */
		bool C_DbPropertyInfoClass::M_IsMaskRequired()
		{
			return m_Private->m_MaskRequired;
		}

		void C_DbPropertyInfoClass::M_SetType(const std::string &type)
		{
			m_Private->m_TypeName = type;
			std::string t = type;
			m_Private->m_Type = C_Variant::M_S_FromStringToDataType(type);
		}

		void C_DbPropertyInfoClass::M_SetType(const t_DataType &type)
		{
			m_Private->m_Type = type;
			m_Private->m_TypeName = C_Variant::M_S_FromDataTypeToString(type);
		}

		bool C_DbClassClass::M_GetInstanceSet(C_DbClassInstances *instances, int fetchMax)
		{
			if (!instances) return false;
			Mia_THIS_ROUTINE("C_DbClassClass::M_GetInstanceSet");
			std::string properties;
			C_DbPropertyInfos pies;
			M_GetProperties(&pies);
			{
				for (C_DbPropertyInfos::iterator iter = pies.begin(); iter != pies.end(); iter++)
				{
					C_DbPropertyInfo pi = *iter;
					if (properties.length() != 0) properties += ",";
					properties += ("RAW:" + pi->M_GetName());
				}
			}
			TRY
			{
				std::string data = m_Private->m_Driver->M_FetchClassData(m_Private->m_ClassName, properties, fetchMax);
				if (data.size())
				{
					C_Variant jos(data, g_ARRAY);
			
					int size = jos.M_GetArraySize();
					int i = 0;
					while (i < size)
					{
						C_DbClass c = m_Private->m_Driver->M_GetClass(m_Private->m_ClassName);
						C_DbClassInstanceClass *instance = DBG_NEW C_DbClassInstanceClass(c);
						C_Locker locker(&m_Private->m_PropertyLock);

						for (unordered_map<std::string, C_DbPropertyInfo>::iterator iter = m_Private->m_PropertyInfos.begin();
							iter != m_Private->m_PropertyInfos.end(); iter++)
						{
							TRY
							{
								// Create a new Class Instance
								if (i == size) break;
								const C_Variant value = jos[i++];
								instance->M_AssignValue(iter->second, value.M_ToString());
							} CATCH(C_PropertyNotFoundException &ex, 
							{
								ex.M_SetHandled();
							})
						}

						instances->push_back(C_DbClassInstance(instance));
					}
				}
			} CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, fetchMax);
			})

			return false;
		}

		bool C_DbClassClass::M_GetInstanceSet(C_DbClassInstances *instances, const std::string &whereString, const C_ArgumentList &arguments)
		{
			return M_GetInstanceSet(instances, -1, whereString, arguments);
		}

		bool C_DbClassClass::M_GetInstanceSet(C_DbClassInstances *instances, int fetchMax, const std::string &whereString, const C_ArgumentList &arguments)
		{
			Mia_THIS_ROUTINE("C_DbClassClass::M_GetInstanceSet");
			if (!instances) return false;
			std::string properties;
			std::string argumentstring;

			TRY
			{

				{
					C_Locker locker(&m_Private->m_PropertyLock);

					for (unordered_map<std::string, C_DbPropertyInfo>::iterator iter = m_Private->m_PropertyInfos.begin();
					   iter != m_Private->m_PropertyInfos.end(); iter++)
					{
						C_DbPropertyInfo pi = iter->second;
						if (properties.length() != 0) properties += ",";

						/*if (pi->M_GetRawType() == "String")*/ properties += ("RAW:" + pi->M_GetName());
						/*else properties += pi->M_GetName();*/
					}
				}
				std::string data;
				switch (arguments.size())
				{
				case 1:
					data = m_Private->m_Driver->M_FetchClassData(m_Private->m_ClassName, properties, fetchMax, whereString, *arguments[0]);
					break;
				case 2:
					data = m_Private->m_Driver->M_FetchClassData(m_Private->m_ClassName, properties, fetchMax, whereString, 
						*arguments[0], *arguments[1]);
					break;
				case 3:
					data = m_Private->m_Driver->M_FetchClassData(m_Private->m_ClassName, properties, fetchMax, whereString, 
						*arguments[0], *arguments[1], *arguments[2]);
					break;
				case 4:
					data = m_Private->m_Driver->M_FetchClassData(m_Private->m_ClassName, properties, fetchMax, whereString, 
						*arguments[0], *arguments[1], *arguments[2], *arguments[3]);
					break;
				case 5:
					data = m_Private->m_Driver->M_FetchClassData(m_Private->m_ClassName, properties, fetchMax, whereString, 
						*arguments[0], *arguments[1], *arguments[2], *arguments[3], *arguments[4]);
					break;
				case 6:
					data = m_Private->m_Driver->M_FetchClassData(m_Private->m_ClassName, properties, fetchMax, whereString, 
						*arguments[0], *arguments[1], *arguments[2], *arguments[3], *arguments[4], *arguments[5]);
					break;
				case 7:
					data = m_Private->m_Driver->M_FetchClassData(m_Private->m_ClassName, properties, fetchMax, whereString, 
						*arguments[0], *arguments[1], *arguments[2], *arguments[3], *arguments[4], *arguments[5], *arguments[6]);
					break;
				case 8:
					data = m_Private->m_Driver->M_FetchClassData(m_Private->m_ClassName, properties, fetchMax, whereString, 
						*arguments[0], *arguments[1], *arguments[2], *arguments[3], *arguments[4], *arguments[5], *arguments[6], *arguments[7]);
					break;
				default:
					break;
				}

				C_WebsocketStream s(data);
				bool initial = true;
				std::string ss;
				for (;;)
				{
					if (initial)
					{
						ss = "";
						s >> ss;
						initial = false;
					}
					unsigned int propertyparsed = 0;
					if (ss.size())
					{
						// Create a new Class Instance
						C_DbClass c = m_Private->m_Driver->M_GetClass(m_Private->m_ClassName);
						C_DbClassInstanceClass * instance = DBG_NEW C_DbClassInstanceClass(c);
						C_Locker locker(&m_Private->m_PropertyLock);
						for (unordered_map<std::string, C_DbPropertyInfo>::iterator iter = m_Private->m_PropertyInfos.begin();
						   iter != m_Private->m_PropertyInfos.end(); iter++)
						{
							if (ss.size())
							{
								instance->M_AssignValue(iter->second, ss);
							}
							propertyparsed++;

							if (iter++ == m_Private->m_PropertyInfos.end())
							{
								break;
							}
							if (s.M_HasMore()) s >> ss;
							else
							{
								ss = "";
								break;
							}
						}
						if (propertyparsed == m_Private->m_PropertyInfos.size()) instances->push_back(C_DbClassInstance(instance));
					} else return instances->size() > 0;

				}
			} 
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, fetchMax, whereString, argumentstring);
				return false;
			})
		}

		bool C_DbClassClass::M_GetInstanceSet(C_DbClassInstances *instances, int fetchMax, const std::string &whereString, const std::string & arguments)
		{
			if (!instances) return false;
			Mia_THIS_ROUTINE("C_DbClassClass::M_GetInstanceSet");
			TRY
			{
				std::string properties = m_Private->M_GetPropertiesString();
				std::string data = m_Private->m_Driver->M_FetchClassData(m_Private->m_ClassName, properties, fetchMax, whereString, arguments);

				C_Variant parseddata = C_Variant::M_S_FromJSON(data);
				if (parseddata.M_IsValid())
				{
					int index = 0;
					int arraysize = parseddata.M_GetArraySize();
					int propertiessize = m_Private->m_PropertyInfos.size();
					int count = (int)arraysize / (int)propertiessize;
					for (int j = 0; j < count; j++)
					{
						unsigned int propertyparsed = 0;

						// Create a new Class Instance
						C_DbClass c = m_Private->m_Driver->M_GetClass(m_Private->m_ClassName);
						C_DbClassInstanceClass * instance = DBG_NEW C_DbClassInstanceClass(c);
						C_Locker locker(&m_Private->m_PropertyLock);
						for (unordered_map<std::string, C_DbPropertyInfo>::iterator iter = m_Private->m_PropertyInfos.begin();
							iter != m_Private->m_PropertyInfos.end(); iter++)
						{
							instance->M_AssignValue(iter->second, parseddata[index++]);

							propertyparsed++;
						}
						if (propertyparsed == propertiessize) instances->push_back(C_DbClassInstance(instance));
					}

					return instances->size() > 0;
				} else
				{
					return false;
				}
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, fetchMax, whereString, arguments);
				return false;
			})
		}

		C_DbClassInstance C_DbClassClass::M_GetInstanceById(const C_Variant &guid)
		{
			std::string properties, ss;
			Mia_THIS_ROUTINE("C_DbClassClass::M_GetInstanceById");

			TRY
			{
				{
					C_Locker locker(&m_Private->m_PropertyLock);
					for (unordered_map<std::string, C_DbPropertyInfo>::iterator iter = m_Private->m_PropertyInfos.begin();
					   iter != m_Private->m_PropertyInfos.end(); iter++)
					{
						C_DbPropertyInfo pi = iter->second;
						if (properties.length() != 0) properties += ",";
						/*if (pi->M_GetRawType() == "String")*/ properties += ("RAW:" + pi->M_GetName());
						/*else properties += pi->M_GetName();*/
					}
				}

				std::string data = m_Private->m_Driver->M_FetchClassData(m_Private->m_ClassName, properties, 1, "ID = ?", guid.M_ToString());
				C_WebsocketStream s(data);
				s >> ss;
				if (ss.size())
				{
					// Create a new Class Instance
					C_DbClass c = m_Private->m_Driver->M_GetClass(m_Private->m_ClassName);
					C_DbClassInstanceClass * instance = DBG_NEW C_DbClassInstanceClass(c);
					C_Locker locker(&m_Private->m_PropertyLock);
					for (unordered_map<std::string, C_DbPropertyInfo>::iterator iter = m_Private->m_PropertyInfos.begin();
					   iter != m_Private->m_PropertyInfos.end(); iter++)
					{
						if (ss.size())
						{
							instance->M_AssignValue(iter->second, ss);
						}
						s >> ss;
					};
					return C_DbClassInstance(instance);
				} else return C_DbClassInstance();
			}
			CATCH (C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, guid);
				return C_DbClassInstance();
			})
		}

		/******************************************************************************************************************/
		// C_DbClassInstance
		//

		class C_DbClassInstanceClass::C_DbClassInstancePrivate
		{
			public: C_DbClassInstancePrivate(): m_Updating(false) {}

			public: C_DbClassInstancePrivate(C_DbClass c):	m_Updating(false), m_Class(c) {}
			public: ~C_DbClassInstancePrivate()
			{
				C_Locker locker(&m_PropertyLock);
				m_Properties.clear();
			}

			private: void M_BeginUpdate()
			{
				m_Updating = true;
				C_Locker locker(&m_ModifiedLock);
				m_ModifiedPropertyList.clear();
				m_ModifiedRawList.clear();

				C_DbPropertyInfo pi;
				TRY
				{
					pi = m_Class->M_GetProperty("Id");
					m_ModifiedPropertyList["id"] = this->m_Properties["id"];
				}
				CATCH(C_PropertyNotFoundException &e,
				{
					e.M_SetHandled();
				})
			}
			private: C_ThreadLock m_PropertyLock;
			private: unordered_map<std::string, C_SharedPointer<C_Variant> > m_Properties;
			private: C_ThreadLock m_ModifiedLock;
			private: unordered_map<std::string, C_SharedPointer<C_Variant> > m_ModifiedPropertyList;
			private: std::set<std::string> m_ModifiedRawList;
			private: bool m_Updating;
			private: C_DbClass m_Class;
			friend class C_DbClassInstanceClass;
			friend class C_DbClassClass;
		};

		C_DbClassInstanceClass::C_DbClassInstanceClass() { m_Private = DBG_NEW C_DbClassInstanceClass::C_DbClassInstancePrivate(); }

		C_DbClassInstanceClass::C_DbClassInstanceClass(C_DbClass c) {  m_Private = DBG_NEW C_DbClassInstanceClass::C_DbClassInstancePrivate(c); }

		C_DbClassInstanceClass::~C_DbClassInstanceClass() {}

		C_DbClassInstance C_DbClassClass::M_Add()
		{
			C_DbClass c = m_Private->m_Driver->M_GetClass(m_Private->m_ClassName);
			C_DbClassInstanceClass *i = DBG_NEW C_DbClassInstanceClass(c);
			unordered_map<std::string, C_DbPropertyInfo>::iterator piter;

			{
				C_Locker locker(&m_Private->m_PropertyLock);

				for (piter = m_Private->m_PropertyInfos.begin(); piter != m_Private->m_PropertyInfos.end(); piter++)
				{
					i->M_AssignValue(piter->first);
				}
			}
			i->m_Private->m_Updating = true;
			return C_DbClassInstance(i);
		}

		C_DbClass C_DbClassInstanceClass::M_GetClass()
		{
			return m_Private->m_Class;
		}

		const C_Variant &C_DbClassInstanceClass::M_GetId()
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetId");
			C_SharedPointer<C_Variant> id;
			static const C_Variant invalid;
			TRY
			{
				id = M_GetValue("id");
			} 
			CATCH(C_PropertyNotFoundException &e,
			{
				(void)e;
				id = M_GetValue("Id");
			})

			if (id.get() && id->M_IsValid()) return *(id.get());
			THROW(C_PropertyNotFoundException(Mia_THIS_LOCATION, "id"));
			NO_THROW(return invalid;);
		}

		std::string C_DbClassInstanceClass::M_GetName()
		{
			if (M_GetValue("Name").get())
			{
				return M_GetValue("Name")->M_ToString();
			}
			return "";
		}

		void C_DbClassInstanceClass::M_BeginUpdate()
		{
			m_Private->M_BeginUpdate();
		}

		bool C_DbClassInstanceClass::M_CommitChanges()
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_CommitChanges");
			std::string properties;
			std::string propertyvalues;
			bool noidproperty = false;
			C_DbPropertyInfo pi;
			TRY
			{
				pi = m_Private->m_Class->M_GetProperty("Id");
			} 
			CATCH(C_PropertyNotFoundException &e,
			{
				e.M_SetHandled();
				noidproperty = true;
			})

			if (!noidproperty)
			{
				bool hasproperty = false;
				{
					C_Locker locker(&m_Private->m_ModifiedLock);
					for (unordered_map<std::string, C_SharedPointer<C_Variant> >::iterator iter =
					   m_Private->m_ModifiedPropertyList.begin(); iter != m_Private->m_ModifiedPropertyList.end(); iter++)
					{
						if (iter->first == "id" || iter->first == "Id" || iter->first == "ID")
						{
							hasproperty = true;
							break;
						}
					}
				}

				if (!hasproperty)
				{
					t_DataType type = pi->M_GetType();
					std::string value;
					if (type == g_STRING) value = "null";
					else if (type == g_DATETIME || type == g_UNKNOWN || type == g_OBJECT || type == g_CLASS) value = "null";
					else if (type == g_GUID) value = "\"" + (C_Guid().M_ToString()) + "\"";
					else value = "\"0\"";

					m_Private->m_ModifiedPropertyList["id"] = C_SharedPointer<C_Variant> (new C_Variant (value));
					m_Private->m_ModifiedRawList.insert("id");
				}
			}

			{
				C_Locker locker(&m_Private->m_ModifiedLock);
				for (unordered_map<std::string, C_SharedPointer<C_Variant> >::iterator iter =
					m_Private->m_ModifiedPropertyList.begin(); iter != m_Private->m_ModifiedPropertyList.end(); iter++)
				{
					if (properties.length() != 0)
					{
						properties += ",";
						propertyvalues += ",";
					}
					if (m_Private->m_ModifiedRawList.find(iter->first) != m_Private->m_ModifiedRawList.end())
					{
						properties += ("RAW:" + iter->first);
						C_WebsocketStream ss;
						ss.M_AppendRaw(iter->second->M_ToString());

						propertyvalues += ss.M_GetString();
					} else
					{
						C_DbPropertyInfo pi = m_Private->m_Class->M_GetProperty(iter->first);
						properties += (iter->first);

						std::string val = iter->second->M_ToString();
						propertyvalues += ("\"" + val + "\"");
					}
				}
			}

			// Get id here
			C_Variant id;
			std::string ids;
			std::string result;
			TRY
			{
				id = M_GetId();
				ids = id.M_ToString();
			} 
			CATCH(C_PropertyNotFoundException &ex,
			{
				ex.M_SetHandled();
				ids = "";
			})
			TRY
			{
				result = m_Private->m_Class->M_GetDriver()->M_CommitClassDataRaw(m_Private->m_Class->M_GetClassName(), properties, ids, propertyvalues, "");

				if (!result.size()) return false;
				C_Variant json(result, g_ARRAY);
				if (!json.M_IsValid() || json.M_GetArraySize() < m_Private->m_ModifiedPropertyList.size()) return false;

				C_Locker locker(&m_Private->m_ModifiedLock);

				int i = 0;
				for (unordered_map<std::string, C_SharedPointer<C_Variant> >::iterator iter =
					m_Private->m_ModifiedPropertyList.begin(); iter != m_Private->m_ModifiedPropertyList.end(); iter++)
				{
					C_Variant* v = M_GetProperty(iter->first);
					C_Variant temp;
					json.M_GetValue(i++, temp);
					C_DbPropertyInfo pi = m_Private->m_Class->M_GetProperty(iter->first);
					v->M_FromString(temp.M_ToString(), pi->M_GetType());
					if (!v->M_IsValid())
					{
						*v = temp;;
					}
				}
				m_Private->m_Updating = false;


				m_Private->m_ModifiedPropertyList.clear();
				m_Private->m_ModifiedRawList.clear();
				return true;
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION);
				return false;
			})
		}

		void C_DbClassInstanceClass::M_Cancel()
		{
			m_Private->m_Updating = false;
			C_Locker locker(&m_Private->m_ModifiedLock);
			m_Private->m_ModifiedPropertyList.clear();
			m_Private->m_ModifiedRawList.clear();
		}

		bool C_DbClassInstanceClass::M_Remove()
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_Remove");
			C_Variant id = M_GetId();
			// try composite key
			std::string ids = id.M_ToString();

			TRY
			{
				C_Variant compkey(ids, g_ARRAY);
				if (compkey.M_IsValid())
				{
					std::string result = m_Private->m_Class->M_GetDriver()->M_CommitClassDataComposite(m_Private->m_Class->M_GetClassName(), "", compkey, "", "");
					if (result.find(".Exception") != std::string::npos)
					{
						THROW(C_ServerException(Mia_THIS_LOCATION, result));
						NO_THROW(return false;);
					}
					return true;
				}
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION);
			})

			TRY
			{
				std::string result = m_Private->m_Class->M_GetDriver()->M_CommitClassDataRaw(m_Private->m_Class->M_GetClassName(), "", ids, "", "");
				if (result.find(".Exception") != std::string::npos)
				{
					THROW(C_ServerException(Mia_THIS_LOCATION, result));
					NO_THROW(return false;);
				}
			} 
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION);
			})
			return true;
		}

		C_SharedPointer<C_Variant> C_DbClassInstanceClass::M_GetValue(const std::string &propertyName)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			std::string pn = G_StringToLower(propertyName);
			
			C_Locker locker(&m_Private->m_PropertyLock);
			unordered_map<std::string, C_SharedPointer<C_Variant> >::iterator iter = m_Private->m_Properties.find(pn);
			static C_SharedPointer<C_Variant> invalid_ptr;

			if (iter != m_Private->m_Properties.end())
			{
				return iter->second;
			}
			THROW(C_PropertyNotFoundException(Mia_THIS_LOCATION, propertyName));
			NO_THROW(return invalid_ptr;);
		}

		C_SharedPointer<C_Variant> C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			std::string pn = G_StringToLower(propertyInfo->M_GetName());
			static C_SharedPointer<C_Variant> invalid_ptr;
			C_Locker locker(&m_Private->m_PropertyLock);
			unordered_map<std::string, C_SharedPointer<C_Variant> >::iterator iter = m_Private->m_Properties.find(pn);

			if (iter != m_Private->m_Properties.end())
			{
				return iter->second.get();
			}
			THROW(C_PropertyNotFoundException(Mia_THIS_LOCATION, propertyInfo->M_GetName()));
			NO_THROW(return invalid_ptr;);
		}

		void C_DbClassInstanceClass::M_SetValue(const std::string &propertyName, const C_Variant& value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_SetValue");
			if (!m_Private->m_Updating)
			{
				THROW(C_InvalidStateException(Mia_THIS_LOCATION, "Updating = false", "Updating = true", propertyName, value));
			}
			TRY
			{
				C_Variant *v = M_GetProperty(propertyName);

				if (v)
				{
					C_Locker locker(&m_Private->m_ModifiedLock);
					C_Variant *mv = M_GetModifiedProperty(propertyName);
					if (mv) mv->M_SetValue(value);
					else
					{
						std::string pn = G_StringToLower(propertyName);
						mv = DBG_NEW C_Variant(value);
						m_Private->m_ModifiedPropertyList[pn] = C_SharedPointer<C_Variant>(mv);
					}
				} else
				{
					THROW(C_PropertyNotFoundException(Mia_THIS_LOCATION, propertyName));
				}
			} CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION);
			});
		}

		void C_DbClassInstanceClass::M_SetRawValue(const std::string &propertyName, const C_Variant& value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_SetRawValue");
			if (!m_Private->m_Updating)
			{
				THROW(C_InvalidStateException(Mia_THIS_LOCATION, "Updating = false", "Updating = true", propertyName, value));
			}
			C_Variant *v = M_GetProperty(propertyName);
			if (v)
			{
				C_Locker locker(&m_Private->m_ModifiedLock);
				C_Variant *mv = M_GetModifiedProperty(propertyName);
				if (mv) mv->M_SetValue(value);
				else
				{
					std::string pn = G_StringToLower(propertyName);
					mv = DBG_NEW C_Variant(value);
					m_Private->m_ModifiedPropertyList[pn] = C_SharedPointer<C_Variant>(mv);
					m_Private->m_ModifiedRawList.insert(pn);
				}
			} else
			{
				THROW(C_PropertyNotFoundException(Mia_THIS_LOCATION, propertyName));
			}
		}

		void C_DbClassInstanceClass::M_SetValue(const C_DbPropertyInfo &propertyInfo, const C_Variant& value)
		{
			M_SetValue(propertyInfo->M_GetName(), value);
		}

		void C_DbClassInstanceClass::M_AssignValue(const std::string &propertyName, const std::string &value)
		{
			std::string pn = G_StringToLower(propertyName);
			C_Locker locker(&m_Private->m_PropertyLock);
			unordered_map<std::string, C_SharedPointer<C_Variant> >::iterator piter = m_Private->m_Properties.find(pn);
			C_DbPropertyInfo dbi = m_Private->m_Class->M_GetProperty(propertyName);
			if (piter != m_Private->m_Properties.end())
			{
				C_Variant *oldv = piter->second.get();
				oldv->M_SetValue(value);
			} else
			{
				m_Private->m_Properties[pn] = C_SharedPointer<C_Variant>(new C_Variant(value, dbi->M_GetType())); // TODO: ADD CONVERSION TYPE HERE
			}
		}

		void C_DbClassInstanceClass::M_AssignValue(const C_DbPropertyInfo &propertyInfo, C_Variant &value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_AssignValue");
			std::string pn = G_StringToLower(propertyInfo->M_GetName());
			C_Locker locker(&m_Private->m_PropertyLock);
			unordered_map<std::string, C_SharedPointer<C_Variant> >::iterator piter = m_Private->m_Properties.find(pn);

			
			if (piter != m_Private->m_Properties.end())
			{
				C_Variant *oldv = piter->second.get();
				oldv->M_SetValue(value);
			} else
			{
				C_Variant *v = 0;
				if (value.M_GetType() != propertyInfo->M_GetType())
				{
					TRY
					{
						v = new C_Variant(value.M_ToString(), propertyInfo->M_GetType());
						if (!v->M_IsValid())
						{
							delete v;
							v = 0;
						}
					} 
					CATCH(C_Exception &ex,
					{
						//ex.M_RethrowTraceback(Mia_THIS_LOCATION, propertyInfo->M_GetType(), value);
						ex.M_SetHandled();
					})
				}

				if (!v)
				{
					v = new C_Variant(value);
				}

				m_Private->m_Properties[pn] = C_SharedPointer<C_Variant>(v);
			}
		}

		void C_DbClassInstanceClass::M_AssignValue(const C_DbPropertyInfo &propertyInfo, const std::string &value)
		{
			std::string pn = G_StringToLower(propertyInfo->M_GetName());
			C_Locker locker(&m_Private->m_PropertyLock);
			unordered_map<std::string, C_SharedPointer<C_Variant> >::iterator piter = m_Private->m_Properties.find(pn);

			if (piter != m_Private->m_Properties.end())
			{
				C_Variant *oldv = piter->second.get();
				oldv->M_SetValue(value);
			} else
			{
				C_Variant *newvar = new C_Variant(value, propertyInfo->M_GetType());
				if (!newvar->M_IsValid()) { *newvar = value; }
				m_Private->m_Properties[pn] = C_SharedPointer<C_Variant>(newvar);
			}
		}

		void C_DbClassInstanceClass::M_AssignValue(const std::string &propertyName)
		{
			std::string pn = G_StringToLower(propertyName);
			C_Locker locker(&m_Private->m_PropertyLock);
			unordered_map<std::string, C_SharedPointer<C_Variant> >::iterator piter = m_Private->m_Properties.find(pn);
			if (piter == m_Private->m_Properties.end())
			{
				m_Private->m_Properties[pn] = C_SharedPointer<C_Variant>(new C_Variant());
			}
		}

		C_Variant* C_DbClassInstanceClass::M_GetProperty(const std::string &propertyName)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetProperty");
			std::string pn = G_StringToLower(propertyName);
			C_Locker locker(&m_Private->m_PropertyLock);
			unordered_map<std::string, C_SharedPointer<C_Variant> >::iterator piter = m_Private->m_Properties.find(pn);
			if (piter != m_Private->m_Properties.end())
			{
				return piter->second.get();
			} else
			{
				static C_Variant invalid;
				THROW(C_PropertyNotFoundException(Mia_THIS_LOCATION, propertyName));
				NO_THROW(return &invalid;); 
			}
		}

		C_Variant* C_DbClassInstanceClass::M_GetModifiedProperty(const std::string &propertyName)
		{
			std::string pn = G_StringToLower(propertyName);
			unordered_map<std::string, C_SharedPointer<C_Variant> >::iterator piter = m_Private->m_ModifiedPropertyList.find(pn);
			if (piter != m_Private->m_ModifiedPropertyList.end())
			{
				return piter->second.get();
			} else
			{
				return 0;
			}
		}

		void C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo, C_Variant* value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			C_Variant *v = M_GetProperty(propertyInfo->M_GetName());
			if (v)
			{
				C_Variant *mv = M_GetModifiedProperty(propertyInfo->M_GetName());
				if (mv && mv->M_IsValid()) *value = *mv;
			} else
			{
				THROW(C_PropertyNotFoundException(Mia_THIS_LOCATION, propertyInfo->M_GetName()));
			}
		}

		void C_DbClassInstanceClass::M_GetValue(const std::string &propertyName, bool *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyName, value);
		}

		void C_DbClassInstanceClass::M_GetValue(const std::string &propertyName, char *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyName, value);
		}

		void C_DbClassInstanceClass::M_GetValue(const std::string &propertyName, unsigned char *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyName, value);
		}

		void C_DbClassInstanceClass::M_GetValue(const std::string &propertyName, short *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyName, value);
		}

		void C_DbClassInstanceClass::M_GetValue(const std::string &propertyName, unsigned short *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyName, value);
		}

		void C_DbClassInstanceClass::M_GetValue(const std::string &propertyName, int *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyName, value);
		}

		void C_DbClassInstanceClass::M_GetValue(const std::string &propertyName, uint *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyName, value);
		}

		void C_DbClassInstanceClass::M_GetValue(const std::string &propertyName, int64 *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyName, value);
		}

		void C_DbClassInstanceClass::M_GetValue(const std::string &propertyName, uint64 *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyName, value);
		}

		void C_DbClassInstanceClass::M_GetValue(const std::string &propertyName, float *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyName, value);
		}

		void C_DbClassInstanceClass::M_GetValue(const std::string &propertyName, double *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyName, value);
		}

		void C_DbClassInstanceClass::M_GetValue(const std::string &propertyName, std::string *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			C_Variant *v = M_GetProperty(propertyName);
			if (v)
			{
				C_Variant *mv = M_GetModifiedProperty(propertyName);

				if (mv && mv->M_IsValid())
				{
					*value = mv->M_ToString();
					return;
				}

				if (v->M_IsValid())
				{
					*value = v->M_ToString();
				}
			} else
			{
				THROW(C_PropertyNotFoundException(Mia_THIS_LOCATION, propertyName));
			}
		}

		void C_DbClassInstanceClass::M_GetValue(const std::string &propertyName, C_DateTime *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyName, value);
		}

		void C_DbClassInstanceClass::M_GetValue(const std::string &propertyName, C_Guid *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			C_Variant* v = M_GetProperty(propertyName);

			if (v)
			{
				if (m_Private->m_Updating)
				{
					C_Locker locker(&m_Private->m_ModifiedLock);
					C_Variant* mv = M_GetModifiedProperty(propertyName);
					if (mv && mv->M_IsValid()) { mv->M_GetValue(value); return; }
				}
				if (v->M_IsValid()) v->M_GetValue(value);
			} else
			{
				THROW(C_PropertyNotFoundException(Mia_THIS_LOCATION, propertyName));
			}
			//Mia_GET_PROPERTY(propertyName, value);
		}

		void C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo, bool *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyInfo->M_GetName(), value);
		}

		void C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo, char *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyInfo->M_GetName(), value);
		}

		void C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo, unsigned char *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyInfo->M_GetName(), value);
		}

		void C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo, short *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyInfo->M_GetName(), value);
		}

		void C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo, unsigned short *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyInfo->M_GetName(), value);
		}

		void C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo, int *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyInfo->M_GetName(), value);
		}

		void C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo, uint *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyInfo->M_GetName(), value);
		}

		void C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo, int64 *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyInfo->M_GetName(), value);
		}

		void C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo, uint64 *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyInfo->M_GetName(), value);
		}

		void C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo, float *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyInfo->M_GetName(), value);
		}

		void C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo, double *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyInfo->M_GetName(), value);
		}

		void C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo, std::string *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			C_Variant *v = M_GetProperty(propertyInfo->M_GetName());
			if (v)
			{
				C_Variant *mv = M_GetModifiedProperty(propertyInfo->M_GetName());

				if (mv && mv->M_IsValid())
				{
					*value = mv->M_ToString();
				}
			} else
			{
				THROW(C_PropertyNotFoundException(Mia_THIS_LOCATION, propertyInfo->M_GetName()));
			}
		}

		void C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo, C_DateTime *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyInfo->M_GetName(), value);
		}

		void C_DbClassInstanceClass::M_GetValue(const C_DbPropertyInfo &propertyInfo, C_Guid *value)
		{
			Mia_THIS_ROUTINE("C_DbClassInstanceClass::M_GetValue");
			Mia_GET_PROPERTY(propertyInfo->M_GetName(), value);
		}

		C_SharedPointer<C_Variant> C_DbClassInstanceClass::operator[](const std::string &propertyName)
		{
			return M_GetValue(propertyName);
		}

		C_SharedPointer<C_Variant> C_DbClassInstanceClass::operator[](const C_DbPropertyInfo &propertyInfo)
		{
			return M_GetValue(propertyInfo);
		}
	/** End C_DbClassInstance **/
	} /* namespace Mia */
} /* namespace ABB */
