/*
 * Mia_Device.h
 *
 *  Created on: Apr 27, 2016
 *      Author: Tuan Vu
 */

#ifndef _MIA_DEVICE_H_
#define _MIA_DEVICE_H_

#include <string>
#include <list>
#include <assert.h>
#include "Mia_Base.h"

#include "Mia_Driver.h"
#include "Mia_DriverBase.h"
namespace ABB
{
	namespace Mia
	{
		class C_EventSubscription;
		class C_DeviceEvent;
		class C_FunctionSubscription;
		class C_DeviceProperty;
		class C_Device;

		/**
		* @brief This class defines the device interface.
		* Client implements this device interface and publish the device to the server using this interface.
		*
		* \ingroup EquipmentAPI
		*/
		class Mia_EXPORT C_DeviceEvent
		{
			/**
			* @brief Create a device event with the message
			*/
			protected: C_DeviceEvent(const std::string &message);

			/**
			* @brief Create a device event with the message and eventTime
			* @param message Event message
			* @param eventTime Event time
			*/
			protected: C_DeviceEvent(const std::string &message, const C_DateTime &eventTime);

			/**
			* @brief Create a device event with the message and eventTime
			* @param message	 Event message
			* @param data		 Event data, must be of type Object, otherwise will be discarded
			*/
			protected: C_DeviceEvent(const std::string &message, const C_Variant &data);


			/**
			* @brief Create a device event with the message and eventTime
			* @param subscription A pointer to C_EventSubscription
			* @param message	 Event message
			* @param data		 Event data, must be of type Object, otherwise will be discarded
			* @param eventTime Event time
			*/
			protected: C_DeviceEvent(const std::string &message, const C_Variant &data, const C_DateTime &eventTime);

			public: virtual ~C_DeviceEvent();

			/**
			* @brief Add event data
			* @param metadata Event metadata
			* @param data Event data
			*/
			public: void M_SetData(const std::string &metadata, const C_Variant &data);

			/**
			* @brief Access event data defined by metadata
			* @param metadata Event metadata
			*/
			public: C_Variant &operator[](const std::string &metadata);

			/**
			* @brief Set event time
			* @param time Event time
			*/
			public: void M_SetTime(const C_DateTime &time);

			/**
			* @brief Set event message
			* @param message Event message
			*/
			public: void M_SetMessage(const std::string &message) { m_Message = message; }

			/**
			* @brief Get event message
			*/
			public: const std::string &M_GetMessage() const { return m_Message; }

			/**
			* @brief Get event data
			*/
			public: const C_Variant &M_GetData() const { return m_EventData; }

			/**
			* @brief Get event time
			*/
			public: const C_DateTime &M_GetTime() { if (!m_EventTime) { m_EventTime = DBG_NEW C_DateTime(); } return *m_EventTime; }
			/**
			* @brief Return the JSON string of the property
			*/
			public: virtual std::string M_ToJson();

			public: virtual void M_ToJson(C_Variant *value);

			private: C_EventSubscription*		m_Subscription;
			private: std::string					m_Message;
			private: C_Variant					m_EventData;
			private: C_DateTime					*m_EventTime;

					friend class C_EventSubscription;
		};

		/**
		 * \defgroup EquipmentAPI cpmPlus Embedded SDK - EquipmentAPI API
		 */
		/**
		 * @brief This class defines the event subscription of the EquipmentAPI.
		 *
		 * \ingroup EquipmentAPI
		 */
		class Mia_EXPORT C_EventSubscription
		{
			/**
			 * @brief Create a C_EventSubcription identified by eventName
			 */
			protected: C_EventSubscription(const std::string &eventName);

			/**
			* @brief Create a C_EventSubcription identified by eventName
			* @param message	 Event message
			* @param eventTime Event time
			*/
			public: C_SharedPointer<C_DeviceEvent> M_CreateEvent(const std::string &message, const C_DateTime &eventTime);

			/**
			* @brief Create a device event with the message and eventTime
			* @param message	 Event message
			* @param data		 Event data, must be of type Object, otherwise will be discarded
			*/
			public: C_SharedPointer<C_DeviceEvent> M_CreateEvent(const std::string &message, const C_Variant &data);


