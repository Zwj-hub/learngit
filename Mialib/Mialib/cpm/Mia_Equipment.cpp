/*
 * Mia_Equipment.cpp
 *
 *  Created on: Apr 28, 2016
 *      Author: Tuan Vu
 */

#include "Mia_Equipment.h"
#include "Mia_Exception.h"

namespace ABB
{
namespace Mia
{
		class C_EquipmentClass::C_EquipmentClassPrivate
		{
			public: C_EquipmentClassPrivate(const C_Guid &guid, const std::string &type, const std::string &name,
			   const std::string &base, const std::string &description) :
				m_Guid(guid), m_Type(type), m_Name(name), m_Base(base), m_Description(description)
			{}

			private:	unordered_map<std::string, C_EquipmentPropertyInfo> m_Properties;
			private:	C_ThreadLock m_Lock;
			private:	C_Guid m_Guid;
			private:	std::string m_Type;
			private:	std::string m_Name;
			private:	std::string m_Base;
			private:	std::string m_Description;
			private:	C_DbClassInstance m_Equipment;

			friend class C_MiaDriverBase;
			friend class C_MiaDriver;
			friend class C_EquipmentClass;
			friend class C_EquipmentInstanceClass;
			friend class C_EquipmentPropertyClass;
		};

		C_EquipmentClass::C_EquipmentClass(const C_Guid &guid, const std::string &type, const std::string &name,
		   const std::string &base, const std::string &description)
		{
			// TODO Auto-generated constructor stub
			m_Private = DBG_NEW C_EquipmentClassPrivate(guid, type, name, base, description);
		}

		C_EquipmentClass::C_EquipmentClass(C_DbClassInstance &equipment)
		{
			m_Private = DBG_NEW C_EquipmentClassPrivate(equipment->M_GetValue("Id")->M_ToGuid(),
				"",
				equipment->M_GetValue("Name")->M_ToString(),
				equipment->M_GetValue("Base")->M_ToString(),
				"");
			m_Private->m_Equipment = equipment;
			m_Private->m_Guid = equipment->M_GetValue("Id")->M_ToGuid();
			//m_Description = equipment->M_GetValue("Description")->M_ToString();
			m_Private->m_Base = equipment->M_GetValue("Base")->M_ToString();
			m_Private->m_Name = equipment->M_GetValue("Name")->M_ToString();
		}

		C_EquipmentPropertyInfos C_EquipmentClass::M_GetProperties()
		{
			C_Locker locker(&m_Private->m_Lock);
			C_EquipmentPropertyInfos props;
			for (unordered_map<std::string, C_EquipmentPropertyInfo>::iterator iter = m_Private->m_Properties.begin();
			   iter != m_Private->m_Properties.end(); iter++)
			{
				props.push_back(iter->second);
			}
			return props;
		}

		C_EquipmentPropertyInfo C_EquipmentClass::operator[](const std::string &propertyName)
		{
			unordered_map<std::string, C_EquipmentPropertyInfo>::iterator iter;
			C_Locker locker(&m_Private->m_Lock);
			iter = m_Private->m_Properties.find(propertyName);
			if (iter != m_Private->m_Properties.end()) return iter->second;
			return C_EquipmentPropertyInfo();
		}

		C_EquipmentClass::~C_EquipmentClass()
		{
			{
				C_Locker locker(&m_Private->m_Lock);
				m_Private->m_Properties.clear();
			}

			delete m_Private;
		}

		C_EquipmentPropertyInfoClass::C_EquipmentPropertyInfoClass(C_Equipment &equipment, const std::string &displayName,
		   const std::string &category, const std::string &description, const std::string &type,
		   const std::string &applicationtype, const std::string &unit, const std::string &mapping, bool historized,
		   const std::string &transformation, const C_Variant &min, const C_Variant &max, const C_Variant &defaultValue) :

			m_DisplayName(displayName), m_Category(category), m_Description(description), m_Type(g_PT_INT), m_Applicationtype(
			   applicationtype), m_Unit(unit), m_Mapping(mapping), m_Historized(historized), m_Transformation(
			   transformation)
		{
			m_Equipment = (equipment);
			m_MinValue = min;
			m_MaxValue = max;
			m_DefaultValue = defaultValue;
			m_Type = C_Variant::M_S_FromStringToPropertyType(type);
		}

