
/*
 * Mia_Equipment.h
 *
 *  Created on: Apr 28, 2016
 *      Author: Tuan Vu
 */

#ifndef _MIA_EQUIPMENT_H_
#define _MIA_EQUIPMENT_H_

#include <string>
#include <list>
#include <assert.h>
#include "Mia_Base.h"
#include "Mia_DriverBase.h"
#include "Mia_Driver.h"

namespace ABB
{
namespace Mia
{
	Mia_CLASS(C_Equipment);
	Mia_CLASS(C_EquipmentPropertyInfo);
	Mia_CLASS(C_EquipmentProperty);
	Mia_CLASS(C_EquipmentInstance);
	typedef C_EquipmentPropertys C_EquipmentProperties;

	/**
	 * \defgroup EquipmentAPI cpmPlus Embedded SDK - Equipment API
	 */
	 /**
	 * @brief This class contains the equipment model and its properties.
	 * User can create instance of this class using the driver.
	 * This class is immutable, which means users cannot modified its
	 * properties or instantiate it manually.
	 *
	 * This class is wrapped by C_Equipment
	 *
	 * \ingroup EquipmentAPI
	 */

	class C_EquipmentClass
	{
		private:	C_EquipmentClass(){}
		private:	C_EquipmentClass(C_DbClassInstance &equipment);
		private:	C_EquipmentClass(const C_Guid &guid, const std::string &type, const std::string &name,
			const std::string &base = "", const std::string &description = "");
		private:	void operator=(const C_Equipment& model)	{}

		private:	bool M_UpdateProperties(C_Equipment &e);

		private:	static void M_S_ParseEquipment(const std::string &data, C_Equipments &list);

		private:	void M_ParseEquipmentInstances(C_EquipmentInstances *instances, C_EquipmentPropertyInfos *propertyinfos, const std::string &result);

		public:	virtual ~C_EquipmentClass();

		/**
		 * @brief Update equipment with the new instance
		 */
		public:	void M_Update(C_Equipment& equipment, C_Equipment &ownequipment);

		/**
		 * @brief Add an instance of this class. Created object must be saved by calling
		 * M_CommitChanges(), otherwise all changes will be lost.
		 *
		 * @see C_AutoPointer
		 * @see M_CommitChanges
		 * @return An object of this class. The object is managed inside the C_AutoPointer.
		 */
		public:	C_EquipmentInstance M_Add(const std::string &path, const C_Guid &guid = C_Guid());

		/**
		 * @brief Remove the instance from database.
		 *
		 * @param instance Instance to be removed.
		 *
		 * @return true if the method succeeds
		 */
		public:	bool M_Remove(C_EquipmentInstance &instance);

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
		public:	void M_GetProperties(C_EquipmentPropertyInfos *properties, bool refresh = false);

		/**
		 * @brief Get a property info using the property name
		 *
		 * @param propertyName Name of the property
		 *
		 * @return The property info associated with the given name. Null pointer received if not found.
		 * @throw C_PropertyNotFoundException if the property is not found and the ENABLE_EXCEPTION preprocessor is set
		 * @throw C_ServerException if there is exception happens on the server
		 */
		public:	C_EquipmentPropertyInfo M_GetProperty(const std::string &propertyName);

		/**
		 * @brief Get a list of instances limited by the fetchMax
		 *
		 * @param instances The list of instances to be updated
		 * @param fetchMax The maximum number of instance to be fetched
		 *
		 * @return -1 If the function encounters exception.
		 * @return The number of instances fetched if the function is able to update the instances list.
		 * @throw C_ServerException if there is exception happens on the server
		 *
		 */
		public:	int M_GetInstanceSet(C_EquipmentInstances *instances, int fetchMax = -1);