			/**
			* @brief Create a device event with the message and eventTime
			* @param subscription A pointer to C_EventSubscription
			* @param message	 Event message
			* @param data		 Event data, must be of type Object, otherwise will be discarded
			* @param eventTime Event time
			*/
			public: C_SharedPointer<C_DeviceEvent> M_CreateEvent(const std::string &message, const C_Variant &data, const C_DateTime &eventTime);

			/**
			 * @brief Create an event identified by the message
			 */
			public: C_SharedPointer<C_DeviceEvent> M_CreateEvent(const std::string &message);

			/**
			 * @brief Send the event
			 */
			public: void 						M_SendEvent(C_SharedPointer<C_DeviceEvent> &event);
			/**
			* @brief Check if the event is subscribed
			*/
			public: bool						M_IsSubscribed() const;

			/**
			 * @brief Get the event name
			 */
			public: const std::string 		&M_GetName() const;

			protected: void 					M_ToJson(C_Variant *value);
			protected: void					M_SetDevice(C_Device* device);

			private: C_Device 		*m_Device;
			private: bool				m_IsSubscribed;
			private: std::string		m_EventName;
			private: t_CallId			m_EventId;

			friend class C_MiaDriver;
			friend class C_Device;
			friend class C_DeviceEvent;
		};

		/**
		* \defgroup Equipment API cpmPlus Embedded SDK - Equipment API
		*/
		/**
		* @brief This class defines the function subscription of the Equipment API.
		*
		* \ingroup EquipmentAPI
		*/
		class Mia_EXPORT C_FunctionSubscription
		{
			public: struct t_FunctionArgument
			{
				std::string m_Name;
				t_DataType m_Type;
				std::string m_Description;
				std::string m_Unit;
				C_Variant m_MinValue;
				C_Variant m_MaxValue;
			};

			public: struct t_ArgumentValue
			{
				std::string m_Name;
				ABB::Mia::C_Variant m_Variant;
			};
			
			public: typedef std::vector<t_FunctionArgument> t_FunctionArguments;
			public: typedef std::vector<t_ArgumentValue> t_ArgumentValues;
			public: typedef std::pair<std::string, t_DataType> ArgumentPair;
			public: typedef void (*F_Callback)(C_FunctionSubscription* functionSubscription, ABB::Mia::C_ArgumentList&); // ARM Keil compiler does not support std::function

			public: C_FunctionSubscription(const std::string &functionName, const C_Variant &paramsMetadata, F_Callback functionCallback);

			public: const std::string	&M_GetName() const;
			public: bool					M_IsSubscribed() const;
			public: t_CallId				M_GetId() const;
			public: C_Variant				&operator[](const unsigned int &index);
			public: C_Variant				&operator[](const std::string &paramName);
			public: void					M_SetHandler(F_Callback callback);
			public: void					M_SetParam(const uint &index, const C_FunctionSubscription::t_FunctionArgument& funcArgument);
			public: bool					M_Execute(t_ArgumentValues arguments);
			public: C_Device*				M_GetDevice();

			public: void					M_ProcessPending();
			protected: void				M_OnFunctionCalled(int64 callid, int64 returnId, const C_Variant& functionArguments);
			protected: void				M_AddPending(int64 returnID, const C_ArgumentList& params);
			protected: size_t				M_PendingCount() const;
			protected: void				M_ToJson(C_Variant *value);
			protected: void				M_SetDevice(C_Device* device);
		
			private:	C_Device 		*m_Device;
			private:	bool				m_IsSubscribed;
			private:	std::string		m_FunctionName;
			private: C_Variant		m_ParamMetadata;
			private: F_Callback		m_Callback;
			private: t_CallId 		m_FunctionId;
			private: C_ThreadLock	m_FunctionLock;
			private: std::list<std::pair<int, ABB::Mia::C_ArgumentList> > m_Pendings;

			friend class C_MiaDriver;
			friend class C_Device;
			friend class C_DeviceEvent;
		};

