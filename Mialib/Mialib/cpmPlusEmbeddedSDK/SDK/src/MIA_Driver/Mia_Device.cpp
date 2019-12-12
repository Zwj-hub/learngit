/*
 * Mia_Device.cpp
 *
 *  Created on: Apr 27, 2016
 *      Author: Tuan Vu
 */

#include "Mia_Device.h"
#include "Mia_Exception.h"
#include <functional>
#include <stdlib.h> 

#ifndef DEVICE_DIE_TIMEOUT
#define DEVICE_DIE_TIMEOUT 2000000000
#endif

namespace ABB
{
	namespace Mia
	{
		void C_Device::M_PropertyValueChanged(C_DeviceProperty* property, const C_Variant &value)
		{
			property->M_ValueChanged(value);
		}

		void C_Device::M_PropertyValueChanged(const std::string &propertyName, const std::string &value)
		{
			C_Locker locker(&m_DeviceLock);
			std::string key = propertyName;
			G_MIA_TRANSFORM_TO_LOWER(key);
			t_PropertyMap::iterator iter = m_PropertyMap.find(key);
			if (iter != m_PropertyMap.end())
			{
				C_DeviceProperty* property = iter->second.get();
				property->M_ValueChanged(C_Variant(value, property->M_GetType()));
			}
		}

		void C_Device::M_PropertyValueChanged(const std::string &propertyName, const C_Variant &value)
		{
			C_Locker locker(&m_DeviceLock);
			std::string key = propertyName;
			G_MIA_TRANSFORM_TO_LOWER(key);
			t_PropertyMap::iterator iter = m_PropertyMap.find(key);
			if (iter != m_PropertyMap.end())
			{
				C_DeviceProperty* property = iter->second.get();
				property->M_ValueChanged(value);
			}
		}

		void C_Device::M_PropertyValueChanged(C_DeviceProperty* property, const C_Variant &value,
		   const C_DateTime &timeValue, const t_ValueStatus &valueStatus)
		{
			property->M_ValueChanged(value, timeValue, valueStatus);
		}

		void C_Device::M_ToJson(C_Variant *di)
		{
			*di = C_Variant(g_OBJECT);
			(*di)["EquipmentId"] = this->M_GetId().M_ToString();
			(*di)["EquipmentType"] = this->M_GetEquipmentType();
			(*di)["EquipmentName"] = this->M_GetEquipmentName();
			(*di)["EquipmentCategory"] = this->M_GetEquipmentCategory();
			(*di)["EquipmentDescription"] = this->M_GetEquipmentDescription();

			C_Locker locker(&m_DeviceLock);

			// Subcribing properties
			if (m_PropertyMap.size())
			{
				C_Variant props(g_ARRAY, (int)m_PropertyMap.size());
				int i = 0;
				for (t_PropertyMap::iterator iter = m_PropertyMap.begin(); iter != m_PropertyMap.end(); iter++)
				{
					C_Variant value(g_OBJECT);
					iter->second->M_ToJson(&value);
					props[i++] = value;
				}
				(*di)["Properties"] = props;
			}

			// Subcribing events
			if (m_EventMap.size())
			{
				C_Variant eventjson(g_ARRAY, (int)m_EventMap.size());
				int i = 0;
				for (t_EventMap::iterator iter = m_EventMap.begin(); iter != m_EventMap.end(); iter++)
				{
					C_SharedPointer<C_EventSubscription> event = iter->second;
					C_Variant value(g_OBJECT);
					event->M_ToJson(&value);
					eventjson[i++] = value;
				}
				(*di)["Events"] = eventjson;
			}

			// Subcribing SubEquipment
			if (m_Subdevices.size())
			{
				C_Variant devices(g_ARRAY, (int)m_Subdevices.size());
				int i = 0;
				for (std::list<C_Device*>::iterator iter = m_Subdevices.begin(); iter != m_Subdevices.end(); iter++)
				{
					C_Variant device;
					C_Device *d = *iter;
					d->M_ToJson(&device);
					devices[i++] = device;
				}
				(*di)["SubEquipment"] = devices;
			}

			// Subcribing functions
			if (m_FunctionMap.size())
			{
				C_Variant functionjson(g_ARRAY, (int)m_FunctionMap.size());
				int i = 0;
				for (t_FunctionMap::iterator iter = m_FunctionMap.begin(); iter != m_FunctionMap.end(); iter++)
				{
					C_SharedPointer<C_FunctionSubscription> function = iter->second;
					C_Variant value(g_OBJECT);
					function->M_ToJson(&value);
					functionjson[i++] = value;
				}
				(*di)["Functions"] = functionjson;
			}
		}

		C_SharedPointer<C_EventSubscription> C_Device::M_GetEvent(const std::string &eventName)
		{
			C_Locker locker(&m_DeviceLock);
			return m_EventMap[eventName];
		}

		void C_Device::M_OnReconnect()
		{
			M_ResubscribeSession();
		}

		std::string C_Device::M_GetEquipmentCategory() const
		{
			return this->m_EquipmentCategory;
		}

		std::string C_Device::M_GetEquipmentDescription() const
		{
			return this->m_EquipmentDescription;
		}

		bool C_Device::M_IsAvailable() const
		{
			return true;
		}

		std::string C_Device::M_ToJson()
		{
			C_Variant dj;
			M_ToJson(&dj);

			return dj.M_ToString();
		}

		C_Device::C_Device(const std::string &equipmentName, const std::string &equipmentType) : C_Thread(std::string("C_Device_") + equipmentName), m_Driver(0), m_IsSubscriped(false)
		{
			m_EquipmentName = equipmentName;
			m_EquipmentType = equipmentType;
		}