		void C_EquipmentPropertyInfoClass::M_Update(const C_EquipmentPropertyInfo &info)
		{
			if (m_DisplayName != info->M_GetName()) m_DisplayName = info->M_GetName();
			if (m_Applicationtype != info->M_GetApplicationType()) m_Applicationtype = info->M_GetApplicationType();
			if (m_Category != info->M_GetCategory()) m_Category = info->M_GetCategory();
			if (m_DefaultValue != info->M_GetDefaultValue()) m_DefaultValue = info->M_GetDefaultValue();
			if (m_Description != info->M_GetDescription()) m_Description = info->M_GetDescription();
			if (m_Historized != info->M_IsHistorized()) m_Historized = info->M_IsHistorized();
			if (m_Mapping != info->M_GetMapping()) m_Mapping = info->M_GetMapping();
			if (m_MaxValue != info->M_GetMaxValue()) m_MaxValue = info->M_GetMaxValue();
			if (m_MinValue != info->M_GetMinValue()) m_MinValue = info->M_GetMinValue();
			if (m_Type != info->M_GetType()) m_Type = info->M_GetType();
			if (m_Unit != info->M_GetUnit()) m_Unit = info->M_GetUnit();
			if (m_Transformation != info->M_GetTransformation()) m_Transformation = info->M_GetTransformation();
		}

		C_EquipmentPropertyInfoClass::C_EquipmentPropertyInfoClass(C_Equipment& equipment,
		   C_DbClassInstance &propertyInfo)
		{
			m_Equipment = equipment;
			m_DisplayName = propertyInfo->M_GetValue("DisplayName")->M_ToString();
			m_Category = propertyInfo->M_GetValue("Category")->M_ToString();
			m_Description = propertyInfo->M_GetValue("Description")->M_ToString();
			std::string type = propertyInfo->M_GetValue("Type")->M_ToString();
			m_Type = C_Variant::M_S_FromStringToPropertyType(type);
			m_Applicationtype = propertyInfo->M_GetValue("Applicationtype")->M_ToString();
			m_Unit = propertyInfo->M_GetValue("Unit")->M_ToString();
			//propertyInfo->M_GetValue("Unit")->M_GetValue(&m_Historized);
			m_MinValue.M_SetValue(*(propertyInfo->M_GetValue("ValueMin").get()));
			m_MaxValue.M_SetValue(*(propertyInfo->M_GetValue("ValueMax").get()));
			m_DefaultValue.M_SetValue(*(propertyInfo->M_GetValue("DefaultValue").get()));
			m_Historized = propertyInfo->M_GetValue("Historized")->M_ToBool();
			TRY
			{
				m_Mapping = propertyInfo->M_GetValue("DaDefinition")->M_ToString();
			}
			CATCH(ABB::Mia::C_PropertyNotFoundException &ex,
			{
				ex.M_SetHandled();
				m_Mapping = "";
			}
			)
		}

		bool C_EquipmentClass::M_UpdateProperties(C_Equipment &e)
		{
			C_MiaDriver *driver = this->m_Private->m_Equipment->M_GetClass()->M_GetDriver();

			C_DbClass ep = driver->M_GetClass("EquipmentPropertyInfo");
			C_DbClassInstances pis;

			if (ep->M_GetInstanceSet(&pis, -1, "Equipment = ?", this->m_Private->m_Name))
			{
				for (C_DbClassInstances::iterator iter = pis.begin(); iter != pis.end(); iter++)
				{
					C_DbClassInstance pi = *iter;
					std::string lcname = G_StringToLower(pi->M_GetValue("DisplayName")->M_ToString());

					C_EquipmentPropertyInfoClass *dbp = DBG_NEW C_EquipmentPropertyInfoClass(e, pi);
					C_Locker locker(&m_Private->m_Lock);
					m_Private->m_Properties[lcname] = C_EquipmentPropertyInfo(dbp);
				}
				return true;
			}

			return false;
		}

		void C_EquipmentClass::M_S_ParseEquipment(const std::string &data, C_Equipments &list)
		{
			if (data.length() < 3) return;
			//unsigned int index = 0, oldindex;
			std::string name, base, description, guid;
			C_WebsocketStream s(data);
			for (;;)
			{
				guid = "";
				name = "";
				base = "";
				description = "";
				s >> guid;
				s >> name;
				s >> base;
				s >> description;

				if (name.length() < 1) break;
				C_Guid g(guid);
				C_EquipmentClass *e = DBG_NEW C_EquipmentClass(g, description, name, base, description);
				list.push_back(C_Equipment(e));
				if (!s.M_HasMore()) break;
			}
		}

