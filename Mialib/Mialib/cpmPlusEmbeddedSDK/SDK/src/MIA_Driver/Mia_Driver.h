/*
 * Mia_MiaDriver.h
 *
 *  Created on: Jun 12, 2014
 *      Author: Tuan Vu
 */

#ifndef RTDB_MIADRIVER_H_
#define RTDB_MIADRIVER_H_

#include <string>
#include <list>
#include <assert.h>

#include "Mia_Base.h"
#include "Mia_DriverBase.h"
#include "Mia_Exception.h"

namespace ABB
{
	namespace Mia
	{
		/**
		 * @brief enum t_ValueStatus used for data production to indicate status of the value
		 * \ingroup MiaDriver
		 */
		enum t_ValueStatus
		{
			g_VS_OK = 0,
			g_VS_UserStatus = 255,
			g_VS_ManuallySet = 256,
			g_VS_Increasing = 512,
			g_VS_Decreasing = 1024,
			g_VS_StartOfSlice = 2048,
			g_VS_EndOfSlice = 4096,
			g_VS_Interpolated = 8192,
			g_VS_Extrapolated = 16384,
			g_VS_Fake = 24576,
			g_VS_NoStatus = 32768,
#ifndef WIN32 // Windows compiler does compute larger that 32 bit enums
			g_VS_Representativeness = 268369920UL,
			g_VS_Producer = 16911433728ULL,
			g_VS_LastInSequence = 274877906944ULL,
			g_VS_HistorySubstituted = 549755813888ULL,
			g_VS_Uninitialized = 2199023255552ULL,
			g_VS_AccessDenied = 10995116277760ULL,
			g_VS_Reason = 16492674416640ULL,
			g_VS_Frozen = 17592186044416ULL,
			g_VS_Incomplete = 35184372088832ULL,
			g_VS_Substituted = 140737488355328ULL,
			g_VS_SuppressedAlarm = 281474976710656ULL,
			g_VS_Questionable = 562949953421312ULL,
			g_VS_AlarmLevel = 1151795604700004352ULL,
			g_VS_Bad = 1152921504606846976ULL,
			g_VS_Disabled = 2305843009213693952ULL,
			g_VS_Invalid = 4611686018427387904ULL,
			g_VS_NonAckAlarm = 9223372036854775808ULL
#endif
		};

		Mia_CLASS(C_DbClass);
		Mia_CLASS(C_DbClassInstance);
		Mia_CLASS(C_DbPropertyInfo);

		class C_MiaDriver;
		class C_MiaDriverBase;
		class C_DeviceEvent;
		Mia_CLASS(C_Equipment);

		class C_PropertyValuePusherClass;
		class C_CurrentValuePusherClass;
		class C_Device;
		class C_DeviceProperty;

		typedef shared_ptr<C_PropertyValuePusherClass> C_PropertyValuePusher;
		typedef shared_ptr<C_CurrentValuePusherClass> C_CurrentValuePusher;
		class C_DbProperty;

		enum t_ClassType
		{
			g_CLASSTYPE_COMMAND, g_CLASSTYPE_TABLE
		};

		typedef unordered_map<std::string, C_SharedPointer<C_DbPropertyInfo> >::iterator C_DbClassPropertyIterator;

		/**
		 * \defgroup MiaDriver cpmPlus Embedded SDK - Driver
		 */
		/**
		 * @brief C_DbClass allows handling of Vtrin Class
		 * \ingroup MiaDriver
		 */
		class C_DbClassClass
		{
			public: const static char* m_k_s_ClassProperties;

			// Hide the constructor to prevent intialization / destruction
			private: C_DbClassClass();
			private: C_DbClassClass(C_MiaDriver* driver, const std::string &className);
			private: C_DbClassClass(C_MiaDriver* driver, const std::string &className, const std::string &displayName,
			   bool nonCacheable, const std::string &baseTable, const std::string &classType);

			public: ~C_DbClassClass();
			/**
			 * @brief Add an instance of this class. Created object must be saved by calling
			 * M_CommitChanges(), otherwise all changes will be lost.
			 *
			 * @see C_AutoPointer
			 * @see M_CommitChanges
			 * @return An object of this class. The object is managed inside the C_AutoPointer.
			 */
			public: C_DbClassInstance M_Add();

			/**
			 * @brief Get the driver.
			 */
			public: C_MiaDriver * M_GetDriver();

			/**
			 * @brief Remove the instance from database.
			 *
			 * @param instance Instance to be removed.
			 *
			 * @return true if the method succeeds
			 */
			public:  bool M_Remove(C_DbClassInstance &instance);

			/**
			 * @brief Get a list of properties
			 *
			 * @param properties The list of properties to be updated
			 * @param refresh If refresh is true, the property list is synced with the database before updating
			 * the result list.
			 *
			 * @throw C_ServerException when the query encounters a server problem
			 * @see C_ServerException
			 */
			public: void M_GetProperties(C_DbPropertyInfos *properties, bool refresh = false);