		/**
		 * @brief Get a list of instances defined by the whereString and arguments list
		 *
		 * @param instances The list of instances to be updated
		 * @param fetchMax The maximum number of instance to be fetched
		 * @param whereString The string to filter the result
		 * @param data1 The argument for the whereString
		 * @param data2 The argument for the whereString
		 * @param data3 The argument for the whereString
		 * @param data4 The argument for the whereString
		 * @param data5 The argument for the whereString
		 *
		 * @return True if the function is able to update the instances list.
		 * @throw C_ServerException if there is exception happens on the server
		 *
		 */
		public:	bool M_GetInstanceSet(C_EquipmentInstances *instances, int fetchMax, const std::string &whereString,
			const C_Variant& data1 = C_Variant(), const C_Variant& data2 = C_Variant(), const C_Variant& data3 =
				C_Variant(), const C_Variant& data4 = C_Variant(), const C_Variant& data5 = C_Variant());

		/**
		 * @brief Get an instance specified by the instance id
		 *
		 * @param id The instance id
		 *
		 * @return True if the function is able to update the instances list.
		 * @throw C_ServerException if there is exception happens on the server
		 */
		public:	C_EquipmentInstance M_GetInstanceById(const C_Variant &id);

		/**
		 * @brief Get the GUID of the equipment model
		 */
		public:	const C_Guid &M_GetGuid();

		/**
		 * @brief Get the name of the equipment model
		 */
		public:	const std::string &M_GetName();

		/**
		 * @brief Get the base of the equipment model
		 */
		public:	const std::string &M_GetBase();

		/**
		 * @brief Get the type of the equipment model
		 */
		public:	const std::string &M_GetType();

		/**
		 * @brief Get the description of the equipment model
		 */
		public:	const std::string &M_Description();

		/**
		 * @brief Get the list of properties of the equipment model
		 */
		public:	C_EquipmentPropertyInfos M_GetProperties();

		/**
		 * @brief Get the specific property by name
		 * @param propertyName The name of the property
		 *
		 * @return The matching property. null will be return if no matching property is found.
		 */
		public:	C_EquipmentPropertyInfo operator[](const std::string &propertyName);

		private: class C_EquipmentClassPrivate;
		private: C_EquipmentClassPrivate *m_Private;

		friend class C_MiaDriverBase;
		friend class C_MiaDriver;
		friend class C_EquipmentPropertyInfoClass;
		friend class C_AutoPointer<C_Equipment> ;
	};

	
	/**
	 * @brief This class contains the property information, corresponding to a record
	 * of EquipmentPropertyInfo table on the server.
	 * This class is an immutable class, which means it cannot be instantiated and its
	 * properties cannot be modified, but rather be retrieved from the C_EquipmentModel
	 * and C_EquipmentInstance.
	 *
	 * This class is wrapped by C_EquipmentPropertyInfo
	 * \ingroup EquipmentAPI
	 */
	class C_EquipmentPropertyInfoClass
	{
		protected:	C_EquipmentPropertyInfoClass() : m_Equipment(), m_Type(g_PT_UNKNOWN), m_Historized(false), m_Interval(0){}

		protected:	C_EquipmentPropertyInfoClass(C_Equipment &equipment, const std::string &displayName,
			const std::string &category, const std::string &description, const std::string &type,
			const std::string &applicationtype, const std::string &unit, const std::string &mapping, bool historized,
			const std::string &transformation, const C_Variant &min = C_Variant(), const C_Variant &max = C_Variant(),
			const C_Variant &defaultValue = C_Variant());

		protected:	C_EquipmentPropertyInfoClass(C_Equipment &equipment, C_DbClassInstance &propertyInfo);

		private:	void operator=(const C_EquipmentPropertyInfo& property) {}

		protected:	void M_Update(const C_EquipmentPropertyInfo &info);

		/**
		 * @brief Get the EquipmentModel of this property
		 */
		public:	const C_Equipment M_GetEquipment() const
		{
			return m_Equipment;
		}

		/**
		 * @brief Get the DisplayName of this property
		 */
		public:	const std::string &M_GetName() const
		{
			return m_DisplayName;
		}

		/**
		 * @brief Get the category of this property
		 */
		public:	const std::string &M_GetCategory() const
		{
			return m_Category;
		}

		/**
		 * @brief Get the Description of this property
		 */
		public:	const std::string &M_GetDescription() const
		{
			return m_Description;
		}