		void C_EquipmentPropertyInfoClass::M_S_ParseProperties(C_Equipment& e, const std::string &data,
		   C_EquipmentPropertyInfos &list)
		{
			if (data.length() < 3) return;
			//unsigned int index = 0, oldindex;
			std::string displayname, category, description, type, applicationtype, equipment, unit, mapping,
			   transformation, value;
			C_Variant min, max, def;

			bool historized;
			C_WebsocketStream s(data);
			for (;;)
			{
				displayname = "";
				category = "";
				description = "";
				type = "";
				applicationtype = "";
				equipment = "";
				unit = "";
				mapping = "";
				transformation = "";
				value = "";
				s >> displayname;
				s >> category;
				s >> description;
				s >> type;
				s >> applicationtype;
				s >> equipment;
				s >> value;
				if (value.length())
				{
					min = std::atof(value.c_str());
				}

				value = "";
				s >> value;
				if (value.length())
				{
					max = std::atof(value.c_str());
				}

				value = "";
				s >> value;
				if (value.length())
				{
					def = std::atof(value.c_str());
				}

				s >> unit;
				s >> mapping;
				s >> historized;
				s >> transformation;

				if (!displayname.length()) break;

				C_EquipmentPropertyInfoClass *p = DBG_NEW C_EquipmentPropertyInfoClass(e, displayname, category, description,
				   type, applicationtype, unit, mapping, historized, transformation, min, max, def);

				list.push_back(C_EquipmentPropertyInfo(p));
				if (!s.M_HasMore()) break;
			}
		}

		void C_EquipmentInstanceClass::M_S_ParseEquipmentModels(C_Equipment &equipment, const std::string &data,
		   C_EquipmentInstances &list, C_MiaDriver *driver)
		{
			if (data.length() < 3) return;
			unsigned int index = 0, oldindex;
			std::string id, e, path, displayname, parent;

			for (;;)
			{
				// Id
				oldindex = index;
				index = (int)data.find(",", index);
				if (index == data.npos) return;
				id = data.substr(oldindex, index - oldindex);

				// Name
				oldindex = index;
				index = (int)data.find(",", index);
				if (index == data.npos) return;
				path = data.substr(oldindex, index - oldindex);

				// DisplayName
				oldindex = index;
				index = (int)data.find(",", index);
				if (index == data.npos) return;
				displayname = data.substr(oldindex, index - oldindex);

				// Equipment
				oldindex = index;
				index = (int)data.find(",", index);
				if (index == data.npos) return;
				e = data.substr(oldindex, index - oldindex);

				// Parent
				oldindex = index;
				index = (int)data.find(",", index);
				if (index == data.npos) parent = data.substr(oldindex);
				parent = data.substr(oldindex, index - oldindex);

				C_EquipmentInstanceClass *e = DBG_NEW C_EquipmentInstanceClass(id, path, displayname, parent, equipment,
				   driver);
				C_EquipmentInstance ei(e);
				C_EquipmentPropertyInfos infos;
				equipment->M_GetProperties(&infos);

				for (C_EquipmentPropertyInfos::iterator iter = infos.begin(); iter != infos.end(); iter++)
				{
					C_EquipmentPropertyInfo epi = *iter;
					C_EquipmentPropertyClass * epc = DBG_NEW C_EquipmentPropertyClass(ei, epi);
					e->m_PropertyInstances[epi->M_GetName()] = C_EquipmentProperty(epc);
				}

				list.push_back(ei);

				if (index == data.npos) return;
			}
		}

		C_EquipmentInstanceClass::C_EquipmentInstanceClass(const std::string &id, const std::string& name,
		   const std::string& displayName, const std::string& parent, C_Equipment& equipment, C_MiaDriver* driver) :
			m_Path(name), m_DisplayName(displayName), m_Parent(parent), m_Equipment(equipment.get()), m_Driver(driver)
		{
			m_Id.M_SetData(id);

		}

		C_EquipmentInstanceClass::C_EquipmentInstanceClass(C_MiaDriver* driver, C_Equipment& equipment,
		   const C_Variant& guid, const std::string& name, const std::string& displayName, const std::string& parent) :
			m_Path(name), m_DisplayName(displayName), m_Parent(parent), m_Equipment(equipment.get()), m_Driver(driver)
		{
			m_Id = guid.M_ToGuid();
		}

		C_EquipmentInstanceClass::~C_EquipmentInstanceClass()
		{
			C_Locker locker(&m_Lock);
			m_PropertyInstances.clear();
		}

		void C_EquipmentInstanceClass::M_Remove()
		{
			M_GetInstance()->M_Remove();
		}