			/**
			 * @brief Get a property info using the property name
			 *
			 * @param propertyName Name of the property
			 *
			 * @return The property info associated with the given name. Null pointer received if not found.
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 * @throw C_ServerException if there is exception happens on the server
			 */
			public: const C_DbPropertyInfo& operator[](const std::string &propertyName);

			/**
			 * @brief Get a property info using the property name
			 *
			 * @param propertyName Name of the property
			 *
			 * @return The property info associated with the given name. Null pointer received if not found.
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 * @throw C_ServerException if there is exception happens on the server
			 */
			public: const C_DbPropertyInfo& M_GetProperty(const std::string &propertyName);

			/**
			 * @brief Get a list of instances limited by the fetchMax
			 *
			 * @param instances The list of instances to be updated
			 * @param fetchMax The maximum number of instance to be fetched
			 *
			 * @return True if the function is able to update the instances list.
			 * @throw C_ServerException if there is exception happens on the server
			 *
			 */
			public: bool M_GetInstanceSet(C_DbClassInstances *instances, int fetchMax = -1);

			/**
			 * @brief Get a list of instances defined by the whereString and arguments list
			 *
			 * @param instances The list of instances to be updated
			 * @param whereString The string to filter the result
			 * @param arguments The arguments for the whereString
			 *
			 * @return True if the function is able to update the instances list.
			 * @throw C_ServerException if there is exception happens on the server
			 *
			 */
			public: bool M_GetInstanceSet(C_DbClassInstances *instances, const std::string &whereString,
			   const C_ArgumentList &arguments);

			/**
			 * @brief Get a list of instances defined by the whereString and arguments list
			 *
			 * @param instances The list of instances to be updated
			 * @param fetchMax The maximum number of instance to be fetched
			 * @param whereString The string to filter the result
			 * @param arguments The arguments for the whereString
			 *
			 * @return True if the function is able to update the instances list.
			 * @throw C_ServerException if there is exception happens on the server
			 *
			 */
			public: bool M_GetInstanceSet(C_DbClassInstances *instances, int fetchMax, const std::string &whereString,
			   const std::string & arguments = "");

			/**
			 * @brief Get a list of instances defined by the whereString and arguments list
			 *
			 * @param instances The list of instances to be updated
			 * @param whereString The string to filter the result
			 * @param arguments The arguments for the whereString
			 * @param fetchMax The maximum number of instance to be fetched
			 *
			 * @return True if the function is able to update the instances list.
			 * @throw C_ServerException if there is exception happens on the server
			 */
			public: bool M_GetInstanceSet(C_DbClassInstances *instances, int fetchMax, const std::string &whereString,
			   const C_ArgumentList &arguments);

			/**
			 * @brief Get an instance specified by the instance id
			 *
			 * @param id The instance id
			 *
			 * @return True if the function is able to update the instances list.
			 * @throw C_ServerException if there is exception happens on the server
			 */
			public: C_DbClassInstance M_GetInstanceById(const C_Variant &id);

			/**
			 * @brief Get class name
			 */
			public: const std::string &M_GetClassName() const;

			/**
			 * @brief Get display name
			 */
			public: const std::string &M_GetDisplayName() const;

			/**
			 * @brief Check if class is cacheable
			 */
			public: bool M_IsCacheable() const;

			/**
			 * @brief Check if class in non-cacheable
			 */
			public: bool M_IsNonCacheable() const;

			/**
			 * @brief Get base table name
			 */
			public: const std::string &M_GetBaseTable() const;

			/**
			 * @brief Get class type
			 */
			public: const t_ClassType &M_GetClassType() const;

			private: void M_ClearProperties();
			private: void M_FetchAndUpdateProperties(C_DbClass *c = 0);

			private: std::string M_GetPropertySerializedString();

			private: class C_DbClassPrivate;
			private: C_AutoPointer<C_DbClassPrivate> m_Private;

			friend class C_MiaDriver;
			friend class C_DbPropertyInfoClass;
			friend class C_SharedPointer<C_DbClass> ;
			friend class C_AutoPointer<C_DbClass> ;
			friend class C_DbClassInstanceClass;
			friend class C_DbClassPrivate;
		};

		/**
		 * @brief Allow handling of Vtrin class instances. This class is un-instantiable but initiated via the C_DbClass
		 * automatically via calls.
		 * \ingroup MiaDriver
		 * @see C_MiaDriver
		 *
		 */   
		class C_DbClassInstanceClass
		{
			private: C_DbClassInstanceClass();
			private: C_DbClassInstanceClass(C_DbClass c);
			public: ~C_DbClassInstanceClass();

			/**
			 * @brief Get the class of this instance
			 */
			public: C_DbClass M_GetClass();

			/**
			 * @brief Get id of this instance
			 */
			public: const C_Variant &M_GetId();

			/**
			 * @brief Get name of this instance
			 */
			public: std::string M_GetName();

			/**
			 * @brief Call before modifying instance values. After values had been updated,
			 * M_CommitChanges will finalize the changes and update the server.
			 *
			 * @see M_CommitChanges
			 */
			public: void M_BeginUpdate();

