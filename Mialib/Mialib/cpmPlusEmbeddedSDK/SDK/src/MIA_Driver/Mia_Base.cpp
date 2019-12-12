/*
 * Mia_Base.cpp
 *
 *  Created on: Jun 14, 2014
 *      Author: Tuan Vu
 */

#include <stdlib.h>
#include <string>
#include <algorithm>
#include <string.h>
#include <time.h>
#include <iostream>
#include <sstream>
#include <map>
#include <iomanip>

#include "Mia_Base.h"
#include "Mia_Exception.h"

typedef unordered_map<std::string, ABB::Mia::C_Variant> t_JsonObjectMap;
typedef std::map<int, ABB::Mia::C_Variant> t_JsonArrayMap;

namespace ABB
{
	namespace Mia
	{
		C_NullBufferF G_NullBuffer;

		static const C_Variant invalid_variant;
		static C_Variant invalid_variant_tmp;
		#define RETURN_INVALID_VARIANT() NO_THROW({ invalid_variant_tmp = invalid_variant; return invalid_variant_tmp; });

		class C_JsonReader
		{
			public: enum t_TokenType
			{
				k_TokenEndOfStream = 0,
				k_TokenObjectBegin,
				k_TokenObjectEnd,
				k_TokenArrayBegin,
				k_TokenArrayEnd,
				k_TokenString,
				k_TokenNumber,
				k_TokenTrue,
				k_TokenFalse,
				k_TokenNull,
				k_TokenArraySeparator,
				k_TokenMemberSeparator,
				k_TokenComment,
				k_TokenError
			};

			public: class C_JsonToken
			{
				public:   C_Variant		m_Value;
				public:   t_TokenType 	m_Type;
				public:   int 				m_Start;
				public:   int 				m_End;
			};

			private: int m_CurrentIndex;
			private: int m_EndIndex;
			private: std::string m_Json;

			public: C_JsonReader(const std::string &json) : m_CurrentIndex(0), m_EndIndex(0)
			{
				m_Json = json;
			}

			public: void M_Read(C_Variant& value)
			{
				Mia_THIS_ROUTINE("C_JsonReader::M_Read");
				m_CurrentIndex = 0;
				m_EndIndex = (int) m_Json.length();
				C_JsonToken root;
				TRY
				{
					M_ReadValue(root);
				}
				CATCH (C_Exception &ex,
				{
					ex.M_RethrowTraceback(Mia_THIS_LOCATION, m_Json);
				})

				value = root.m_Value;
			}

			private: void M_SkipSpaces()
			{
				while (m_CurrentIndex != m_EndIndex)
				{
					char c = m_Json[m_CurrentIndex];
					if (c == ' ' || c == '\t' || c == '\r' || c == '\n') m_CurrentIndex++;
					else break;
			  }
			}

			private: bool M_SkipSpaces(const std::string &expected, bool skipCase = false)
			{
				int length = (int) expected.length();
				int index = 0;
				while (index < length)
				{
					char c = m_Json[m_CurrentIndex + index];
					if (skipCase)
					{
						if (std::tolower(c) != std::tolower(expected[index])) return false;
					} else
					{
						if (c != expected[index]) return false;
					}
					index ++;
				}
				m_CurrentIndex += length;
				return true;
			}

			private: char M_ReadNext()
			{
				if (m_CurrentIndex == m_EndIndex) return 0;
				return m_Json[m_CurrentIndex++];
			}

			private: bool M_ReadString()
			{
				char c = 0;
				while (m_CurrentIndex != m_EndIndex)
				{
					c = M_ReadNext();
					if (c == '\\') M_ReadNext();
					else if (c == '"') break;
				}
				return c == '"';
			}

			private: bool M_ReadComment()
			{
				char c = M_ReadNext();
				if (c == '*') return M_ReadCStyleComment();
				else if (c == '/') return M_ReadCppStyleComment();

				return false;
			}

			private: bool M_ReadCStyleComment()
			{
				while (m_CurrentIndex != m_EndIndex)
				{
					char c = M_ReadNext();
					if (c == '*' && m_Json[m_CurrentIndex] == '/') break;
				}
				return M_ReadNext() == '/';
			}

			private: bool M_ReadCppStyleComment()
			{
				while (m_CurrentIndex != m_EndIndex)
				{
					char c = M_ReadNext();
					if (c == '\r' || c == '\n') break;
				}
				return true;
			}

			private: void M_ReadNumber(C_JsonToken &token)
			{
				t_DataType type = g_INT64;
				while (m_CurrentIndex != m_EndIndex)
				{
					char c = m_Json[m_CurrentIndex];
					if (!(c >= '0' && c <= '9') && (c != '.' && c != 'e' && c !='E' && c !='+' && c !='-')) break;
					if (c == '.' || c == 'e' || c !='E') type = g_DOUBLE;
					++m_CurrentIndex;
				}
				token.m_Value = C_Variant(type);
			}

			private: bool M_ParseNumber(C_JsonToken &token)
			{
				std::string number = m_Json.substr(token.m_Start, token.m_End - token.m_Start);
				token.m_Value = C_Variant(number, token.m_Value.M_GetType());
				return token.m_Value.M_IsValid();
			}

			private: bool M_ParseString(C_JsonToken &token)
			{
				if (token.m_End - token.m_Start < 2) return false;
				std::string str = m_Json.substr(token.m_Start + 1, token.m_End - token.m_Start - 2);
				token.m_Value = C_Variant(str);
				return token.m_Value.M_IsValid();
			}

			private: bool M_ReadObject(C_JsonToken &token)
			{
				Mia_THIS_ROUTINE("C_JsonReader::M_ReadObject");
				C_JsonToken tokenName;
				std::string name;
				token.m_Value = C_Variant(g_OBJECT);
				while (M_ReadToken(tokenName))
				{
					bool initialTokenOk = true;
					while (tokenName.m_Type == k_TokenComment && initialTokenOk) initialTokenOk = M_ReadToken(tokenName);
					if (!initialTokenOk) break;
					if (tokenName.m_Type == k_TokenObjectEnd && name.empty()) return true;

					name = "";

					if (tokenName.m_Type == k_TokenString)
					{
						M_ParseString(tokenName);
						name = tokenName.m_Value.M_ToString();
					}
					else break;

					C_JsonToken colon;
					if (!M_ReadToken(colon) || colon.m_Type != k_TokenMemberSeparator)
					{
						THROW(C_InvalidJsonFormatException(Mia_THIS_LOCATION, "Missing ':' after object member name", name));
					}

					C_JsonToken value;
					bool ok = M_ReadValue(value);
					if (!ok)
					{
						THROW(C_InvalidJsonFormatException(Mia_THIS_LOCATION, "Unable to parse value for property", name));
					}
					token.m_Value.M_SetValue(name, value.m_Value);

					C_JsonToken comma;
					if (!M_ReadToken(comma) ||	(comma.m_Type != k_TokenObjectEnd && comma.m_Type != k_TokenArraySeparator &&	comma.m_Type != k_TokenComment))
					{
						THROW(C_InvalidJsonFormatException(Mia_THIS_LOCATION, "Missing ',' or '}' in object declaration"));
					}
					bool finalizeTokenOk = true;
					while (comma.m_Type == k_TokenComment && finalizeTokenOk) finalizeTokenOk = M_ReadToken(comma);
					if (comma.m_Type == k_TokenObjectEnd) return true;
				}
				THROW(C_InvalidJsonFormatException(Mia_THIS_LOCATION, "Missing ',' or '}' in object name"));
				NO_THROW(return false;);
			}

			private: bool M_ReadArray(C_JsonToken &token)
			{
				Mia_THIS_ROUTINE("C_JsonReader::M_ReadArray");
				token.m_Value = C_Variant(g_ARRAY);
				M_SkipSpaces();

				if (m_Json[m_CurrentIndex] == ']') // empty array
				{
					C_JsonToken endArray;
					M_ReadToken(endArray);
					return true;
				}
				int index = 0;
				for (;;)
				{
					C_JsonToken value;
					bool ok = M_ReadValue(value);
					if (ok)
					{
						token.m_Value.M_SetValue(index++, value.m_Value);
					} else
					{
						THROW(C_InvalidJsonFormatException(Mia_THIS_LOCATION, "Unable to parse JSON Array", m_CurrentIndex));
					}

					while (value.m_Type == k_TokenComment && ok)
					{
						ok = M_ReadToken(value);
					}

					// read separator
					ok = M_ReadToken(value);
					if (!ok || (value.m_Type != k_TokenArraySeparator && value.m_Type != k_TokenArrayEnd))
					{
						THROW(C_InvalidJsonFormatException(Mia_THIS_LOCATION, "Missing ',' or ']' in array declaration", m_CurrentIndex));
					}
					if (value.m_Type == k_TokenArrayEnd) break;
				}
				return true;
			}

			private: bool M_ReadValue(C_JsonToken &token)
			{
				Mia_THIS_ROUTINE("C_JsonReader::M_ReadValue")

				bool successful = true;
				successful = M_ReadToken(token);
				if (successful)
				switch (token.m_Type)
				{
					case k_TokenObjectBegin: successful = M_ReadObject(token); break;
					case k_TokenArrayBegin: successful = M_ReadArray(token);	break;
					case k_TokenNumber: successful = M_ParseNumber(token); break;
					case k_TokenString: successful = M_ParseString(token); break;
					case k_TokenTrue:	token.m_Value = C_Variant(true);	break;
					case k_TokenFalse: token.m_Value = C_Variant(false); break;
					case k_TokenNull:	token.m_Value = C_Variant(g_NULL); break;
					case k_TokenArraySeparator: token.m_Value = C_Variant(g_NULL);	break;

					default:	THROW(C_InvalidJsonFormatException(Mia_THIS_LOCATION, "Syntax error: value, object or array expected."));
				}

				return successful;
			}

			private: bool M_ReadToken(C_JsonToken &token)
			{
				M_SkipSpaces();
				token.m_Start = m_CurrentIndex;
				char c = M_ReadNext();
				bool ok = true;
				switch (c)
				{
					case '{':
						token.m_Type = k_TokenObjectBegin;
						break;
					case '}':
						token.m_Type = k_TokenObjectEnd;
						break;
					case '[':
						token.m_Type = k_TokenArrayBegin;
						break;
					case ']':
						token.m_Type = k_TokenArrayEnd;
						break;
					case '"':
						token.m_Type = k_TokenString;
						ok = M_ReadString();
						break;
					case '/':
						token.m_Type = k_TokenComment;
						ok = M_ReadComment();
						break;
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					case '-':
						token.m_Type = k_TokenNumber;
						M_ReadNumber(token);
						break;
					case 't':
					case 'T':
						token.m_Type = k_TokenTrue;
						ok = M_SkipSpaces("rue", true);
						break;
					case 'f':
					case 'F':
						token.m_Type = k_TokenFalse;
						ok = M_SkipSpaces("alse", true);
						break;
					case 'n':
					case 'N':
						token.m_Type = k_TokenNull;
						ok = M_SkipSpaces("ull", true);
						break;
					case ',':
						token.m_Type = k_TokenArraySeparator;
						break;
					case ':':
						token.m_Type = k_TokenMemberSeparator;
						break;
					case 0:
						token.m_Type = k_TokenEndOfStream;
						break;
					default:
						ok = false;
						break;
				}
				if (!ok) { token.m_Type = k_TokenError;}
				token.m_End = m_CurrentIndex;
				return ok;
			}
		};
		std::string G_StringToLower(const std::string &data)
		{
			std::string v = data;
			G_MIA_TRANSFORM_TO_LOWER(v);
			return v;
		}

		void C_WebsocketStream::operator<<(const int &value)
		{
			m_Stream << (value);
		}

		void C_WebsocketStream::operator<<(const int64 &value)
		{
			m_Stream << "\"" <<(value)<< "\"";
		}

		void C_WebsocketStream::operator<<(const uint64 &value)
		{
			m_Stream << "\"" << (value) << "\"";
		}

		void C_WebsocketStream::operator<<(const double &value)
		{
			m_Stream << (value);
		}

		void C_WebsocketStream::operator<<(const std::string &value)
		{
			m_Stream << ("\"" + value + "\"");
		}

		void C_WebsocketStream::operator<<(const C_DateTime& value)
		{
			int64 tick = value.M_GetTicks();
			m_Stream << "\"" << tick << "\"";
		}

		void C_WebsocketStream::operator<<(const char* value)
		{
			m_Stream << "\"" << value << "\"";
		}

		void C_WebsocketStream::operator<<(const C_Variant &value)
		{
			t_DataType type = value.M_GetType();
			switch (type)
			{
				case g_INT32:
				case g_INT16:
				case g_BYTE:
				case g_UINT32:
				case g_UINT16:
				case g_DOUBLE:
				case g_FLOAT:
				case g_ARRAY:
				{
					m_Stream << value.M_ToString();
					break;
				}
				case g_INT64:
				case g_UINT64:
				case g_STRING:
				{
					m_Stream << "\"" << value.M_ToString() << "\"";
					break;
				}

				case g_DATETIME:
				{
					m_Stream << "\"";
					(*this) << value.M_ToDate();
					m_Stream << "\"";
					break;
				}

				case g_GUID:
				{
					m_Stream << "\"" << value.M_ToGuid() << "\"";
					break;
				}

				default:
					m_Stream << "\"" << value.M_ToString() << "\"";
					break;

			};
		}

		C_Variant &C_Variant::operator[](const unsigned int &index)
		{
			Mia_THIS_ROUTINE("C_Variant::operator[]");
			if (m_Type == g_ARRAY && index < M_GetArraySize())
			{
				t_JsonArrayMap *map = (t_JsonArrayMap*) m_Data.m_Data;
				return (*map)[index];
			}

			THROW(C_InvalidVariantTypeException(Mia_THIS_LOCATION, index));
			RETURN_INVALID_VARIANT();
		}

		const C_Variant & ABB::Mia::C_Variant::operator[](const unsigned int & index) const
		{
			Mia_THIS_ROUTINE("C_Variant::operator[]");
			if (m_Type == g_ARRAY && index < M_GetArraySize())
			{
				t_JsonArrayMap *map = (t_JsonArrayMap*)m_Data.m_Data;
				return (*map)[index];
			}

			THROW(C_InvalidVariantTypeException(Mia_THIS_LOCATION, index));
			RETURN_INVALID_VARIANT();
		}

		C_Variant &C_Variant::operator[](const std::string &index)
		{
			Mia_THIS_ROUTINE("C_Variant::operator[]");
			if (m_Type == g_OBJECT)
			{
				t_JsonObjectMap *map = (t_JsonObjectMap*) m_Data.m_Data;
				if (map->find(index) != map->end())	return (*map)[index];

				(*map)[index] = C_Variant();
				return (*map)[index];
			}

			THROW(C_InvalidVariantTypeException(Mia_THIS_LOCATION, index));
			RETURN_INVALID_VARIANT();
		}

		const C_Variant & ABB::Mia::C_Variant::operator[](const std::string & index) const
		{
			Mia_THIS_ROUTINE("C_Variant::operator[]");
			if (m_Type == g_OBJECT)
			{
				t_JsonObjectMap *map = (t_JsonObjectMap*)m_Data.m_Data;
				if (map->find(index) != map->end())	return (*map)[index];

				(*map)[index] = C_Variant();
				return (*map)[index];
			}

			THROW(C_InvalidVariantTypeException(Mia_THIS_LOCATION, index));
			RETURN_INVALID_VARIANT();
		}