		/*void C_EquipmentInstanceClass::M_Update(C_EquipmentInstance &instance)
		 {

		 }
		 */

		/**
		 * @brief Get the list of properties of the equipment model instance
		 */
		C_EquipmentProperties C_EquipmentInstanceClass::M_GetProperties()
		{
			C_EquipmentProperties result;

			C_Locker locker(&m_Lock);
			for (unordered_map<std::string, C_EquipmentProperty>::iterator iter = m_PropertyInstances.begin();
			   iter != m_PropertyInstances.end(); iter++)
			{
				result.push_back(iter->second);
			}
			return result;
		}

		void C_EquipmentInstanceClass::M_SetValue(const std::string &property, const C_Variant& value)
		{
			MIA_OUT_DEBUG << "C_EquipmentInstanceClass::M_SetValue " << property << " = " << value << std::endl;

			C_EquipmentProperty pi = this->M_GetProperty(property);
			if (!pi.get())
			{
				C_EquipmentPropertyInfo pii = this->m_Equipment->M_GetProperty(property);
				if (pii)
				{
					// TODO: Request for C_EquipmentProperty here
					C_Locker locker(&m_Lock);
					m_PropertyInstances[property] = C_EquipmentProperty(new C_EquipmentPropertyClass(this, pii));
				}
			} else
			{
				pi->M_SetValue(value);
			}
		}

		void C_EquipmentInstanceClass::M_SetValue(const std::string &property, const C_Variant& value, const C_DateTime &timestamp)
		{
			Mia_THIS_ROUTINE("C_EquipmentInstanceClass::M_SetValue");
			C_EquipmentProperty pi = this->M_GetProperty(property);
			if (!pi.get())
			{
				THROW(C_PropertyNotFoundException(Mia_THIS_LOCATION, property, timestamp));
			} else
			{
				pi->M_SetValue(value, &timestamp);
			}
		}

		C_DbClassInstance C_EquipmentInstanceClass::M_GetInstance()
		{
			if (!m_Instance)
			{
				C_DbClass eqc = (*this->m_Driver)["Path"];
				m_Instance = eqc->M_GetInstanceById(M_GetId());
			}
			return m_Instance;
		}

		/**
		 * @brief Get the specific property by name
		 * @param propertyName The name of the property
		 *
		 * @return The matching property. null will be return if no matching property is found.
		 */
		C_EquipmentProperty C_EquipmentInstanceClass::operator[](const std::string &propertyName)
		{
			C_Locker locker(&m_Lock);
			return m_PropertyInstances[propertyName];
		}

		/**
		 * @brief Get the specific property by name
		 *
		 * @param propertyName  The property to get the instance
		 *
		 * @return The matching property. null will be return if no matching property is found.
		 */
		C_EquipmentProperty C_EquipmentInstanceClass::operator[](const C_EquipmentPropertyInfo &propertyName)
		{
			C_Locker locker(&m_Lock);
			return m_PropertyInstances[propertyName->M_GetName()];
		}

		C_EquipmentProperty C_EquipmentInstanceClass::M_GetProperty(const C_EquipmentPropertyInfo& property)
		{
			C_Locker locker(&m_Lock);
			return m_PropertyInstances[property->M_GetName()];
		}

		C_EquipmentProperty C_EquipmentInstanceClass::M_GetProperty(const std::string &propertyName)
		{
			C_Locker locker(&m_Lock);
			return m_PropertyInstances[propertyName];
		}

		void C_EquipmentPropertyClass::operator=(const C_Variant& value)
		{
			if (this->m_PropertyInfo->M_IsHistorized())
			{
				// if this is a historized property then just produce the value
				C_PropertyValuePusher pusher = M_GetPusher();
				m_CurrentValue = value;
				m_LastUpdateTime = C_DateTime::M_S_Now();
				pusher->M_Push(value);
			} else
			{
				m_CurrentValue = value;
				m_LastUpdateTime = C_DateTime::M_S_Now();
				M_Save();
			}
		}

		void C_EquipmentPropertyClass::M_SetValue(const C_Variant& value, const C_DateTime* time,
		   const t_ValueStatus &status)
		{
			if (this->m_PropertyInfo->M_IsHistorized())
			{
				// if this is a historized property then just produce the value
				C_PropertyValuePusher pusher = M_GetPusher();
				m_CurrentValue = value;
				if (time)
				{
					m_LastUpdateTime = *time;
					pusher->M_Push(value, *time, status);
				} else
				{
					m_LastUpdateTime = C_DateTime::M_S_Now();
					pusher->M_Push(value);
				}
			} else
			{
				m_CurrentValue = value;
				m_LastUpdateTime = C_DateTime::M_S_Now();
				M_Save();
			}
		}