		C_Device::~C_Device()
		{
			if (m_Running) M_Stop(true, DEVICE_DIE_TIMEOUT);

			if (m_PropertyMap.size()) m_PropertyMap.clear();
		}

		C_SharedPointer<C_Device> C_Device::M_S_FromJson(const std::string &json)
		{
			C_Variant value;

			if (C_Variant::M_S_FromJSON(json, value) && value.M_IsValid())
			{
				if (value["EquipmentName"].M_ToString().length() > 0 && value["EquipmentType"].M_ToString().length() > 0)
				{
					std::string eqname =		value["EquipmentName"].M_ToString();
					std::string eqtype =		value["EquipmentType"].M_ToString();
					std::string eqid =		value["EquipmentId"].M_ToString();
					std::string eqcat =		value["EquipmentCategory"].M_ToString();
					std::string eqdesc =		value["EquipmentDescription"].M_ToString();
					C_Variant properties =	value["Properties"];

					C_SharedPointer<C_Device> device = DBG_NEW C_Device(eqname, eqtype);
					device->m_Guid = C_Guid(eqid);
					device->m_EquipmentType = eqtype;
					device->m_EquipmentName = eqname;
					device->m_EquipmentCategory = eqcat;
					device->m_EquipmentDescription = eqdesc;

					if (properties.M_IsValid())
					{
						for (unsigned int i = 0; i < properties.M_GetArraySize(); i++)
						{
							C_Variant prop = properties[i];

							std::string propname =	prop["Name"].M_ToString();
							std::string proptype =	prop["Type"].M_ToString();
							G_MIA_TRANSFORM_TO_LOWER(proptype);
							std::string dirs =		prop["Direction"].M_ToString();
							G_MIA_TRANSFORM_TO_LOWER(dirs);
							C_DeviceProperty::t_Direction direction = (dirs == "in") ? (C_DeviceProperty::g_DIRECTION_TOSERVER) :
								(dirs == "out" ? C_DeviceProperty::g_DIRECTION_FROMSERVER : C_DeviceProperty::g_DIRECTION_BOTH);

							std::string category =		prop["Category"].M_ToString();
							std::string description =	prop["Description"].M_ToString();
							std::string propunit =		prop["Unit"].M_ToString();

							int64 bufferLength = -1;
							if (prop["BufferLength"].M_IsValid())
							{
								bufferLength = prop["BufferLength"].M_ToInt64();
							}

							C_Variant min, max;

							C_Variant temp = prop["MinimumValue"];
							if (temp.M_IsValid())
							{
								min = temp.M_ToDouble();
							}

							temp = prop["MaximumValue"];
							if (temp.M_IsValid())
							{
								max = temp.M_ToDouble();
							}

							C_SharedPointer<C_DeviceProperty> property = DBG_NEW C_DeviceProperty(device.get(),
								propname, 
								C_Variant::M_S_FromStringToPropertyType(proptype), 
								direction,
								category,
								description,
								propunit, 
								min, 
								max,
								bufferLength);
							temp = prop["Rate"];
							if (temp.M_IsValid())
							{
								int rate = temp.M_ToInt32();
								property->m_ProductionRate = rate;
							}
							device->M_AddProperty(property);
						}
					}
					return device;
				}
			}
			return C_SharedPointer<C_Device>();
		}

		void C_Device::M_OnDeviceUpdate(void *updateData)
		{
#ifdef THIS_REMOVED_BECAUSE_CAUSED_CRASHING_WHEN_SERVER_SEND_WIERD_DATA

			C_Variant datajson2(databuffer, g_ARRAY);
			if (!datajson2.M_GetArraySize()) return;

			const C_Variant &datajson = datajson2[0];
			M_OnDeviceUpdate(datajson[1]);
#endif
			MIA_OUT_WARNING << "C_Device::M_OnDeviceUpdate warning removed func" << std::endl;
		}

		void C_Device::M_OnDeviceUpdate(const C_Variant & dataobj)
		{
			std::list<std::string> properties = dataobj.M_GetProperties();
			for (std::list<std::string>::iterator propiter = properties.begin(); propiter != properties.end(); propiter++)
			{
				std::string operation = *propiter;
				const C_Variant &opdata = dataobj[operation];
				M_ProcessUpdate(operation, opdata);
			}
		}

		void C_Device::M_ProcessUpdate(const std::string & operation, const C_Variant & operationData)
		{
			if (operation == "Call")
			{
				M_ProcessCallRequest(operationData);
			}
			else if (operation == "Subscriptions")
			{
				M_ProcessSubscriptionsRequest(operationData);
			}
			else if (operation == "EventSubscriptions")
			{
				M_ProcessEventSubscriptionsRequest(operationData);
			}
			else if (operation == "Resend")
			{
				M_ProcessResendRequest(operationData);
			}
			else if (operation == "Change")
			{
				M_ProcessChangeRequest(operationData);
			}
		}

		void C_Device::M_ProcessCallRequest(const C_Variant & operationData)
		{
			int64 funcsub = G_MIA_STRTOLL(operationData[0].M_ToString().c_str());
			t_FunctionSubsriptionMap::const_iterator iter = m_FunctionSubsMap.find(funcsub);
			if (iter != m_FunctionSubsMap.end())
			{
				int64 returnid = G_MIA_STRTOLL(operationData[1].M_ToString().c_str());
				C_SharedPointer<C_FunctionSubscription> sub = iter->second;
				sub->M_OnFunctionCalled(funcsub, returnid, operationData);
			}
		}