		C_Variant &C_Variant::M_GetPropertyValue(const std::string &index, C_Variant &defaultValue)
		{
			Mia_THIS_ROUTINE("C_Variant::operator[]");
			if (m_Type == g_OBJECT)
			{
				t_JsonObjectMap *map = (t_JsonObjectMap*) m_Data.m_Data;
				if (map->find(index) != map->end())	return (*map)[index];
				return defaultValue;
			}

			THROW(C_InvalidVariantTypeException(Mia_THIS_LOCATION, index));
			RETURN_INVALID_VARIANT();
		}

		void *C_Variant::M_Pointer()
		{
			return m_Data.m_Data;
		}
		void C_WebsocketStream::M_AppendRaw(const std::string &value)
		{
			m_Stream << value;
		}

		void ABB::Mia::C_WebsocketStream::M_AppendDirective(const std::string directive)
		{
			m_Stream << ("\"" + directive + "\"");
			m_Stream << (":");
		}

		void ABB::Mia::C_WebsocketStream::M_NextDirective()
		{
			m_Stream << (",");
		}

		void C_WebsocketStream::operator>>(int &value)
		{
			m_Stream >> value;
		}

		void C_WebsocketStream::operator>>(double &value)
		{
			m_Stream >> value;
		}

		void C_WebsocketStream::operator>>(int64 &value)
		{
			m_Stream >> value;
		}

		void C_WebsocketStream::operator>>(std::string &value)
		{
			// Now change the reading stategy
			// Read first char and decide what to do with it
			value = "";
			int startwith = 0;
			char p = '\0';
			for (;;)
			{
				if (m_Stream.eof() || !m_Stream.good()) break;
				p = m_Stream.get();
				if (p == -1) return;
				if (p == ']' && startwith == 1) return;
				else if ((p == ']' || p == '}') && !value.size()) continue;
				else if (p == '[')
				{
					startwith = 1;
					continue;
				} else if (p == '{')
				{
					startwith = 2;
					continue;
				} else if (p == '\"')
				{
					startwith = 3;
					break;
				} else if (p == ' ' || p == ',') continue;
				else
				{
					startwith = 4;
					break;
				}
			}

			switch (startwith)
			{
				case 1:
				{	// This is an array

					break;
				}

				case 2:
				{
					break;
				}

				case 3:
				{
					for (;;)
					{
						if (m_Stream.eof() || !m_Stream.good()) break;
						p = m_Stream.get();
						if (p == -1) break;
						if (p == '\"' && !value.size()) break;
						if (p == '\"' && value[value.length() - 1] != '\\') break;
						else value += p;
					}
					if (!value.size()) value = "";
					break;
				}

				case 4:
				{
					// probably read null
					for (;;)
					{
						if (m_Stream.eof() || !m_Stream.good()) break;
						value += p;
						p = m_Stream.get();
						if (p == -1 || p == ',' || p == ']' || p == '}') break;
					}
					break;
				}

				default:
					break;
			}
		}

		void C_WebsocketStream::operator>>(bool &value)
		{
			std::string dat;
			(*this) >> dat;
			value = (dat == "True" || dat == "TRUE" || dat == "true");
		}

		std::string C_WebsocketStream::M_GetString()
		{
			return m_Stream.str();
		}

		void C_WebsocketStream::M_BeginObject()
		{
			m_Stream << ("{");
		}

		void C_WebsocketStream::M_BeginArray()
		{
			m_Stream << ("[");
		}

		void C_WebsocketStream::M_EndObject()
		{
			m_Stream << ("}");
		}

		void C_WebsocketStream::M_EndArray()
		{
			m_Stream << ("]");
		}

		void C_WebsocketStreamBinary::M_Write(char *data, const int &length)
		{
			int i = length;
			while (i--)
			{
				m_Buffer.push_back(*(data++));
			}
		}

		void C_WebsocketStreamBinary::M_Write(const C_Variant &data)
		{
			t_DataType type = data.M_GetType();
			switch (type)
			{
				case g_BOOLEAN:
				{
					m_Buffer.push_back(data.M_ToBool() ? 1 : 0);
					break;
				}
				case g_BYTE:
				{
					m_Buffer.push_back(data.M_ToByte());
					break;
				}
				case g_CHAR:
				{
					m_Buffer.push_back(data.M_ToChar());
					break;
				}
				case g_UINT16:
				{
					uint16 value = data.M_ToUInt16();
					m_Buffer.push_back(value & 0xFF);
					m_Buffer.push_back((value >> 8) & 0xFF);
					break;
				}
				case g_INT16:
				{
					int16 value = data.M_ToInt16();
					m_Buffer.push_back(value & 0xFF);
					m_Buffer.push_back((value >> 8) & 0xFF);
					break;
				}
				case g_UINT32:
				{
					uint32 value = data.M_ToUInt32();
					m_Buffer.push_back(value & 0xFF);
					m_Buffer.push_back((value >> 8) & 0xFF);
					m_Buffer.push_back((value >> 16) & 0xFF);
					m_Buffer.push_back((value >> 24) & 0xFF);
					break;
				}
				case g_INT32:
				{
					int32 value = data.M_ToInt32();
					m_Buffer.push_back(value & 0xFF);
					m_Buffer.push_back(((value >> 8) & 0xFF));
					m_Buffer.push_back(((value >> 16) & 0xFF));
					m_Buffer.push_back(((value >> 24) & 0xFF));
					break;
				}
				case g_UINT64:
				{
					uint64 value = data.M_ToUInt64();
					m_Buffer.push_back(value & 0xFF);
					m_Buffer.push_back((value >> 8) & 0xFF);
					m_Buffer.push_back((value >> 16) & 0xFF);
					m_Buffer.push_back((value >> 24) & 0xFF);
					m_Buffer.push_back((value >> 32) & 0xFF);
					m_Buffer.push_back((value >> 40) & 0xFF);
					m_Buffer.push_back((value >> 48) & 0xFF);
					m_Buffer.push_back((value >> 56) & 0xFF);
					break;
				}
				case g_INT64:
				{
					int64 value = data.M_ToUInt64();
					m_Buffer.push_back(value & 0xFF);
					m_Buffer.push_back((value >> 8) & 0xFF);
					m_Buffer.push_back((value >> 16) & 0xFF);
					m_Buffer.push_back((value >> 24) & 0xFF);
					m_Buffer.push_back((value >> 32) & 0xFF);
					m_Buffer.push_back((value >> 40) & 0xFF);
					m_Buffer.push_back((value >> 48) & 0xFF);
					m_Buffer.push_back((value >> 56) & 0xFF);
					break;
				}
				case g_FLOAT:
				{
					float value = data.M_ToFloat();
					int32 *ivalue = (int32*)&value;
					m_Buffer.push_back((*ivalue & 0xFF));
					m_Buffer.push_back((*ivalue >> 8) & 0xFF);
					m_Buffer.push_back((*ivalue >> 16) & 0xFF);
					m_Buffer.push_back((*ivalue >> 24) & 0xFF);
					break;
				}
				case g_DOUBLE:
				{
					double value = data.M_ToDouble();
					int64 *ivalue = (int64*)&value;
					m_Buffer.push_back((*ivalue & 0xFF));
					m_Buffer.push_back((*ivalue >> 8) & 0xFF);
					m_Buffer.push_back((*ivalue >> 16) & 0xFF);
					m_Buffer.push_back((*ivalue >> 24) & 0xFF);
					m_Buffer.push_back((*ivalue >> 32) & 0xFF);
					m_Buffer.push_back((*ivalue >> 40) & 0xFF);
					m_Buffer.push_back((*ivalue >> 48) & 0xFF);
					m_Buffer.push_back((*ivalue >> 56) & 0xFF);
					break;
				}
				case g_DATETIME:
				{
					C_DateTime dt = data.M_ToDate();
					int64 value = dt.M_GetTicks();
					m_Buffer.push_back(value & 0xFF);
					m_Buffer.push_back((value >> 8) & 0xFF);
					m_Buffer.push_back((value >> 16) & 0xFF);
					m_Buffer.push_back((value >> 24) & 0xFF);
					m_Buffer.push_back((value >> 32) & 0xFF);
					m_Buffer.push_back((value >> 40) & 0xFF);
					m_Buffer.push_back((value >> 48) & 0xFF);
					m_Buffer.push_back((value >> 56) & 0xFF);
				}

				case g_GUID:
				{
					C_Guid guid = data.M_ToGuid();
					int i = 0;
					for (i = 0; i < 16; i++)
					{
						m_Buffer.push_back(guid[i] & 0xFF);
					}
				}
			}
		}

		void C_WebsocketStream::M_AppendNull()
		{
			m_Stream << ("null");
		}

		void C_WebsocketStream::M_AppendString(const std::string & data)
		{
			m_Stream << data;
		}

		bool C_WebsocketStream::M_HasMore()
		{
			return !m_Stream.eof() && m_Stream.good();
		}

		C_WebsocketStream::C_WebsocketStream(const std::string &data) : m_Stream(data) {	}

		C_WebsocketStreamBinary::C_WebsocketStreamBinary() {}

		char * C_WebsocketStreamBinary::M_GetBuffer()
		{
			return &m_Buffer[0];
		}

		int  C_WebsocketStreamBinary::M_GetSize()
		{
			return m_Buffer.size();
		}

		void C_WebsocketStreamBinary::M_BeginObject()
		{
			m_Buffer.push_back(g_TC_Object);
		}

		void C_WebsocketStreamBinary::M_BeginArray()
		{
			m_Buffer.push_back(g_TC_ObjectArray);
		}

		void C_WebsocketStreamBinary::M_EndObject()
		{
			m_Buffer.push_back(g_TC_End);
		}

		void C_WebsocketStreamBinary::M_EndArray()
		{
			m_Buffer.push_back(g_TC_End);
		}

		void ABB::Mia::C_WebsocketStreamBinary::M_BeginArray(int type)
		{
			m_Buffer.push_back(g_TC_ArrayFlag | type);
		}

		void C_WebsocketStreamBinary::M_AppendNull()
		{
			m_Buffer.push_back(g_TC_Null);
		}

		void ABB::Mia::C_WebsocketStreamBinary::M_Reserve(size_t size)
		{
			m_Buffer.reserve(size);
		}

		void C_WebsocketStreamBinary::operator<<(const bool & value)
		{
			m_Buffer.push_back((value ? g_TC_BoolTrue : g_TC_BoolFalse));
		}

		void C_WebsocketStreamBinary::operator<<(const byte & value)
		{
			m_Buffer.push_back((value ? g_TC_Byte : g_TC_ByteZero));
			if (value) m_Buffer.push_back(value);
		}

		void C_WebsocketStreamBinary::operator<<(const char & value)
		{
			m_Buffer.push_back((value ? g_TC_Int8 : g_TC_Int8Zero));
			if (value) m_Buffer.push_back(value);
		}

		void C_WebsocketStreamBinary::operator<<(const int16 & value)
		{
			volatile uint16 written = 0;
			int16 cvalue = value;
			if (cvalue < 0)
				cvalue *= -1;

			if (!cvalue)
			{
				m_Buffer.push_back(g_TC_Int16Zero);
				written = 1;
			}
			else if (cvalue < 0x80)
			{
				m_Buffer.push_back(g_TC_Int16SingleByte);
				m_Buffer.push_back(value & 0xFF);
				written = 2;
			}
			else
			{
				m_Buffer.push_back(g_TC_Int16);
				m_Buffer.push_back((value & 0xFF));
				m_Buffer.push_back((value >> 8) & 0xFF);
				written = 3;
			}
		}

		void C_WebsocketStreamBinary::operator<<(const int32 &value)
		{
			volatile uint16 written = 0;
			int32 cvalue = value;
			if (cvalue < 0)
				cvalue *= -1;

			if (!cvalue)
			{
				m_Buffer.push_back(g_TC_Int32Zero);
				written = 1;
			}
			else if (cvalue < 0x80)
			{
				m_Buffer.push_back(g_TC_Int32SingleByte);
				m_Buffer.push_back((value & 0xFF));
				written = 2;
			}
			else if (cvalue < 0x8000)
			{
				m_Buffer.push_back(g_TC_Int32TwoByte);
				m_Buffer.push_back((value & 0xFF));
				m_Buffer.push_back(((value >> 8) & 0xFF));
				written = 3;
			}
			else if (cvalue < 0x800000)
			{
				m_Buffer.push_back(g_TC_Int32ThreeByte);
				m_Buffer.push_back((value & 0xFF));
				m_Buffer.push_back(((value >> 8) & 0xFF));
				m_Buffer.push_back(((value >> 16) & 0xFF));
				written = 4;
			}
			else
			{
				m_Buffer.push_back(g_TC_Int32);
				m_Buffer.push_back((value & 0xFF));
				m_Buffer.push_back(((value >> 8) & 0xFF));
				m_Buffer.push_back(((value >> 16) & 0xFF));
				m_Buffer.push_back(((value >> 24) & 0xFF));
				written = 5;
			}
		}

		void C_WebsocketStreamBinary::operator<<(const int64 &value)
		{
			volatile uint16 written = 0;
			int64 cvalue = value;
			if (cvalue < 0)
				cvalue *= -1;

			if (!cvalue)
			{
				m_Buffer.push_back(g_TC_Int64Zero);
				written = 1;
			}
			else if (cvalue < 0x80)
			{
				m_Buffer.push_back(g_TC_Int64SingleByte);
				m_Buffer.push_back(value & 0xFF);
				written = 2;
			}
			else if (cvalue < 0x8000)
			{
				m_Buffer.push_back(g_TC_Int64TwoByte);
				m_Buffer.push_back(value & 0xFF);
				m_Buffer.push_back((value >> 8) & 0xFF);
				written = 3;
			}
			else if (cvalue < 0x800000)
			{
				m_Buffer.push_back(g_TC_Int64ThreeByte);
				m_Buffer.push_back(value & 0xFF);
				m_Buffer.push_back((value >> 8) & 0xFF);
				m_Buffer.push_back((value >> 16) & 0xFF);
				written = 4;
			}
			else if (cvalue < 0x80000000ULL)
			{
				m_Buffer.push_back(g_TC_Int64FourByte);
				m_Buffer.push_back(value & 0xFF);
				m_Buffer.push_back((value >> 8) & 0xFF);
				m_Buffer.push_back((value >> 16) & 0xFF);
				m_Buffer.push_back((value >> 24) & 0xFF);
				written = 5;
			}
			else if (cvalue < 0x8000000000ULL)
			{
				m_Buffer.push_back(g_TC_Int64FiveByte);
				m_Buffer.push_back(value & 0xFF);
				m_Buffer.push_back((value >> 8) & 0xFF);
				m_Buffer.push_back((value >> 16) & 0xFF);
				m_Buffer.push_back((value >> 24) & 0xFF);
				m_Buffer.push_back((value >> 32) & 0xFF);
				written = 6;
			}
			else if (cvalue < 0x800000000000ULL)
			{
				m_Buffer.push_back(g_TC_Int64SixByte);
				m_Buffer.push_back(value & 0xFF);
				m_Buffer.push_back((value >> 8) & 0xFF);
				m_Buffer.push_back((value >> 16) & 0xFF);
				m_Buffer.push_back((value >> 24) & 0xFF);
				m_Buffer.push_back((value >> 32) & 0xFF);
				m_Buffer.push_back((value >> 40) & 0xFF);
				written = 7;
			}
			else if (cvalue < 0x80000000000000ULL)
			{
				m_Buffer.push_back(g_TC_Int64SevenByte);
				m_Buffer.push_back(value & 0xFF);
				m_Buffer.push_back((value >> 8) & 0xFF);
				m_Buffer.push_back((value >> 16) & 0xFF);
				m_Buffer.push_back((value >> 24) & 0xFF);
				m_Buffer.push_back((value >> 32) & 0xFF);
				m_Buffer.push_back((value >> 40) & 0xFF);
				m_Buffer.push_back((value >> 48) & 0xFF);
				written = 8;
			}
			else
			{
				m_Buffer.push_back(g_TC_Int64);
				m_Buffer.push_back(value & 0xFF);
				m_Buffer.push_back((value >> 8) & 0xFF);
				m_Buffer.push_back((value >> 16) & 0xFF);
				m_Buffer.push_back((value >> 24) & 0xFF);
				m_Buffer.push_back((value >> 32) & 0xFF);
				m_Buffer.push_back((value >> 40) & 0xFF);
				m_Buffer.push_back((value >> 48) & 0xFF);
				m_Buffer.push_back((value >> 56) & 0xFF);
				written = 9;
			}
		}