		/**
		 * @brief This class defines the device property interface.
		 * Client use this device interface and publish the device to the server using this interface.
		 *
		 * \ingroup EquipmentAPI
		 */
		class Mia_EXPORT C_DeviceProperty
		{
			public: typedef void(*F_DataHandler)(C_DeviceProperty *deviceProperty, const C_DateTime&, const C_DateTime&); // ARM Keil compiler does not support std::function
			public: typedef void(*F_ChangeHandler)(C_DeviceProperty *deviceProperty, const C_Variant&, const C_DateTime&, t_ValueStatus status, const C_DateTime&); // ARM Keil compiler does not support std::function

			public: enum t_Direction
			{
				g_DIRECTION_TOSERVER,
				g_DIRECTION_FROMSERVER,
				g_DIRECTION_BOTH
			};
			/**
			 * @brief Construct a C_DeviceProperty with the given settings
			 * Property constructed with this method is released by the device automatically
			 */
			public: C_DeviceProperty(C_Device* device, 
				const std::string &name, 
				const t_PropertyType type = g_PT_DOUBLE, 
				const t_Direction direction = g_DIRECTION_TOSERVER,
				const std::string &category = "",
				const std::string &description = "",
				const std::string &unit = "", 
				const C_Variant& min = C_Variant(), 
				const C_Variant& max = C_Variant(),
				int64 bufferLengthMs = -1
			);

			/**
			* @brief Construct a C_DeviceProperty with the given settings
			* Property constructed with this method is released by the device automatically
			*/
			public: C_DeviceProperty(C_Device* device, 
				const std::string &name, 
				const C_Variant& staticValue, 
				const t_PropertyType type = g_PT_UNKNOWN, 
				const std::string &category ="", 
				const std::string &description = "", 
				const std::string &unit = "",
				const C_Variant& min = C_Variant(), 
				const C_Variant& max = C_Variant()
			);

			public:	virtual ~C_DeviceProperty() {}

			/**
			 * @brief Return the name of the property
			 */
			public: virtual std::string M_GetName() const { return m_Name; }

			/**
			 * @brief Return the data type of the property. Default is double.
			 */
			public: virtual t_PropertyType M_GetType() const { return m_Type;}

		   /**
		    * @brief Return the static value of the property
			 */
			public: virtual C_Variant M_GetStaticValue() const { return m_Value; }

		   /**
			 * @brief Return the data type of the property. Default is double.
			 */
			public: virtual t_Direction M_GetDirection() const { return m_Direction; }

			/**
			* @brief Return the category of the property
			*/
			public: virtual std::string M_GetCategory() const { return m_Category; }

			/**
			* @brief Return the description of the property
			*/
			public: virtual std::string M_GetDescription() const { return m_Description; }


			/**
			 * @brief Return the unit of the property
			 */
			public: virtual std::string M_GetUnit() const { return m_Unit;}

			/**
			 * @brief Return the lower limit of the property value
			 */
			public: virtual C_Variant M_GetMin() const {	return m_ValueMin;}

			/**
			 * @brief Return the upper limit of the property value
			 */
			public: virtual C_Variant M_GetMax() const {	return m_ValueMax;}

			/**
			* @brief Return the buffer length of the property in ms
			*/
			public: virtual int64 M_GetBufferLength() const { return m_BufferLengthMs; }

			/**
			 * @brief Return true if the property is subscribed for data production.
			 */
			public: bool M_IsSubscribed() const;

			/**
			 * @brief Property value has changed.
			 * @param value The property value
			 * @param time The time value has changed
			 * @param status The status of the value
			 */
			public: virtual void M_ValueChanged(const C_Variant &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);

			/**
			 * @brief Property value has changed
			 */
			public:	virtual void M_ValueChanged(const C_Variant &value);

			/**
			 * @brief Return the JSON string of the property
			 */
			public: virtual std::string M_ToJson();

			/**
			 * @brief Return the JSON string of the property
			 */
			public:	virtual void M_ToJson(C_Variant *json);

			/**
			 * @brief Set minimum production rate
			 */
			public:	void M_SetMinimumRateMs(int minimumRate);

			/**
			* @brief Set data handler
			*/
			public: void M_SetDataHandler(F_DataHandler handler);