		/**
		 * @brief Get date type of this property. If this property is used to store historical
		 * data, the type shall be unsigned integer, not the type of the value to be stored
		 *
		 */
		public:	const t_PropertyType &M_GetType() const
		{
			return m_Type;
		}

		/**
		 * @brief Get the ApplicationType of this property
		 */
		public:	const std::string &M_GetApplicationType() const
		{
			return m_Applicationtype;
		}

		/**
		 * @brief Get Unit of this property
		 */
		public:	const std::string &M_GetUnit() const
		{
			return m_Unit;
		}

		/**
		 * @brief Get the Mapping of this property
		 */
		public:	const std::string M_GetMapping() const
		{
			return m_Mapping;
		}

		/**
		 * @brief Return true is this property is used to stored historical data
		 */
		public:	bool M_IsHistorized() const
		{
			return m_Historized;
		}

		/**
		 * @brief Get the Transformation of this property
		 */
		public:	const std::string &M_GetTransformation() const
		{
			return m_Transformation;
		}

		/**
		 * @brief Get the minimum value of this property
		 */
		public:	const C_Variant &M_GetMinValue() const
		{
			return m_MinValue;
		}

		/**
		 * @brief Get the maximum value of this property
		 */
		public:	const C_Variant &M_GetMaxValue() const
		{
			return m_MaxValue;
		}

		/**
		 * @brief Get the default of this property
		 */
		public:	const C_Variant &M_GetDefaultValue() const
		{
			return m_DefaultValue;
		}

		/**
		 * @brief Get the collection rate of this property
		 */
		public:	const int64 &M_GetRate() const
		{
			return m_Interval;
		}

		/**
		 * @brief Get the EquipmentHint of this property
		 */
		public:	const std::string M_GetEquipmentHint() const
		{
			return m_Mapping;
		}

		/**
		 * @brief Get the extra configuration parameters of this property.
		 * The configuration is given in the configuration field in the EquipmentPropertyInfo table. \n
		 */
		public:	std::string operator[](const std::string &propertyInfo);

		protected:	C_Equipment m_Equipment;
		private:	std::string m_DisplayName;
		private:	std::string m_Category;
		private:	std::string m_Description;
		private:	t_PropertyType m_Type;
		private:	std::string m_Applicationtype;
		private:	std::string m_Unit;
		private:	std::string m_Mapping;
		private:	bool m_Historized;
		private:	std::string m_Transformation;

		private:	C_Variant m_MinValue;
		private:	C_Variant m_MaxValue;
		private:	C_Variant m_DefaultValue;
		private:	int64 m_Interval;

		private:	static void M_S_ParseProperties(C_Equipment& e, const std::string &data, C_EquipmentPropertyInfos &list);
		friend class C_MiaDriverBase;
		friend class C_MiaDriver;
		friend class C_EquipmentClass;
		friend class C_EquipmentInstanceClass;
		friend class C_EquipmentPropertyClass;
	};

	/**
	 * @brief This class contains the property value handler
	 *
	 * This class is wrapped by C_EquipmentProperty
	 *
	 * \ingroup EquipmentAPI
	 */
	class C_EquipmentPropertyClass
	{
		protected:	C_EquipmentPropertyClass(C_EquipmentInstance& equipmentInstance, C_EquipmentPropertyInfo& property,  const C_Variant &value = C_Variant());
		protected:	C_EquipmentPropertyClass(C_EquipmentInstanceClass* equipmentInstance, C_EquipmentPropertyInfo& property,const C_Variant &value = C_Variant());
		protected:	C_EquipmentPropertyClass(): m_EquipmentInstance(0) {}

		public:	~C_EquipmentPropertyClass();

		private:	void operator==(const C_EquipmentInstance & ei) {}

		public:	C_EquipmentPropertyInfo & M_GetPropertyInfo() {	return m_PropertyInfo; }
		/**
		 * @brief Set the property value of this equipment property instance
		 * @param value Value to be setl
		 */
		public:	void operator=(const C_Variant& value);