		void C_WebsocketStreamBinary::operator<<(const uint16 & value)
		{
			volatile uint16 written = 0;
			if (!value)
			{
				m_Buffer.push_back(g_TC_UInt16Zero);
				written = 1;
			}
			else if (value < 0x100)
			{
				m_Buffer.push_back(g_TC_UInt16SingleByte);
				m_Buffer.push_back(value & 0xFF);
				written = 2;
			}
			else
			{
				m_Buffer.push_back(g_TC_UInt16);
				m_Buffer.push_back((value & 0xFF));
				m_Buffer.push_back((value >> 8) & 0xFF);
				written = 3;
			}
		}

		void C_WebsocketStreamBinary::operator<<(const uint32 & value)
		{
			volatile uint16 written = 0;
			if (!value)
			{
				m_Buffer.push_back(g_TC_UInt32Zero);
				written = 1;
			}
			else if (value < 0x100)
			{
				m_Buffer.push_back(g_TC_UInt32SingleByte);
				m_Buffer.push_back(value & 0xFF);
				written = 2;
			}
			else if (value < 0x10000)
			{
				m_Buffer.push_back(g_TC_UInt32TwoByte);
				m_Buffer.push_back((value & 0xFF));
				m_Buffer.push_back((value >> 8) & 0xFF);
				written = 3;
			}
			else if (value < 0x1000000)
			{
				m_Buffer.push_back(g_TC_UInt32ThreeByte);
				m_Buffer.push_back((value & 0xFF));
				m_Buffer.push_back((value >> 8) & 0xFF);
				m_Buffer.push_back((value >> 16) & 0xFF);
				written = 4;
			}
			else
			{
				m_Buffer.push_back(g_TC_UInt32);
				m_Buffer.push_back((value & 0xFF));
				m_Buffer.push_back((value >> 8) & 0xFF);
				m_Buffer.push_back((value >> 16) & 0xFF);
				m_Buffer.push_back((value >> 24) & 0xFF);
				written = 5;
			}
		}

		void C_WebsocketStreamBinary::operator<<(const uint64 &value)
		{
			volatile uint16 written = 0;
			if (!value)
			{
				m_Buffer.push_back(g_TC_UInt64Zero);
				written = 1;
			}
			else if (value < 0x100)
			{
				m_Buffer.push_back(g_TC_UInt64SingleByte);
				m_Buffer.push_back(value & 0xFF);
				written = 2;
			}
			else if (value < 0x10000)
			{
				m_Buffer.push_back(g_TC_UInt64TwoByte);
				m_Buffer.push_back(value & 0xFF);
				m_Buffer.push_back((value >> 8) & 0xFF);

				written = 3;
			}
			else if (value < 0x1000000)
			{
				m_Buffer.push_back(g_TC_UInt64ThreeByte);
				m_Buffer.push_back(value & 0xFF);
				m_Buffer.push_back((value >> 8) & 0xFF);
				m_Buffer.push_back((value >> 16) & 0xFF);
				written = 4;
			}
			else if (value < 0x100000000ULL)
			{
				m_Buffer.push_back(g_TC_UInt64FourByte);
				m_Buffer.push_back(value & 0xFF);
				m_Buffer.push_back((value >> 8) & 0xFF);
				m_Buffer.push_back((value >> 16) & 0xFF);
				m_Buffer.push_back((value >> 24) & 0xFF);
				written = 5;
			}
			else if (value < 0x10000000000ULL)
			{
				m_Buffer.push_back(g_TC_UInt64FiveByte);
				m_Buffer.push_back(value & 0xFF);
				m_Buffer.push_back((value >> 8) & 0xFF);
				m_Buffer.push_back((value >> 16) & 0xFF);
				m_Buffer.push_back((value >> 24) & 0xFF);
				m_Buffer.push_back((value >> 32) & 0xFF);
				written = 6;
			}
			else if (value < 0x1000000000000ULL)
			{
				m_Buffer.push_back(g_TC_UInt64SixByte);
				m_Buffer.push_back(value & 0xFF);
				m_Buffer.push_back((value >> 8) & 0xFF);
				m_Buffer.push_back((value >> 16) & 0xFF);
				m_Buffer.push_back((value >> 24) & 0xFF);
				m_Buffer.push_back((value >> 32) & 0xFF);
				m_Buffer.push_back((value >> 40) & 0xFF);
				written = 7;
			}
			else if (value < 0x100000000000000ULL)
			{
				m_Buffer.push_back(g_TC_UInt64SevenByte);
				m_Buffer.push_back(value & 0xFF);
				m_Buffer.push_back((value >> 8) & 0xFF);
				m_Buffer.push_back((value >> 16) & 0xFF);
				m_Buffer.push_back((value >> 24) & 0xFF);
				m_Buffer.push_back((value >> 32) & 0xFF);
				m_Buffer.push_back((value >> 40) & 0xFF);
				m_Buffer.push_back((value >> 48) & 0xFF);
				written = 8;
			}
			else
			{
				m_Buffer.push_back(g_TC_UInt64);
				m_Buffer.push_back(value & 0xFF);
				m_Buffer.push_back((value >> 8) & 0xFF);
				m_Buffer.push_back((value >> 16) & 0xFF);
				m_Buffer.push_back((value >> 24) & 0xFF);
				m_Buffer.push_back((value >> 32) & 0xFF);
				m_Buffer.push_back((value >> 40) & 0xFF);
				m_Buffer.push_back((value >> 48) & 0xFF);
				m_Buffer.push_back((value >> 56) & 0xFF);
				written = 9;
			}
		}

		void C_WebsocketStreamBinary::operator<<(const double &value)
		{
			int64 *ivalue = (int64*)&value;
			m_Buffer.push_back(g_TC_Double);
			m_Buffer.push_back((*ivalue & 0xFF));
			m_Buffer.push_back((*ivalue >> 8) & 0xFF);
			m_Buffer.push_back((*ivalue >> 16) & 0xFF);
			m_Buffer.push_back((*ivalue >> 24) & 0xFF);
			m_Buffer.push_back((*ivalue >> 32) & 0xFF);
			m_Buffer.push_back((*ivalue >> 40) & 0xFF);
			m_Buffer.push_back((*ivalue >> 48) & 0xFF);
			m_Buffer.push_back((*ivalue >> 56) & 0xFF);
		}

		void C_WebsocketStreamBinary::operator<<(const float & value)
		{
			int32 *ivalue = (int32*)&value;
			m_Buffer.push_back(g_TC_Float);
			m_Buffer.push_back((*ivalue & 0xFF));
			m_Buffer.push_back((*ivalue >> 8) & 0xFF);
			m_Buffer.push_back((*ivalue >> 16) & 0xFF);
			m_Buffer.push_back((*ivalue >> 24) & 0xFF);
		}

		void C_WebsocketStreamBinary::operator<<(const std::string &value)
		{
			// tpdp
// TODO: implement stream binary operator
			int size = (int) value.size();
			if (size)
			{
			}
			else if (size == 1)
			{
			}
		}

		void C_WebsocketStreamBinary::operator<<(const C_DateTime& value)
		{
			int64 ticks = value.M_GetTicks();
			if (!ticks)
			{
				m_Buffer.push_back(g_TC_DateTimeZero);
			}
			else
			{
				m_Buffer.push_back(g_TC_DateTime);
				m_Buffer.push_back(ticks & 0xFF);
				m_Buffer.push_back((ticks >> 8) & 0xFF);
				m_Buffer.push_back((ticks >> 16) & 0xFF);
				m_Buffer.push_back((ticks >> 24) & 0xFF);
				m_Buffer.push_back((ticks >> 32) & 0xFF);
				m_Buffer.push_back((ticks >> 40) & 0xFF);
				m_Buffer.push_back((ticks >> 48) & 0xFF);
				m_Buffer.push_back((ticks >> 56) & 0xFF);
			}
		}

		void C_WebsocketStreamBinary::operator<<(const char* value)
		{
			//todo
		}

		void C_WebsocketStreamBinary::operator<<(const C_Variant &value)
		{
			switch (value.M_GetType())
			{
				case g_BOOLEAN:
				{
					*this << value.M_ToBool();
					break;
				}
				case g_BYTE:
				{
					*this << value.M_ToByte();
					break;
				}
				case g_CHAR:
				{
					*this << value.M_ToChar();
					break;
				}
				case g_UINT16:
				{
					*this << value.M_ToUInt16();
					break;
				}
				case g_INT16:
				{
					*this << value.M_ToInt16();
					break;
				}
				case g_UINT32:
				{
					*this << value.M_ToUInt32();
					break;
				}
				case g_INT32:
				{
					*this << value.M_ToInt32();
					break;
				}
				case g_UINT64:
				{
					*this << value.M_ToUInt64();
					break;
				}
				case g_INT64:
				{
					*this << value.M_ToInt64();
					break;
				}
				case g_FLOAT:
				{
					*this << value.M_ToFloat();
					break;
				}
				case g_DOUBLE:
				{
					*this << value.M_ToDouble();
					break;
				}
				case g_OBJECT:
				{
					t_JsonObjectMap *map = (t_JsonObjectMap*)value.m_Data.m_Data;
					if (map)
					{
						size_t size = map->size();
						// Copy object data
						M_BeginObject();
						for (t_JsonObjectMap::iterator iter = map->begin(); iter != map->end(); iter++)
						{
							*this << iter->second;
						}
						M_EndObject();
					}
					else
					{
						M_BeginObject();
						M_AppendNull();
						M_EndObject();
					}

					break;
				}
				case g_ARRAY:
				{
					t_JsonArrayMap *map = (t_JsonArrayMap*)value.m_Data.m_Data;
					if (map)
					{
						// Copy object data
						size_t size = map->size();
						bool sametype = true;

						// checking types
						if (size)
						{
							t_DataType lasttype = map->begin()->second.M_GetType();
							for (t_JsonArrayMap::iterator iter = map->begin(); iter != map->end(); iter++)
							{
								if (iter->second.M_GetType() != lasttype)
								{
									sametype = false;
									break;
								}
							}

							if (sametype)
							{
								M_BeginArray(C_Variant::M_S_ConvertVariantTypeToTypeCode(value[0].M_GetType()));
								char tempbuff[16];
								int written = G_Write7BitEncodedInt(tempbuff, size, 16);
								for (int i = 0; i < written; i++)
								{
									m_Buffer.push_back(tempbuff[i]);
								}
							}

							for (t_JsonArrayMap::iterator iter = map->begin(); iter != map->end(); iter++)
							{
								M_Write(iter->second);
							}
						}

						if (!sametype)
						{
							M_BeginArray();

							for (t_JsonArrayMap::iterator iter = map->begin(); iter != map->end(); iter++)
							{
								*this << iter->second;
							}
							M_EndArray();
						}
					}
					else
					{
						M_BeginArray();
						M_AppendNull();
						M_EndArray();
					}

					break;
				}
				case g_MEMORY:
				{
					C_MemoryBuffer *buffer = (C_MemoryBuffer*)value.m_Data.m_Data;
					size_t size = buffer->M_GetSize();
					M_BeginArray(C_Variant::M_S_ConvertVariantTypeToTypeCode(g_BYTE));
					char tempbuff[16];
					int written = G_Write7BitEncodedInt(tempbuff, size, 16);
					m_Buffer.insert(m_Buffer.end(), &tempbuff[0], &tempbuff[written]);
					
					char *data = buffer->M_Data();
					m_Buffer.insert(m_Buffer.end(), data, data + size);
					M_EndArray();
					break;
				}
				case g_NULL:
				case g_UNKNOWN:
					M_AppendNull();
					break;
				default:
				{
					break;
				}
			}
		}

		C_DateTime::C_DateTime(int year, int month, int day, int hour, int minute, int second, int n100nanoseconds)
		{
			Mia_THIS_ROUTINE("C_DateTime::C_DateTime");
			time_t rawtime;
			struct tm * timeinfo;
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			timeinfo->tm_year = year - 1900;
			timeinfo->tm_mon = month - 1;
			timeinfo->tm_mday = day;
			timeinfo->tm_hour = hour;
			timeinfo->tm_min = minute;
			timeinfo->tm_sec = second;
			timeinfo->tm_isdst = -1;
			time_t t = mktime(timeinfo);
			if (t == (time_t)-1)
			{
				THROW(C_ExceptionBase(Mia_THIS_LOCATION, C_BaseErrorMessage(g_MiaError_InvalidDateTime, "Invalid time structure"), year, month, day, hour, minute, second, n100nanoseconds));
			}
			m_TimeData.m_FullTime = ((uint64)t << 24) | n100nanoseconds;
		}

		C_DateTime::C_DateTime(uint64 timeS, uint32 time100Ns)
		{
			m_TimeData.m_FullTime = (timeS << 24) | time100Ns;
		}

		C_DateTime C_DateTime::M_Add100Nanosecond(const int64 &n100ns) const
		{
			int64 time100ns = M_Get100Nanoseconds() + M_GetTotalSeconds() * m_k_s_100nsPERSECOND + n100ns;
			uint64 times = (uint64)(time100ns / m_k_s_100nsPERSECOND);
			uint timens = (uint)(time100ns % m_k_s_100nsPERSECOND);
			return C_DateTime(times, timens);
		}

		C_DateTime C_DateTime::M_AddSecond(const int64 &seconds) const
		{
			uint64 times = (uint64) (m_TimeData.m_FullTime >> 24) + seconds;
			return C_DateTime(times, m_TimeData.m_FullTime & 0xFFFFFF);
		}