			protected: virtual void M_SetDevice(C_Device* device);

			protected:	virtual void M_Produce();

			/**
			 * @brief Get current value
			 */
			public: const C_Variant &M_GetCurrentValue();
			/**
			 * @brief Get pusher id
			 */
			public: int64 M_GetId() { return m_PusherId;}
			/**
			 * @brief Get last update time
			 */
			public: const C_DateTime &M_GetLastUpdateTime() { return m_LastUpdateTime;}

			/**
			 * @brief Get last production time
			 */
			public: const C_DateTime &M_GetLastProductionTime() { return m_LastProductionTime;}

			/**
			 * @brief Get last production time
			 */
			public: const int &M_GetProductionRate() { return m_ProductionRate;}

			/**
			* @brief Get sampling interval in ms
			*/
			public: C_TimeSpan      M_GetSamplingIntervalMs() { return m_SamplingIntervalMs; }

			/**
			* @brief Get publishing interval in ms
			*/
			public: C_TimeSpan		M_GetPublishingIntervalMs() { return m_PublishingIntervalMs; }

			/**
			* @brief Get buffer length
			*/
			public: C_TimeSpan		M_GetBufferLength() { return m_BufferLength; }

			protected: F_ChangeHandler	m_ChangeHandler;
			protected: F_DataHandler	m_DataHandler;
			protected: int64				m_PusherId;
			protected: std::string		m_Name;
			protected: t_PropertyType	m_Type;
			protected: C_Variant			m_Value;
			protected: t_Direction		m_Direction;
			protected: std::string		m_Category;
			protected: std::string		m_Description;
			protected: std::string		m_Unit;
			protected: C_Variant			m_ValueMin;
			protected: C_Variant			m_ValueMax;
			protected: int64				m_BufferLengthMs;
			protected: bool				m_IsStatic;

			protected: bool				m_IsSubscribed;
			protected: C_MiaDriver*		m_Driver;
			protected: C_Device			*m_Device;
			protected: C_Variant			m_CurrentValue;
			protected: C_DateTime		m_LastUpdateTime;
			protected: C_DateTime		m_LastProductionTime;
			protected: C_ThreadLock		m_DataLock;
			
			protected: C_TimeSpan      m_SamplingIntervalMs;
			protected: C_TimeSpan		m_PublishingIntervalMs;
			protected: C_TimeSpan		m_BufferLength;
			protected:	int				m_ProductionRate;

			friend class C_MiaDriver;
			friend class C_Device;
		};

		/**
		 * @brief This class defines the device interface.
		 * Client implements this device interface and publish the device to the server using this interface.
		 *
		 * \ingroup EquipmentAPI
		 */
		class Mia_EXPORT C_Device : public C_IReconnectHandler, public C_Thread
		{
			public: typedef unordered_map<std::string, C_SharedPointer<C_DeviceProperty> > t_PropertyMap;
			public: typedef unordered_map<std::string, C_SharedPointer<C_EventSubscription> > t_EventMap;
			public: typedef unordered_map<std::string, C_SharedPointer<C_FunctionSubscription> > t_FunctionMap;
			public: typedef unordered_map<int64, C_SharedPointer<C_FunctionSubscription> > t_FunctionSubsriptionMap;

			protected: C_Device() : C_Thread(std::string("C_Device")) {};
			public:	C_Device(const std::string &equipmentName, const std::string &equipmentType);

			public: virtual ~C_Device();

			public: virtual void M_OnReconnect();
			/**
			 * @brief Return the GUID of the device
			 */
			public: virtual C_Guid M_GetId() const { return m_Guid; };

			/**
			 * @brief Set the GUID of the device
			 */
			public: virtual void M_SetId(const C_Guid &guid) { m_Guid = guid; };

			/**
			 * @brief Return name of the device, will be used as path
			 */
			public:	virtual std::string M_GetEquipmentName(void) const { return m_EquipmentName; };

			/**
			 * @brief Return the type of the device
			 */
			public: virtual std::string M_GetEquipmentType() { return m_EquipmentType; };

			/**
			 * @brief Return a property with name.
			 * @param propertyName Name of the property.
			 */
			public: virtual C_SharedPointer<C_DeviceProperty> M_GetProperty(const std::string &propertyName);