		C_Variant C_EquipmentPropertyClass::M_GetServerValue()
		{
			C_Variant value;
			C_MiaDriver *driver = m_EquipmentInstance->m_Driver;
			C_DbClass epiclass = driver->M_GetClass("EquipmentPropertyInfo");
			C_DbClass epvclass = driver->M_GetClass("EquipmentPropertyValue");
			C_DbClass cvclass = driver->M_GetClass("CurrentValue");

			C_DbClassInstances epinfos;
			C_Variant id;
			bool historized = false;
			std::string pname = this->M_GetPropertyInfo()->M_GetName();
			std::string ename = this->m_EquipmentInstance->M_GetEquipment()->M_GetName();

			// Find the equipment with name = ename
			if (epiclass->M_GetInstanceSet(&epinfos, -1, "Equipment = ?", ename))
			{
				for (C_DbClassInstances::iterator iter = epinfos.begin(); iter != epinfos.end(); iter++)
				{
					C_DbClassInstance epi = *iter;
					if (epi->M_GetValue("DisplayName")->M_ToString() == pname)
					{
						id = epi->M_GetId();
						historized = epi->M_GetValue("Historized")->M_ToBool();
						break;
					}
				}
			}

			if (!(id.M_IsValid()))
			{
				std::cout << "Property: " << pname << " not found" << std::endl;
				return C_Variant();
			}

			C_DbClassInstances epvalues;
			C_Variant epvalue;
			std::string properties = "RAW:Value";
			std::string data = driver->M_FetchClassData("EquipmentPropertyValue", properties, 1, "property = ?", id.M_ToString());
			C_Variant parseddata = C_Variant::M_S_FromJSON(data);
			
			if (parseddata.M_IsValid())
			{
				std::string datas = parseddata[0].M_ToString();
				epvalue = C_Variant(datas, this->M_GetPropertyInfo()->M_GetType());
				if (!epvalue.M_IsValid()) epvalue = parseddata[0];
			}

			if (historized == false)
			{
				// the value field of EquipmentPropertyValue contains the actual value 
				return epvalue;
			}

			C_DbClassInstances cvalues;
			if (cvclass->M_GetInstanceSet(&cvalues, -1, "DisplayName = ?", epvalue.M_ToString()))
			{
				if (!(cvalues.empty()))
					return cvalues[0]->M_GetValue("Value")->M_ToString();
			}
			return C_Variant();
		}

		void C_EquipmentPropertyClass::M_BeginProduction()
		{
			C_PropertyValuePusher pusher = M_GetPusher();
		}

		void C_EquipmentPropertyClass::M_EndProduction()
		{
			C_Locker locker(&m_Lock);
			if (m_Pusher.get())
			{
				m_Pusher.reset();
			}
		}