		C_DateTime C_DateTime::M_AddMilisecond(const int64 &miliseconds) const
		{
			int64 time100ns = M_Get100Nanoseconds() + M_GetTotalSeconds() * m_k_s_100nsPERSECOND
			   + miliseconds * m_k_s_100nsPERMILISECOND;
			uint64 times = (uint64)(time100ns / m_k_s_100nsPERSECOND);
			uint32 timens = (uint32)(time100ns % m_k_s_100nsPERSECOND);
			return C_DateTime(times, timens);
		}

		C_DateTime& C_DateTime::operator=(const C_DateTime& datetime)
		{
			m_TimeData.m_FullTime = datetime.m_TimeData.m_FullTime;
			return *this;
		}

		struct tm* C_DateTime::M_GetTimeStruct() const
		{
			time_t t = (m_TimeData.m_FullTime >> 24);
			return localtime(&t);
		}

		int64 C_DateTime::M_GetTotalNanoseconds() const
		{
			return (M_GetTotalSeconds()) * Mia_LU64(1000000000) + M_Get100Nanoseconds() * 100;
		}

		int64 C_DateTime::M_GetTicks() const
		{
			return M_GetTotalSeconds() * m_k_s_100nsPERSECOND + M_Get100Nanoseconds();
		}

		C_DateTime C_DateTime::M_S_FromTicks(const int64 tickCount)
		{
			int64 nsec = tickCount / m_k_s_100nsPERSECOND;
			int64 n100sec = tickCount%m_k_s_100nsPERSECOND;
			return C_DateTime(nsec, (uint32)n100sec);
		}

		bool C_DateTime::operator<(const C_DateTime &comparand) const
		{
			uint64 curr = (m_TimeData.m_FullTime >> 24);
			uint64 comp = (comparand.m_TimeData.m_FullTime >> 24);
			return (curr < comp)
				|| ((curr == comp) && ((m_TimeData.m_FullTime & 0xFFFFFF) < (comparand.m_TimeData.m_FullTime & 0xFFFFFF)));
		}

		bool C_DateTime::operator<=(const C_DateTime &comparand) const
		{
			return (*this == comparand) || (*this < comparand);
		}

		bool C_DateTime::operator>(const C_DateTime &comparand) const
		{
			uint64 curr = (m_TimeData.m_FullTime >> 24);
			uint64 comp = (comparand.m_TimeData.m_FullTime >> 24);
			return (curr > comp)
			   || ((curr == comp) && ((m_TimeData.m_FullTime & 0xFFFFFF) > (comparand.m_TimeData.m_FullTime & 0xFFFFFF)));
		}

		bool C_DateTime::operator>=(const C_DateTime &comparand) const
		{
			return (*this == comparand) || (*this > comparand);
		}

		bool C_DateTime::operator==(const C_DateTime &comparand) const
		{
			return (((m_TimeData.m_FullTime >> 24) == (comparand.m_TimeData.m_FullTime >> 24))
			   && ((m_TimeData.m_FullTime & 0xFFFFFF) == (comparand.m_TimeData.m_FullTime & 0xFFFFFF)));
		}

		bool C_DateTime::operator!=(const C_DateTime &comparand) const
		{
			return !(*this == comparand);
		}

		uint16 C_DateTime::M_GetDayOfYear() const
		{
			return M_GetTimeStruct()->tm_yday + 1;
		}

		uint16 C_DateTime::M_GetDayOfWeek() const
		{
			return M_GetTimeStruct()->tm_wday;
		}

		uint16 C_DateTime::M_GetYear() const
		{
			return 1900 + M_GetTimeStruct()->tm_year;
		}

		uint16 C_DateTime::M_GetMonth() const
		{
			return M_GetTimeStruct()->tm_mon + 1;
		}

		uint16 C_DateTime::M_GetDay() const
		{
			return M_GetTimeStruct()->tm_mday;
		}

		uint16 C_DateTime::M_GetHour() const
		{
			return M_GetTimeStruct()->tm_hour;
		}

		uint16 C_DateTime::M_GetMinute() const
		{
			return M_GetTimeStruct()->tm_min;
		}

		uint16 C_DateTime::M_GetSecond() const
		{
			return M_GetTimeStruct()->tm_sec;
		}

		int64 C_DateTime::M_GetTotalSeconds() const
		{
			return m_TimeData.m_FullTime >> 24;
		}

		uint32 C_DateTime::M_Get100Nanoseconds() const
		{
			return m_TimeData.m_FullTime & 0xFFFFFF;
		}

		std::ostream& operator<<(std::ostream& os, const C_TimeSpan& timespan)
		{
			int64 hours, minutes;
			int64 seconds = timespan.M_GetValue() / 1000;

			hours = seconds / 3600;
			seconds = seconds % 3600;

			minutes = seconds / 60;
			seconds = seconds % 60;

			os << std::setfill('0') << std::setw(2) << hours << ":" <<
				std::setfill('0') << std::setw(2) << minutes << ":" <<
				std::setfill('0') << std::setw(2) << seconds;
			return os;
		}

		std::ostream& operator<<(std::ostream& os, const C_DateTime& time)
		{
			os << time.M_GetDay() << "-" << time.M_GetMonth() << "-" << time.M_GetYear() << " " << time.M_GetHour() << ":"
			   << time.M_GetMinute() << ":" << time.M_GetSecond();
			return os;
		}

		std::string C_DateTime::M_ToString() const
		{
			std::stringstream ss;
			ss << M_GetDay() << "-" << M_GetMonth() << "-" << M_GetYear() << " " << M_GetHour() << ":" << M_GetMinute()
			   << ":" << M_GetSecond();
			return ss.str();
		}

		C_Variant::C_Variant() : m_Type(g_UNKNOWN)
		{
			m_Data.m_Data = 0;
		}

		C_Variant::C_Variant(const C_Variant& otherVariant) : m_Type(g_UNKNOWN)
		{
			m_Data.m_Data = 0;
			M_SetValue(otherVariant);
		}

		C_Variant::C_Variant(const bool &value) :	m_Type(g_BOOLEAN)
		{
			m_Data.m_uInt = value;
		}
		C_Variant::C_Variant(const char &value) :	m_Type(g_CHAR)
		{
			m_Data.m_uInt = value;
		}
		C_Variant::C_Variant(const byte &value) :	m_Type(g_BYTE)
		{
			m_Data.m_uInt = value;
		}
		C_Variant::C_Variant(const int16 &value) : m_Type(g_INT16)
		{
			m_Data.m_uInt = value;
		}
		C_Variant::C_Variant(const uint16 &value) : m_Type(g_UINT16)
		{
			m_Data.m_uInt = value;
		}
		C_Variant::C_Variant(const int32 &value) :	m_Type(g_INT32)
		{
			m_Data.m_uInt = value;
		}
		C_Variant::C_Variant(const uint32 &value) :	m_Type(g_UINT32)
		{
			m_Data.m_uInt = value;
		}
		C_Variant::C_Variant(const int64 &value) : m_Type(g_INT64)
		{
			m_Data.m_uInt = value;
		}
		C_Variant::C_Variant(const uint64 &value) :	m_Type(g_UINT64)
		{
			m_Data.m_uInt = value;
		}
		C_Variant::C_Variant(const float &value) : m_Type(g_FLOAT)
		{
			m_Data.m_Double = value;
		}
		C_Variant::C_Variant(const double &value) : m_Type(g_DOUBLE)
		{
			m_Data.m_Double = value;
		}
		C_Variant::C_Variant(const C_Guid &value) : m_Type(g_GUID)
		{
			m_Data.m_Data = DBG_NEW C_Guid(value);
		}
		C_Variant::C_Variant(const std::string &value) : m_Type(g_STRING)
		{
			m_Data.m_Data = DBG_NEW std::string(value);
		}
		C_Variant::C_Variant(const char *&value) : m_Type(g_STRING)
		{
			m_Data.m_Data = DBG_NEW std::string(value);
		}
		ABB::Mia::C_Variant::C_Variant(const C_MemoryBuffer & buffer) : m_Type(g_MEMORY)
		{
			m_Data.m_Data = DBG_NEW C_MemoryBuffer(buffer);
		}

		C_Variant::C_Variant(const C_DateTime &value) :	m_Type(g_DATETIME)
		{
			m_Data.m_Data = DBG_NEW C_DateTime(value);
		}
		C_Variant::C_Variant(const char* value, int length) : m_Type(g_UNKNOWN)
		{
			if (length)
			{
				m_Type = g_MEMORY;
				m_Data.m_Data = DBG_NEW C_MemoryBuffer(value, length);
			} else
			{
				m_Type = g_STRING;
				m_Data.m_Data = DBG_NEW std::string(value);
			}
		}

		void C_Variant::M_Release()
		{
			if (m_Type == g_UNKNOWN) return;
			if (m_Data.m_Data)
			{
				switch (m_Type)
				{
					case g_MEMORY:
						delete ((C_MemoryBuffer*) m_Data.m_Data);
						m_Data.m_Data = 0;
						break;

					case g_STRING:
					{
						std::string* s = (std::string*) m_Data.m_Data;
						delete s;
						m_Data.m_Data = 0;
						break;
					}
					case g_GUID:
						delete ((C_Guid*) m_Data.m_Data);
						m_Data.m_Data = 0;
						break;

					case g_DATETIME:
						delete ((C_DateTime*) m_Data.m_Data);
						m_Data.m_Data = 0;
						break;

					case g_OBJECT:
					{
						t_JsonObjectMap *obj = (t_JsonObjectMap *) m_Data.m_Data;
						obj->clear();
						delete obj;
						break;
					}

					case g_ARRAY:
					{
						t_JsonArrayMap *obj = (t_JsonArrayMap *) m_Data.m_Data;
						obj->clear();
						delete obj;
						break;
					}

					default:
						break;
				}
			}
			m_Type = g_UNKNOWN;
		}

		void C_Variant::M_SetValue(const bool &value)
		{
			M_Release();
			m_Data.m_uInt = value;
			m_Type = g_BOOLEAN;
		}

		void C_Variant::M_SetValue(const char &value)
		{
			M_Release();
			m_Data.m_uInt = value;
			m_Type = g_CHAR;
		}

		void C_Variant::M_SetValue(const byte &value)
		{
			M_Release();
			m_Data.m_uInt = value;
			m_Type = g_BYTE;
		}

		C_Variant& C_Variant::operator=(const char &value)
		{
			M_SetValue(value);
			return *this;
		}
		C_Variant& C_Variant::operator=(const bool &value)
		{
			M_SetValue(value);
			return *this;
		}
		C_Variant& C_Variant::operator=(const byte &value)
		{
			M_SetValue(value);
			return *this;
		}
		C_Variant& C_Variant::operator=(const int16 &value)
		{
			M_SetValue(value);
			return *this;
		}
		C_Variant& C_Variant::operator=(const uint16 &value)
		{
			M_SetValue(value);
			return *this;
		}
		C_Variant& C_Variant::operator=(const int32 &value)
		{
			M_SetValue(value);
			return *this;
		}
		C_Variant& C_Variant::operator=(const uint32 &value)
		{
			M_SetValue(value);
			return *this;
		}
		C_Variant& C_Variant::operator=(const int64 &value)
		{
			M_SetValue(value);
			return *this;
		}
		C_Variant& C_Variant::operator=(const uint64 &value)
		{
			M_SetValue(value);
			return *this;
		}
		C_Variant& C_Variant::operator=(const float &value)
		{
			M_SetValue(value);
			return *this;
		}
		C_Variant& C_Variant::operator=(const double &value)
		{
			M_SetValue(value);
			return *this;
		}
		C_Variant& C_Variant::operator=(const C_Guid &value)
		{
			M_SetValue(value);
			return *this;
		}
		C_Variant& C_Variant::operator=(const std::string &value)
		{
			M_SetValue(value);
			return *this;
		}
		C_Variant& C_Variant::operator=(const C_DateTime& value)
		{
			M_SetValue(value);
			return *this;
		}
		C_Variant& C_Variant::operator=(const C_Variant& value)
		{
			M_SetValue(value);
			return *this;
		}

		C_Variant& C_Variant::operator=(const char* value)
		{
			M_SetValue(value);
			return *this;
		}

		bool C_Variant::operator!=(const C_Variant& value)
		{
			return !(this->operator==(value));
		}

		bool C_Variant::operator==(const C_Variant& value)
		{
			if (m_Type != value.m_Type) return false;
			switch (m_Type)
			{
				case g_BOOLEAN:
				case g_INT32:
				case g_UINT32:
				case g_INT16:
				case g_UINT16:
				case g_CHAR:
				case g_BYTE:
				case g_INT64:
				case g_UINT64:
					return m_Data.m_uInt == value.m_Data.m_uInt;
				case g_DOUBLE:
				case g_FLOAT:
				{
					return m_Data.m_Double == value.m_Data.m_Double;
				}

				case g_DATETIME:
				{
					return this->M_ToDate() == value.M_ToDate();
				}

				case g_STRING:
				{
					return this->M_ToString() == value.M_ToString();
				}

				case g_GUID:
				{
					return this->M_ToGuid() == value.M_ToGuid();
				}

				case g_OBJECT:
				case g_ARRAY:
				{
					return m_Data.m_Data == value.m_Data.m_Data;
				}

				default:
					return m_Data.m_uInt == value.m_Data.m_uInt;
			}
		}

		bool C_Variant::M_ToBool() const
		{
			return m_Data.m_uInt > 0;
		}

		int64 C_Variant::M_ToInt64() const
		{
			return m_Data.m_uInt & 0xFFFFFFFFFFFFFFFFLL;
		}

		uint64 C_Variant::M_ToUInt64() const
		{
			return m_Data.m_uInt & 0xFFFFFFFFFFFFFFFFLL;
		}

		int C_Variant::M_ToInt32() const
		{
			return m_Data.m_uInt & 0xFFFFFFFF;
		}

		uint C_Variant::M_ToUInt32() const
		{
			return (unsigned int) m_Data.m_uInt & 0xFFFFFFFF;
		}

		int16 C_Variant::M_ToInt16() const
		{
			return (int16) m_Data.m_uInt & 0xFFFF;
		}

		uint16 C_Variant::M_ToUInt16() const
		{
			return (uint16) m_Data.m_uInt & 0xFFFF;
		}

		char C_Variant::M_ToChar() const
		{
			return (char) m_Data.m_uInt & 0xFF;
		}

		unsigned char C_Variant::M_ToByte() const
		{
			return (byte) m_Data.m_uInt & 0xFF;
		}

		float C_Variant::M_ToFloat() const
		{
			return (float) m_Data.m_Double;
		}

		double C_Variant::M_ToDouble() const
		{
			return m_Data.m_Double;
		}

		C_DateTime &C_Variant::M_ToDate() const
		{
			return *((C_DateTime*) m_Data.m_Data);
		}

		const C_Guid &C_Variant::M_ToGuid() const
		{
			return *((C_Guid*) m_Data.m_Data);
		}

		void C_Variant::M_GetValue(bool *value) const
		{
			*value = (m_Data.m_uInt != 0);
		}

		void C_Variant::M_GetValue(int64 *value) const
		{
			*value = m_Data.m_uInt & 0xFFFFFFFFFFFFFFFFLL;
		}

		void C_Variant::M_GetValue(uint64 *value) const
		{
			*value = m_Data.m_uInt & 0xFFFFFFFFFFFFFFFFLL;
		}