			/**
			 * @brief Return a property with name.
			 * @param propertyName Name of the property.
			 */
			public: virtual C_SharedPointer<C_EventSubscription> M_GetEvent(const std::string &eventName);

			/**
			 * @brief Return the firmware version of the device
			 */
			public:	virtual std::string M_GetEquipmentCategory() const;

			/**
			 * @brief Return the description of the device
			 */
			public:	virtual std::string M_GetEquipmentDescription() const;

			/**
			 * @brief Return true if the device is available
			 */
			public:	virtual bool M_IsAvailable() const;

			/**
			 * @brief Return device information in JSON format
			 */
			public:	virtual std::string M_ToJson();

			/**
			 * @brief Return device information in JSON format
			 * @param json The JSON format
			 */
			public:	virtual void M_ToJson(C_Variant *json);

			/**
			 * @brief Call when the property value changed
			 * @param property The property that changed
			 * @param value The value of the property that changed
			 */
			public:	void M_PropertyValueChanged(C_DeviceProperty* property, const C_Variant &value);

			/**
			 * @brief Call when the property value changed
			 * @param propertyName The name of the property that changed
			 * @param value The value of the property that changed
			 */
			public: void M_PropertyValueChanged(const std::string &propertyName, const std::string &value);

			/**
			 * @brief Call when the property value changed
			 * @param propertyName The name of the property that changed
			 * @param value The value of the property that changed
			 */
			public: void M_PropertyValueChanged(const std::string &propertyName, const C_Variant &value);

			/**
			 * @brief Call when the property value changed
			 * @param property The property that changed
			 * @param value The value of the property that changed
			 * @param timeValue The time value when property changed
			 * @param valueStatus The status of the value
			 */
			public: void M_PropertyValueChanged(C_DeviceProperty* property, const C_Variant &value, const C_DateTime &timeValue, const t_ValueStatus &valueStatus = g_VS_OK);

			/**
			 * @brief Register a subdevice
			 * @param subdevice The subdevice
			 */
			public: void M_AddSubdevice(C_Device* subdevice);

			/**
			* @brief Register a property
			* @param property Property to be added
			*/
			public: void M_AddProperty(C_DeviceProperty * property);
			/**
			 * @brief Register a property
			 * @param property Property to be added
			 */
			public: void M_AddProperty(C_SharedPointer<C_DeviceProperty>& property);

			/**
			 * @brief Register a property
			 * @param name Name of the property
			 * @param type Type of the property, default to be g_Double.
			 * @param direction (optional) Direction to server
			 * @param category (optional) Category of the property
			 * @param description (optional) Description of the property
			 * @param unit (optional) Unit of the property
			 * @param min 	(optional) Minimum value of the property
			 * @param max 	(optional) Maximum value of the property
			 */
			public: C_SharedPointer<C_DeviceProperty> M_AddProperty(const std::string &name,
				const t_PropertyType type = g_PT_DOUBLE,
				const C_DeviceProperty::t_Direction direction = C_DeviceProperty::g_DIRECTION_TOSERVER,
				const std::string &category = "",
				const std::string &description = "",
				const std::string &unit = "",
				const C_Variant& min = C_Variant(),
				const C_Variant& max = C_Variant(),
				int64 bufferLengthMs = -1);

			/**
			 * @brief Register a property
			 * @param name Name of the property
			 * @param type (optional) Type of the property, default to be g_Double.
			 * @param category (optional) Category of the property
			 * @param description (optional) Description of the property
			 * @param unit (optional) Unit of the property
			 */
			C_SharedPointer<C_DeviceProperty> M_AddProperty(const std::string &name,
				const C_Variant& staticValue,
				const t_PropertyType type = g_PT_UNKNOWN,
				const std::string &category = "",
				const std::string &description = "",
				const std::string &unit = "");

			/**
			* @brief Register an event subscription
			* @param event Event subscription
			*/
			C_SharedPointer<C_EventSubscription> M_AddEventSubscription(C_EventSubscription* event);