		bool C_EquipmentPropertyClass::M_Save()
		{
			Mia_THIS_ROUTINE("C_EquipmentPropertyClass::M_Save");
			C_MiaDriver *driver = this->m_EquipmentInstance->m_Driver;
			TRY
			{
				std::string classname = "Path_" + m_EquipmentInstance->M_GetEquipment()->M_GetName();
				std::string instance = m_EquipmentInstance->m_Id.M_ToString();
				std::string value;
				if (m_CurrentValue.M_GetType() == g_ARRAY)
				{
					value = m_CurrentValue.M_ToString();
					driver->M_CommitClassDataRaw(classname, "RAW:" + this->m_PropertyInfo->M_GetName(), instance, value, "");
				}
				else
				{
					value = "\"" + m_CurrentValue.M_ToString() + "\"";
					driver->M_CommitClassDataRaw(classname, this->m_PropertyInfo->M_GetName(), instance, value, "");
				}
				
				return true;
			} 
			CATCH(ABB::Mia::C_Exception& ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION);
				return false;
			})
		}

		C_PropertyValuePusher C_EquipmentPropertyClass::M_GetPusher()
		{
			if (m_PropertyInfo->M_IsHistorized())
			{
				C_Locker locker(&m_Lock);
				if (!m_Pusher)
				{
					m_Pusher = m_EquipmentInstance->m_Driver->M_CreatePropertyValuePusher(m_EquipmentInstance->M_GetId(),
					   this->m_PropertyInfo->M_GetName());
				}
				return m_Pusher;
			}

			return C_PropertyValuePusher();
		}

		const C_Guid &C_EquipmentClass::M_GetGuid()
		{
			return m_Private->m_Guid;
		}

		const std::string &C_EquipmentClass::M_GetName()
		{
			return m_Private->m_Name;
		}

		const std::string &C_EquipmentClass::M_GetBase()
		{
			return m_Private->m_Base;
		}

		const std::string &C_EquipmentClass::M_GetType()
		{
			return m_Private->m_Type;
		}

		const std::string &C_EquipmentClass::M_Description()
		{
			return m_Private->m_Description;
		}

		void C_EquipmentClass::M_Update(C_Equipment& equipment, C_Equipment &ownequipment)
		{
			C_Locker locker(&m_Private->m_Lock);
			C_Locker inlocker(&(equipment->m_Private->m_Lock));
			for (unordered_map<std::string, C_EquipmentPropertyInfo>::iterator iter = equipment->m_Private->m_Properties.begin();
			   iter != equipment->m_Private->m_Properties.end(); iter++)
			{
				C_EquipmentPropertyInfo pi = iter->second;
				unordered_map<std::string, C_EquipmentPropertyInfo>::iterator iiter = m_Private->m_Properties.find(
				   pi->M_GetName());
				if (iiter != m_Private->m_Properties.end())
				{
					C_EquipmentPropertyInfo piiter = iiter->second;
					piiter->M_Update(pi);
				} else
				{
					C_EquipmentPropertyInfoClass *pic = DBG_NEW C_EquipmentPropertyInfoClass(ownequipment, pi->M_GetName(),
					   pi->M_GetCategory(), pi->M_GetDescription(), C_Variant::M_S_FromPropertyTypeToString(pi->M_GetType(), g_RAW),
					   pi->M_GetApplicationType(), pi->M_GetUnit(), pi->M_GetMapping(), pi->M_IsHistorized(),
					   pi->M_GetTransformation(), (pi->M_GetMinValue()), (pi->M_GetMaxValue()), (pi->M_GetDefaultValue()));

					C_EquipmentPropertyInfo pii(pic);
					m_Private->m_Properties[pi->M_GetName()] = pii;
				}
			}
		}

		C_EquipmentInstance C_EquipmentClass::M_Add(const std::string &path, const C_Guid &guid)
		{
			Mia_THIS_ROUTINE("C_EquipmentClass::M_Add");
			C_MiaDriver *driver = m_Private->m_Equipment->M_GetClass()->M_GetDriver();
			C_DbClass c = driver->M_GetClass("Path");
			C_DbClassInstance ei = c->M_Add();

			ei->M_SetValue("Name", path);
			ei->M_SetValue("DisplayName", "");
			ei->M_SetValue("Equipment", m_Private->m_Equipment->M_GetName());
			ei->M_SetRawValue("Id", "\"" + guid.M_ToString() + "\"");

			TRY
			{
				if (ei->M_CommitChanges())
				{
					C_Guid guid;
					ei->M_GetValue("Id", &guid);
					MIA_OUT_DEBUG<<guid <<std::endl;
					return this->M_GetInstanceById(guid);
				} else
				{
					return C_EquipmentInstance();
				}
			} 
			CATCH(C_Exception &ex, 
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, path, guid);
			}
			return C_EquipmentInstance();
			)
		}

		bool C_EquipmentClass::M_Remove(C_EquipmentInstance &instance)
		{
			instance->M_Remove();
			return true;
		}

		void C_EquipmentClass::M_GetProperties(C_EquipmentPropertyInfos *properties, bool refresh)
		{
			C_Locker locker(&m_Private->m_Lock);
			for (unordered_map<std::string, C_EquipmentPropertyInfo>::iterator iter = m_Private->m_Properties.begin();
			   iter != m_Private->m_Properties.end(); iter++)
			{
				properties->push_back(iter->second);
			}
		}

		C_EquipmentPropertyInfo C_EquipmentClass::M_GetProperty(const std::string &propertyName)
		{
			std::string pn = G_StringToLower(propertyName);
			C_Locker locker(&m_Private->m_Lock);
			unordered_map<std::string, C_EquipmentPropertyInfo>::iterator iter = m_Private->m_Properties.find(pn);
			if (iter != m_Private->m_Properties.end())
			{
				return iter->second;
			}

			// throw exception
			return C_EquipmentPropertyInfo();
		}

		C_EquipmentPropertyClass::C_EquipmentPropertyClass(C_EquipmentInstance& equipmentInstance,
		   C_EquipmentPropertyInfo& propertyInfo, const C_Variant& value)
		{
			m_EquipmentInstance = equipmentInstance.get();
			m_PropertyInfo = propertyInfo;
			m_CurrentValue = value;
			m_LastUpdateTime = C_DateTime::M_S_Now();
		}

		C_EquipmentPropertyClass::C_EquipmentPropertyClass(C_EquipmentInstanceClass* equipmentInstance,
		   C_EquipmentPropertyInfo& property, const C_Variant &value)
		{
			m_EquipmentInstance = equipmentInstance;
			m_PropertyInfo = property;
			m_CurrentValue = value;
			m_LastUpdateTime = C_DateTime::M_S_Now();
		}

		C_EquipmentPropertyClass::~C_EquipmentPropertyClass()
		{
			C_Locker locker(&m_Lock);
		}

		bool C_EquipmentClass::M_GetInstanceSet(C_EquipmentInstances *instances, int fetchMax,
		   const std::string &whereString, const C_Variant& data1, const C_Variant& data2, const C_Variant& data3,
		   const C_Variant& data4, const C_Variant& data5)
		{
			Mia_THIS_ROUTINE("C_EquipmentClass::M_GetInstanceSet");

			C_MiaDriver *driver = m_Private->m_Equipment->M_GetClass()->M_GetDriver();

			std::string properties = "Id,Name,DisplayName,Parent";

			C_EquipmentPropertyInfos propertylist;

			{
				C_Locker locker(&m_Private->m_Lock);

				for (unordered_map<std::string, C_EquipmentPropertyInfo>::iterator iter = m_Private->m_Properties.begin();
				   iter != m_Private->m_Properties.end(); iter++)
				{
					if (!iter->second->M_IsHistorized())
					{
						C_EquipmentPropertyInfo pi = iter->second;
						properties += ",";
						properties += pi->M_GetName();
						propertylist.push_back(pi);
					}
				}
			}

			TRY
			{
				std::string query, args;
				if (whereString.size())
				{
					query = "Equipment=? AND " + whereString;
				} else
				{
					query = "Equipment=?";
				}

				std::string result = driver->M_FetchClassData("Path", properties, fetchMax, query, M_GetName(), data1,
				   data2, data3, data4);

				M_ParseEquipmentInstances(instances, &propertylist, result);
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, (int64 ) instances, fetchMax);
			})
			return instances->size() != 0;
		}

		int C_EquipmentClass::M_GetInstanceSet(C_EquipmentInstances *instances, int fetchMax)
		{
			Mia_THIS_ROUTINE("C_EquipmentClass::M_GetInstanceSet");
			C_MiaDriver *driver = m_Private->m_Equipment->M_GetClass()->M_GetDriver();

			std::string properties = "Id,Name,DisplayName,Parent";

			C_EquipmentPropertyInfos propertylist;

			{
				C_Locker locker(&m_Private->m_Lock);
				for (unordered_map<std::string, C_EquipmentPropertyInfo>::iterator iter = m_Private->m_Properties.begin();
				   iter != m_Private->m_Properties.end(); iter++)
				{
					{
						properties += ",";
						properties += iter->second->M_GetName();
						propertylist.push_back(iter->second);
					}
				}
			}

			TRY
			{
				std::string result = driver->M_FetchClassData("Path", properties, fetchMax, "Equipment = ?", M_GetName());
				M_ParseEquipmentInstances(instances, &propertylist, result);
			} 
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, (int64 ) instances, fetchMax);
			})
			return instances->size() != 0;
		}

		C_EquipmentInstance C_EquipmentClass::M_GetInstanceById(const C_Variant &id)
		{
			Mia_THIS_ROUTINE("C_EquipmentClass::M_GetInstanceSet");
			C_MiaDriver *driver = m_Private->m_Equipment->M_GetClass()->M_GetDriver();

			std::string properties = "Id,Name,DisplayName,Parent";

			C_EquipmentPropertyInfos propertylist;

			{
				C_Locker locker(&m_Private->m_Lock);
				for (unordered_map<std::string, C_EquipmentPropertyInfo>::iterator iter = m_Private->m_Properties.begin();
				   iter != m_Private->m_Properties.end(); iter++)
				{
					if (iter->second->M_IsHistorized())
					{
						properties += ",";
						properties += iter->second->M_GetName();
						propertylist.push_back(iter->second);
					}
				}
			}

			C_EquipmentInstances instances;
			TRY
			{
				std::string result = driver->M_FetchClassData("Path", properties, 1, "Equipment = ? AND Id=?", M_GetName(), id);
				M_ParseEquipmentInstances(&instances, &propertylist, result);
			} 
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, id);
			})
			
			if (instances.size())
			{
				return instances.front();
			}

			// throw exception
			return C_EquipmentInstance();
		}

		void C_EquipmentClass::M_ParseEquipmentInstances(C_EquipmentInstances *instances,
		   C_EquipmentPropertyInfos *propertylist, const std::string &result)
		{
			Mia_THIS_ROUTINE("C_EquipmentClass::M_ParseEquipmentInstances");
			C_WebsocketStream ss(result);
			bool begin = true;

			TRY
			{
				for (;;)
				{
					C_EquipmentInstanceClass *instance;

					std::string s;
					ss >> s;
					if (begin)
					{
						begin = false;
						if (s.find(".Exception") != std::string::npos)
						{
							THROW( C_ServerException(Mia_THIS_LOCATION, result));
						}
					}

					if (s.size())
					{
						C_Guid guid(s);
						instance = DBG_NEW C_EquipmentInstanceClass(m_Private->m_Equipment->M_GetClass()->M_GetDriver(), this, guid);
					} else
					{
						return;
					}

					ss >> s;
					if (s.size())
					{
						instance->m_Path = s;
					} else
					{
						return;
					}

					ss >> s;
					if (s.size())
					{
						instance->m_DisplayName = s;
					} else
					{
						return;
					}

					ss >> s;
					if (s.size())
					{
						instance->m_Parent = s;
					}
					C_EquipmentInstance i(instance);
					for (C_EquipmentPropertyInfos::iterator iter = propertylist->begin(); iter != propertylist->end(); iter++)
					{
						C_EquipmentPropertyInfo pi = *iter;
						ss >> s;
						// Check historized property
						if (!pi->M_IsHistorized())
						{
							if (s.size())
							{
								C_Variant v(s, (*iter)->M_GetType());
								C_Locker locker(&instance->m_Lock);
								instance->m_PropertyInstances[(*iter)->M_GetName()] = C_EquipmentProperty(
									new C_EquipmentPropertyClass(i, pi, v));
							}
						} else
						{
							if (s.size())
							{
								C_Variant v(s, g_UINT32);
								C_Locker locker(&instance->m_Lock);
								instance->m_PropertyInstances[pi->M_GetName()] = C_EquipmentProperty(
									new C_EquipmentPropertyClass(i, pi, v));
							}
						}
					}

					C_EquipmentPropertyInfos infos;
					this->M_GetProperties(&infos);
					for (C_EquipmentPropertyInfos::iterator iter = infos.begin(); iter != infos.end(); iter++)
					{
						C_EquipmentPropertyInfo pi = *iter;
						C_Locker locker(&instance->m_Lock);
						if (instance->m_PropertyInstances.find(pi->M_GetName()) == instance->m_PropertyInstances.end())
						{
							instance->m_PropertyInstances[(*iter)->M_GetName()] = C_EquipmentProperty(
								new C_EquipmentPropertyClass(i, pi));
						}
					}

					instances->push_back(i);
				}
			}
			CATCH(C_Exception &ex,
			{
				ex.M_RethrowTraceback(Mia_THIS_LOCATION, result);
			})
		}

		std::ostream &operator<<(std::ostream &os, const C_Equipment &equipment)
		{
			os << "Name: \"" << equipment->M_GetName() << "\" Base:" << equipment->M_GetBase() << std::endl;

			C_EquipmentPropertyInfos infos = equipment->M_GetProperties();
			if (infos.size())
			{
				os << "Properties:" << std::endl;
			} else
			{
				os << "This equipment does not have any properties." << std::endl;
			}

			int i = 1;
			for (C_EquipmentPropertyInfos::iterator iter = infos.begin(); iter != infos.end(); iter++)
			{
				os << i << ". " << *iter << std::endl;
			}
			return os;
		}

		std::ostream &operator<<(std::ostream &os, const C_EquipmentPropertyInfo &pi)
		{
			os << "[" << pi->M_GetName() << "]: (" << ABB::Mia::C_Variant::M_S_FromPropertyTypeToString(pi->M_GetType(), g_RAW)
			   << ") Category: \"" << pi->M_GetCategory() << "\", Description: \"" << pi->M_GetDescription()
			   << "\", Historized: " << pi->M_IsHistorized();
			return os;
		}
	} /* namespace MIA */
} /* namespace ABB */