		void C_Device::M_ProcessResendRequest(const C_Variant & operationData)
		{
			const C_Variant& val = operationData[0];
			int id = atoi(val["Id"].M_ToString().c_str());

			for (t_PropertyMap::iterator iter = m_PropertyMap.begin(); iter != m_PropertyMap.end(); iter++)
			{
				C_SharedPointer<C_DeviceProperty> prop = iter->second;
				if (id == prop->M_GetId() && prop->m_DataHandler)
				{
					C_DateTime starttime = C_DateTime::M_S_FromTicks(G_MIA_STRTOLL(operationData[1].M_ToString().c_str()));
					C_DateTime stoptime = C_DateTime::M_S_FromTicks(G_MIA_STRTOLL(operationData.M_ToString().c_str()));
					prop->m_DataHandler(prop.get(), starttime, stoptime);
				}
			}
		}

		void C_Device::M_ProcessChangeRequest(const C_Variant & operationData)
		{
			const C_Variant& val = operationData[0];
			int id = atoi(val["Id"].M_ToString().c_str());

			for (t_PropertyMap::iterator iter = m_PropertyMap.begin(); iter != m_PropertyMap.end(); iter++)
			{
				C_SharedPointer<C_DeviceProperty> prop = iter->second;
				if (id == prop->M_GetId() && prop->m_ChangeHandler)
				{
					const C_Variant data = operationData[1];
					C_DateTime changetime = C_DateTime::M_S_FromTicks(G_MIA_STRTOLL(operationData[2].M_ToString().c_str()));
					t_ValueStatus status = (t_ValueStatus)operationData[3].M_ToInt32();
					C_DateTime prevtime = C_DateTime::M_S_FromTicks(G_MIA_STRTOLL(operationData[4].M_ToString().c_str()));
					prop->m_ChangeHandler(prop.get(), data, changetime, status, prevtime);
				}
			}
		}

		void C_Device::M_ProcessSubscriptionsRequest(const C_Variant & operationData)
		{
			for (unsigned int i = 0; i < operationData.M_GetArraySize(); i++)
			{
				const C_Variant& val = operationData[i];
				int id = atoi(val["Id"].M_ToString().c_str());
				M_AddSubscribedProperty(id, val);
			}
		}

		void C_Device::M_ProcessEventSubscriptionsRequest(const C_Variant & operationData)
		{
			for (unsigned int i = 0; i < operationData.M_GetArraySize(); i++)
			{
				const C_Variant& val = operationData[i];
				int id = atoi(val["EventId"].M_ToString().c_str()); // XXX why ID is double???
				M_AddSubscribedEvent(id, val);
			}
		}

		C_SharedPointer<C_DeviceProperty> C_Device::M_GetProperty(const std::string &propertyName)
		{
			std::string key(propertyName);
			G_MIA_TRANSFORM_TO_LOWER(key);

			C_Locker locker(&m_DeviceLock);
			t_PropertyMap::iterator iter = m_PropertyMap.find(key);
			
			if(iter != m_PropertyMap.end())
			{
				return iter->second;
			}
			else
			{
				return NULL;
			}		}

		void C_Device::M_AddSubdevice(C_Device* subdevice)
		{
			C_Locker locker(&m_DeviceLock);
			m_Subdevices.push_back(subdevice);
		}
		void C_Device::M_Routine()
		{
			m_OnValueProduced.M_Wait(100);
			m_OnValueProduced.M_Unset();
			{
				C_Locker locker(&m_DeviceLock);
				for (t_PropertyMap::iterator iter = m_PropertyMap.begin(); iter != m_PropertyMap.end(); iter++)
				{
					C_DeviceProperty* pro = iter->second.get();
					if (pro) pro->M_Produce();
				}
			}
			{
				for (t_FunctionMap::iterator iter = m_FunctionMap.begin(); iter != m_FunctionMap.end(); iter++)
				{
					C_SharedPointer<C_FunctionSubscription> subs = iter->second;
					subs->M_ProcessPending();
				}
			}
		}

		void C_Device::M_StartProduction()
		{
			if (!m_Running)
			{
				m_Driver->M_AddOnReconnect(this);
				M_Loop(100);
			}
		}

		void C_Device::M_StopProduction()
		{
			M_Stop(true, DEVICE_DIE_TIMEOUT);
		}

		void C_Device::M_SignalOnValue()
		{
			m_OnValueProduced.M_Set();
		}

		void C_Device::M_AddProperty(C_DeviceProperty *property)
		{
			Mia_THIS_ROUTINE("C_Device::M_AddProperty");
			if (property->m_Device && property->m_Device != this)
			{
				THROW(C_DevicePropertyAlreadyUsedException(Mia_THIS_LOCATION, property->m_Name, property->m_Description));
			}

			C_SharedPointer<C_DeviceProperty> prop(property);
			this->M_AddProperty(prop);
		}

		void C_Device::M_AddProperty(C_SharedPointer<C_DeviceProperty>& property)
		{
			Mia_THIS_ROUTINE("C_Device::M_AddProperty");
			if (property->m_Device && property->m_Device != this)
			{
				std::cout << property->m_Device << std::endl;
				THROW(C_DevicePropertyAlreadyUsedException(Mia_THIS_LOCATION, property->m_Name, property->m_Description));
			}

			C_Locker locker(&m_DeviceLock);
			property->M_SetDevice(this);

			std::string key = property->M_GetName();
			G_MIA_TRANSFORM_TO_LOWER(key);
			m_PropertyMap[key] = property;
		}