		void C_Variant::M_GetValue(int* value) const
		{
			*value = m_Data.m_uInt & 0xFFFFFFFF;
		}

		void C_Variant::M_GetValue(unsigned int *value) const
		{
			*value = (unsigned int) m_Data.m_uInt & 0xFFFFFFFF;
		}

		void C_Variant::M_GetValue(int16 *value) const
		{
			*value = m_Data.m_uInt & 0xFFFF;
		}

		void C_Variant::M_GetValue(uint16 *value) const
		{
			*value = (uint16) m_Data.m_uInt & 0xFFFF;
		}

		void C_Variant::M_GetValue(char* value) const
		{
			*value = (char) m_Data.m_uInt & 0xFF;
		}

		void C_Variant::M_GetValue(byte *value) const
		{
			*value = (byte) m_Data.m_uInt & 0xFF;
		}

		void C_Variant::M_GetValue(float *value) const
		{
			*value = (float) m_Data.m_Double;
		}

		void C_Variant::M_GetValue(double *value) const
		{
			*value = m_Data.m_Double;
		}

		void C_Variant::M_GetValue(C_DateTime *value) const
		{
			*value = *((C_DateTime*) m_Data.m_Data);
		}

		void C_Variant::M_GetValue(C_Guid *value) const
		{
			*value = *((C_Guid*) m_Data.m_Data);
		}

		void C_Variant::M_GetValue(unsigned int index, C_Variant &variant) const
		{
			t_JsonArrayMap *array = (t_JsonArrayMap*) m_Data.m_Data;
			if (m_Type != g_ARRAY || index >= array->size())
			{
				variant.m_Type = g_UNKNOWN;
			}

			variant = (*array)[index];
		}

		void C_Variant::M_GetValue(const std::string propertyName, C_Variant &variant) const
		{
			t_JsonObjectMap *array = (t_JsonObjectMap*) m_Data.m_Data;
			if (m_Type != g_OBJECT)
			{
				variant.m_Type = g_UNKNOWN;
			}
			else
			{
				t_JsonObjectMap::iterator iter = array->find(propertyName);
				if (iter != array->end()) variant = (*array)[propertyName];
				else variant.m_Type = g_UNKNOWN;
			}
		}

		C_Variant::C_Variant(const std::string &data, t_DataType type) : m_Type(g_UNKNOWN)
		{
			M_FromString(data, type);
		}

		C_Variant::C_Variant(const std::string &data, t_PropertyType type) : m_Type(g_UNKNOWN)
		{
			t_DataType dtype = M_S_ConvertPropertyTypeToVariantType(type);
			M_FromString(data, dtype);
		}

		t_DataType C_Variant::M_S_ConvertPropertyTypeToVariantType(const t_PropertyType &type)
		{
			switch (type)
			{
				case g_PT_BOOLEAN: return g_BOOLEAN;
				case g_PT_STRING: return g_STRING;
				case g_PT_CHAR: return g_CHAR;
				case g_PT_BYTE: return g_BYTE;
				case g_PT_INT16: return g_INT16;
				case g_PT_UINT16: return g_UINT16;
				case g_PT_INT: return g_INT32;
				case g_PT_UINT: return g_UINT32;

				case g_PT_INT64: return g_INT64;
				case g_PT_UINT64: return g_UINT64;
				case g_PT_FLOAT: return g_FLOAT;
				case g_PT_DOUBLE: return g_DOUBLE;
				case g_PT_GUID: return g_GUID;
				case g_PT_TIMESTAMP: return g_DATETIME;

				case g_PT_OBJECT: return g_OBJECT;
				case g_PT_CLASS: return g_CLASS;

				case 205:
				case 206:
				case 207:
				case 208:
				case 209:
				case 210:
				case 211:
				case 212:
				case 213:
				case 214:
				case 216:
				case 242:
				case 218: return g_ARRAY;
				default: return g_UNKNOWN;
			}
		}

		C_Variant::C_Variant(t_DataType type, int size)  : m_Type(g_UNKNOWN)
		{
			TRY
			{
				m_Type = g_UNKNOWN;
				m_Data.m_Data = NULL;
				
				switch (type)
				{
					case g_BOOLEAN:
					case g_INT32:
					case g_UINT32:
					case g_INT16:
					case g_UINT16:
					case g_CHAR:
					case g_BYTE:
					case g_INT64:
					case g_UINT64:
					{
						m_Data.m_uInt = 0;
						break;
					}
					case g_DOUBLE:
					case g_FLOAT:
					{
						m_Data.m_Double = 0;
						break;
					}
					case g_STRING:
					{
						m_Data.m_Data = DBG_NEW std::string("");
						break;
					}
					case g_DATETIME:
					{
						m_Data.m_Data = DBG_NEW ABB::Mia::C_DateTime();
						break;
					}
					case g_GUID:
					{
						m_Data.m_Data = DBG_NEW C_Guid();
						break;
					}
					case g_OBJECT:
					{
						m_Data.m_Data = 
#ifndef NO_UNORDERED_MAP
						new t_JsonObjectMap(size);
#else
						new t_JsonObjectMap();
#endif
						break;
					}
					case g_ARRAY:
					{
						t_JsonArrayMap *array;
						array = DBG_NEW t_JsonArrayMap();
						m_Data.m_Data = array;
						for (int i = 0; i < size; i++)
						{
							(*array)[i] = ABB::Mia::C_Variant();
						}
						break;
					}

					case g_MEMORY:
					{
						C_MemoryBuffer* buffer = DBG_NEW C_MemoryBuffer(size);
						memset(buffer->M_Data(), 0, size);
						m_Data.m_Data = buffer;
						break;
					}

					case g_NULL:
					default:
					{
						m_Data.m_Data = 0;
						break;
					}
				}

				m_Type = type;
			} CATCH(std::exception &e,
			{
				(void)e;
				m_Type = g_UNKNOWN;
			}
			)
		}

		std::string C_Variant::M_ToString() const
		{
			switch (m_Type)
			{
				case g_UNKNOWN:
				case g_NULL:
					return "";


				case g_STRING:
					if (m_Data.m_Data) return *((std::string*) m_Data.m_Data);
					else return "";
				case g_ARRAY:
				case g_OBJECT:
				{
					if (m_Data.m_Data)
					{
						std::stringstream ss;
						M_PrintJSON(ss, *this);
						return ss.str();
					} else
					{
						return m_Type == g_ARRAY ? "[]" : "{}";
					}
				}
				case g_FLOAT:
				case g_DOUBLE:
				{
					std::stringstream ss;
					//ss.precision(17); // comment this line for shorter float format.
					ss << *this;
					return ss.str();
				}
				default:
				{
					std::stringstream ss;
					ss << *this;
					return ss.str();
				}
			}
		}
		
		void ABB::Mia::C_Variant::M_ToBinary(std::vector<char> &dataBuffer) const
		{
			dataBuffer.resize(0);

			switch (m_Type)
			{
				case g_UNKNOWN:
				case g_NULL:
					dataBuffer.push_back(g_TC_Null);
					break;

				case g_ARRAY:
				case g_OBJECT:
					{
						if (m_Data.m_Data)
						{
							M_PrintBinary(*this, dataBuffer);
						}
						else
						{
							dataBuffer.push_back(g_ARRAY ? g_TC_ObjectArray : g_TC_Object);
							dataBuffer.push_back(g_TC_End);
						}
					}
					break;

				default:
					break;
			}
			return;
		}

		void C_Variant::M_SetValue(const int16 &value)
		{
			M_Release();
			m_Data.m_uInt = value;
			m_Type = g_INT16;
		}

		void C_Variant::M_SetValue(const uint16 &value)
		{
			M_Release();
			m_Data.m_uInt = value;
			m_Type = g_UINT16;
		}

		void C_Variant::M_SetValue(const int32 &value)
		{
			M_Release();
			m_Data.m_uInt = value;
			m_Type = g_INT32;
		}

		void C_Variant::M_SetValue(const uint32 &value)
		{
			M_Release();
			m_Data.m_uInt = value;
			m_Type = g_UINT32;
		}

		void C_Variant::M_SetValue(const int64 &value)
		{
			M_Release();
			m_Data.m_uInt = value;
			m_Type = g_INT64;
		}

		void C_Variant::M_SetValue(const uint64 &value)
		{
			M_Release();
			m_Data.m_uInt = value;
			m_Type = g_UINT64;
		}

		void C_Variant::M_SetValue(const float &value)
		{
			M_Release();
			m_Data.m_Double = value;
			m_Type = g_FLOAT;
		}

		void C_Variant::M_SetValue(const double &value)
		{
			M_Release();
			m_Data.m_Double = value;
			m_Type = g_DOUBLE;
		}

		void C_Variant::M_SetValue(const C_Guid &value)
		{
			if (m_Type != g_GUID)
			{
				M_Release();
				m_Data.m_Data = DBG_NEW C_Guid(value);
				m_Type = g_GUID;
			} else
			{
				*((C_Guid*) m_Data.m_Data) = value;
			}
		}

		void C_Variant::M_SetValue(const std::string &value)
		{
			if (m_Type != g_STRING)
			{
				M_Release();
				m_Type = g_STRING;
				m_Data.m_Data = DBG_NEW std::string(value);
			} else
			{
				*((std::string*) m_Data.m_Data) = value;
			}
		}

		void C_Variant::M_SetValue(const char* &value)
		{
			if (m_Type != g_STRING)
			{
				M_Release();
				m_Data.m_Data = DBG_NEW std::string(value);
				m_Type = g_STRING;
			} else
			{
				*((std::string*) m_Data.m_Data) = value;
			}
		}

		void C_Variant::M_SetValue(const C_DateTime& value)
		{
			if (m_Type != g_DATETIME)
			{
				M_Release();
				m_Data.m_Data = DBG_NEW C_DateTime(value);
				m_Type = g_DATETIME;
			} else
			{
				*((C_DateTime*) m_Data.m_Data) = value;
			}
		}

		void C_Variant::M_SetValue(int index, const C_Variant& value)
		{
			t_JsonArrayMap *array = (t_JsonArrayMap *)m_Data.m_Data;
			(*array)[index] = value;
		}

		void C_Variant::M_SetValue(std::string propertyName, const C_Variant& value)
		{
			t_JsonObjectMap *map = (t_JsonObjectMap *)m_Data.m_Data;
			(*map)[propertyName] = value;
		}

		void C_Variant::M_SetValue(const C_Variant& value)
		{
			M_Release();

			switch (value.m_Type)
			{
				case g_BOOLEAN:
				case g_INT32:
				case g_UINT32:
				case g_INT16:
				case g_UINT16:
				case g_CHAR:
				case g_BYTE:
				case g_INT64:
				case g_UINT64:
				{
					m_Data.m_uInt = value.m_Data.m_uInt;
					break;
				}
				case g_DOUBLE:
				case g_FLOAT:
				{
					m_Data.m_Double = value.m_Data.m_Double;
					break;
				}
				case g_DATETIME:
				{
					C_DateTime t;
					value.M_GetValue(&t);
					M_SetValue(t);
					break;
				}
				case g_STRING:
				{
					M_SetValue(*(std::string*) value.m_Data.m_Data);
					break;
				}
				case g_GUID:
				{
					M_SetValue(*(C_Guid*) value.m_Data.m_Data);
					break;
				}
				case g_OBJECT:
				{
					t_JsonObjectMap *map = (t_JsonObjectMap*) value.m_Data.m_Data;
					t_JsonObjectMap *cmap = DBG_NEW t_JsonObjectMap();
					// Copy object data
					for (t_JsonObjectMap::iterator iter = map->begin(); iter != map->end(); iter++)
					{
						(*cmap)[iter->first] = iter->second;
					}
					m_Data.m_Data = cmap;
					break;
				}
				case g_ARRAY:
				{
					t_JsonArrayMap *map = (t_JsonArrayMap*) value.m_Data.m_Data;
					t_JsonArrayMap *cmap = DBG_NEW t_JsonArrayMap();
					// Copy object data
					for (t_JsonArrayMap::iterator iter = map->begin(); iter != map->end(); iter++)
					{
						(*cmap)[iter->first] = iter->second;
					}
					m_Data.m_Data = cmap;
					break;
				}

				case g_MEMORY:
				{
					C_MemoryBuffer *buffer = (C_MemoryBuffer*)value.m_Data.m_Data;
					this->m_Data.m_Data = new C_MemoryBuffer(*buffer);
					break;
				}

				// TODO: Add more conversion here
				default:
					m_Data.m_uInt = value.m_Data.m_uInt;
					break;
			}
			m_Type = value.m_Type;
		}

		bool C_Variant::M_FromString(const std::string &data, t_DataType type)
		{
			Mia_THIS_ROUTINE("C_Variant::M_FromString(const std::string &data, t_DataType type)");

			TRY
			{
				t_DataType typetoset = type;
				switch (type)
				{
					case g_BOOLEAN:
					{
						std::string d = data;
						G_MIA_TRANSFORM_TO_LOWER(d);
						if (d == "true") m_Data.m_uInt = 1;
						else m_Data.m_uInt = 0;
						break;
					}
					case g_INT16:
					case g_INT32:
					case g_INT64:
					{
						if (data.find("x") == data.npos) m_Data.m_uInt = strtoll(data.c_str(), 0, 10);
						else
						{
							std::stringstream stream(data);
							int64 temp;
							stream >> std::hex >> temp;
							m_Data.m_uInt = temp;
						}
						break;
					}
					case g_UINT16:
					case g_UINT32:
					case g_UINT64:
					{
						if (data.find("x") == data.npos) m_Data.m_uInt = strtoll(data.c_str(), 0, 10);
						else
						{
							std::stringstream stream(data);
							stream >> std::hex >> m_Data.m_uInt;
						}
						break;
					}
					case g_CHAR:
						if (data.size() == 1 && !std::isdigit(data[0]))
							m_Data.m_uInt = (char)data[0];
						else
							m_Data.m_uInt = strtoll(data.c_str(), 0, 10);
						break;
					case g_BYTE:
						if (data.size() == 1 && !std::isdigit(data[0]))
							m_Data.m_uInt = (byte)data[0];
						else
							m_Data.m_uInt = strtoll(data.c_str(), 0, 10);
						break;
					case g_DOUBLE:
					case g_FLOAT:
					{
						m_Data.m_Double = strtod(data.c_str(), 0);
						break;
					}
					case g_STRING:
					{
						m_Data.m_Data = DBG_NEW std::string(data);
						break;
					}
					case g_DATETIME:
					{
						size_t dashpos = data.find("-");

						if (dashpos == 0)
						{
							// this is negative numeric value, we shouldn't get this from server
							THROW(C_InvalidVariantTypeException(Mia_THIS_LOCATION, data));
							return false;
						}
						else if (dashpos == std::string::npos)
						{
							// int64 bit type data time
							
							TRY
							{
								uint64 timedata = G_MIA_STRTOLL(data.c_str());
								C_DateTime *atime = DBG_NEW C_DateTime();
								*atime = C_DateTime::M_S_FromTicks(timedata);
								m_Data.m_Data = atime;
							}
							CATCH (std::exception a,
							{
								m_Type = g_UNKNOWN;
							}
							)
						}
						else
						{
							m_Data.m_Data = DBG_NEW C_DateTime(data, "%d-%m-%Y %H:%M:%S");
						}
						break;
					}
					case g_GUID:
					{
						m_Data.m_Data = DBG_NEW C_Guid(data);
						break;
					}
					case g_ARRAY:
					case g_OBJECT:
					{
						if (!this->M_FromJSON(data))
						{
							m_Type = g_UNKNOWN;
							return false;
						}
						else
						{
							return true;
						}
					}
					default:
					{
						m_Data.m_Data = DBG_NEW std::string(data);
						break;
					}
				}
				m_Type = typetoset;
				return true;
			}
			CATCH(std::exception &e,
			{
				(void)e;
				return false;
			}
			)
		}