		/**
		 * @brief Set the property value of the equipment instance
		 * @param value Value to be set
		 * @param time The time to set the value, only applied to property that has history.
		 * @param status Status of the value, only applied to property that has history
		 */
		public:	void M_SetValue(const C_Variant& value, const C_DateTime* time = 0, const t_ValueStatus &status = g_VS_OK);

		/**
		 * @brief Get the current value of this property.
		 *
		 */
		public:	const C_Variant &M_GetValue() const { return m_CurrentValue; }

		/**
		* @brief Get the value saved on server of this property.
		*
		*/
		public: C_Variant M_GetServerValue();

		/**
		 * @brief Get the current value of this property.
		 *
		 */
		public:	const C_DateTime &M_GetCurrentTime() const { return m_LastUpdateTime; }

		/**
		 * @brief Prepare to start value production. This function prepares a channel for sending value.
		 * Resource associated with this channel shall be removed upon calling M_EndProduction(). If there
		 * is already an existing channel, it will not create another before the current one is released.
		 *
		 * @see M_BeginProduction
		 */
		public:	void M_BeginProduction();

		/**
		 * @brief Stop value production and release associated resource.
		 * @see M_EndProduction
		 */
		public:	void M_EndProduction();

		private:	bool M_Save();

		private:	C_PropertyValuePusher M_GetPusher();

		private:	C_ThreadLock m_Lock;
		private:	C_Variant m_CurrentValue;
		private:	C_EquipmentInstanceClass* m_EquipmentInstance;
		private:	C_DateTime m_LastUpdateTime;
		private:	C_PropertyValuePusher m_Pusher;
		private:	C_EquipmentPropertyInfo m_PropertyInfo;

		friend class C_MiaDriverBase;
		friend class C_MiaDriver;
		friend class C_EquipmentClass;
		friend class C_EquipmentPropertyInfoClass;
		friend class C_EquipmentInstanceClass;
	};
	
	/**
	 * @brief An instance of the C_Equipment in the server
	 *
	 * This class is wrapped by C_EquipmentInstance
	 *
	 * \ingroup EquipmentAPI
	 */
	class C_EquipmentInstanceClass
	{
		private:	C_EquipmentInstanceClass(C_MiaDriver* driver, C_EquipmentClass* equipment, const C_Variant& guid):	m_Driver(driver)
		{
			if (equipment)
			{
				m_Equipment = equipment;
			}
			m_Id = guid.M_ToGuid();
		}

		private:	C_EquipmentInstanceClass(C_MiaDriver* driver, C_EquipmentClass* equipment, C_DbClassInstance equipmentInstance): m_Driver(driver)
		{
			if (equipment)
			{
				m_Equipment = equipment;
			}
			if (equipmentInstance)
			{
				m_Instance = equipmentInstance;
				m_Instance->M_GetValue("Id", &m_Id);
				m_Instance->M_GetValue("Name", &m_Path);
				m_Instance->M_GetValue("Parent", &m_Parent);
			}
		}

		private:	C_EquipmentInstanceClass(const std::string &id, const std::string& name, const std::string& displayName,
			const std::string& parent, C_Equipment& equipment, C_MiaDriver *driver);

		private:	C_EquipmentInstanceClass(C_MiaDriver* driver, C_Equipment& equipment, const C_Variant& guid = C_Guid(),
			const std::string& name = "", const std::string& displayName = "", const std::string& parent = "");

		private:	static void M_S_ParseEquipmentModels(C_Equipment &equipment, const std::string &data,
			C_EquipmentInstances &list, C_MiaDriver * driver);

		private:	void operator==(const C_EquipmentInstance & ei)
		{}

		public:	~C_EquipmentInstanceClass();

		/**
		 * @brief Remove the instance
		 */
		public:	void M_Remove();

		/**
		 * @brief Update the instance
		 */
		/*public: void M_Update(C_EquipmentInstance &instance);*/

		/**
		 * @brief Get the list of properties of the equipment model instance
		 */
		public:	C_EquipmentProperties M_GetProperties();

		/**
		 * @brief Get the specific property by name
		 * @param propertyName The name of the property
		 *
		 * @return The matching property. null will be return if no matching property is found.
		 */
		public:	C_EquipmentProperty operator[](const std::string &propertyName);