		C_SharedPointer<C_DeviceProperty> C_Device::M_AddProperty(const std::string &name,
			const t_PropertyType type,
			const C_DeviceProperty::t_Direction direction,
			const std::string &category,
			const std::string &description,
			const std::string &unit,
			const C_Variant& min,
			const C_Variant& max,
			int64 bufferLengthMs)
		{
			C_SharedPointer<C_DeviceProperty> property = DBG_NEW C_DeviceProperty(this, name, type,
				direction, category, description, unit, min, max);;
			C_Locker locker(&m_DeviceLock);

			std::string key = name;
			G_MIA_TRANSFORM_TO_LOWER(key);
			if (m_PropertyMap.find(key) == m_PropertyMap.end())
			{
				m_PropertyMap[key] = property;
			}
			return property;
		}

		C_SharedPointer<C_DeviceProperty> C_Device::M_AddProperty(const std::string &name,
			const C_Variant& staticValue,
			const t_PropertyType type,
			const std::string &category,
			const std::string &description,
			const std::string &unit)
		{
			C_SharedPointer<C_DeviceProperty> property = DBG_NEW C_DeviceProperty(this, name, staticValue,
				type, category, description, unit);;
			C_Locker locker(&m_DeviceLock);

			std::string key = name;
			G_MIA_TRANSFORM_TO_LOWER(key);
			m_PropertyMap[key] = property;

			return property;
		}

		C_SharedPointer<C_EventSubscription> C_Device::M_AddEventSubscription(C_EventSubscription* event)
		{
			C_Locker locker(&m_DeviceLock);
			event->M_SetDevice(this);
			C_SharedPointer<C_EventSubscription> cpe(event);
			std::string key = event->M_GetName();
			G_MIA_TRANSFORM_TO_LOWER(key);
			m_EventMap[key] = cpe;

			return cpe;
		}

		C_SharedPointer<C_EventSubscription>  C_Device::M_AddEventSubscription(const std::string &eventName)
		{
			C_Locker locker(&m_DeviceLock);
			C_SharedPointer<C_EventSubscription> cpe = DBG_NEW C_EventSubscription(eventName);
			cpe->M_SetDevice(this);
			std::string key = eventName;
			G_MIA_TRANSFORM_TO_LOWER(key);
			m_EventMap[key] = cpe;

			return cpe;
		}

		void C_Device::M_AddFunctionSubscription(C_SharedPointer<C_FunctionSubscription> function)
		{
			C_Locker locker(&m_DeviceLock);
			function->M_SetDevice(this);

			std::string key = function->M_GetName();
			G_MIA_TRANSFORM_TO_LOWER(key);

			m_FunctionMap[key] = function;
			m_FunctionSubsMap[function->M_GetId()] = function;
		}

		C_SharedPointer<C_FunctionSubscription>  C_Device::M_AddFunctionSubscription(const std::string &functionName,
			const C_Variant& params,
			C_FunctionSubscription::F_Callback callback)
		{
			C_Locker locker(&m_DeviceLock);
			C_SharedPointer<C_FunctionSubscription> func = DBG_NEW C_FunctionSubscription(functionName, params, callback);
			M_AddFunctionSubscription(func);
			return func;
		}

		C_SharedPointer<C_FunctionSubscription> C_Device::M_AddFunctionSubscription(const std::string & functionName, const C_FunctionSubscription::t_FunctionArguments & params, C_FunctionSubscription::F_Callback callback)
		{
			typedef  C_FunctionSubscription::t_FunctionArguments::const_iterator t_Iterator;
			C_Locker locker(&m_DeviceLock);
			C_Variant params_object(g_ARRAY, (int)params.size());
			
			for (uint i = 0; i < params.size(); i++)
			{ 
				C_Variant obj(g_OBJECT);
				C_FunctionSubscription::t_FunctionArgument arg = params[i];
				obj["Name"] = arg.m_Name;
				obj["Type"] = C_Variant::M_S_FromDataTypeToString(arg.m_Type);
				if (arg.m_Description.length()) obj["Description"] = arg.m_Description;
				if (arg.m_Unit.length()) obj["Unit"] = arg.m_Unit;
				if (arg.m_MinValue.M_IsValid()) obj["MinimumValue"] = arg.m_MinValue;
				if (arg.m_MaxValue.M_IsValid()) obj["MaximumValue"] = arg.m_MaxValue;
				params_object.M_SetValue(i, obj);
			}

			C_SharedPointer<C_FunctionSubscription> func = DBG_NEW C_FunctionSubscription(functionName, params_object, callback);
			M_AddFunctionSubscription(func);
			return func;
		}

		void C_Device::M_ResubscribeSession()
		{
			if (m_Driver)
			{
				for (t_PropertyMap::iterator iter = m_PropertyMap.begin(); iter != m_PropertyMap.end(); iter++)
				{
					C_SharedPointer<C_DeviceProperty> prop = iter->second;
					if(prop)
						prop->m_IsSubscribed = false;
				}
				m_Driver->M_SubscribeEquipmentSession(this);
			}
		}

		C_Device::t_PropertyMap &C_Device::M_GetPropertyMap()
		{
			return m_PropertyMap;
		}
	
		C_Device::t_EventMap &C_Device::M_GetEventMap()
		{
			return m_EventMap;
		}

		bool C_Device::M_IsSubscripted()
		{
			return m_IsSubscriped;
		}

		C_Device::t_FunctionMap &C_Device::M_GetFunctionMap()
		{
			return m_FunctionMap;
		}