			/**
			* @brief Register an event subscription
			* @param eventName Name of the event subscription
			*/
			C_SharedPointer<C_EventSubscription>  M_AddEventSubscription(const std::string &eventName);

			/**
			* @brief Register a function
			* @param function The C_FunctionSubscription
			* @see C_FunctionSubscription
			*/
			public: void M_AddFunctionSubscription(C_SharedPointer<C_FunctionSubscription> function);

			/**
			* @brief Register a function subscription
			* @param functionName The function name
			* @see C_FunctionSubscription
			*/
			public: C_SharedPointer<C_FunctionSubscription> M_AddFunctionSubscription(const std::string &functionName, 
				const C_Variant& params, 
				C_FunctionSubscription::F_Callback callback);

			/**
			* @brief Register a function subscription
			* @param functionName The function name
			* @see C_FunctionSubscription
			*/
			public: C_SharedPointer<C_FunctionSubscription> M_AddFunctionSubscription(const std::string &functionName,
				const C_FunctionSubscription::t_FunctionArguments& params,
				C_FunctionSubscription::F_Callback callback);

			/**
			 * @brief Recreate the session
			 */
			public: void M_ResubscribeSession();

			/**
			 * @brief Get the internal property map
			 */
			public: t_PropertyMap &M_GetPropertyMap();

			/**
			 * @brief Get the internal event map
			 */
			public: t_EventMap &M_GetEventMap();

			/**
			 * @brief Was device subscription succesfully done
			 */
			public: bool M_IsSubscripted();

			/**
			 * @brief Get the internal function map
			 */
			public: t_FunctionMap &M_GetFunctionMap();

			/**
			 * @brief Get the internal device lock
			 */
			public: C_ThreadLock &M_GetDeviceLock();

			/**
			 * @brief Start the data production routine
			 */
			public: void M_StartProduction();

			/**
			 * @brief Stop the data production routine
			 */
			public: void M_StopProduction();

			/**
			 * @brief Call when there are values after long waiting
			 */
			public: void M_SignalOnValue();

			private: virtual void M_Routine();
			/**
			* @brief Return device from the input JSON string
			* @param json The JSON string
			*/
			public: static C_SharedPointer<C_Device> M_S_FromJson(const std::string &json);
					
			private: void M_OnDeviceUpdate(void *updateData);
			private: void M_OnDeviceUpdate(const C_Variant& operationData);
			private: void M_ProcessUpdate(const std::string &operation, const C_Variant& operationData);
			private: void M_ProcessCallRequest(const C_Variant& operationData);
			private: void M_ProcessResendRequest(const C_Variant& operationData);
			private: void M_ProcessChangeRequest(const C_Variant& operationData);

			private: void M_ProcessSubscriptionsRequest(const C_Variant& operationData);
			private: void M_ProcessEventSubscriptionsRequest(const C_Variant& operationData);

			protected: void M_AddSubscribedProperty(const std::string &property);
			protected: void M_AddSubscribedProperty(const int &subscriptionId, const C_Variant& subcriptionData);
			protected: void M_AddSubscribedEvent(const int &eventId, const C_Variant& eventData);

			protected: void M_SetDriver(C_MiaDriver *driver);

			protected:	C_MiaDriver* 	m_Driver;
			protected:	std::list<C_Device*> 	m_Subdevices;
			protected:	C_ThreadLock 	m_DeviceLock;
			protected: 	C_Event 			m_OnValueProduced;
			protected:	t_PropertyMap 	m_PropertyMap;
			protected: 	t_EventMap  	m_EventMap;
			protected:	t_FunctionMap	m_FunctionMap;
			protected:	t_FunctionSubsriptionMap m_FunctionSubsMap;
			protected: 	std::string 	m_EquipmentName;
			protected: 	std::string 	m_EquipmentType;
			protected: 	std::string 	m_EquipmentCategory;
			protected: 	std::string 	m_EquipmentDescription;
			protected:  C_Guid			m_Guid;
			protected:  bool			m_IsSubscriped;

			friend class C_MiaDriver;
			friend class C_EventSubscription;
			friend class C_FunctionSubscription;
		};
	}
}
#endif /* SRC_MIA_DEVICE_H_ */