		char  C_Variant::M_S_ConvertVariantTypeToTypeCode(const t_DataType& type)
		{
			switch (type)
			{
			case g_INT32:	return g_TC_Int32;
			case g_UINT32:	return g_TC_UInt32;
			case g_INT16:	return g_TC_Int16;
			case g_UINT16: return g_TC_UInt16;
			case g_CHAR:	return g_TC_Int8;
			case g_BYTE:	return g_TC_Byte;
			case g_INT64:	return g_TC_Int64;
			case g_UINT64: return g_TC_UInt64;
			case g_DOUBLE: return g_TC_Double;
			case g_FLOAT:	return g_TC_Float;
			case g_STRING: return g_TC_String7Bit;
			case g_DATETIME: return g_TC_DateTime;
			case g_GUID:	return g_TC_GUID;
			}

			return 0;
		}


		size_t C_Variant::M_GetEstimatedSize() const
		{
			switch (this->M_GetType())
			{
			case g_STRING:
				return this->M_ToString().size() + 8;

			case g_CHAR:
			case g_BYTE:
			case g_BOOLEAN:
				return 1;

			case g_INT16:
			case g_UINT16:
				return 2;

			case g_INT32:
			case g_UINT32:
			case g_FLOAT:
				return 4;

			case g_INT64:
			case g_UINT64:
			case g_DOUBLE:
			case g_DATETIME:
				return 8;

			case g_GUID:
				return 16;

			case g_OBJECT:
			{
				t_JsonObjectMap *map = (t_JsonObjectMap*)m_Data.m_Data;
				if (map && map->size())
				{
					size_t totalsize = 0;
					for (t_JsonObjectMap::iterator iter = map->begin(); iter != map->end(); iter++)
					{
						int subsize = iter->second.M_GetEstimatedSize();
						if (subsize == -1) return (size_t)-1;
						totalsize += (size_t)subsize;
					}

					return totalsize;
				}
				else
				{
					return 4;
				}

			}
			case g_ARRAY:
			{
				t_JsonArrayMap *map = (t_JsonArrayMap*)m_Data.m_Data;
				if (map && map->size())
				{
					size_t totalsize = 0;
					for (t_JsonArrayMap::iterator iter = map->begin(); iter != map->end(); iter++)
					{
						int subsize = iter->second.M_GetEstimatedSize();
						if (subsize == -1) return (size_t)-1;
						totalsize += subsize;
					}

					return totalsize;
				}
				else
				{
					return 4;
				}

			}
			case g_MEMORY:
				return ((C_MemoryBuffer*)m_Data.m_Data)->M_GetSize() + 8; // Buffer size + possible 7Bit Encoding length
			default:
				return (size_t)-1;
			}
		}

		bool C_Variant::M_FromJSON(const std::string &json)
		{
			TRY
			{
				C_JsonReader reader(json);
				reader.M_Read(*this);

				return this->M_IsValid() && (this->m_Type == g_ARRAY || this->m_Type == g_OBJECT);
			} 
			CATCH(C_Exception &ex,
			{
				ex.M_SetHandled();
				return false;
			}
			)
		}

		C_Variant & ABB::Mia::C_Variant::M_S_Null()
		{
			// TODO: insert return statement here
			static C_Variant variant;
			return variant;
		}

		C_Variant C_Variant::M_S_FromJSON(const std::string &json)
		{
			C_Variant v;
			v.M_FromJSON(json);
			return v;
		}

		bool ABB::Mia::C_Variant::M_S_FromJSON(const std::string & jsonString, C_Variant & ouputJson)
		{
			return ouputJson.M_FromJSON(jsonString);
		}

		unsigned int C_Variant::M_GetArraySize() const
		{
			if (m_Type == g_ARRAY)
			{
				t_JsonArrayMap *map = (t_JsonArrayMap*) m_Data.m_Data;
				return (unsigned int)map->size();
			}
			return (unsigned int)-1; // XXX Dangerous. Maybe Throw something?
		}

		std::list<std::string> C_Variant::M_GetProperties() const
		{
			std::list<std::string> properties;
			if (m_Type == g_OBJECT)
			{
				t_JsonObjectMap *map = (t_JsonObjectMap*) m_Data.m_Data;

				for (t_JsonObjectMap::iterator iter = map->begin(); iter != map->end(); iter++)
				{
					properties.push_back(iter->first);
				}
			}

			return properties;
		}

		C_Variant::~C_Variant()
		{
			M_Release();
		}

		const t_DataType &C_Variant::M_GetType() const
		{
			return m_Type;
		}

		std::string C_Variant::M_GetTypeName()
		{
			return C_Variant::M_S_FromDataTypeToString(m_Type);
		}

		bool C_Variant::M_IsValid() const
		{
			return m_Type != g_UNKNOWN;
		}

		t_DataType C_Variant::M_S_FromStringToDataType(const std::string &type)
		{
			if (!type.size()) return g_UNKNOWN;
			std::string lt = type;
			G_MIA_TRANSFORM_TO_LOWER(lt);
			if (lt == "boolean") return g_BOOLEAN;
			if (lt == "string") return g_STRING;
			if (lt == "char") return g_CHAR;
			if (lt == "byte") return g_BYTE;
			if (lt == "int16") return g_INT16;
			if (lt == "uint16") return g_UINT16;
			if (lt == "int") return g_INT32;
			if (lt == "uint") return g_UINT32;
			if (lt == "int32") return g_INT32;
			if (lt == "uint32") return g_UINT32;
			if (lt == "int64") return g_INT64;
			if (lt == "uint64") return g_UINT64;
			if (lt == "single") return g_FLOAT;
			if (lt == "double") return g_DOUBLE;
			if (lt == "guid") return g_GUID;
			if (lt == "datetime") return g_DATETIME;
			if (lt == "object") return g_OBJECT;
			//if (lt == "array") return g_ARRAY;
			if (lt[0] == 'c') return g_CLASS;
			//if (lt.find("[]") != std::string::npos) return g_ARRAY;

			MIA_OUT_WARNING << "{ WARNING }:[C_Variant::M_S_FromStringToDataType]  Unknown type " << lt << std::endl;
			return g_UNKNOWN;
		}

		t_PropertyType C_Variant::M_S_FromStringToPropertyType(const std::string &type)
		{
			if (!type.size()) return g_PT_UNKNOWN;
			std::string lt = type;
			G_MIA_TRANSFORM_TO_LOWER(lt);
			if (lt == "boolean" || lt == "3") return g_PT_BOOLEAN;
			if (lt == "string" || lt == "18") return g_PT_STRING;
			if (lt == "char" || lt == "6") return g_PT_CHAR;
			if (lt == "byte" || lt == "5") return g_PT_BYTE;
			if (lt == "int16" || lt == "7") return g_PT_INT16;
			if (lt == "uint16" || lt == "8") return g_PT_UINT16;
			if (lt == "int" || lt == "9") return g_PT_INT;
			if (lt == "uint"  || lt == "10") return g_PT_UINT;
			if (lt == "int32") return g_PT_INT;
			if (lt == "uint32") return g_PT_UINT;
			if (lt == "int64" || lt == "11") return g_PT_INT64;
			if (lt == "uint64" || lt == "12") return g_PT_UINT64;
			if (lt == "single" || lt == "13") return g_PT_FLOAT;
			if (lt == "double" || lt == "14") return g_PT_DOUBLE;
			if (lt == "guid" || lt == "241") return g_PT_GUID;
			if (lt == "datetime"  || lt == "16") return g_PT_TIMESTAMP;

			if (lt == "object") return g_PT_OBJECT;
			if (lt[0] == 'c') return g_PT_CLASS;
			//if (lt.find("[]") != std::string::npos) return g_ARRAY;

			int itype = atoi(lt.c_str());
			switch (itype)
			{
				case 205: return g_PT_ARRAYBYTE;
				case 206: return g_PT_ARRAYCHAR;
				case 207: return g_PT_ARRAYINT16;
				case 208: return g_PT_ARRAYUINT16;
				case 209: return g_PT_ARRAYINT;
				case 210: return g_PT_ARRAYUINT;
				case 211: return g_PT_ARRAYINT64;
				case 212: return g_PT_ARRAYUINT64;
				case 213: return g_PT_ARRAYFLOAT;
				case 214: return g_PT_ARRAYDOUBLE;
				case 216: return g_PT_ARRAYTIMESTAMP;
				case 218: return g_PT_ARRAYSTRING;
				case 241: return g_PT_GUID;
				case 242: return g_PT_ARRAYGUID;
			}

			MIA_OUT_WARNING << "{ WARNING }:[C_Variant::M_S_FromStringToPropertyType]  Unknown type " << lt << std::endl;
			return g_PT_UNKNOWN;
		}

		std::string C_Variant::M_S_FromDataTypeToString(const t_DataType &type)
		{
			switch (type)
			{
				case g_BOOLEAN:
					return "Boolean";
				case g_STRING:
					return "String";
				case g_CHAR:
					return "Char";
				case g_BYTE:
					return "Byte";
				case g_INT16:
					return "Int16";
				case g_UINT16:
					return "UInt16";
				case g_INT32:
					return "Int";
				case g_UINT32:
					return "UInt";
				case g_INT64:
					return "Int64";
				case g_UINT64:
					return "UInt64";
				case g_FLOAT:
					return "Single";
				case g_DOUBLE:
					return "Double";
				case g_GUID:
					return "GUID";
				case g_DATETIME:
					return "DateTime";
				case g_CLASS:
					return "Class";
				case g_OBJECT:
					return "Object";
				case g_ARRAY:
					return "Array";
				default:
					return "Unknown";
			}
		}

		std::string C_Variant::M_S_FromPropertyTypeToString(const t_PropertyType &type, const t_OutputType outputFormat)
		{
			//const int arraylength = 100; //todo temporary solution!! this should be passed as parameter
			#define ARRAY_LEN "100"
			
			if (outputFormat == g_RAW)
			{
				std::stringstream ss;
				ss << (int) type;
				return ss.str();
			}
			else if (outputFormat == g_JSON)
			{
				switch (type)
				{
				case g_PT_BOOLEAN:
					return "Boolean";
				case g_PT_STRING:
					return "String";
				case g_PT_CHAR:
					return "Char";
				case g_PT_BYTE:
					return "Byte";
				case g_PT_INT16:
					return "Int16";
				case g_PT_UINT16:
					return "UInt16";
				case g_PT_INT:
					return "Int32";
				case g_PT_UINT:
					return "UInt32";
				case g_PT_INT64:
					return "Int64";
				case g_PT_UINT64:
					return "UInt64";
				case g_PT_FLOAT:
					return "Single";
				case g_PT_DOUBLE:
					return "Double";
				case g_PT_GUID:
					return "GUID";
				case g_PT_TIMESTAMP:
					return "DateTime";
				case g_PT_CLASS:
					return "Class";
				case g_PT_OBJECT:
					return "Object";
				case g_PT_ARRAYSTRING:
					return "String["  ARRAY_LEN  "]";
				case g_PT_ARRAYCHAR:
					return "Char["  ARRAY_LEN  "]";
				case g_PT_ARRAYBYTE:
					return "Byte[max]"; //todo is max an ok value?
				case g_PT_ARRAYINT16:
					return "Int16["  ARRAY_LEN  "]";
				case g_PT_ARRAYUINT16:
					return "Uint16["  ARRAY_LEN  "]";
				case g_PT_ARRAYINT:
					return "Int32["  ARRAY_LEN  "]";
				case g_PT_ARRAYUINT:
					return "Uint32["  ARRAY_LEN  "]";
				case g_PT_ARRAYINT64:
					return "Int64["  ARRAY_LEN  "]";
				case g_PT_ARRAYUINT64:
					return "Uint64["  ARRAY_LEN  "]";
				case g_PT_ARRAYFLOAT:
					return "Single["  ARRAY_LEN  "]";
				case g_PT_ARRAYDOUBLE:
					return "Double["  ARRAY_LEN  "]";
				default:
					return "Unknown";
				}
			}
			else if (outputFormat == g_LOGGER)
			{
				switch (type)
				{
				case g_PT_BOOLEAN:
					return "Boolean";
				case g_PT_STRING:
					return "String";
				case g_PT_CHAR:
					return "Char";
				case g_PT_BYTE:
					return "Byte";
				case g_PT_INT16:
					return "Int16";
				case g_PT_UINT16:
					return "UInt16";
				case g_PT_INT:
					return "Int32";
				case g_PT_UINT:
					return "UInt32";
				case g_PT_INT64:
					return "Int64";
				case g_PT_UINT64:
					return "UInt64";
				case g_PT_FLOAT:
					return "Float";
				case g_PT_DOUBLE:
					return "Double";
				case g_PT_GUID:
					return "GUID";
				case g_PT_TIMESTAMP:
					return "DateTime";
				case g_PT_CLASS:
					return "Class";
				case g_PT_OBJECT:
					return "Object";
				case g_PT_ARRAYSTRING:
					return "StringArray";
				case g_PT_ARRAYCHAR:
					return "CharArray";
				case g_PT_ARRAYBYTE:
					return "ByteArray";
				case g_PT_ARRAYINT16:
					return "Int16Array";
				case g_PT_ARRAYUINT16:
					return "Uint16Array";
				case g_PT_ARRAYINT:
					return "Int32Array";
				case g_PT_ARRAYUINT:
					return "Uint32Array";
				case g_PT_ARRAYINT64:
					return "Int64Array";
				case g_PT_ARRAYUINT64:
					return "Uint64Array";
				case g_PT_ARRAYFLOAT:
					return "FloatArray";
				case g_PT_ARRAYDOUBLE:
					return "DoubleArray";
				default:
					return "Unknown";
				}
			}
			
			return "Unknown";
		}