		C_ThreadLock &C_Device::M_GetDeviceLock()
		{
			return m_DeviceLock;
		}

		void C_Device::M_AddSubscribedProperty(const std::string &property)
		{
			C_SharedPointer<C_DeviceProperty> prop = this->M_GetProperty(property);
			if (prop)
			{
				prop->m_IsSubscribed = true;
			}
		}

		void C_Device::M_AddSubscribedProperty(const int &subscriptionId, const C_Variant& subscriptionData)
		{
			C_Locker locker(&m_DeviceLock);

			for (t_PropertyMap::iterator iter = m_PropertyMap.begin(); iter != m_PropertyMap.end(); iter++)
			{
				C_SharedPointer<C_DeviceProperty> prop = iter->second;
				if (prop->m_PusherId == subscriptionId)
				{
					prop->m_IsSubscribed = true;
					prop->m_Driver = this->m_Driver;
					if (subscriptionData["SamplingInterval"].M_IsValid())
					{
						prop->m_SamplingIntervalMs = G_MIA_STRTOLL(subscriptionData["SamplingInterval"].M_ToString().c_str());					}

					if (subscriptionData["PublishingInterval"].M_IsValid())
					{
						prop->m_PublishingIntervalMs = G_MIA_STRTOLL(subscriptionData["PublishingInterval"].M_ToString().c_str());					}

					if (subscriptionData["BufferLength"].M_IsValid())
					{
						prop->m_BufferLength = G_MIA_STRTOLL(subscriptionData["BufferLength"].M_ToString().c_str());
					}
				}
			}
		}

		void C_Device::M_SetDriver(C_MiaDriver *driver)
		{
			m_Driver = driver;
		}

		void C_Device::M_AddSubscribedEvent(const int &eventId, const C_Variant& eventData)
		{
			C_Locker locker(&m_DeviceLock);

			for (t_EventMap::iterator iter = m_EventMap.begin(); iter != m_EventMap.end(); iter++)
			{
				C_SharedPointer<C_EventSubscription> evsubs = iter->second;

				if (evsubs->m_EventId == eventId)
				{
					evsubs->m_IsSubscribed = true;
				}
			}
		}

		C_DeviceProperty::C_DeviceProperty(C_Device* device, 
			const std::string &name, 
			const t_PropertyType type, 
			const t_Direction direction,
			const std::string &category,
			const std::string &description,
			const std::string &unit, 
			const C_Variant& min,
			const C_Variant& max,
			int64 bufferLengthMs) : m_IsSubscribed(false)
		{
			m_PusherId = C_MiaDriverBase::M_S_GetCallId();
			m_ProductionRate = 1;
			m_Driver = 0;
			m_Name = name;
			m_Type = type;
			m_Direction = direction;
			m_Category = category;
			m_Description = description;
			m_Unit = unit;
			m_ValueMin = min;
			m_ValueMax = max;
			m_BufferLengthMs = bufferLengthMs;
			if (device)
			{
				m_Device = device;
			}

			m_IsStatic = false;
		}

		C_DeviceProperty::C_DeviceProperty(C_Device* device,
			const std::string &name,
			const C_Variant& staticValue,
			const t_PropertyType type,
			const std::string &category,
			const std::string &description,
			const std::string &unit,
			const C_Variant& min,
			const C_Variant& max)
		{
			m_PusherId = C_MiaDriverBase::M_S_GetCallId();
			m_Driver = 0;
			m_Name = name;
			m_Value = staticValue;
			m_IsStatic = true;
			if (type != g_PT_UNKNOWN)
			{
				m_Type = type;
			}
			else
			{
				m_Type = C_Variant::M_S_FromStringToPropertyType(m_Value.M_GetTypeName());
			}
			m_Category = category;
			m_Description = description;
			m_Unit = unit;
			m_ValueMin = min;
			m_ValueMax = max;
			if (device) m_Device = device;
		}

		void C_DeviceProperty::M_SetDevice(C_Device* device) { m_Device = device;}

		void C_DeviceProperty::M_ValueChanged(const C_Variant &value, const C_DateTime &timeval, const t_ValueStatus &status)
		{
			{
				C_Locker locker(&m_DataLock);
				m_CurrentValue = value;
				m_LastUpdateTime = C_DateTime::M_S_Now();
			}

			if (m_IsSubscribed && m_Driver && m_Driver->M_GetStatus() == C_MiaDriver::g_READY)
			{
				std::string message;
				C_WebsocketStream ws;
				ws.M_BeginArray();
				ws << m_PusherId;
				ws.M_NextDirective();
				ws.M_BeginArray();
				ws << value;
				ws.M_NextDirective();
				ws << timeval;
				if (status != g_VS_OK)
				{
					ws.M_NextDirective();
					ws << (int) status;
				}
				ws.M_EndArray();
				ws.M_EndArray();
				m_Driver->M_Send(ws.M_GetString());

				MIA_OUT_DEBUG << M_GetName() << ": " << ws.M_GetString()<< std::endl;
			}
			else
			{
				MIA_OUT_DEBUG << M_GetName() << " is in bad condition"<< std::endl;
			}
		}