			/**
			 * @brief Push the changes and update values on the server
			 */
			public: bool M_CommitChanges();

			/**
			 * @brief Cancel the update mode
			 */
			public: void M_Cancel();

			/**
			 * @brief Remove the instance from the database. Will be removed immediately.
			 */
			public: bool M_Remove();

			/**
			 * @brief Refresh local values with new one from server
			 * @throw C_ServeException if server encounters an exception and ENABLE_EXCEPTION flag is set
			 */
			public: void M_Refresh();

			/**
			 * @brief Get value of the given propertyName
			 *
			 * @param propertyName Name of the property
			 *
			 * @return C_Variant containing the property value
			 */
			public: C_SharedPointer<C_Variant> M_GetValue(const std::string &propertyName);

			/**
			 * @brief Get value of the given propertyName
			 *
			 * @param propertyInfo The property info
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 * @return C_Variant containing the property value
			 */
			public: C_SharedPointer<C_Variant> M_GetValue(const C_DbPropertyInfo &propertyInfo);

			/**
			 * @brief Set value of the given propertyName with the variant value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName Name of the property
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_SetValue(const std::string &propertyName, const C_Variant& value);

			/**
			 * @brief Set value of the given propertyName with the variant value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName Name of the property
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_SetRawValue(const std::string &propertyName, const C_Variant& value);

			/**
			 * @brief Set value of the given propertyName with the variant value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_SetValue(const C_DbPropertyInfo &propertyInfo, const C_Variant& value);

			/**
			 * @brief Set value of the given propertyName with the variant value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const C_DbPropertyInfo &propertyInfo, C_Variant* value);

			/** Property setter */

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const std::string &propertyName, bool *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const std::string &propertyName, unsigned char *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const std::string &propertyName, short *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const std::string &propertyName, unsigned short *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const std::string &propertyName, int *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const std::string &propertyName, uint *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const std::string &propertyName, int64 *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const std::string &propertyName, uint64 *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const std::string &propertyName, float *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const std::string &propertyName, double *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const std::string &propertyName, std::string *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const std::string &propertyName, char *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public:	void M_GetValue(const std::string &propertyName, C_DateTime *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyName The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const std::string &propertyName, C_Guid *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const C_DbPropertyInfo &propertyInfo, bool *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const C_DbPropertyInfo &propertyInfo, char *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const C_DbPropertyInfo &propertyInfo, unsigned char *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const C_DbPropertyInfo &propertyInfo, short *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: 	void M_GetValue(const C_DbPropertyInfo &propertyInfo, unsigned short *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const C_DbPropertyInfo &propertyInfo, int *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const C_DbPropertyInfo &propertyInfo, uint *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const C_DbPropertyInfo &propertyInfo, int64 *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const C_DbPropertyInfo &propertyInfo, uint64 *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const C_DbPropertyInfo &propertyInfo, float *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const C_DbPropertyInfo &propertyInfo, double *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public:void M_GetValue(const C_DbPropertyInfo &propertyInfo, std::string *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const C_DbPropertyInfo &propertyInfo, C_DateTime *value);

			/**
			 * @brief Set value of the given propertyName with the value. Changes will not be saved before
			 * M_CommitChanges is called.
			 *
			 * @param propertyInfo The property to be updated
			 * @param value New value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: void M_GetValue(const C_DbPropertyInfo &propertyInfo, C_Guid *value);

			/**
			 * @brief Get value of the given propertyName.
			 *
			 * @param propertyName The name property to get
			 * @return value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public: C_SharedPointer<C_Variant> operator[](const std::string &propertyName);

			/**
			 * @brief Get value of the given propertyName.
			 *
			 * @param propertyInfo The name property to get
			 * @return value of the property
			 *
			 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
			 */
			public:	C_SharedPointer<C_Variant> operator[](const C_DbPropertyInfo &propertyInfo);

			protected: void M_AssignValue(const std::string &propertyName, const std::string &value);
			protected: void M_AssignValue(const C_DbPropertyInfo &propertyInfo, const std::string &value);
			protected: void M_AssignValue(const C_DbPropertyInfo &propertyInfo, C_Variant &value);
			protected: void M_AssignValue(const std::string &propertyName);
			protected: C_Variant* M_GetProperty(const std::string &propertyName);
			protected: C_Variant* M_GetModifiedProperty(const std::string &propertyName);

			private: class C_DbClassInstancePrivate;
			private: C_AutoPointer<C_DbClassInstancePrivate> m_Private;

			friend class C_DbClassClass;
			friend class shared_ptr<C_DbClassInstance>;
		};

		/**
		 * @brief Class that handles Class property info
		 * \ingroup MiaDriver
		 */
		class C_DbPropertyInfoClass
		{
			public: const static char* m_k_s_ClassProperties;
			private: class C_DbPropertyInfoPrivate;

			private: C_DbPropertyInfoClass(C_DbClass parent, const std::string &propname, const std::string & category,
			   const std::string & displayname, const std::string & defaultval, const std::string & description,
			   const std::string & index, const std::string & size, const std::string & type, const std::string & rawtype,
			   bool nullable, bool numeric, bool interned, bool readonly, bool unique, bool isvirtual, bool visibletouser,
			   bool maskrequired, const std::string & referencetarget);
						
			public: ~C_DbPropertyInfoClass();
			/**
			 * @brief Get name of the property
			 * */
			public: const std::string &M_GetName() const;

			/**
			 * @brief Get category
			 */
			public: const std::string &M_GetCategory() const;

			/**
			 * @brief Get display name
			 */
			public: const std::string &M_GetDisplayName() const;

			/**
			 * @brief Get default value
			 */
			public: const std::string &M_GetDefaulValue() const;

			/**
			 * @brief Get description
			 */
			public: const std::string &M_GetDescription() const;

			/**
			 * @brief Get index
			 */
			public: const uint32 &M_GetIndex() const;

			/**
			 * @brief Get size of the property
			 */
			public: const uint32 &M_GetSize() const;

			/**
			 * @brief Get type of the property
			 */
			public: const t_DataType &M_GetType() const;

			/**
			 * @brief Get type of the property
			 */
			public: const std::string &M_GetTypeName() const;

			/**
			 * @brief Get raw type of the property
			 */
			public: const std::string &M_GetRawType() const;

			/**
			 * @brief Get reference target
			 */
			public: const std::string &M_GetReferenceTarget() const;
			/**
			 * @brief Get class of this property
			 */
			public: C_DbClass M_GetClass();

			/**
			 * @brief Check if the property is numeric
			 */
			public: bool M_IsNumeric();

			/**
			 * @brief Check if the property is nullable
			 */
			public: bool M_IsNullable();

			/**
			 * @brief Check if the property is interned
			 */
			public: bool M_IsInterned();

			/**
			 * @brief Check if the property is readonly
			 */
			public: bool M_IsReadonly();

			/**
			 * @brief Check if the property is unique
			 */
			public: bool M_IsUnique();

			/**
			 * @brief Check if the property is virtual
			 */
			public: bool M_IsVirtual();

			/**
			 * @brief Check if the property is visible to other user
			 */
			public: bool M_IsVisibleToUser();

			/**
			 * @brief Check if the property required masking
			 */
			public: bool M_IsMaskRequired();

			protected: void M_SetType(const std::string &type);
			protected: void M_SetType(const t_DataType &type);

			private: C_AutoPointer<C_DbPropertyInfoPrivate> m_Private;

			friend class C_MiaDriver;
			friend class C_DbClassClass;
			friend class C_EquipmentClass;
		};

		/**
		 * @brief A class to help with current value production.
		 * Making use of an C_MiaDriver object, the class resolves the
		 * variable name and produces value when it was resolved
		 */
		class Mia_EXPORT C_CurrentValuePusherClass : public C_IReconnectHandler
		{
			/**
			 * @brief Constructor: Create an C_CurrentValuePusher instance with a
			 * C_MiaDriver and a pusherid
			 *
			 */
			public:		C_CurrentValuePusherClass(C_MiaDriver *driver, t_CallId pusherid, const std::string &className);

			public:		virtual ~C_CurrentValuePusherClass();

			/**
			 * @brief Push value to the variableName
			 * This method will push current value to the server
			 * using the variableName.
			 * Note that variableName must exist on the server prior to production
			 *
			 * @param variableName The name the variable to produce data
			 * @param value Current value of the variable
			 * @calls M_Push(const int &variableId, value) overloads
			 */
			public:		void M_Push(const std::string &variableName, const std::string &value);
			public:		void M_Push(const std::string &variableName, const char &value);
			public:		void M_Push(const std::string &variableName, const byte &value);
			public:		void M_Push(const std::string &variableName, const int16 &value);
			public:		void M_Push(const std::string &variableName, const int32 &value);
			public:		void M_Push(const std::string &variableName, const int64 &value);
			public:		void M_Push(const std::string &variableName, const uint16 &value);
			public:		void M_Push(const std::string &variableName, const uint32 &value);
			public:		void M_Push(const std::string &variableName, const uint64 &value);
			public:		void M_Push(const std::string &variableName, const float &value);
			public:		void M_Push(const std::string &variableName, const double &value);
			public:		void M_Push(const std::string &variableName, const bool &value);
			public:		void M_Push(const std::string &variableName, const C_Variant &value);

			/**
			 * @brief Push value to the variableId
			 * This method will push current value to the server
			 * using the variableId.
			 * Note that variableId must exist on the server prior to production
			 *
			 * @param variableId The id the variable to produce data
			 * @param value Current value of the variable
			 */
			public:		void M_Push(const int &variableId, const std::string &value);
			public:		void M_Push(const int &variableId, const char &value);
			public:		void M_Push(const int &variableId, const byte &value);
			public:		void M_Push(const int &variableId, const int16 &value);
			public:		void M_Push(const int &variableId, const int32 &value);
			public:		void M_Push(const int &variableId, const int64 &value);
			public:		void M_Push(const int &variableId, const uint16 &value);
			public:		void M_Push(const int &variableId, const uint32 &value);
			public:		void M_Push(const int &variableId, const uint64 &value);
			public:		void M_Push(const int &variableId, const float &value);
			public:		void M_Push(const int &variableId, const double &value);
			public:		void M_Push(const int &variableId, const bool &value);
			public:		void M_Push(const int &variableId, const C_Variant &value);

			/**
			* @brief Push value to the variableName
			* This method will push current value with time (and status) to the server
			* using the variableName.
			* Note that variableName must exist on the server prior to production
			*
			* @param variableName The name the variable to produce data
			* @param value Current value of the variable
			* @calls M_Push(const int &variableId, value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK) overloads
			*/
			public:		void M_Push(const std::string &variableName, std::string &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const std::string &variableName, const char &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const std::string &variableName, const byte &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const std::string &variableName, const int16 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const std::string &variableName, const int32 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const std::string &variableName, const int64 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const std::string &variableName, const uint16 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const std::string &variableName, const uint32 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const std::string &variableName, const uint64 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const std::string &variableName, const float &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const std::string &variableName, const double &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const std::string &variableName, const bool &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const std::string &variableName, const C_Variant &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);

			/**
			* @brief Push value to the variableId
			* This method will push current value with time (and status) to the server
			* using the variableId.
			* Note that variableId must exist on the server prior to production
			*
			* @param variableId The id the variable to produce data
			* @param value Current value of the variable
			* @calls M_Push(const int &variableId, value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK) overloads
			*/
			public:		void M_Push(const int &variableId, std::string &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const int &variableId, const char &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const int &variableId, const byte &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const int &variableId, const int16 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const int &variableId, const int32 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const int &variableId, const int64 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const int &variableId, const uint16 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const int &variableId, const uint32 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const int &variableId, const uint64 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const int &variableId, const float &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const int &variableId, const double &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const int &variableId, const bool &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			public:		void M_Push(const int &variableId, const C_Variant &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);

			protected:	virtual void M_OnReconnect();

			private:	void M_ReconnectHandler();

			public:		int M_ResolveVariableId(const std::string &variableName);
			private:	template <typename T> void M_PushBinary(const int &variableId, const T &value, const C_DateTime &time = NULL, const t_ValueStatus &status = g_VS_OK);
			private:	void M_PushBinary(const int &variableId, const C_Variant &value, const C_DateTime &time = NULL, const t_ValueStatus &status = g_VS_OK);
			private:	void M_PushJson(const int &variableId, const std::string &value, const C_DateTime &time = NULL, const t_ValueStatus &status = g_VS_OK);

			private:	int64 											m_PusherId;
			private:	C_ThreadLock 									m_Lock;
			private:	std::string										m_ClassName;
			private:	unordered_map<std::string, int> 				m_Variables;
			private:	C_MiaDriver 									*m_Driver;
		};

		/**
		 * @brief A class to help with current value production.
		 * Making use of an C_MiaDriver object, the class resolves the
		 * variable name and produces value when it was resolved
		 * \ingroup MiaDriver
		 */
		class Mia_EXPORT C_PropertyValuePusherClass
		{
			/**
			 * @brief Constructor: Create an C_CurrentValuePusher instance with a
			 * C_MiaDriver and a pusherid
			 *
			 */
			public:		C_PropertyValuePusherClass(C_MiaDriver *driver, t_CallId pusherid);

			/**
			* @brief Push value to the the property.
			* This method will push current value to the server
			*
			* @param value Current value of the variable
			*/
			public:	void M_Push(const std::string &value);
			private:	void M_Push(const char &value);				// private to hide this until it works
			private:	void M_Push(const byte &value);				// private to hide this until it works
			private:	void M_Push(const int16 &value);			// private to hide this until it works
			private:	void M_Push(const int32 &value);			// private to hide this until it works
			public:	void M_Push(const int64 &value);
			private:	void M_Push(const uint16 &value);			// private to hide this until it works 
			private:	void M_Push(const uint32 &value);			// private to hide this until it works
			private:	void M_Push(const uint64 &value);			// private to hide this until it works
			private:	void M_Push(const float &value);			// private to hide this until it works
			public:	void M_Push(const double &value);
			private:	void M_Push(const bool &value);				// private to hide this until it works
			public:	void M_Push(const C_Variant &value);
			private:	void M_Push(const int64 *value, int &size); // private to hide this until it works

			/**
			* @brief Push value to the the property.
			* This method will push current value with time (and status) to the server
			*
			* @param value Current value of the variable, generation time, value status
			*/
			public:		void M_Push(const std::string &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			private:		void M_Push(const char &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);	// private to hide this until it works
			private:		void M_Push(const byte &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);	// private to hide this until it works
			private:		void M_Push(const int16 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);	// private to hide this until it works
			private:		void M_Push(const int32 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);	// private to hide this until it works
			public:		void M_Push(const int64 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			private:		void M_Push(const uint16 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);// private to hide this until it works
			private:		void M_Push(const uint32 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);// private to hide this until it works
			private:		void M_Push(const uint64 &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);// private to hide this until it works
			private:		void M_Push(const float &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);	// private to hide this until it works
			public:		void M_Push(const double &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);
			private:		void M_Push(const bool &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);	// private to hide this until it works
			public:		void M_Push(const C_Variant &value, const C_DateTime &time, const t_ValueStatus &status = g_VS_OK);

			private:	template <typename T> void M_PushBinary(const T &value, const C_DateTime &time = NULL, const t_ValueStatus &status = g_VS_OK);
			private:	template <typename T> void M_PushBinary(const T *value, int &size, const C_DateTime &time = NULL, const t_ValueStatus &status = g_VS_OK);
			private:	void M_PushBinary(const C_Variant &value, const C_DateTime &time = NULL, const t_ValueStatus &status = g_VS_OK);
			private:	void M_PushJson(const std::string &value, const C_DateTime &time = NULL, const t_ValueStatus &status = g_VS_OK);

			private:	C_MiaDriver		*m_Driver;
			private:	t_CallId			m_PusherId;
		};

		/**
		 * @brief C_MiaDriver class is used to interface with Vtrin-NetServer (enabled with Websocket connection).
		 * The driver implements secure websocket connections together with
		 * basic authentication. Methods in this class operate in a synchronous order,
		 * despite its base operates in the asynchronous style. That means that user
		 * shall take into account the time it takes to execute a call and handle also
		 * possible timeout exception.
		 *
		 * For further instructions on each commands, please query for further
		 * document on the server application.
		 * \ingroup MiaDriver
		 */
		class Mia_EXPORT C_MiaDriver: public C_MiaDriverBase
		{
			/**
			 * @brief Create a C_MiaDriver instance which will be used to connect
			 * to the RTDB History server given by the connection string.
			 *
			 * @param connectionString The connection string will be used to
			 * connect to Mia Server. <br>
			 * Example: ws://localhost/connectionstring.data: this will create an unsecure connection. <br>
			 * 			  wss://localhost:443/connectionstring.data: this will create a secure connection. <br>
			 * 			  wst://localhost:443/connectionstring.data: this will create a secure connection. <br>
			 * 			  It will also discard server certificate verification.
			 */
			public:	C_MiaDriver(const std::string &connectionString);

			public:	virtual ~C_MiaDriver();

			/**
			 * @brief Get class.
			 * @throw C_ClassNotFoundException is the className is not found.
			 */
			public:	C_DbClass M_GetClass(const std::string &className);
			public:	C_DbClass operator[](const std::string &className);

			/**
			 * @brief Connect the driver.
			 * Also apply authentication on a secure connection.
			 *
			 * @param username The username for authentication.
			 * @param password The password for authentication.
			 *
			 * @throw C_SSLException If the SSL connection is unable to established.
			 * @throw C_SocketException If the socket connection is unable to established.
			 * @throw C_UnauthorizedException If the connection is not authorized, or username or password is incorrect.
			 * @throw C_BadConnectionStringException If the connection string is in bad shape.
			 * @throw C_ServerException If server is unable to handle the request.
			 */
			public:	bool M_Connect(const std::string &username, const std::string &password);

			/**
			 * @brief Get the state of the driver.
			 * @return Driver state.
			 */
			public:	t_State M_GetStatus();

			/**
			 * @brief Return true if the driver is ready
			 *
			 */
			public: bool M_IsReady();

			/**
			 * @brief Call the method methodName with parameters given by parameters.
			 * The driver will queue the message into its internal buffer and wait until the method returns.
			 *
			 * @param methodName Method name of the call.
			 * @param parameters Parameters of the call.
			 * @param callId when given, the method will assign this parameter with the callId stored
			 * internally.
			 *	@param retJson The result json from the call
			 * @return The result of the call if no retJson is inserted.
			 *
			 * @throw C_ServerException If the server experiences error in the call.
			 */
			public:	std::string M_Call(const std::string &methodName, const std::string &parameters, t_CallId* callId = 0, C_Variant* retJson = 0);

			/**
			 * @brief Subscribe the method methodName with parameters given by parameters.
			 * The driver will queue the message into its internal buffer and wait until the method returns.
			 *
			 * @param methodName Method name of the call.
			 * @param parameters Parameters of the call.
			 * @param callId when given, the method will assign this parameter with the callId stored
			 * internally.
			 *
			 * @return The result of the call.
			 *
			 * @throw C_ServerException If the server experiences error in the call.
			 */
			public:	bool M_Subscribe(const std::string &methodName, const std::string &parameters, C_Variant *result, F_Callback callback = 0, t_CallId* callId = 0);

			/**
			 * @brief UnSubscribe the call indentified by callId.
			 * The driver remove the internal buffered callud.
			 *
			 * @param callId the id to be unsubscribed internally.
			 *
			 * @return The result of the call.
			 *
			*/
			public:	bool M_UnSubscribe(int64 callId);

			/**
			 * @return The server time in UTC
			 */
			public: C_DateTime M_GetServerTimeUtc();
			/**
			 * @brief Fetch property data from the given className and properties.
			 * The fetch uses fetchMax to limit the amount of records return.
			 * Where and whereData are utilized as filtering string.
			 *
			 * @param className Name of the class to fetch data from.
			 * @param properties Fetch data only from these properties.
			 * @param fetchMax Maximum number of records return. -1 for unlimited.
			 * @param where The filtering string
			 * @param data1 (in-opt) Data for the properties.
			 * @param data2 (in-opt) Data for the properties.
			 * @param data3 (in-opt) Data for the properties.
			 * @param data4 (in-opt) Data for the properties.
			 * @param data5 (in-opt) Data for the properties.
			 * @param data6 (in-opt) Data for the properties.
			 * @param data7 (in-opt) Data for the properties.
			 * @param data8 (in-opt) Data for the properties.
			 *
			 * @return The result of the call
			 *
			 * @throw C_ServerException If the server experiences error in the call.
			 */
			public:	std::string M_FetchClassData(const std::string &className, const std::string &properties, int fetchMax = -1,
			   const std::string &where = "", const C_Variant& data1 = C_Variant(), const C_Variant& data2 = C_Variant(),
			   const C_Variant& data3 = C_Variant(), const C_Variant& data4 = C_Variant(), const C_Variant& data5 =
			      C_Variant(), const C_Variant& data6 = C_Variant(), const C_Variant& data7 = C_Variant(),
			   const C_Variant& data8 = C_Variant());

			/**
			 * @brief Subscribe device session to begin data production.
			 *
			 * @param value Device meta-data.
			 * @param returnValue A pointer to the returning device subscription.
			 *
			 * @throw C_ServerException If the server experiences error in the call.
			 */
			public:	void M_SubscribeEquipmentSession(const C_Variant &value, C_Variant *returnValue);

			/**
			 * @brief Subscribe device session to begin data production.
			 *
			 * @param value Device meta-data.
			 * @param returnValue A pointer to the returning device subscription.
			 *
			 * @throw C_ServerException If the server experiences error in the call.
			 */
			public:	void M_SubscribeEquipmentSession(const std::string &value, std::string *returnValue);

			/**
			 * @brief Subscribe device session to begin data production.
			 *
			 * @param device Device to be subscribed. The device input will be updated with corresponding subscription.
			 *
			 * @throw C_ServerException If the server experiences error in the call.
			 */
			public:	bool M_SubscribeEquipmentSession(C_Device* device);

			/**
			 * @brief Update or insert values into the class given by className.
			 * Number of newValues and oldValues should correspond to number of
			 * properties and instances.
			 *
			 * @param className Name of the class to fetch data from.
			 * @param properties Fetch data only from these properties.
			 * @param instanceId The guid of the instance to be updated, give an empty string
			 * in case of insert operation.
			 * @param newValues Values to be inserted or updated.
			 * @param oldValues Old values.
			 *
			 * @return The result of the call.
			 *
			 * @throw C_ServerException If the server experiences error in the call.
			 */
			public:	std::string M_CommitClassDataRaw(const std::string &className, const std::string &properties,
			   const std::string &instanceId, const std::string &newValues, const std::string &oldValues);

			/**
			* @brief Update or insert values into the class given by className.
			* Number of newValues and oldValues should correspond to number of
			* properties and instances.
			*
			* @param className Name of the class to fetch data from.
			* @param properties Fetch data only from these properties.
			* @param instanceId The guid of the instance to be updated, give an empty string
			* in case of insert operation.
			* @param newValues Values to be inserted or updated.
			* @param oldValues Old values.
			*
			* @return The result of the call.
			*
			* @throw C_ServerException If the server experiences error in the call.
			*/
			public:	std::string M_CommitClassDataComposite(const std::string &className, const std::string &properties,
				const C_Variant& compositeId, const std::string &newValues, const std::string &oldValues);

			/**
			 * @brief Update or insert values into the class given by className.
			 * Number of newValues and oldValues should correspond to number of
			 * properties and instances.
			 *
			 * @param className Name of the class to fetch data from.
			 * @param properties Fetch data only from these properties.
			 * @param instanceId The guid of the instance to be updated, give an empty string
			 * in case of insert operation.
			 * @param data1 (in-opt) Data for the properties.
			 * @param data2 (in-opt) Data for the properties.
			 * @param data3 (in-opt) Data for the properties.
			 * @param data4 (in-opt) Data for the properties.
			 * @param data5 (in-opt) Data for the properties.
			 * @param data6 (in-opt) Data for the properties.
			 * @param data7 (in-opt) Data for the properties.
			 * @param data8 (in-opt) Data for the properties.
			 *
			 * @return The result of the call. Usually the class instance with commited data.
			 *
			 * @throw C_ServerException If the server experiences error in the call.
			 */
			public: std::string M_CommitClassData(const std::string &className, const std::string &properties,
			   const std::string &instanceId, const C_Variant& data1 = C_Variant(), const C_Variant& data2 = C_Variant(),
			   const C_Variant& data3 = C_Variant(), const C_Variant& data4 = C_Variant(), const C_Variant& data5 = C_Variant(),
			   const C_Variant& data6 = C_Variant(), const C_Variant& data7 = C_Variant(), const C_Variant& data8 = C_Variant());

			/**
			 * @brief Create a general pusher.
			 * Number of newValues and oldValues should correspond to number of
			 * properties and instances.
			 *
			 * @param className Name of the class to produce data to.
			 * @param properties Produce data only to these properties.
			 * @param instanceId The Guid of the instance to be updated, give an empty string
			 * in case of insert operation.
			 * @param operationMode Mode of the pusher.
			 *
			 * @return The pusher id.
			 *
			 * @throw C_ServerException If the server experiences error in the call.
			 */
			public: t_CallId M_CreatePusher(const std::string &className, const std::string &instanceId, const std::string &properties, const int &operationMode);

			/**
			 * @brief Create a value pusher which uses variable name or variable id to produce value.
			 *
			 * @param className Name of the class to fetch data from.
			 *
			 * @return An instance of C_CurrenrValuePusher class, user need to free this class when completed.
			 *
			 * @throw C_ServerException If the server experiences error in the call.
			 */
			public:	C_CurrentValuePusher M_CreateValuePusher(const std::string &className = "CurrentValue");

			/**
			 * @brief Create a value pusher which uses variable name or variable id to produce value
			 *
			 * @param instanceId Instance id of the path to fetch data from
			 * @param property Property to produce value  to
			 *
			 * @return An instance of C_PropertyValuePusher class.
			 *
			 * @throw C_ServerException If the server experiences error in the call.
			 */
			public: C_PropertyValuePusher M_CreatePropertyValuePusher(const std::string &instanceId, const std::string &property);

			/**
			 * @brief Create a value pusher which uses variable name or variable id to produce value
			 *
			 * @param instanceId Instance id of the path to fetch data from
			 * @param property Property to produce value  to
			 *
			 * @return An instance of C_PropertyValuePusher class.
			 *
			 * @throw C_ServerException If the server experiences error in the call.
			 */
			public:	C_PropertyValuePusher M_CreatePropertyValuePusher(const C_Guid &instanceId, const std::string &property);

			/**
			 * @brief Retrieve an equipment using the equipment name.
			 * @param equipmentName The name of equipment.
			 *
			 * @return The matching equipment name.
			 *
			 * @throw C_ServerException If the server experiences error in the call.
			 */
			public:	C_Equipment M_GetEquipment(const std::string &equipmentName);

			/**
			 * @brief Retrieve list of equipments matching the filter, filterData and limited by fetchMax.
			 * @param equipments The list of equipments to be updated.
			 * @param filter The filter string.
			 * @param filterData The filter data.
			 * @param fetchMax The maximum number of equipments to be fetched.
			 *
			 * @return The matching equipment name.
			 *
			 * @throw C_ServerException If the server experiences error in the call.
			 */
			public:	bool M_GetEquipment(C_Equipments *equipments, const std::string &filter, const std::string &filterData,  int fetchMax = -1);

			/**
			 * @brief Retrieve list of equipments matching the filter, filterData and limited by fetchMax.
			 * @param equipments The list of equipments to be updated.
			 * @param filter The filter string.
			 * @param filterData The filter data.
			 * @param fetchMax The maximum number of equipments to be fetched.
			 *
			 * @return The matching equipment name.
			 *
			 * @throw C_ServerException If the server experiences error in the call.
			 */
			public:	bool M_GetEquipment(C_Equipments *equipments, const std::string &filter, const C_ArgumentList &filterData,  int fetchMax = -1);

			private:	void M_ParseDeviceSubscription(C_Device* device, C_Variant* out);
			private: C_Variant M_GetJsonArray(const std::string &in, const std::string &element);

			/* Private implementation sections*/
			private:	class C_MiaDriverPrivate;

			friend class C_MiaDriverPrivate;
			private:	C_MiaDriverPrivate *m_BlockingPrivate;
		};

		template<typename T>
		inline void C_CurrentValuePusherClass::M_PushBinary(const int & variableId, const T & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			Mia_THIS_ROUTINE("C_CurrentValuePusherClass::M_PushBinary");
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

		template<typename T>
		inline void C_PropertyValuePusherClass::M_PushBinary(const T & value, const C_DateTime & time, const t_ValueStatus & status)
		{
			Mia_THIS_ROUTINE("C_PropertyValuePusherClass::M_PushBinary");
			TRY
			{
				C_WebsocketStreamBinary stream;
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

		template<typename T>
		inline void C_PropertyValuePusherClass::M_PushBinary(const T *value, int &size, const C_DateTime & time, const t_ValueStatus & status)
		{
			Mia_THIS_ROUTINE("C_PropertyValuePusherClass::M_PushBinary");
			TRY
			{
				C_WebsocketStreamBinary stream;
				stream.M_BeginArray();
				stream << m_PusherId;
				stream.M_BeginArray();
				stream << g_TC_ArrayFlag;
				for (int i = 0; i < size; i++)
				{
					stream << value[i];
				}
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
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, *value, time, (int)status);
			})
		}
} /* namespace Mia */
} /* namespace ABB */

#endif /* RTDB_MIADRIVER_H_ */