		uint32 M_PrintBinary(const C_Variant& dt, std::vector<char> &dataBuffer)
		{
			uint32 written = 0;
			switch (dt.M_GetType())
			{
				case g_BOOLEAN:
				{
					dataBuffer.push_back(dt.M_ToBool() ? g_TC_BoolTrue : g_TC_BoolFalse);
					written++;
					break;
				}
				case g_BYTE:
				{
					byte b = (dt.M_ToByte());
					dataBuffer.push_back(b ? g_TC_Byte : g_TC_ByteZero);
					if (b) dataBuffer.push_back(b);
					written = b ? 2 : 1;
					break;
				}
				case g_CHAR:
				{
					char b = (dt.M_ToChar());
					dataBuffer.push_back(b ? g_TC_Int8 : g_TC_Int8Zero);
					if (b) dataBuffer.push_back(b);
					written = b ? 2 : 1;
					break;
				}
				case g_UINT16:
				{
					uint16 b = dt.M_ToUInt16();
					if (!b)
					{
						dataBuffer.push_back(g_TC_UInt16Zero);
						written = 1;
					}
					else if (b < 0x100)
					{
						dataBuffer.push_back(g_TC_UInt16SingleByte);
						dataBuffer.push_back(b & 0xFF);
						written = 2;
					}
					else
					{
						dataBuffer.push_back(g_TC_UInt16);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						written = 3;
					}
					break;
				}
				case g_INT16:
				{
					int16 b = dt.M_ToInt16();
					int16 cb = b;
					if (cb < 0)
						cb *= -1;
					if (!cb)
					{
						dataBuffer.push_back(g_TC_Int16Zero);
						written = 1;
					}
					else if (cb < 0x80)
					{
						dataBuffer.push_back(g_TC_Int16SingleByte);
						dataBuffer.push_back(b & 0xFF);
						written = 2;
					}
					else
					{
						dataBuffer.push_back(g_TC_Int16);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						written = 3;
					}

					break;
				}
				case g_UINT32:
				{
					uint32 b = dt.M_ToUInt32();
					if (!b)
					{
						dataBuffer.push_back(g_TC_UInt32Zero);
						written = 1;
					}
					else if (b < 0x100)
					{
						dataBuffer.push_back(g_TC_UInt32SingleByte);
						dataBuffer.push_back(b & 0xFF);
						written = 2;
					}
					else if (b < 0x10000)
					{
						dataBuffer.push_back(g_TC_UInt32TwoByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						written = 3;
					}
					else if (b < 0x1000000)
					{
						dataBuffer.push_back(g_TC_UInt32ThreeByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						written = 4;
					} else 
					{
						dataBuffer.push_back(g_TC_UInt32);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						dataBuffer.push_back((b >> 24) & 0xFF);
						written = 5;
					}
					break;
				}
				case g_INT32:
				{
					int32 b = dt.M_ToInt32();
					int32 cb = b;
					if (cb < 0)
						cb *= -1;
					if (!cb)
					{
						dataBuffer.push_back(g_TC_Int32Zero);
						written = 1;
					}
					else if (cb < 0x80)
					{
						dataBuffer.push_back(g_TC_Int32SingleByte);
						dataBuffer.push_back(b & 0xFF);
						written = 2;
					}
					else if (cb < 0x8000)
					{
						dataBuffer.push_back(g_TC_Int32TwoByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8 & 0xFF));
						written = 3;
					}
					else if (cb < 0x800000)
					{
						dataBuffer.push_back(g_TC_Int32ThreeByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						written = 4;
					}
					else
					{
						dataBuffer.push_back(g_TC_Int32);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						dataBuffer.push_back((b >> 24) & 0xFF);
						written = 5;
					}

					break;
				}
				case g_UINT64:
				{
					uint64 b = dt.M_ToUInt64();
					if (!b)
					{
						dataBuffer.push_back(g_TC_UInt64Zero);
						written = 1;
					}
					else if (b < 0x100)
					{
						dataBuffer.push_back(g_TC_UInt64SingleByte);
						dataBuffer.push_back(b & 0xFF);
						written = 2;
					}
					else if (b < 0x10000)
					{
						dataBuffer.push_back(g_TC_UInt64TwoByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						written = 3;
					}
					else if (b < 0x1000000)
					{
						dataBuffer.push_back(g_TC_UInt64ThreeByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						written = 4;
					}
					else if (b < 0x100000000ULL)
					{
						dataBuffer.push_back(g_TC_UInt64FourByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						dataBuffer.push_back((b >> 24) & 0xFF);
						written = 5;
					}
					else if (b < 0x10000000000ULL)
					{
						dataBuffer.push_back(g_TC_UInt64FiveByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						dataBuffer.push_back((b >> 24) & 0xFF);
						dataBuffer.push_back((b >> 32) & 0xFF);
						written = 6;
					}
					else if (b < 0x1000000000000ULL)
					{
						dataBuffer.push_back(g_TC_UInt64SixByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						dataBuffer.push_back((b >> 24) & 0xFF);
						dataBuffer.push_back((b >> 32) & 0xFF);
						dataBuffer.push_back((b >> 40) & 0xFF);
						written = 7;
					}
					else if (b < 0x100000000000000ULL)
					{
						dataBuffer.push_back(g_TC_UInt64SevenByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						dataBuffer.push_back((b >> 24) & 0xFF);
						dataBuffer.push_back((b >> 32) & 0xFF);
						dataBuffer.push_back((b >> 40) & 0xFF);
						dataBuffer.push_back((b >> 48) & 0xFF);
						written = 8;
					}
					else
					{
						dataBuffer.push_back(g_TC_UInt64);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						dataBuffer.push_back((b >> 24) & 0xFF);
						dataBuffer.push_back((b >> 32) & 0xFF);
						dataBuffer.push_back((b >> 40) & 0xFF);
						dataBuffer.push_back((b >> 48) & 0xFF);
						dataBuffer.push_back((b >> 56) & 0xFF);
						written = 9;
					}

					break;
				}
				case g_INT64:
				{
					int64 b = dt.M_ToInt64();
					int64 cb = b;
					if (cb < 0)
						cb *= -1;
					if (!cb)
					{
						dataBuffer.push_back(g_TC_Int64Zero);
						written = 1;
					}
					else if (cb < 0x80)
					{
						dataBuffer.push_back(g_TC_Int64SingleByte);
						dataBuffer.push_back(b & 0xFF);
						written = 2;
					}
					else if (cb < 0x8000)
					{
						dataBuffer.push_back(g_TC_Int64TwoByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						written = 3;
					}
					else if (cb < 0x800000)
					{
						dataBuffer.push_back(g_TC_Int64ThreeByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						written = 4;
					}
					else if (cb < 0x80000000ULL)
					{
						dataBuffer.push_back(g_TC_Int64FourByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						dataBuffer.push_back((b >> 24) & 0xFF);
						written = 5;
					}
					else if (cb < 0x8000000000ULL)
					{
						dataBuffer.push_back(g_TC_Int64FiveByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						dataBuffer.push_back((b >> 24) & 0xFF);
						dataBuffer.push_back((b >> 32) & 0xFF);
						written = 6;
					}
					else if (cb < 0x800000000000ULL)
					{
						dataBuffer.push_back(g_TC_Int64SixByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						dataBuffer.push_back((b >> 24) & 0xFF);
						dataBuffer.push_back((b >> 32) & 0xFF);
						dataBuffer.push_back((b >> 40) & 0xFF);
						written = 7;
					}
					else if (cb < 0x80000000000000ULL)
					{
						dataBuffer.push_back(g_TC_Int64SevenByte);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						dataBuffer.push_back((b >> 24) & 0xFF);
						dataBuffer.push_back((b >> 32) & 0xFF);
						dataBuffer.push_back((b >> 40) & 0xFF);
						dataBuffer.push_back((b >> 48) & 0xFF);
						written = 8;
					}
					else
					{
						dataBuffer.push_back(g_TC_Int64);
						dataBuffer.push_back(b & 0xFF);
						dataBuffer.push_back((b >> 8) & 0xFF);
						dataBuffer.push_back((b >> 16) & 0xFF);
						dataBuffer.push_back((b >> 24) & 0xFF);
						dataBuffer.push_back((b >> 32) & 0xFF);
						dataBuffer.push_back((b >> 40) & 0xFF);
						dataBuffer.push_back((b >> 48) & 0xFF);
						dataBuffer.push_back((b >> 56) & 0xFF);
						written = 9;
					}
					break;
				}
				case g_FLOAT:
				{
					float fd = dt.M_ToFloat();
					int32 *b = (int32*) &fd;
					dataBuffer.push_back(g_TC_Float);
					dataBuffer.push_back((*b & 0xFF));
					dataBuffer.push_back((*b >> 8) & 0xFF);
					dataBuffer.push_back((*b >> 16) & 0xFF);
					dataBuffer.push_back((*b >> 24) & 0xFF);
					written = 5;
					break;
				}
				case g_DOUBLE:
				{
					double fd = dt.M_ToDouble();
					int64 *b = (int64*)&fd;
					dataBuffer.push_back(g_TC_Double);
					dataBuffer.push_back((*b & 0xFF));
					dataBuffer.push_back((*b >> 8) & 0xFF);
					dataBuffer.push_back((*b >> 16) & 0xFF);
					dataBuffer.push_back((*b >> 24) & 0xFF);
					dataBuffer.push_back((*b >> 32) & 0xFF);
					dataBuffer.push_back((*b >> 40) & 0xFF);
					dataBuffer.push_back((*b >> 48) & 0xFF);
					dataBuffer.push_back((*b >> 56) & 0xFF);
					written = 9;
					break;
				}
				case g_OBJECT:
				{
					t_JsonObjectMap *map = (t_JsonObjectMap*)dt.m_Data.m_Data;
					if (map)
					{
						uint32 index = 0;

						// Copy object data
						dataBuffer.push_back(g_TC_Object);
						index++;
						for (t_JsonObjectMap::iterator iter = map->begin(); iter != map->end(); iter++)
						{
							index += M_PrintBinary(iter->second, dataBuffer);
						}
						dataBuffer.push_back(g_TC_End);
						index++;
						written = index;
					}
					else
					{
						dataBuffer[0] = g_TC_Object;
						dataBuffer[1] = g_TC_Null;
						dataBuffer[2] = g_TC_End;
						written = 3;
					}

					break;
				}
				case g_ARRAY:
				{
					t_JsonArrayMap *map = (t_JsonArrayMap*)dt.m_Data.m_Data;
					if (map)
					{
						uint32 index = 0;

						// Copy object data
						dataBuffer.push_back(g_TC_ObjectArray);
						index++;
						for (t_JsonArrayMap::iterator iter = map->begin(); iter != map->end(); iter++)
						{
							index += M_PrintBinary(iter->second, dataBuffer);
						}
						dataBuffer.push_back(g_TC_End);
						index++;
						written = index;
					}
					else
					{
						dataBuffer.push_back(g_TC_ObjectArray);
						dataBuffer.push_back(g_TC_Null);
						dataBuffer.push_back(g_TC_End);
						written = 3;
					}

					break;
				}
				case g_NULL:
				case g_UNKNOWN:
					dataBuffer.push_back(g_TC_Null);
					written = 1;
					break;
				default:
				{
					break;
				}
			}

			return written;
		}

		void M_PrintJSON(std::ostream& os, const C_Variant& dt)
		{
			switch (dt.M_GetType())
			{
				case g_BOOLEAN:
				{
					os << ((dt.m_Data.m_uInt) ? "true" : "false");
					break;
				}
				case g_INT32:
				{
					os << (int32)dt.m_Data.m_uInt;
					break;
				}
				case g_INT16:
				{
					os << (int16)dt.m_Data.m_uInt;
					break;
				}
				case g_CHAR:
				{
					os << (char)dt.m_Data.m_uInt;
					break;
				}
				case g_INT64:
				{
					os << (int64)dt.m_Data.m_uInt;
					break;
				}

				case g_UINT16:
				case g_UINT32:
				case g_BYTE:
				case g_UINT64:
				{
					os << (unsigned long long int) dt.m_Data.m_uInt;
					break;
				}
				case g_DOUBLE:
				case g_FLOAT:
				{
					os.precision(17);
					os << dt.m_Data.m_Double;
					break;
				}
				case g_STRING:
				{
					os << "\"" << (*((std::string*) dt.m_Data.m_Data)) << "\"";
					break;
				}
				case g_DATETIME:
				{
					C_DateTime* data = (C_DateTime*) dt.m_Data.m_Data;
					os <<  "\"" << data->M_GetTicks() << "\"";
					break;
				}
				case g_GUID:
				{
					os << "\"" << *((C_Guid*) dt.m_Data.m_Data) << "\"";
					break;
				}
				case g_OBJECT:
				{
					t_JsonObjectMap *map = (t_JsonObjectMap*) dt.m_Data.m_Data;
					if (map)
					{
						int size = (int) map->size();
						int i = 0;
						// Copy object data
						os << "{";
						for (t_JsonObjectMap::iterator iter = map->begin(); iter != map->end(); iter++)
						{
							os << "\"" << iter->first << "\"";
							os << ":";
							M_PrintJSON(os, iter->second);
							if (++i < size) {	os << ","; }
						}
						os << "}";
					} else
					{
						os << "{}";
					}

					break;
				}
				case g_ARRAY:
				{
					t_JsonArrayMap *map = (t_JsonArrayMap*) dt.m_Data.m_Data;
					if (map)
					{
						// Copy object data
						int size = (int) map->size();
						int i = 0;
						// Copy object data
						os << "[";
						for (t_JsonArrayMap::iterator iter = map->begin(); iter != map->end(); iter++)
						{
							M_PrintJSON(os, iter->second);
							if (++i < size) {	os << ","; }
						}
						os << "]";
					} else
					{
						os << "[]";
					}

					break;
				}
				case g_NULL:
				case g_UNKNOWN:
					os << "null";
					break;
				default:
				{
					break;
				}
			}
		}

		std::ostream& operator<<(std::ostream& os, const C_Variant& dt)
		{
			switch (dt.M_GetType())
			{
				case g_BOOLEAN:
				{
					os << ((dt.m_Data.m_uInt) ? "true" : "false");
					break;
				}
				case g_INT32:
				case g_INT16:
				case g_CHAR:
				case g_INT64:
				{
					os << dt.m_Data.m_uInt;
					break;
				}
				case g_UINT16:
				case g_UINT32:
				case g_BYTE:
				case g_UINT64:
				{
					os << (unsigned long long int) dt.m_Data.m_uInt;
					break;
				}
				case g_DOUBLE:
				case g_FLOAT:
				{
					os << dt.m_Data.m_Double;
					break;
				}
				case g_CLASS:
				case g_STRING:
				{
					os << (*((std::string*) dt.m_Data.m_Data));
					break;
				}
				case g_DATETIME:
				{
					os << *((C_DateTime*) dt.m_Data.m_Data);
					break;
				}
				case g_GUID:
				{
					os << *((C_Guid*) dt.m_Data.m_Data);
					break;
				}
				case g_OBJECT:
				case g_ARRAY:
				{
					M_PrintJSON(os, dt);
					break;
				}
				default:
				{
					break;
				}
			}
			return os;
		}


		bool C_Thread::M_Start()
		{
			// Create a thread if is not running
			if (M_IsRunning()) return false;
			m_Running = true;
			m_Loop = false;
			m_Stopped = false;

			if(0 != M_Start_OS())
			{
				m_Running = false;
				m_Stopped = true;
				return false;
			}

			return true;
		}

		bool C_Thread::M_Loop(int intervalMs)
		{
			m_IntervalMs = intervalMs;
			if (M_IsRunning()) return false;
			m_Loop = true;
			m_Running = true;
			m_Stopped = false;

			if(0 != M_Start_OS())
			{
				m_Running = false;
				m_Stopped = true;
				return false;
			}

			return true;
		}


		bool C_Thread::M_Stop(bool waitForStopping, int timeout)
		{
			m_Running = false;
			if (waitForStopping)
			{
				int divider = 0;

				if (timeout > 0)
				{
					while (!m_Stopped && timeout > 0)
					{
						timeout -= 5;
						C_Thread::M_S_Sleep(5);
						if(!(++divider % 1000))
						{
							MIA_OUT_DEBUG << "Waiting task " << m_RoutineName  << "to stop. Time left " << timeout << "\n";
						}
					}
				} else
				{
					timeout = g_INFINITE; // safe mechanism for service lockup
					while (!m_Stopped) 
					{
						C_Thread::M_S_Sleep(5);
						if(!(++divider % 1000))
						{
							MIA_OUT_DEBUG << "Waiting task " << m_RoutineName  << "to stop\n";
						}
					}
				}
			}
			return m_Stopped;
		}

		bool C_Thread::M_IsRunning()
		{
			return (!m_Stopped);
		}

		SYSTEM_THREAD_RET C_Thread::M_S_Routine(SYSTEM_THREAD_PARAM thread)
		{
			C_Thread *t = (C_Thread*)thread;
			if (t->m_Running && !t->m_Stopped)
			{
				if (t->m_Loop)
				{
					while (t->m_Running)
					{
						t->M_Routine();
						if (t->m_IntervalMs > 0)
						{
							C_Thread::M_S_Sleep(t->m_IntervalMs);
						}
					}
				}
				else
				{
					t->M_Routine();
				}
			}
			t->m_Running = false;
			t->m_Stopped = true;
			t->m_Loop = false;
			t->M_Die_OS(); // Dying mus be last because this does not return after killing itself
			return 0;
		}

		C_Locker::C_Locker(C_ILock *lock, bool locknow) :
			m_Lock(lock), m_IsLock(false)
		{
			if (locknow)
			{
				m_Lock->M_Lock();
				m_IsLock = true;
			}
		}

		C_Locker::~C_Locker()
		{
			if (m_IsLock)
			{
				m_Lock->M_Unlock();
			}
		}

		void C_Locker::M_Lock()
		{
			m_Lock->M_Lock();
			m_IsLock = true;
		}

		void C_Locker::M_Unlock()
		{
			m_Lock->M_Unlock();
			m_IsLock = false;
		}

		C_Guid::C_Guid()
		{
			memset(&m_Data, 0, sizeof(m_Data));
		}

		C_Guid C_Guid::M_S_GetRandomGuid()
		{
			static bool once;

			if(!once) 
			{
				srand((int)time(0));
				once = true;
			}
			C_Guid data;
			for (int i = 0; i < 16; i++)
			{
				data.m_Data.m_Data[i] = (byte) rand() % 0xFF;
			}
			return data;
		}

		C_Guid::C_Guid(const std::string &data)
		{
			memset(&m_Data, 0, sizeof(m_Data));

			// convert from string to hex
			M_SetData(data);
		}

		C_Guid::C_Guid(const C_Guid &guid)
		{
			memcpy(&m_Data, &(guid.m_Data), sizeof(m_Data));
		}

		byte &C_Guid::operator[](const int &index)
		{
			return m_Data.m_Data[index % 16];
		}

		const byte &C_Guid::operator[](const int &index) const
		{
			return m_Data.m_Data[index % 16];
		}

		void C_Guid::operator=(const C_Guid& value)
		{
			memcpy(m_Data.m_Data, value.m_Data.m_Data, 16);
		}

		void C_Guid::M_SetData(const std::string &data)
		{
			unsigned int j = 0, i = 0;
			char c1, c2;
			char buffer[33];
			if (data.size() == 32)
			{
				strncpy(buffer, data.c_str(), 32);
				buffer[32] = '\0';
			} else if (data.size() > 32)
			{
				while (i < data.length() && j < 32)
				{
					while (data[i] == '-' && i < data.length())
						i++;
					buffer[j++] = data[i++];
				}
			} else
			{
				memset(&m_Data, 0, sizeof(m_Data));
				return;
			}

			i = 0;
			j = 0;
			while (i < 32)
			{
				while ((c1 = M_S_CharToHex(buffer[i++])) == -1 && i < 32)
				{
				};
				while ((c2 = M_S_CharToHex(buffer[i++])) == -1 && i < 32)
				{
				};
				m_Data.m_Data[j++] = (c1 << 4) | c2;
				if (j == 16) return;
			}
		}

		void C_Guid::M_SetRawData(const char *data)
		{
			memcpy(&m_Data, data, sizeof(m_Data));
		}

		bool C_Guid::operator==(const C_Guid& value) const
		{
			for (int i = 0; i < 16; i++)
			{
				if (m_Data.m_Data[i] != value.m_Data.m_Data[i]) return false;
			}
			return true;
		}

		std::string C_Guid::M_ToString() const
		{
			std::string ret;
			for (int i = 0; i < 16; i++)
			{
				char d = m_Data.m_Data[i];
				ret += M_S_HexToChar(((d >> 4) & 0xF));
				ret += M_S_HexToChar(d & 0xF);
				if (i == 3 || i == 5 || i == 7 || i == 9) ret += "-";
			};
			return ret;
		}

		uint32 C_Guid::M_GetData1() const
		{
#if defined BYTE_ORDER && BYTE_ORDER == BIG_ENDIAN
			return m_Data.m_SubData.m_Data1;
#else
			uint32 ret = 0;
			char *p = (char*) &ret;
			for (int i = 0; i < 4; i++)
			{
				p[i] = m_Data.m_Data[3 - i];
			}
			return ret;
#endif
		}

		void C_Guid::M_SetData1(const uint32 &data)
		{
#if defined BYTE_ORDER && BYTE_ORDER == BIG_ENDIAN
			m_Data.m_SubData.m_Data1 = data;
#else
			char *p = ((char*) &data);
			for (int i = 0; i < 4; i++)
			{
				m_Data.m_Data[i] = p[3 - i];
			}
#endif
		}

		uint16 C_Guid::M_GetData2() const
		{
#if defined BYTE_ORDER && BYTE_ORDER == BIG_ENDIAN
			return m_Data.m_SubData.m_Data2;
#else
			uint16 ret = 0;
			char *p = (char*) &ret;
			p[0] = m_Data.m_Data[5];
			p[1] = m_Data.m_Data[4];
			return ret;
#endif
		}

		void C_Guid::M_SetData2(const int16 &data)
		{
#if defined BYTE_ORDER && BYTE_ORDER == BIG_ENDIAN
			m_Data.m_SubData.m_Data2 = data;
#else
			char *p = ((char*) &data);
			m_Data.m_Data[5] = p[0];
			m_Data.m_Data[4] = p[1];
#endif
		}

		uint16 C_Guid::M_GetData3() const
		{
#if defined BYTE_ORDER && BYTE_ORDER == BIG_ENDIAN
			return m_Data.m_SubData.m_Data3;
#else
			uint16 ret = 0;
			char *p = (char*) &ret;
			p[0] = m_Data.m_Data[7];
			p[1] = m_Data.m_Data[6];
			return ret;
#endif
		}

		void C_Guid::M_SetData3(const int16 &data)
		{
#if defined BYTE_ORDER && BYTE_ORDER == BIG_ENDIAN
			m_Data.m_SubData.m_Data3 = data;
#else
			char *p = ((char*) &data);
			m_Data.m_Data[7] = p[0];
			m_Data.m_Data[6] = p[1];
#endif
		}

		bool C_Guid::M_IsNull()
		{
			return (m_Data.m_SubData.m_Data1 == 0 && m_Data.m_SubData.m_Data2 == 0 && m_Data.m_SubData.m_Data3 == 0
			   && m_Data.m_SubData.m_Data4 == 0);
		}

		uint64 C_Guid::M_GetData4() const
		{
#if defined BYTE_ORDER && BYTE_ORDER == BIG_ENDIAN
			return m_Data.m_SubData.m_Data4;
#else
			uint64 ret = 0;
			char *p = (char*) &ret;
			for (int i = 0; i < 8; i++)
			{
				p[i] = m_Data.m_Data[15 - i];
			}
			return ret;
#endif
		}

		void C_Guid::M_SetData4(const byte data[8])
		{
#if defined BYTE_ORDER && BYTE_ORDER == BIG_ENDIAN
			*((int64*)m_Data.m_SubData.m_Data4) = *((int64*)data);
#else
			char *p = (char*) &data[0];
			for (int i = 0; i < 8; i++)
			{
				m_Data.m_Data[15 - i] = p[i];
			}
#endif
		}

		void C_Guid::M_SetData4(const uint64 &data)
		{
#if defined BYTE_ORDER && BYTE_ORDER == BIG_ENDIAN
			m_Data.m_SubData.m_Data4 = data;
#else
			char *p = (char*) &data;
			for (int i = 0; i < 8; i++)
			{
				m_Data.m_Data[15 - i] = p[i];
			}
#endif
		}

		std::ostream& operator<<(std::ostream& os, const C_Guid &guid)
		{
			os << guid.M_ToString();
			return os;
		}


		char C_Guid::M_S_HexToChar(char c)
		{
			return "0123456789ABCDEF"[c % 16];
		}

		char C_Guid::M_S_CharToHex(char c)
		{
			switch (c)
			{
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				{
					return c - '0';
				}

				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
				{
					return (c - 'A') + 10;
				}

				case 'a':
				case 'b':
				case 'c':
				case 'd':
				case 'e':
				case 'f':
				{
					return (c - 'a') + 10;
				}

				default:
					return -1;
			}
		}
		int C_MemoryBuffer::m_s_InstanceCount = 0;
		C_MemoryBuffer::C_MemoryBuffer(unsigned int size) 
		{
			m_Size = size;
			m_AutoRelease = true;
			m_Buffer = (size) ? DBG_NEW char[m_Size] : 0;
			m_AutoRelease = m_Size > 0;
			m_s_InstanceCount++;
		}

		C_MemoryBuffer::~C_MemoryBuffer()
		{
			if (m_AutoRelease && m_Buffer)
			{
				delete[] m_Buffer;
				m_Buffer = 0;
			}
			m_s_InstanceCount--;
			MIA_OUT_DEBUG << "[C_MemoryBuffer] Current instance count " << m_s_InstanceCount << std::endl;
		}

		C_MemoryBuffer::C_MemoryBuffer(const char* buffer, int size) : m_AutoRelease(true)
		{
			m_Size = size;
			m_Buffer = DBG_NEW char[m_Size];
			memcpy(m_Buffer, buffer, m_Size);
			m_s_InstanceCount++;
		}

		C_MemoryBuffer::C_MemoryBuffer(const C_MemoryBuffer &buffer) : m_AutoRelease(true)
		{
			m_Size = buffer.m_Size;
			m_Buffer = DBG_NEW char[m_Size];
			memcpy(m_Buffer, buffer.m_Buffer, m_Size);
			m_s_InstanceCount++;
		}

		void C_MemoryBuffer::operator=(const C_MemoryBuffer &buffer)
		{
			m_Size = buffer.m_Size;
			m_Buffer = DBG_NEW char[m_Size];
			m_AutoRelease = true;
			memcpy(m_Buffer, buffer.m_Buffer, m_Size);
		}

		char &C_MemoryBuffer::operator[](int index)
		{
			return m_Buffer[index];
		}

		const char &C_MemoryBuffer::operator[](int index) const
		{
			return m_Buffer[index];
		}

		char &C_MemoryBuffer::operator[](uint index)
		{
			return m_Buffer[index];
		}

		const char &C_MemoryBuffer::operator[](uint index) const
		{
			return m_Buffer[index];
		}

		C_MemoryBuffer * ABB::Mia::C_MemoryBuffer::M_CreateShadowCopy()
		{
			C_MemoryBuffer *newinstance = new C_MemoryBuffer();
			newinstance->m_Buffer = this->m_Buffer;
			newinstance->m_Size = this->m_Size;
			m_s_InstanceCount++;

			return newinstance;
		}

		const int &C_MemoryBuffer::M_GetSize() const
		{
			return m_Size;
		}
		void C_MemoryBuffer::M_Resize(int newSize)
		{
			char* newbufer = DBG_NEW char[newSize];
			memcpy(newbufer, m_Buffer, m_Size);
			m_Buffer = 0;
			m_Buffer = newbufer;
			m_Size = newSize;
		}

		void C_MemoryBuffer::M_Clear()
		{
			memset(m_Buffer, 0, m_Size);
		}

		char* C_MemoryBuffer::M_Data() const
		{
			return m_Buffer;
		}

		void C_MemoryBuffer::M_SetData(char* data, bool autoRelease)
		{
			m_Buffer = data;
			m_AutoRelease = autoRelease;
		}

		C_BaseErrorMessage::C_BaseErrorMessage(const uint64 &erroCode, const std::string &errorMessage)
		{
			m_ErrorCode = erroCode;
			m_ErrorMessage = errorMessage;
		}

		C_BaseErrorMessage C_BaseErrorMessage::M_S_AddTrace()
		{
			static C_BaseErrorMessage base;
			return base;
		}

		const uint64 &C_BaseErrorMessage::M_GetErrorCode()
		{
			return m_ErrorCode;
		}
		const std::string &C_BaseErrorMessage::M_GetErrorMessage()
		{
			return m_ErrorMessage;
		}

		C_ErrorLocationData::C_ErrorLocationData(const std::string &routineName, const std::string &sourceFileName,
		   int sourceFileLine, const std::string &compilationTime, const std::string &sourceFileTime,
		   const std::string &sourceCodeControlVersion) :
			m_RoutineName(routineName), m_SourceFileName(sourceFileName), m_SourceFileLine(sourceFileLine), m_CompilationTime(
			   compilationTime), m_SourceFileTime(sourceFileTime), m_SourceCodeControlVersion(sourceCodeControlVersion)
		{

		}

		void C_ErrorLocationData::M_Dump(std::ostream &os) const
		{
			os << "> " << m_SourceFileName << ":" << m_SourceFileLine << ":" << m_RoutineName;
		}

		Mia_EXPORT size_t G_Read7BitEncodedInt(const void* x, size_t maxbytes)
		{
			size_t readbytes;
			return G_Read7BitEncodedInt(x, &readbytes, maxbytes);
		};

		Mia_EXPORT size_t G_Read7BitEncodedInt(const void* x, size_t *readbytes, size_t maxbytes)
		{
			uint8 num3;
			uint num = 0, num2 = 0;
			*readbytes = 0;
			const uint8 *y = (uint8*)x;
			if (maxbytes == 0) return 0;
			do
			{
				num3 = *y++;
				num |= (num3 & 0x7f) << num2;
				num2 += 7;
			} while (++*readbytes < maxbytes && (num3 & 0x80) != 0);
			return num;
		};

		Mia_EXPORT size_t G_Write7BitEncodedInt(void* x, size_t value, size_t maxbytes)
		{
			if (maxbytes == 0) return 0;
			size_t lengthbytes = 1;
			uint8 *y = (uint8*)x;
			while (value >= 0x80 && lengthbytes < maxbytes)
			{
				*y++ = (uint8)(value | 0x80);
				value >>= 7;
				++lengthbytes;
			}
			*y = (uint8)value;
			return lengthbytes;
		}

	} /* namespace Mia */
} /* namespace ABB */