		void C_DeviceProperty::M_ValueChanged(const C_Variant &value)
		{
			{
				C_Locker locker(&m_DataLock);
				m_CurrentValue = value;
				m_LastUpdateTime = C_DateTime::M_S_Now();
			}

			if (m_IsSubscribed && m_Driver && m_Driver->M_GetStatus() == C_MiaDriver::g_READY)
			{
				C_Variant data(g_ARRAY, 2);
				std::vector<char> buffer;
				data[0] = m_PusherId;
				data[1] = C_Variant(g_ARRAY, 1);
				data[1][0] = value;
				buffer.reserve(32);
				data.M_ToBinary(buffer);
				if (!buffer.empty()) m_Driver->M_Send(&buffer[0], buffer.size());
				MIA_OUT_DEBUG << M_GetName() << ": " << data.M_ToString() << std::endl;
			}
			else
			{
				MIA_OUT_DEBUG << M_GetName() << " is in bad condition" << std::endl;				
			}
		}

		void C_DeviceProperty::M_Produce()
		{
			C_Locker locker(&m_DataLock);
			if (m_IsSubscribed && m_Driver && m_Driver->M_GetStatus() == C_MiaDriver::g_READY
			   && C_DateTime::M_S_Now() > m_LastProductionTime.M_AddMilisecond(m_ProductionRate)
			   && m_LastUpdateTime > m_LastProductionTime)
			{
				m_LastProductionTime = C_DateTime::M_S_Now();
				std::string message;
				C_WebsocketStream ws;
				ws.M_BeginArray();
				ws << m_PusherId;
				ws.M_NextDirective();
				ws.M_BeginArray();
				ws << m_CurrentValue;
				ws.M_EndArray();
				ws.M_EndArray();
				m_Driver->M_Send(ws.M_GetString());
			}
		}

		const C_Variant &C_DeviceProperty::M_GetCurrentValue() { return m_CurrentValue;}

		bool C_DeviceProperty::M_IsSubscribed() const { return m_IsSubscribed;}

		void C_DeviceProperty::M_SetMinimumRateMs(int minimumRate) { m_ProductionRate = minimumRate;}

		void C_DeviceProperty::M_SetDataHandler(F_DataHandler handler) { m_DataHandler = handler; }

		void C_DeviceProperty::M_ToJson(C_Variant *pj)
		{
			if (m_IsStatic)
			{
				if (m_PusherId) (*pj)["Id"] = m_PusherId;
				(*pj)["Name"] = this->M_GetName();
				if (M_GetType() != g_UNKNOWN)	(*pj)["Type"] = C_Variant::M_S_FromPropertyTypeToString(M_GetType());
				(*pj)["Value"] = m_Value;
			}
			else
			{
				(*pj)["Id"] = m_PusherId;
				(*pj)["Name"] = this->M_GetName();
				(*pj)["Type"] = C_Variant::M_S_FromPropertyTypeToString(M_GetType());
			}

			if (this->M_GetDirection() != C_DeviceProperty::g_DIRECTION_TOSERVER)
			{
				(*pj)["Direction"] = (this->M_GetDirection() == C_DeviceProperty::g_DIRECTION_FROMSERVER) ?
					"Out" : "In/Out";
			}
			else
			{
				(*pj)["Direction"] = "In";
			}

			if (this->M_GetCategory().length())
			{
				(*pj)["Category"] = this->M_GetCategory();
			}

			if (this->M_GetDescription().length())
			{
				(*pj)["Description"] = this->M_GetDescription();
			}

			if (this->M_GetUnit().length())
			{
				(*pj)["Unit"] = this->M_GetUnit();
			}

			C_Variant min = this->M_GetMin();
			if (min.M_IsValid())
			{
				(*pj)["MinimumValue"] = min.M_ToString();
			}

			C_Variant max = this->M_GetMax();
			if (max.M_IsValid())
			{
				(*pj)["MaximumValue"] = max.M_ToString();
			}

			if (m_BufferLengthMs > -1)
			{
				(*pj)["MaximumValue"] = m_BufferLengthMs;
			}
		}

		std::string C_DeviceProperty::M_ToJson()
		{
			C_Variant pj;
			M_ToJson(&pj);

			return pj.M_ToString();
		}

		C_DeviceEvent::C_DeviceEvent(const std::string &message) : m_EventData(g_OBJECT)
		{
			m_Message = message;
			m_EventTime = 0;
		}

		C_DeviceEvent::C_DeviceEvent(const std::string &message, const C_DateTime &eventTime) : m_EventData(g_OBJECT)
		{
			m_Message = message;
			m_EventTime = DBG_NEW C_DateTime(eventTime);
		}

		C_DeviceEvent::C_DeviceEvent(const std::string &message, const C_Variant& data) : m_EventTime(0)
		{
			m_Message = message;

			if (data.M_GetType() == g_OBJECT) m_EventData = data;
			else m_EventData = C_Variant(g_OBJECT);
		}

		C_DeviceEvent::C_DeviceEvent(const std::string &message, const C_Variant& data, const C_DateTime &eventTime)
		{
			m_Message = message;

			if (data.M_GetType() == g_OBJECT) m_EventData = data;
			else m_EventData = C_Variant(g_OBJECT);

			m_EventTime = DBG_NEW C_DateTime(eventTime);
		}

		C_DeviceEvent::~C_DeviceEvent()
		{
			if (m_EventTime) delete m_EventTime;
		}

		void C_DeviceEvent::M_SetData(const std::string &metadata, const C_Variant &data)
		{
			m_EventData[metadata] = data;
		}

		C_Variant &C_DeviceEvent::operator[](const std::string &metadata)
		{
			return m_EventData[metadata];
		}

		void C_DeviceEvent::M_SetTime(const C_DateTime &time)
		{
			if (m_EventTime)
			{
				*m_EventTime = time;
			} else
			{
				m_EventTime = DBG_NEW C_DateTime(time);
			}
		}