		/**
		 * @brief Get the specific property by name
		 * @param propertyName The name of the property
		 *
		 * @return The matching property. null will be return if no matching property is found.
		 */
		public:	C_EquipmentProperty operator[](const C_EquipmentPropertyInfo& property);

		/**
		 * @brief Get the specific property by name
		 * @param property The property
		 *
		 * @return The matching property. null will be return if no matching property is found.
		 */
		public:	C_EquipmentProperty M_GetProperty(const C_EquipmentPropertyInfo& property);

		/**
		 * @brief Get the specific property by name
		 * @param propertyName The name of the property
		 *
		 * @return The matching property. null will be return if no matching property is found.
		 */
		public:	C_EquipmentProperty M_GetProperty(const std::string &propertyName);

		/**
		 * @brief Get the GUID of the equipment instance
		 */
		public:	const C_Guid &M_GetId()
		{
			return m_Id;
		}

		/**
		 * @brief Set the GUID of the equipment instance
		 */
		public:	void M_SetId(const C_Guid & guid)
		{
			m_Id = guid;
		}

		/**
		 * @brief Get the path of the equipment instance
		 */
		public:	const std::string &M_GetPath()
		{
			return m_Path;
		}

		/**
		 * @brief Set the path of the equipment instance. Setting path also
		 * changes the DisplayName
		 */
		public:	void M_SetPath(const std::string &path)
		{
			m_Path = path;
		}

		/**
		 * @brief Get the path of the distplay
		 */
		public:	const std::string &M_GetDisplayName()
		{
			return m_DisplayName;
		}

		/**
		 * @brief Set the path of the distplay
		 */
		public:	void M_SetDisplayName(const std::string &displayName)
		{
			m_DisplayName = displayName;
		}

		/**
		 * @brief Get the parent of the equipment instance
		 */
		public:	const std::string &M_GetParent()
		{
			return m_Parent;
		}

		/**
		 * @brief Get the EquipmentModel of the equipment instance
		 */
		public:	C_EquipmentClass* M_GetEquipment()
		{
			return m_Equipment;
		}

		/*
		 * @brief Set the property value of the equipment instance
		 * @param property The property to set value to
		 * @param value Value to be set
		 * @param time The time to set the value, only applied to property that has history.
		 * @param status Status of the value, only applied to property that has history
		 *//*
		public:	void M_SetValue(C_EquipmentPropertyInfo& property, const C_Variant& value, const C_DateTime &time = C_DateTime::M_S_Now(), const t_ValueStatus status = g_VS_OK);
	  */
		/**
		 * @brief Set the property value of the equipment instance
		 * @param property The property to set value to
		 * @param value Value to be set
		 */
		public:	void M_SetValue(const std::string &property, const C_Variant& value);

		/**
		 * @brief Set the property value of the equipment instance
		 * @param property The property to set value to
		 * @param value Value to be set
		 * @param timestamp Timestamp of the value
		 */
		public:	void M_SetValue(const std::string &property, const C_Variant& value, const C_DateTime &timestamp);

		private: C_DbClassInstance M_GetInstance();

		private:	C_Guid m_Id;
		private:	std::string m_Path;
		private:	std::string m_DisplayName;
		private:	std::string m_Parent;
		private:	C_EquipmentClass *m_Equipment;

		private:	C_ThreadLock m_Lock;
		private: C_MiaDriver *m_Driver;
		private:	C_DbClassInstance m_Instance;
		private:	unordered_map<std::string, C_EquipmentProperty> m_PropertyInstances;

		friend class C_MiaDriverBase;
		friend class C_MiaDriver;
		friend class C_EquipmentClass;
		friend class C_EquipmentPropertyInfoClass;
		friend class C_EquipmentPropertyClass;
	};

	
	std::ostream &operator<<(std::ostream &os, const C_Equipment &equipment);
	std::ostream &operator<<(std::ostream &os, const C_EquipmentPropertyInfo &equipmentPropertyInfo);
	
} /* namespace MIA */
} /* namespace ABB */

#endif /* _MIA_EQUIPMENT_H_ */