		std::string C_DeviceEvent::M_ToJson()
		{
			std::string ret;
			C_Variant value(g_ARRAY);
			M_ToJson(&value);

			return value.M_ToString();
		}

		void C_DeviceEvent::M_ToJson(C_Variant *value)
		{
			(*value)["Id"] = m_Subscription->m_EventId;
			C_Variant events(g_ARRAY);
			C_Variant thisevent(g_ARRAY);
			thisevent[0] = this->M_GetMessage();
			if (m_EventData.M_IsValid() || m_EventTime) thisevent[1] = m_EventData;
			if (m_EventTime) thisevent[2] = *m_EventTime;
		}

		C_EventSubscription::C_EventSubscription(const std::string &eventName) :  
			m_IsSubscribed(false), 
			m_EventName(eventName)
		{
			m_EventId = C_MiaDriver::M_S_GetCallId();
		}

		C_SharedPointer<C_DeviceEvent> C_EventSubscription::M_CreateEvent(const std::string &message)
		{
			C_SharedPointer<C_DeviceEvent> event = DBG_NEW C_DeviceEvent(message);
			event->m_Subscription = this;
			return event;
		}

		C_SharedPointer<C_DeviceEvent> C_EventSubscription::M_CreateEvent(const std::string &message, const C_DateTime &eventTime)
		{
			C_SharedPointer<C_DeviceEvent> event = DBG_NEW C_DeviceEvent(message, eventTime);
			event->m_Subscription = this;
			return event;
		}

		C_SharedPointer<C_DeviceEvent> C_EventSubscription::M_CreateEvent(const std::string &message, const C_Variant &data)
		{
			C_SharedPointer<C_DeviceEvent> event = DBG_NEW C_DeviceEvent(message, data);
			event->m_Subscription = this;
			return event;
		}
		
		C_SharedPointer<C_DeviceEvent> C_EventSubscription::M_CreateEvent(const std::string &message,
			const C_Variant &data, 
			const C_DateTime &eventTime)
		{
			C_SharedPointer<C_DeviceEvent> event = DBG_NEW C_DeviceEvent(message, data, eventTime);
			event->m_Subscription = this;
			return event;
		}

		void C_EventSubscription::M_SetDevice(C_Device* device) { m_Device = device;}

		const std::string &C_EventSubscription::M_GetName() const { return m_EventName;}

		void C_EventSubscription::M_ToJson(C_Variant *value)
		{
			(*value)["Id"] = this->m_EventId;
			(*value)["Name"] = this->m_EventName;
		}

		void C_EventSubscription::M_SendEvent(C_SharedPointer<C_DeviceEvent> &eventData)
		{
			Mia_THIS_ROUTINE("C_EventSubscription::M_SendEvent");

			if (!m_IsSubscribed)
			{
				THROW(C_InvalidStateException(Mia_THIS_LOCATION, eventData->M_ToJson(), "Subscribed == true"));
			}

			TRY
			{
				std::string message;
				C_WebsocketStream ws;
				ws.M_BeginArray();
				ws << m_EventId;
				ws.M_NextDirective();
				ws.M_BeginArray();
				ws.M_BeginArray();
				ws << eventData->M_GetMessage();
				ws.M_NextDirective();
				ws.M_AppendRaw(eventData->m_EventData.M_ToString());
				if (eventData->m_EventTime && *(eventData->m_EventTime) != C_DateTime())
				{
					ws.M_NextDirective();
					ws << eventData->M_GetTime(); 
				}
				ws.M_EndArray();
				ws.M_EndArray();
				ws.M_EndArray();
				std::string msg = ws.M_GetString();

				MIA_OUT_DEBUG << "[C_EventSubscription::M_SendEvent]: " << msg << std::endl;

				m_Device->m_Driver->M_Send(msg);
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION);
			})
		}

		bool	C_EventSubscription::M_IsSubscribed() const
		{
			return m_IsSubscribed;
		}

		C_FunctionSubscription::C_FunctionSubscription(const std::string &name, 
			const C_Variant &params,
			F_Callback callback)
			: m_Device(0), m_IsSubscribed(false), m_FunctionName(name), m_ParamMetadata(params)
		{
			m_Callback = callback;
			m_FunctionId = C_MiaDriver::M_S_GetCallId();
		}

		void C_FunctionSubscription::M_SetDevice(C_Device* device) { m_Device = device;}

		const std::string &C_FunctionSubscription::M_GetName() const { return m_FunctionName;}

		bool C_FunctionSubscription::M_IsSubscribed() const { return m_IsSubscribed; }

		t_CallId  C_FunctionSubscription::M_GetId() const { return m_FunctionId; }

		void C_FunctionSubscription::M_ToJson(C_Variant *value)
		{
			if (value->M_GetType() != g_OBJECT)
			{
				*value = C_Variant(g_OBJECT);
			}

			(*value)["Id"] = this->m_FunctionId;
			(*value)["Name"] = this->m_FunctionName;
			(*value)["Parameters"] = m_ParamMetadata;
		}

		C_Device* C_FunctionSubscription::M_GetDevice()
		{
			return m_Device;
		}

		void C_FunctionSubscription::M_ProcessPending()
		{
			Mia_THIS_ROUTINE("C_FunctionSubscription::M_ProcessPending");

			if (m_Pendings.empty())
			{
				return;
			}

			TRY
			{
				std::pair<int, ABB::Mia::C_ArgumentList> func_arg;
				{
					C_Locker locker(&m_FunctionLock);
					func_arg = m_Pendings.front();
					m_Pendings.pop_front();
				}

				int64 returnID = func_arg.first;
				ABB::Mia::C_ArgumentList& params = func_arg.second;
				m_Callback(this, params);

				C_WebsocketStream ws;
				ws.M_BeginArray();
				{
					ws << m_FunctionId;
					ws.M_NextDirective();
					ws.M_BeginArray();
					{
						ws << returnID;
						ws.M_NextDirective();
						int i = 0;
						for (ABB::Mia::C_ArgumentList::const_iterator iter = params.begin(); iter != params.end(); iter++)
						{
							C_SharedPointer<C_Variant> data = *iter;
							ws<<*(data.get());
							if (++i != params.size())  ws.M_NextDirective();
						}
					}
					ws.M_EndArray();
				}
				ws.M_EndArray();
				std::string msg = ws.M_GetString();
				MIA_OUT_DEBUG << "[C_FunctionSubscription::M_ProcessPending]: " << msg << std::endl;
				m_Device->m_Driver->M_Send(msg);
			}
			CATCH (C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION);
			})
		}

		size_t C_FunctionSubscription::M_PendingCount() const
		{
			return m_Pendings.size();
		}

		C_Variant &ABB::Mia::C_FunctionSubscription::operator[](const unsigned int & index)
		{
			return m_ParamMetadata[index];
		}

		C_Variant & ABB::Mia::C_FunctionSubscription::operator[](const std::string & paramName)
		{
			for (unsigned int i = 0; i < m_ParamMetadata.M_GetArraySize(); i++)
			{
				if (m_ParamMetadata[i]["Name"] == paramName)
				{
					return m_ParamMetadata[i];
				}
			}
			return C_Variant::M_S_Null();
		}

		void ABB::Mia::C_FunctionSubscription::M_SetHandler(F_Callback callback)
		{
			m_Callback = callback;
		}

		void ABB::Mia::C_FunctionSubscription::M_SetParam(const uint & index, const C_FunctionSubscription::t_FunctionArgument &funcArgument)
		{
			C_Variant obj(g_OBJECT);
			obj["Name"] = funcArgument.m_Name;
			obj["Type"] = C_Variant::M_S_FromDataTypeToString(funcArgument.m_Type);
			if (funcArgument.m_Description.length())	obj["Description"] = funcArgument.m_Description;
			if (funcArgument.m_Unit.length())			obj["Unit"] = funcArgument.m_Unit;
			if (funcArgument.m_MinValue.M_IsValid())	obj["MinimumValue"] = funcArgument.m_MinValue;
			if (funcArgument.m_MaxValue.M_IsValid())	obj["MaximumValue"] = funcArgument.m_MaxValue;
			this->m_ParamMetadata[index] = obj;
		}

		bool	ABB::Mia::C_FunctionSubscription::M_Execute(t_ArgumentValues arguments)
		{
			C_SharedPointer<C_Variant> results;
			const std::string path = "Path_" + this->m_Device->M_GetEquipmentType() + "!" + this->M_GetName();
			C_MiaDriver *driver = this->m_Device->m_Driver;
			ABB::Mia::C_DbClass modelClass = driver->M_GetClass(path);
			ABB::Mia::C_DbClassInstance f = modelClass->M_Add();

			f->M_SetValue("Equipment", ABB::Mia::C_Variant(this->M_GetName()));
			std::string propertyvalues = "";
			std::string properties = "";
			properties = "Equipment";
			propertyvalues = "\"" + this->m_Device->M_GetEquipmentName() + "\"";
			for (C_FunctionSubscription::t_ArgumentValues::const_iterator iter = arguments.begin(); 
				iter != arguments.end(); iter++)
			{
				properties += ("," + iter->m_Name);
				if (iter->m_Name.length() > 2 && iter->m_Name.substr(0, 3) == "RAW")
				{
					propertyvalues += ("," + iter->m_Variant.M_ToString());
				}
				else
				{
					propertyvalues += (",\"" + iter->m_Variant.M_ToString() + "\"");
				}
			}
			
			std::string result = driver->M_CommitClassDataRaw(path, properties, "", propertyvalues, "");
			C_Variant retsult(result);
			if (retsult.M_GetType() != g_ARRAY && retsult.M_GetArraySize() != arguments.size() + 1) return false;
			for (int i = 1; i <= arguments.size(); i++)
			{
				arguments[i - 1].m_Variant = result[i];
			}

			return true;
		}

		void C_FunctionSubscription::M_OnFunctionCalled(int64 callid, int64 returnId,  const C_Variant& functionArguments)
		{
			C_ArgumentList arguments;
			for (unsigned int i = 0; i < (unsigned int)this->m_ParamMetadata.M_GetArraySize(); ++i)
			{
				const C_Variant& value = functionArguments[i + 2];
				C_SharedPointer<C_Variant> variant;
				if (m_ParamMetadata[i]["Type"].M_ToString() == "DateTime")
				{
					variant = DBG_NEW C_Variant(value);
				}
				else
				{
					variant = DBG_NEW C_Variant(value.M_ToString(), C_Variant::M_S_FromStringToDataType(m_ParamMetadata[i]["Type"].M_ToString())); // convert value
				}
				arguments.push_back(variant);
			}

			this->M_AddPending(returnId, arguments);
		}

		void C_FunctionSubscription::M_AddPending(int64 returnID, const C_ArgumentList& params)
		{
			C_Locker locker(&m_FunctionLock);
			m_Pendings.push_back(std::make_pair((int)returnID, params));
		}
	}
}
