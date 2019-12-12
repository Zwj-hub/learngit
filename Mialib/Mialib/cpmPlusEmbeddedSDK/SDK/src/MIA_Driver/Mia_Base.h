/*
 * Mia_Base.h
 *
 *  Created on: Jun 14, 2014
 *      Author: Tuan Vu
 */

#ifndef WS_BASE_H_
#define WS_BASE_H_
#include <sstream>
#include <set>
#include <string>
#include <memory>
#include <vector>
#include <ctime>
#include <list>
#include <string.h>
#include <cctype>


#ifdef _WIN32
	#define _CRTDBG_MAP_ALLOC
	#include <cstdlib>
	#include <crtdbg.h>

	#ifdef _DEBUG
	#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	 // Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
	 // allocations to be of _CLIENT_BLOCK type
	#else
		#define DBG_NEW new
	#endif
#else
	#define DBG_NEW new
#endif

#ifndef NO_UNORDERED_MAP
#include <unordered_map>
using std::shared_ptr;
using std::unordered_map;
#else

#include "AssocVector.h"
#define unordered_map Loki::AssocVector

//#include <map>
//typedef std::map unordered_map
#include "shared_ptr.h"
#endif


#define G_MIA_TO_LOWER static_cast<int (*)(int)>(std::tolower)
#define G_MIA_TRANSFORM_TO_LOWER(x) std::transform(x.begin(), x.end(), x.begin(), G_MIA_TO_LOWER);

extern "C"
{
	#include <WS_Error.h>
}

#ifdef NO_EXCEPTIONS

#define TRY if(1)
#define CATCH(a, b)
#define THROW(...)
#define CATCH_CODE_EX(a, b)
#define NO_THROW(a) a

#else

#define TRY try
#define CATCH(a, b) catch(a) b
#define THROW(...) throw __VA_ARGS__
#define NO_THROW(a)

#endif

#ifdef _WIN32
#include "Mia_Base_WIN32.h"
#elif defined(__QNX__)
#include "Mia_Base_QNX.h"
#elif defined(__linux__)
#include "Mia_Base_LINUX.h"
#else
#include "Mia_Base_OS_TYPES.h"
#endif

#ifndef MIA_OUT_INFO
#define MIA_OUT_WARNING          std::cout
#define MIA_OUT_WARNING_STREAM   MIA_OUT_WARNING
#define MIA_OUT_DEBUG            MIA_OUT_WARNING
#define MIA_OUT_DEBUG_WEBSOCKET  MIA_OUT_WARNING
#define MIA_OUT_INFO             MIA_OUT_WARNING
#endif

namespace ABB
{
	namespace Mia
	{
		class C_NullBufferF;
		extern C_NullBufferF G_NullBuffer;
	}
}

#ifdef NDEBUG
#undef  MIA_OUT_DEBUG
// This could be done mauch better with templates outstream<DEBUG> << "hello\n"; but for supporting embedded targets CE try to avoid templates
// with this "if(1) {} else stream" We try to avoid evalution of stream. We save CPU and hopefully compiler can optimise else branch away
#define MIA_OUT_DEBUG if(1) {} else G_NullBuffer
#endif

#ifndef WEBSOCKET_DEBUG
#undef MIA_OUT_DEBUG_WEBSOCKET
#define MIA_OUT_DEBUG_WEBSOCKET if(1) {} else G_NullBuffer
#endif


#ifndef SYSTEM_THREAD_RET
	#define SYSTEM_THREAD_RET   void*
	#define SYSTEM_THREAD_PARAM void*
#endif


#ifndef UNUSED_VAR
	#define UNUSED_VAR
#endif

#define Mia_THIS_ROUTINE(a)  UNUSED_VAR static const char sThisRoutine[] = a;
#define Mia_THIS_LOCATION ABB::Mia::C_ErrorLocationData(sThisRoutine, __FILE__, __LINE__, __DATE__ " " __TIME__, __TIMESTAMP__)

#ifndef Mia_CLASS
#define Mia_CLASS(x) \
	class x##Class; \
	typedef C_SharedPointer<x##Class> x; \
	typedef std::vector<x> x##s; \
	/**  \
	 * @brief Class x##\
	 */ \

#endif

/**
 * \defgroup MiaDriverCore cpmPlus Embedded SDK - Core classes
 */

namespace ABB
{
	namespace Mia
	{
		void Mia_EXPORT G_PlatformInit();
		void Mia_EXPORT G_Platform_DeInit();

		class C_ErrorRecords;
		typedef std::list<C_ErrorRecords*> T_ErrorsList;
		/*------------------------------------------------------------------------------
		 ------------------------------------------------------------------------------*/
		class C_ThreadLocalData
		{
			public: T_ErrorsList m_errorsList;
		};

		C_ThreadLocalData *G_GetThreadLocalData();

		class C_ThreadLock;
		class C_Variant;
		class C_IFHeap;

		class C_NullBufferF : public std::ostream {
		private:
			class C_NullBufferData : public std::stringbuf {
			private:
			public:
				C_NullBufferData() { }
				virtual ~C_NullBufferData() {  pubsync(); }
				virtual int sync() { str(""); return 0; }
			};

			public:
			C_NullBufferF() : std::ostream(new C_NullBufferData()) {}
			virtual ~C_NullBufferF() { delete rdbuf(); }
		};

#ifndef UTF16
		typedef std::string tstring;
#else

#endif
		typedef long long t_CallId;
		typedef unsigned char byte;

		class C_Guid;

		/** o
		 * @brief Managed template pointer, a smart pointer that retains shared ownership of an object through a pointer.
		 * Several shared_ptr objects may own the same object. The object is destroyed and its memory deallocated when either of the following happens:<br>
		 * - the last remaining shared_ptr owning the object is destroyed.
		 * - the last remaining shared_ptr owning the object is assigned another pointer via operator= or reset(). <br>
		 * The object is destroyed using delete-expression or a custom deleter that is supplied to shared_ptr during construction.
		 * \ingroup MiaDriverCore
		 */
		template<class T>
		class Mia_EXPORT C_SharedPointer: public shared_ptr<T>
		{
			public: C_SharedPointer(): shared_ptr<T>(){}
			public: C_SharedPointer(T* pointer): shared_ptr<T>(pointer){}
		};

		/**
		 * @brief Collection of managed template pointer.
		 * @see C_SharedPointer
		 * \ingroup MiaDriverCore
		 * */
		template<class T>
		class Mia_EXPORT C_SharedPointerContainer: public std::list<C_SharedPointer<T> >
		{
			public:C_SharedPointerContainer() :	std::list<C_SharedPointer<T> >(){};
		};

		std::string G_StringToLower(const std::string &data);

		/**
		 * @brief Driver data types.
		 * \ingroup MiaDriverCore
		 * */
		enum Mia_EXPORT t_DataType
		{
			g_BOOLEAN = 9,
			g_CHAR = 8,
			g_BYTE = 10,
			g_INT16 = 7,
			g_UINT16 = 11,
			g_INT32 = 5,
			g_UINT32 = 12,
			g_INT64 = 1,
			g_UINT64 = 13,
			g_FLOAT = 4,
			g_DOUBLE = 0,
			g_MEMORY = 16,
			g_STRING = 3,
			g_GUID = 18,
			g_DATETIME = 17,
			g_CLASS = 19,
			g_OBJECT = 20,
			g_ARRAY = 21,
			g_UNKNOWN = 22,
			g_NULL = 23
		};

		enum Mia_EXPORT t_PropertyType
		{
			g_PT_BOOLEAN = 3,
			g_PT_BYTE = 5,
			g_PT_CHAR = 6,
			g_PT_INT16 = 7,
			g_PT_UINT16 = 8,
			g_PT_INT = 9,
			g_PT_UINT = 10,
			g_PT_INT64 = 11,
			g_PT_UINT64 = 12,
			g_PT_FLOAT = 13,
			g_PT_DOUBLE = 14,
			g_PT_STRING = 18,
			g_PT_TIMESTAMP = 16,

			g_PT_ARRAYBYTE = 205,
			g_PT_ARRAYCHAR = 206,
			g_PT_ARRAYINT16 = 207,
			g_PT_ARRAYUINT16 = 208,
			g_PT_ARRAYINT = 209,
			g_PT_ARRAYUINT = 210,
			g_PT_ARRAYINT64 = 211,
			g_PT_ARRAYUINT64 = 212,
			g_PT_ARRAYFLOAT = 213,
			g_PT_ARRAYDOUBLE = 214,
			g_PT_ARRAYTIMESTAMP = 216,
			g_PT_ARRAYSTRING = 218,

			g_PT_GUID = 241,
			g_PT_ARRAYGUID = 242,
			g_PT_OBJECT,
			g_PT_CLASS,
			g_PT_UNKNOWN = 0
		};

		enum Mia_EXPORT t_TypeCode
		{
			g_TC_End = 0x00,
			g_TC_Null = 0x01,
			g_TC_ObjectArray = 0x02,
			g_TC_Object = 0x03,
			g_TC_ObjectOfPreviousType = 0x04,
			g_TC_ZeroGUID = 0x05,
			g_TC_GUID = 0x06,
			g_TC_BoolFalse = 0x07,
			g_TC_BoolTrue = 0x08,
			g_TC_ByteZero = 0x09,
			g_TC_Byte = 0x0A,
			g_TC_Char16Zero = 0x0B,
			g_TC_Char16SingleByte = 0x0C,
			g_TC_Char16 = 0x0D,
			g_TC_Int8Zero = 0x0E,
			g_TC_Int8 = 0x0F,
			g_TC_UInt16Zero = 0x10,
			g_TC_UInt16SingleByte = 0x11,
			g_TC_UInt16 = 0x12,
			g_TC_Int16Zero = 0x13,
			g_TC_Int16SingleByte = 0x14,
			g_TC_Int16 = 0x15,
			g_TC_UInt32Zero = 0x16,
			g_TC_UInt32SingleByte = 0x17,
			g_TC_UInt32TwoByte = 0x18,
			g_TC_UInt32ThreeByte = 0x19,
			g_TC_UInt32 = 0x1A,
			g_TC_Int32Zero = 0x1B,
			g_TC_Int32SingleByte = 0x1C,
			g_TC_Int32TwoByte = 0x1D,
			g_TC_Int32ThreeByte = 0x1E,
			g_TC_Int32 = 0x1F,
			g_TC_UInt64Zero = 0x20,
			g_TC_UInt64SingleByte = 0x21,
			g_TC_UInt64TwoByte = 0x22,
			g_TC_UInt64ThreeByte = 0x23,
			g_TC_UInt64FourByte = 0x24,
			g_TC_UInt64FiveByte = 0x25,
			g_TC_UInt64SixByte = 0x26,
			g_TC_UInt64SevenByte = 0x27,
			g_TC_UInt64 = 0x28,
			g_TC_Int64Zero = 0x29,
			g_TC_Int64SingleByte = 0x2A,
			g_TC_Int64TwoByte = 0x2B,
			g_TC_Int64ThreeByte = 0x2C,
			g_TC_Int64FourByte = 0x2D,
			g_TC_Int64FiveByte = 0x2E,
			g_TC_Int64SixByte = 0x2F,
			g_TC_Int64SevenByte = 0x30,
			g_TC_Int64 = 0x31,
			g_TC_FloatZero = 0x32,
			g_TC_FloatSingleByte = 0x33,
			g_TC_FloatTwoByte = 0x34,
			g_TC_FloatThreeByte = 0x35,
			g_TC_Float = 0x36,
			g_TC_DoubleZero = 0x37,
			g_TC_DoubleTwoByte = 0x38,//Single byte double omitted on purpose
			g_TC_DoubleThreeByte = 0x39,
			g_TC_DoubleFourByte = 0x3A,
			g_TC_DoubleFiveByte = 0x3B,
			g_TC_DoubleSixByte = 0x3C,
			g_TC_DoubleSevenByte = 0x3D,
			g_TC_Double = 0x3E,
			g_TC_DateTimeZero = 0x3F,
			g_TC_DateTimeMinutes = 0x40,
			g_TC_DateTimeSecs2015 = 0x41, // DateTime signed seconds since 2015-01-01
			g_TC_DateTime = 0x42,
			g_TC_DateTimeLocal = 0x43,
			g_TC_TimeSpanZero = 0x44,
			g_TC_TimeSpanSingleByte = 0x45,
			g_TC_TimeSpanTwoByte = 0x46,
			g_TC_TimeSpanThreeByte = 0x47,
			g_TC_TimeSpanFourByte = 0x48,
			g_TC_TimeSpanFiveByte = 0x49,
			g_TC_TimeSpanSixByte = 0x4A,
			g_TC_TimeSpanSevenByte = 0x4B,
			g_TC_TimeSpan = 0x4C,
			g_TC_String7BitEmpty = 0x4D,
			g_TC_String7Bit1Char = 0x4E,
			g_TC_String7Bit2Char = 0x4F,
			g_TC_String7Bit3Char = 0x50,
			g_TC_String7Bit4Char = 0x51,
			g_TC_String7Bit5Char = 0x52,
			g_TC_String7Bit6Char = 0x53,
			g_TC_String7Bit7Char = 0x54,
			g_TC_String7Bit8Char = 0x55,
			g_TC_String7Bit9Char = 0x56,
			g_TC_String7Bit10Char = 0x57,
			g_TC_String7Bit11Char = 0x58,
			g_TC_String7Bit12Char = 0x59,
			g_TC_String7Bit13Char = 0x5A,
			g_TC_String7Bit14Char = 0x5B,
			g_TC_String7Bit15Char = 0x5C,
			g_TC_String7Bit16Char = 0x5D,
			g_TC_String7Bit17Char = 0x5E,
			g_TC_String7Bit18Char = 0x5F,
			g_TC_String7Bit19Char = 0x60,
			g_TC_String7Bit20Char = 0x61,
			g_TC_String7Bit21Char = 0x62,
			g_TC_String7Bit22Char = 0x63,
			g_TC_String7Bit23Char = 0x64,
			g_TC_String7Bit24Char = 0x65,
			g_TC_String7Bit = 0x66,//7bit encoded length, then 7bit encoded chars
			g_TC_DecimalZero = 0x67,
			g_TC_DecimalSingleByte = 0x68,
			g_TC_DecimalTwoByte = 0x69,
			g_TC_DecimalThreeByte = 0x6A,
			g_TC_DecimalFourByte = 0x6B,
			g_TC_DecimalFiveByte = 0x6C,
			g_TC_DecimalSixByte = 0x6D,
			g_TC_DecimalSevenByte = 0x6E,
			g_TC_DecimalEightByte = 0x6F,
			g_TC_DecimalNineByte = 0x70,
			g_TC_DecimalTenByte = 0x71,
			g_TC_Decimal11Byte = 0x72,
			g_TC_Decimal12Byte = 0x73,
			g_TC_Decimal13Byte = 0x74,
			g_TC_Decimal14Byte = 0x75,
			g_TC_AdvancedTimeSpan = 0x76,
			g_TC_LocalTimeSpan = 0x77,
			g_TC_BigInteger = 0x78,//7bit encoded length, then bytes
			g_TC_Empty = 0x7D, // Empty Value signature (no actual data follows)
			g_TC_Default = 0x7E, // Default Value signature (no actual data follows)
			g_TC_CustomStream = 0x7F, // Rest of the message is a byte stream provided by a custom serialization code, will also cause closing of all open objects / arrays
			g_TC_ArrayFlag = 0x80 // Array of listed types, lower bits containing type code. First a 7bit encoded number containing the number of elements, then differential encoded values without typecodes.
		};

		/**
		* @brief Output format for functions.
		* \ingroup MiaDriverCore
		* */
		enum Mia_EXPORT t_OutputType
		{
			g_RAW = 1,
			g_JSON = 2,
			g_LOGGER = 3
		};

		/**
		 * @brief Autorelease pointer. The first created object will own the pointer.
		 * @see C_SharedPointer
		 * \ingroup MiaDriverCore
		 * */
		template<class t_Type>
		class Mia_EXPORT C_AutoPointer
		{
			public:	t_Type *m_Pointer;
			public:	bool m_AutoRelease;
			public:	C_AutoPointer(t_Type *ptr = NULL, bool autoRelease = true);
			public:	C_AutoPointer(const C_AutoPointer<t_Type> &p);

			public:	~C_AutoPointer();

			public:	void operator=(C_AutoPointer &p);

			public:	t_Type *operator=(t_Type *ptr);

			public:	t_Type & operator*() const;

			public:	t_Type *operator->() const;

			public:	t_Type *M_Pointer() const;

			public:	operator t_Type *() const;

			public:	void M_Free();
		};

		/**
		 * @brief Class C_Guid provides GUID handling functions.
		 * \ingroup MiaDriverCore
		 */
		class Mia_EXPORT C_Guid
		{
			/**
			 * @brief Create an empty C_Guid object
			 */
			public:	C_Guid();

			/**
			 * @brief Create random C_Guid
			 * @return Random C_Guid
			 */
			public:	static C_Guid M_S_GetRandomGuid();

			/**
			 * @brief Create a C_Guid object with data from the given data string
			 * Example: "21EC2020-3AEA-4069-A2DD-08002B30309D"
			 */
			public:	C_Guid(const std::string &data);

			/**
			 * @brief Copy constructor
			 */
			public:	C_Guid(const C_Guid &guid);

			/**
			 * @brief Access the specific data byte at offset
			 * @return The value of the byte at the index
			 */
			public:	byte &operator[](const int &index);

			/**
			 * @brief Access the specific data byte at offset
			 * @return The value of the byte at the index
			 */
			public:	const byte &operator[](const int &index) const;

			/**
			 * @brief Copy the C_Guid value
			 */
			public:	void operator=(const C_Guid& value);

			/**
			 * @brief Define the C_Guid data with the string data
			 */
			public:	void M_SetData(const std::string &data);

			/**
			 * @brief Define the C_Guid data with the raw data
			 */
			public:	void M_SetRawData(const char *data);

			/**
			 * @brief Compare between two C_Guid object.
			 * @return true If two guid is the same.
			 */
			public:	bool operator==(const C_Guid& value) const;

			/**
			 * @brief Convert the C_Guid into string format
			 * @return The string form of the C_Guid object. Example: "21EC2020-3AEA-4069-A2DD-08002B30309D"
			 */
			public:	std::string M_ToString() const;

			/**
			 * @brief Get the value of the first four byte
			 */
			public:	uint32 M_GetData1() const;

			/**
			 * @brief Set the value of the first four byte, byte 0-3
			 */
			public:	void M_SetData1(const uint32 &data);

			/**
			 * @brief Get the value of the next two byte, byte 4-5
			 */
			public:	uint16 M_GetData2() const;

			/**
			 * @brief Set the value of the next two byte, byte 4-5
			 */
			public:	void M_SetData2(const int16 &data);

			/**
			 * @brief Get the value of the next two byte, byte 6-7
			 */
			public:	uint16 M_GetData3() const;

			/**
			 * @brief Set the value of the next two byte, byte 6-7
			 */
			public:	void M_SetData3(const int16 &data);

			/**
			 * @brief Check if the C_Guid is null
			 */
			public:	bool M_IsNull();

			/**
			 * @brief Get the value of the last 8 byte, byte 8-15
			 */
			public:	uint64 M_GetData4() const;

			/**
			 * @brief Set the value of the last 8 byte, byte 8-15
			 */
			public:	void M_SetData4(const byte data[8]);

			/**
			 * @brief Set the value of the last 8 byte, byte 8-15
			 */
			public:	void M_SetData4(const uint64 &data);

			/**
			 * @brief Print the guid content to the stream.
			 */
			friend std::ostream& operator<<(std::ostream& os, const C_Guid &guid);

			private:	static char M_S_HexToChar(char c);

			private:	static char M_S_CharToHex(char c);

			private:	union t_Data
			{
				byte m_Data[16];
				struct t_Subdata
				{
					uint32 m_Data1;
					uint16 m_Data2;
					uint16 m_Data3;
					uint64 m_Data4;
				} m_SubData;
			} m_Data;
		};

		class Mia_EXPORT C_MemoryBuffer
		{
			static int m_s_InstanceCount;
			public:	C_MemoryBuffer(unsigned int size = 0);
			public:	C_MemoryBuffer(const char* buffer, int size);
			public:	C_MemoryBuffer(const C_MemoryBuffer &buffer);
			public:	void operator=(const C_MemoryBuffer &buffer);
			public:	~C_MemoryBuffer();
			public:	char &operator[](int index);
			public:	const char &operator[](int index) const;
			public:	char &operator[](uint index);
			public:	const char &operator[](uint index) const;
			public:	C_MemoryBuffer *M_CreateShadowCopy();

			public:	const int &M_GetSize() const;
			public:	void M_Resize(int newSize);
			public:	void M_Clear();
			public:	char* M_Data() const;
			public:	void M_SetData(char* data, bool autoRelease);

			private:	int m_Size;
			private:	char* m_Buffer;
			private: bool m_AutoRelease;
		};

		/*typedef struct _Buffer
		{
			char		*m_Buffer;
			int		m_Length;
		} t_Buffer;*/

		class Mia_EXPORT C_TimeSpan
		{
			public: C_TimeSpan(int timespan = 0) : m_timespan(timespan) {}
			public: int64 M_GetValue() const { return m_timespan; }
			public:	friend std::ostream& operator<<(std::ostream &out, const C_TimeSpan& time);

			private: int64 m_timespan;
		};

		/**
		 * @brief Class C_DateTime provides date and time function to handle date/time data.
		 * Internally, time is segmented into seconds since EPOC and milliseconds of the seconds.
		 * \ingroup MiaDriverCore
		 */
		class Mia_EXPORT C_DateTime
		{
			static const uint32 m_k_s_100nsPERSECOND = 10000000ULL;
			static const uint32 m_k_s_100nsPERMILISECOND = 10000UL;

			/**
			 * @brief Create a C_DateTime object with segmented seconds since EPOC and 100nanoseconds of a second.
			 * @param timeS Seconds since EPOC
			 * @param time100Ns 100 nanoseconds of a second
			 */
			public:	C_DateTime(uint64 timeS = 0, uint32 time100Ns = 0);

			/**
			 * @brief Create a C_DateTime object with given year, month, data, hour, minute, second and hundred nanoseconds.
			 * @param year Number of Year, 1900~.
			 * @param month Number of Month, 1~12.
			 * @param day Number of Day, 1~31.
			 * @param hour Number of Hour, 0~23.
			 * @param minute Number of Minute, 0~59.
			 * @param second Number of Second, 0~59.
			 * @param n100nanoseconds Number of hundred nanoseconds. 0~10^7-1
			 */
			public:	C_DateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0, int n100nanoseconds = 0);

			public:	C_DateTime(const std::string &data, const std::string &format);

			/**
			 * @brief Create a C_DateTime object represent current time
			 */
			public:	static const C_DateTime M_S_Now();

		   /**
			 * @brief Create a C_DateTime object represent the tick time
			 */
			public: static C_DateTime M_S_FromTicks(const int64 tickCount);

			/**
			 * @brief Return a C_DateTime object which differs than this C_DateTime object
			 * by a number of given ticks. \n
			 * Notice that this functions does not change the time stored
			 * in this object.
			 *
			 * @param n100ns The number of 100-nanoseconds to add to the current time
			 */
			public:	C_DateTime M_Add100Nanosecond(const int64 &n100ns) const;

			/**
			 * @brief Return a C_DateTime object which differs than this C_DateTime object
			 * by a number of given seconds. <br>\n
			 * Notice that this functions does not change the time stored
			 * in this object.
			 *
			 * @param seconds The number of seconds to add to the current time
			 */
			public:	C_DateTime M_AddSecond(const int64 &seconds) const;

			/**
			 * @brief Return a C_DateTime object which differs than this C_DateTime object
			 * by a number of given mili-seconds
			 *
			 * @param miliseconds The number of mili-seconds to add to the current time
			 */
			public:	C_DateTime M_AddMilisecond(const int64 &miliseconds) const;

			/**
			 * @brief Return the total number of ticks
			 *
			 */
			public:	C_DateTime& operator=(const C_DateTime& datetime);

			/**
			 * @brief Return the current day of the year. Range from 1 .. 366
			 *
			 * @return Current day as a number in the range of 1 .. 366, which January 1st = 1
			 */
			public:	uint16 M_GetDayOfYear() const;

			/**
			 * @brief Return the current day of the week. Range from 0 .. 6. <br>
			 * Sunday = 0.
			 *
			 * @return Current day as a number in the range of 0 .. 6, which Sunday = 0
			 */
			public:	uint16 M_GetDayOfWeek() const;

			/**
			 * @brief Return the current year from 1900
			 *
			 * @return Current year as a number from 1900
			 */
			public:	uint16 M_GetYear() const;

			/**
			 * @brief Return the current month of the year. Range from 1 .. 12
			 *
			 * @return Current month as a number in the range of 1 .. 12
			 */
			public:	uint16 M_GetMonth() const;

			/**
			 * @brief Return the current day of the month. Range from 1 .. 31
			 *
			 * @return Current day as a number in the range of 1 .. 31
			 */
			public:	uint16 M_GetDay() const;

			/**
			 * @brief Return the current hour of the day. Range from 0 .. 23
			 *
			 * @return Current hour as a number in the range of 0 .. 23
			 */
			public:	uint16 M_GetHour() const;

			/**
			 * @brief Return the current minute of the hour. Range from 0 .. 59
			 *
			 * @return Current minute as a number in the range of 0 .. 59
			 */
			public:	uint16 M_GetMinute() const;

			/**
			 * @brief Return the second of the minute, range from 0 .. 59
			 *
			 * @return Current second as a number from 0 to 59
			 */
			public:	uint16 M_GetSecond() const;

			/**
			 * @brief Return the number of seconds segment
			 *
			 */
			public:	int64 M_GetTotalSeconds() const;

			/**
			 * @brief Return the number of ticks segment
			 *
			 */
			public:	uint32 M_Get100Nanoseconds() const;

			/**
			 * @brief Return the total number of nanoseconds
			 *
			 */
			public:	int64 M_GetTotalNanoseconds() const;

			/**
			 * @brief Return the total number of ticks
			 *
			 */
			public:	int64 M_GetTicks() const;

			/**
			 * @brief Compare the time with comparand time, return true if current time
			 * object is less than comparand.
			 *
			 */
			public:	bool operator<(const C_DateTime &comparand) const;

			/**
			 * @brief Compare the time with comparand time, return true if current time
			 * object is less than of equal with comparand.
			 *
			 */
			public:	bool operator<=(const C_DateTime &comparand) const;

			/**
			 * @brief Compare the time with comparand time, return true if current time
			 * object is greater than comparand.
			 *
			 */
			public:	bool operator>(const C_DateTime &comparand) const;

			/**
			 * @brief Compare the time with comparand time, return true if current time
			 * object is greater than or equal with comparand.
			 *
			 */
			public:	bool operator>=(const C_DateTime &comparand) const;

			/**
			 * @brief Compare the time with comparand time, return true if current time
			 * object is equal with comparand.
			 *
			 */
			public:	bool operator==(const C_DateTime &comparand) const;

			/**
			 * @brief Compare the time with comparand time, return true if current time
			 * object is equal with comparand.
			 *
			 */
			public:	bool operator!=(const C_DateTime &comparand) const;

			/**
			 * @brief Compare the time with comparand time, return true if current time
			 * object is equal with comparand.
			 *
			 */
			public:	std::string M_ToString() const;

			public:	friend std::ostream& operator<<(std::ostream &out, const C_DateTime& time);
			public:	friend std::istream& operator>>(std::ostream &in, C_DateTime& time);

			public:	struct tm* M_GetTimeStruct() const;

			/**
			 * @brief Second since EPOC  Jan 1, 1970 UTC 00:00 +0000
			 * BIG_ENDIAN vs LITTLE ENDIAN in negative time
			 */

			private:	union
			{
				int64 m_FullTime;
			} m_TimeData;
		};

		/**
		 * @brief Class C_Event provides event synchronization mechanism between threads of a process.
		 * \ingroup MiaDriverCore
		 *
		 */
		class Mia_EXPORT C_Event
		{
			/**
			 * @brief Create a event object
			 * @param initialState The intial state of the event object
			 */
			public:	C_Event(bool initialState = false);

			/**
			 * @brief release state object
			 */
			public:	~C_Event();

			/**
			 * @brief Put the state of the object to set
			 */
			public:	void M_Set();

			/**
			 * @brief Put the state of the object to unset
			 */
			public:	void M_Unset();
			/**
			 * @brief Wait until the object change state from unset to set
			 */
			public:	bool M_Wait(int timeout = -1);

			private:	t_EventHandle m_Event;
		};

		/**
		 * @brief This class handle data types serialization to text to be sent with C_Websocket
		 * Internally, the class maintain an instance of std::stringstream and return the string
		 * result with method M_GetString()
		 * \ingroup MiaDriverCore
		 */
		class C_WebsocketStream
		{
			public:	C_WebsocketStream() {}

			public:	C_WebsocketStream(const std::string &data);

			/**
			 * @brief Return the internal constructed string
			 *
			 * @return The result string
			 */
			public:	std::string M_GetString();

			/**
			 * @brief Start an object serialization. Appending '{' for text stream
			 */
			public:	void M_BeginObject();
			/**
			 * @brief Start an array serialization. Appending '[' for text stream
			 */

			public:	void M_BeginArray();
			/**
			 * @brief End an object serialization. Appending '}' for text stream
			 */



			public:	void M_EndObject();
			/**
			 * @brief End an array serialization. Appending ']' for text stream
			 */

			public:	void M_EndArray();

			/**
			 * @brief Append null to the stream
			 */
			public:	void M_AppendNull();
			/**
			 * @brief Append string to the stream
			 * @param data String to append
			 */
			public:	void M_AppendString(const std::string &data);

			/**
			 * @brief Append the string data to the stream without conversion
			 * @param data String to append
			 */
			public:	void M_AppendRaw(const std::string &data);

			/**
			 * @brief Append the directive and ':' to the stream
			 * @param directive The directive to be appended
			 */
			public:	void M_AppendDirective(const std::string directive);

			/**
			 * @brief Append ',' to the stream
			 */
			public:	void M_NextDirective();

			/**
			 * @brief Check if there is more characters in the stream
			 * @return true if there is more data in the stream
			 */
			public:	bool M_HasMore();

			/**
			 * @brief Write value into the stream
			 * @param value Value to write to
			 */
			void operator<<(const int &value);
			/**
			 * @brief Write value into the stream
			 * @param value Value to write to
			 */
			void operator<<(const int64 &value);

			/**
			 * @brief Write value into the stream
			 * @param value Value to write to
			 */
			void operator<<(const uint64 &value);
			/**
			 * @brief Write value into the stream
			 * @param value Value to write to
			 */
			void operator<<(const double &value);
			/**
			 * @brief Write value into the stream
			 * @param value Value to write to
			 */
			void operator<<(const std::string &value);

			/**
			 * @brief Write date time into the stream
			 */
			void operator<<(const C_DateTime& value);

			/**
			 * @brief Write value into the stream
			 * @param value Value to write to
			 */
			void operator<<(const char *value);
			/**
			 * @brief Write value into the stream
			 * @param value Value to write to
			 */
			void operator<<(const C_Variant &value);

			/**
			 * @brief Read value from the stream
			 * @param value Value to read to
			 */
			void operator>>(int &value);

			/**
			 * @brief Read value from the stream
			 * @param value Value to read to
			 */
			void operator>>(int64 &value);

			/**
			 * @brief Read value from the stream
			 * @param value Value to read to
			 */
			void operator>>(double &value);

			/**
			 * @brief Read value from the stream
			 * @param value Value to read to
			 */
			void operator>>(std::string &value);

			/**
			 * @brief Read value from the stream
			 * @param value Value to read to
			 */
			void operator>>(bool &value);

			private:	std::stringstream m_Stream;
		};

		/**
		* @brief This class handle data types serialization to binary to be sent with C_Websocket
		* Internally, the class maintain an instance of char[] buffer with the help of iterator char* and buffer size indicator and return the buffer and the buffer size
		* result with method M_GetBuffer() and M_GetBufferSize()
		* \ingroup MiaDriverCore
		*/
		class Mia_EXPORT C_WebsocketStreamBinary
		{
			public:	C_WebsocketStreamBinary();

			/**
			* @brief Return the internal constructed binary buffer
			*
			* @return The result char* buffer
			*/
			public: char* M_GetBuffer();
			/**
			* @brief Return the internal constructed binary buffer size
			*
			* @return The result char* buffer size
			*/
			public: int M_GetSize();

			/**
			* @brief Start an object serialization. Appending g_TC_Object for text stream
			*/
			public:	void M_BeginObject();
			/**
			* @brief Start an object serialization. Appending g_TC_ObjectArray for text stream
			*/
			public:	void M_BeginArray();
			/**
			* @brief End an object serialization. Appending g_TC_End for text stream
			*/
			public:	void M_EndObject();
			/**
			* @brief End an array serialization. Appending g_TC_End for text stream
			*/
			public:	void M_EndArray();

			/**
			* @brief Start an array of type type
			*/
			public:	void M_BeginArray(int type);

			/**
			* @brief Write raw data to stream
			*/
			public:	void M_Write(char *data, const int &length);

			/**
			* @brief Write C_Variant to stream
			*/
			public:	void M_Write(const C_Variant &data);

			/**
			* @brief Append g_TC_Null to the stream
			*/
			public:	void M_AppendNull();

			/**
			* @brief Reserve space
			*/
			public:	void M_Reserve(size_t size);

			/**
			* @brief Write value into the stream
			* @param value Value to write to
			*/
			public: void operator<<(const bool &value);
			public: void operator<<(const byte &value);
			public: void operator<<(const char &value);
			public: void operator<<(const int16 &value);
			public: void operator<<(const int32 &value);
			public: void operator<<(const int64 &value);
			public: void operator<<(const uint16 &value);
			public: void operator<<(const uint32 &value);
			public: void operator<<(const uint64 &value);
			public: void operator<<(const double &value);
			public: void operator<<(const float &value);
			public: void operator<<(const std::string &value);
			public: void operator<<(const C_DateTime& value);
			public: void operator<<(const char *value);
			public: void operator<<(const C_Variant &value);

			private:	std::vector<char> m_Buffer;
		};

		/**
		 * @brief Wrapper class for most of the datatypes available by this driver
		 * \ingroup MiaDriverCore
		 */
		class Mia_EXPORT C_Variant
		{
			/**
			 * @brief Create a null variant object
			 */
			public:	C_Variant();

			public:	C_Variant(const C_Variant& otherVariant);
			/**
			 * @brief Create a variant object of character
			 */
			public:	C_Variant(const bool &value);

			/**
			 * @brief Create a variant object of character
			 */
			public:	C_Variant(const char &value);

			/**
			 * @brief Create a variant object of type byte
			 */
			public:	C_Variant(const byte &value);

			/**
			 * @brief Create a variant object of type short
			 */
			public:	C_Variant(const int16 &value);

			/**
			 * @brief Create a variant object of type unsigned short
			 */
			public:	C_Variant(const uint16 &value);

			/**
			 * @brief Create a variant object of type integer
			 */
			public:	C_Variant(const int32 &value);

			/**
			 * @brief Create a variant object of type unsigned integer
			 */
			public:	C_Variant(const uint32 &value);

			/**
			 * @brief Create a variant object of type int64
			 */
			public:	C_Variant(const int64 &value);

			/**
			 * @brief Create a variant object of type uint64
			 */
			public:	C_Variant(const uint64 &value);

			/**
			 * @brief Create a variant object of type float
			 */
			public:	C_Variant(const float &value);

			/**
			 * @brief Create a variant object of type double
			 */
			public:	C_Variant(const double &value);

			/**
			 * @brief Create a variant object of type C_Guid
			 */
			public:	C_Variant(const C_Guid &value);

			/**
			 * @brief Create a variant object of type string
			 */
			public:	C_Variant(const std::string &value);

			/**
			 * @brief Create a variant object of type string
			 */
			public:	C_Variant(const char *&value);

			/**
			* @brief Create a variant object of type memory
			*/
			public:	C_Variant(const C_MemoryBuffer& buffer);
			/**
			 * @brief Create a variant object of type data/string data
			 * @param value The data buffer.
			 * @param length The length of the buffer. If buffer is not given
			 * Constructor assume that an array of character was given and handle
			 * it similar to string.
			 */
			public:	C_Variant(const char* value, int length = 0);

			/**
			 * @brief Create a variant object of type datetime
			 */
			public:	C_Variant(const C_DateTime& value);

			/**
			 * @brief Create a variant object with given data type and parse the data from the given string
			 * DateTime must be given in the following format: "6 Dec 2001 12:33:45"
			 * If the variant cannot be formatted from the string, the variant is set to unknown and
			 * method M_IsValid return false
			 */
			public:	C_Variant(const std::string &data, t_DataType type);

			/**
			 * @brief Create a variant object with given data type and parse the data from the given string
			 * DateTime must be given in the following format: "6 Dec 2001 12:33:45"
			 * If the variant cannot be formatted from the string, the variant is set to unknown and
			 * method M_IsValid return false
			 */
			public:	C_Variant(const std::string &data, t_PropertyType type);

			/**
			 * @brief Create a variant object with given data type.
			 * @param size Size of Array or Object
			 */
			public:	C_Variant(t_DataType type, int size = 0);

			public:	~C_Variant();

			/**
			 * @brief Set the variant object with given data type and parse the data from the given string
			 * DateTime must be given in the following format: "6 Dec 2001 12:33:45"
			 * @return false If the variant cannot be formatted from the string
			 */
			public:	bool M_FromString(const std::string &data, t_DataType type);

			/**
			 * @brief Get the coresponding variant type fromt property type
			 */
			public: static	t_DataType  M_S_ConvertPropertyTypeToVariantType(const t_PropertyType &type);

			/**
			* @brief Get the coresponding variant type fromt property type
			*/
			public: static char  M_S_ConvertVariantTypeToTypeCode(const t_DataType& type);

			/**
			* @brief Get estimated binary size
			* @return -1 if the size cannot be estimated
			*/
			public: size_t M_GetEstimatedSize() const;

			/**
			 * @brief Set the variant object with given data type and parse the data from the given JSON string
			 */
			public:	bool M_FromJSON(const std::string &json);

			/**
			* @brief Convert to binary buffer
			*/
			public: void M_ToBinary(std::vector<char> &dataBuffer) const;

			/**
			* @brief Get an unknown variant
			*/
			public: static  C_Variant& M_S_Null();

			/**
			 * @brief Set the variant object with given data type and parse the data from the given JSON string
			 */
			public: static  C_Variant M_S_FromJSON(const std::string &jsonString);

			/**
			 * @brief Set the variant object with given data type and parse the data from the given JSON string
			 */
			public: static bool M_S_FromJSON(const std::string &jsonString, C_Variant& ouputJson);

			/**
			 * @brief Get size of the Array
			 * @return -1 if the variant is not an array type
			 */
			public: unsigned int M_GetArraySize() const;

			/**
			 * @brief Get object properties
			 */
			public: std::list<std::string> M_GetProperties() const;

			/**
			 * @brief Get the type of the variant object
			 */
			public:	const t_DataType &M_GetType() const;

			/**
			 * @brief Get the typename of the variant object
			 */
			public:	std::string M_GetTypeName();

			/**
			 * @brief Return true if the variant object is valid
			 *
			 * @return false if the object is invalid, the reason may be it was initialized with null or
			 * it was unable to parse from the string or it was released
			 */
			public:	bool M_IsValid() const;

			/**
			 * @brief Set variant value to boolean
			 */
			public:	void M_SetValue(const bool &value);
			/**
			 * @brief Set variant value to char
			 */
			public:	void M_SetValue(const char &value);

			/**
			 * @brief Set variant value to unsigned char
			 */
			public:	void M_SetValue(const byte &value);

			/**
			 * @brief Set variant value to short
			 */
			public:	void M_SetValue(const int16 &value);

			/**
			 * @brief Set variant value to unsigned short
			 */
			public:	void M_SetValue(const uint16 &value);

			/**
			 * @brief Set variant value to int
			 */
			public:	void M_SetValue(const int32 &value);

			/**
			 * @brief Set variant value to unsigned int
			 */
			public:	void M_SetValue(const uint32 &value);

			/**
			 * @brief Set variant value to int64
			 */
			public:	void M_SetValue(const int64 &value);

			/**
			 * @brief Set variant value to uint64
			 */
			public:	void M_SetValue(const uint64 &value);

			/**
			 * @brief Set variant value to float
			 */
			public:	void M_SetValue(const float &value);

			/**
			 * @brief Set variant value to double
			 */
			public:	void M_SetValue(const double &value);

			/**
			 * @brief Set variant value to C_Guid
			 */
			public:	void M_SetValue(const C_Guid &value);

			/**
			 * @brief Set variant value to string
			 */
			public:	void M_SetValue(const std::string &value);

			/**
			 * @brief Set variant value to string
			 */
			public:	void M_SetValue(const char* &value);

			/**
			 * @brief Set variant value to DateTime
			 */
			public:	void M_SetValue(const C_DateTime& value);

			/**
			 * @brief Set variant value
			 */
			public:	void M_SetValue(const C_Variant& value);

			/**
			 * @brief Set value of variant type array
			 */
			public:	void M_SetValue(int index, const C_Variant& value);

			/**
			 * @brief Set property value of variant object value
			 */
			public:	void M_SetValue(std::string propertyName, const C_Variant& value);

			/**
			 * @brief Release the memory allocated for the variant
			 */
			public:	void M_Release();

			/**
			 * @brief Set variant value to char
			 */
			public:	C_Variant& operator=(const char &value);

			/**
			 * @brief Set variant value to char
			 */
			public:	C_Variant& operator=(const bool &value);

			/**
			 * @brief Set variant value to byte
			 */
			public:	C_Variant& operator=(const byte &value);

			/**
			 * @brief Set variant value to int16
			 */
			public:	C_Variant& operator=(const int16 &value);

			/**
			 * @brief Set variant value to uint16
			 */
			public:	C_Variant& operator=(const uint16 &value);

			/**
			 * @brief Set variant value to int32
			 */
			public:	C_Variant& operator=(const int32 &value);

			/**
			 * @brief Set variant value to uint32
			 */
			public:	C_Variant& operator=(const uint32 &value);

			/**
			 * @brief Set variant value to int64
			 */
			public:	C_Variant& operator=(const int64 &value);

			/**
			 * @brief Set variant value to uint64
			 */
			public:	C_Variant& operator=(const uint64 &value);

			/**
			 * @brief Set variant value to float
			 */
			public:	C_Variant& operator=(const float &value);

			/**
			 * @brief Set variant value to double
			 */
			public:	C_Variant& operator=(const double &value);

			/**
			 * @brief Set variant value to C_Guid
			 */
			public:	C_Variant& operator=(const C_Guid &value);

			/**
			 * @brief Set variant value to string
			 */
			public:	C_Variant& operator=(const std::string &value);

			/**
			 * @brief Set variant value to C_DateTime
			 */
			public:	C_Variant& operator=(const C_DateTime& value);

			/**
			 * @brief Set variant value to C_Variant value
			 */
			public:	C_Variant& operator=(const C_Variant& value);

			/**
			* @brief Set variant value to C_Variant value
			*/
			public:	C_Variant& operator=(const char* value);

			/**
			 * @brief Compare variant value to C_Variant value
			 */
			public:	bool operator==(const C_Variant& value);

			/**
			 * @brief Compare variant value to C_Variant value
			 */
			public:	bool operator!=(const C_Variant& value);

			/**
			 * @brief Get variant value of int64
			 */
			public:	bool M_ToBool() const;

			/**
			 * @brief Get variant value of int64
			 */
			public:	int64 M_ToInt64() const;

			/**
			 * @brief Get variant value of uint64
			 */
			public:	uint64 M_ToUInt64() const;

			/**
			 * @brief Get variant value of int32
			 */
			public:	int M_ToInt32() const;
			/**
			 * @brief Get variant value of uint32
			 */
			public:	uint M_ToUInt32() const;
			/**
			 * @brief Get variant value of int16
			 */
			public:	int16 M_ToInt16() const;
			/**
			 * @brief Get variant value of uint16
			 */
			public:	uint16 M_ToUInt16() const;
			/**
			 * @brief Get variant value of char
			 */
			public:	char M_ToChar() const;
			/**
			 * @brief Get variant value of byte
			 */
			public:	unsigned char M_ToByte() const;

			/**
			 * @brief Get variant value of float
			 */
			public:	float M_ToFloat() const;

			/**
			 * @brief Get variant value of double
			 */
			public:	double M_ToDouble() const;

			/**
			 * @brief Get variant value of C_DateTime
			 */
			public:	C_DateTime &M_ToDate() const;

			/**
			 * @brief Get variant value of C_DateTime
			 */
			public:	const C_Guid &M_ToGuid() const;
			/**
			 * @brief Get variant value of string
			 */
			public:	std::string M_ToString() const;

			/**
			 * @brief Get variant value of int64
			 */
			public:	void M_GetValue(bool *value) const;

			/**
			 * @brief Get variant value of int64
			 */
			public:	void M_GetValue(int64 *value) const;
			/**
			 * @brief Get variant value of uint64
			 */
			public:	void M_GetValue(uint64 *value) const;

			/**
			 * @brief Get variant value of int32
			 */
			public:	void M_GetValue(int* value) const;

			/**
			 * @brief Get variant value of uint32
			 */
			public:	void M_GetValue(unsigned int *value) const;

			/**
			 * @brief Get variant value of int16
			 */
			public:	void M_GetValue(int16 *value) const;
			/**
			 * @brief Get variant value of uint16
			 */
			public:	void M_GetValue(uint16 *value) const;

			/**
			 * @brief Get variant value of char
			 */
			public:	void M_GetValue(char* value) const;
			/**
			 * @brief Get variant value of byte
			 */
			public:	void M_GetValue(byte *value) const;
			/**C
			 * @brief Get variant value of float
			 */
			public:	void M_GetValue(float *value) const;
			/**
			 * @brief Get variant value of double
			 */
			public:	void M_GetValue(double *value) const;
			/**
			 * @brief Get variant value of C_DateTime
			 */
			public:	void M_GetValue(C_DateTime *value) const;
			/**
			 * @brief Get variant value of C_DateTime
			 */
			public:	void M_GetValue(C_Guid *value) const;

			/**
			 * @brief Get value of the array at index
			 */
			public: void M_GetValue(unsigned int index, C_Variant &variant) const;

			/**
			 * @brief Get value of the property propertyName
			 */
			public: void M_GetValue(const std::string propertyName, C_Variant &variant) const;

			/**
			 * @brief Get corresponding Variant type from the string
			 */

			public:	static t_DataType M_S_FromStringToDataType(const std::string &type);

			/**
			 * @brief Get corresponding property type from the string
			 */

			public:	static t_PropertyType M_S_FromStringToPropertyType(const std::string &type);

			/**
			 * @brief Get corresponding string of Variant type
			 */
			public:	static std::string M_S_FromDataTypeToString(const t_DataType &type);

			/**
			 * @brief Get corresponding string of Variant type
			 */
			public:	static std::string M_S_FromPropertyTypeToString(const t_PropertyType &type, const t_OutputType outputFormat = g_JSON);

			friend std::ostream& operator<<(std::ostream& os, const C_Variant& dt);

			/**
			 * @brief Printf the Json Variant to os stream
			 */
			public: friend void M_PrintJSON(std::ostream& os, const C_Variant& dt);
			public: friend uint32 M_PrintBinary(const C_Variant& dt, std::vector<char> &dataBuffer);

			/**
			 * @brief Access variant at array index
			 */
			public:	C_Variant &operator[](const unsigned int &index);

			/**
			* @brief Access variant at array index
			*/
			public:	const C_Variant &operator[](const unsigned int &index) const;
			/**
			 * @brief Access variant at property index
			 */
			public:	C_Variant &operator[](const std::string &index);
			public:	const C_Variant &operator[](const std::string &index) const;

			/**
			 * @brief Access variant at property index
			 */
			public:	C_Variant &M_GetPropertyValue(const std::string &index, C_Variant &defaultValue);

			/**
			 * @brief Get the internal pointer
			 */
			public: void* M_Pointer();

			/**
			* @brief Give access to C_WebsocketStreamBinary in order to get C_Variant support there
			*/
			private: friend class Mia_EXPORT C_WebsocketStreamBinary;

			private:	t_DataType m_Type;

			private:	union
			{
				unsigned long long m_uInt;
				double m_Double;
				void* m_Data;
			} m_Data;
		};

		/**
		 * @brief Collection of managed template pointer in vector style.
		 * @see C_SharedPointer
		 * \ingroup MiaDriverCore
		 * */
		class Mia_EXPORT C_ArgumentList: public std::vector<C_SharedPointer<C_Variant> >
		{
			public: C_ArgumentList(unsigned int size = 0) :	std::vector<C_SharedPointer<C_Variant> >(size){}
		};


		class Mia_EXPORT C_ILock
		{
			public:	virtual ~C_ILock() {}

			public:	virtual void M_Lock() = 0;
			public:	virtual void M_Unlock() = 0;
			public:	virtual bool M_TryLock(int timeout = 0) = 0;
		};

		/**
		 * @brief Class to handle mutual exclusion access.
		 * \ingroup MiaDriverCore
		 */
		class Mia_EXPORT C_ThreadLock: public C_ILock
		{
			public:	C_ThreadLock();
			public:	virtual ~C_ThreadLock();

			/**
			 * @brief Acquire the lock object, will block until object acquisition succeeds
			 */
			public:	virtual void M_Lock();

			/**
			 * @brief Release the lock object
			 */
			public:	virtual void M_Unlock();

			/**
			 * @brief try to acquire the lock within the timeout
			 * @return 	true if the acquisition succeeds within the given time
			 * 			false if the acquisition fails within the given time
			 */
			public:	virtual bool M_TryLock(int timeout = 0);

			/**
			 * @brief OS specific data stucture for locking
			 */
			private:	void * m_Lock;

		};

		/**
		 * @brief Class to handle automatic lock and unlock of the C_ILock interface implementation.
		 * \ingroup MiaDriverCore
		 */
		class C_Locker
		{
			/**
			 * @brief Create a C_Locker object from the given C_ILock interface
			 * @param lock The C_ILock object that implements the C_ILock interface
			 * @param locknow Specify if the lock begin with the initialization
			 */
			public:	C_Locker(C_ILock *lock, bool locknow = true);
			public:	~C_Locker();

			/**
			 * @brief Acquire the C_ILock object
			 */
			public:	void M_Lock();

			/**
			 * @brief Release the C_ILock object
			 */
			public:	void M_Unlock();

			private:	C_ILock *m_Lock;
			private:	bool m_IsLock;
		};

		class Mia_EXPORT  C_WaitableObject
		{
#ifdef _WIN32
		protected: HANDLE m_Handle;
		public: C_WaitableObject() { m_Handle = INVALID_HANDLE_VALUE; }
		public: C_WaitableObject(HANDLE handle) { m_Handle = handle; }
		public: HANDLE M_GetHandle() { return m_Handle; }
		public: virtual bool M_IsInitialized() { return m_Handle != INVALID_HANDLE_VALUE; }
		public: virtual void M_Free() = 0;
#else
		protected: int m_Handle; /* Eventfd file descriptor for waiting multiple objects. */
		public: C_WaitableObject();
		public: void M_Initialize(bool semaphore = true);
		public: virtual void M_Free();
		public: virtual ~C_WaitableObject() { M_Free(); }
		public: virtual bool M_IsInitialized() = 0;
		public: int M_GetHandle() const { return m_Handle; }
#endif
		public: virtual void M_Signal() = 0;
		public: virtual bool M_Wait(uint timeoutms = g_INFINITE) = 0;
		public: static int M_WaitMultiple(C_WaitableObject **objects, int count, bool waitall = false, uint timeoutms = g_INFINITE);
		public: static bool M_SignalAndWait(C_WaitableObject *object_to_signal, C_WaitableObject *object_to_wait, uint timeoutms = g_INFINITE);
		};
		/**
		 * @brief Class to handle threading. In order to use this class, user must inherit this class
		 * and override the method 'void M_Routine()'. This method will be executed when the Start or
		 * Loop method is called.
		 * \ingroup MiaDriverCore
		 */
		class Mia_EXPORT C_Thread
		{
			/**
			 * @brief Create a C_Thread object
			 */
			public:	C_Thread(const std::string &routineName = "");

			public:	virtual ~C_Thread();

			/**
			 * @brief Start the thread
			 */
			public:	bool M_Start();

			/**
			 * @brief OS specific code for starting thread
			 */
			private: int M_Start_OS();

			/**
			 	 * @brief OS specific code releasing thread
			*/
			private: void M_Die_OS();

			/**
			 * @brief Start looping on the thread with the interval intervalMs
			 * @param intervalMs The looping interval in miliseconds
			 */
			public: bool M_Loop(int intervalMs);

			/**
			 * @brief Stop the thread
			 * @param waitForStopping Wait until the thread is stopped before continue
			 * @param timeoutMs Timeout if the thread does not stopped within the given time
			 * @return bool if the thread is able to stop within the given time
			 */
			public:	bool M_Stop(bool waitForStopping = false, int timeoutMs = 1000);

			/**
			 * @brief Routine to be overloaded by the inheritance class
			 */
			public:	virtual void M_Routine() = 0;

			/**
			 * @brief Check if the thread is running
			 */
			public:	bool M_IsRunning();

			/**
			 * @brief Sleep for intervalMs mili-second
			 */
			public:	static void M_S_Sleep(int intervalMs);

			private:	static SYSTEM_THREAD_RET M_S_Routine(SYSTEM_THREAD_PARAM thread);

			protected:  volatile bool            m_Running;
			protected:  volatile bool            m_Stopped;
			protected:  bool                     m_Loop;
			protected:  int                      m_IntervalMs;
			protected:  volatile t_ThreadHandle  m_Thread;
			protected:  std::string              m_RoutineName;
			private:    int                      m_ThreadCounter;
		};

		typedef int t_LanguageId;
		typedef int t_ErrorNumber;

		class C_ErrorLocationData;
		class C_Exception;
		class C_ErrorRecord;
		class C_BaseErrorMessage;
		class C_ExSerialBytes;
		struct C_ErrMsgStore;

		class C_ErrorRecords: public std::list<C_ErrorRecord*>
		{
			public:	C_ErrorRecords()
			{}
		};

		class Mia_EXPORT C_BaseErrorMessage
		{
			public:	C_BaseErrorMessage(const uint64 &erroCode = 0, const std::string &errorMessage = "");
			public:	static C_BaseErrorMessage M_S_AddTrace();

			public:	const uint64 &M_GetErrorCode();

			public:	const std::string &M_GetErrorMessage();

			private:	uint64 m_ErrorCode;
			private:	std::string m_ErrorMessage;
		};

		//CLASS C_Exception ************************************************************
		/**
		 * @brief This is the class that can be used in the throw-statements for throwing
		 * exceptions. Derived classes can be used instead if the exception needs to
		 * be identified in the catch statements.
		 * A C_Exception  object is very small and the copying of the object is
		 * a lightweight operation.
		 * <p>
		 * The first function that detects an error creates a C_Exception class
		 * by passing a zero to the constructor. The C_Exception will then create a new
		 * error record container object (C_ErrorRecords).
		 * The function can then insert one or more error
		 * records to the exception. The catchers of the exception can in turn insert
		 * more error records to the exception.
		 * <p>
		 * In addition of being the base class for the derived exception classes, the
		 * C_Exception class acts as a kind of handle class for the C_ErrorRecords
		 * class. When C_Exception objects are copied, all copies refer to the same
		 * C_ErrorRecords object. The deletion of the C_ErrorRecords object is
		 * handled via reference counting. Also, the pointers to currently existing
		 * C_ErrorRecords objects of each thread is stored in a thread local store.
		 * By this way, the error records can be reported in the terminate handler
		 * that the constructor of C_ErrorRecords object defines. The destructor
		 * of C_ErrorRecords object restores the terminate handler. Unless the M_SetHandled
		 * has been called, the destructor of the C_ErrorRecords object reports the
		 * error through the default error reporter object (returned by
		 * G_ExGetDefaultReporter, set by G_ExSetDefaultReporter).
		 * Also the terminate handler uses the default error reporter object for
		 * reporting the error records.
		 * <p>
		 * Instructions for derivation: Usually, the following members must be overridden:
		 * M_GetClassReg(), M_Throw(). In addition, the following members must be
		 * overridden if the derived class contains additional data members:
		 * M_Serialize(), M_DeSerialize(). Remember also to implement the copy constructor
		 * when the default is not good enough (remember that exception objects get
		 * copy constructed during the throw statement). You should also implement
		 * the assignment operator.
		 * When you create a derived class, you must
		 * also define a registry class for the exception class. Define a registry
		 * class by deriving from C_ExClassRegistry. An example of a derived exception
		 * class can be found from CSCommon_SeTranslator.cpp.
		 * <p>
		 * File to include: CSCommon_Exception.h.
		 * \ingroup MiaDriverException
		 */
		// -----------------------------------------------------------------------------
		class Mia_EXPORT C_ExceptionBase
		{
			/**
			 * @brief The constructor.
			 * Typically, the original detector of the error creates an C_Exception
			 * object and adds one or several error records to it. After that, the catchers
			 * of the exception may insert additional error records to it. The copying
			 * and assignment of C_Exception objects is a light operation. The copied
			 * C_Exception objects all refer to the same error record container object.
			 *
			 * @param errorRecords (use)Pointer to the error record container object. If given as zero
			 * the constructor creates a new error record container. Note
			 * that the C_Exception contains a type conversion operator to
			 * a pointer to the error record container, so the parameter can also be
			 * a C_Exception object. This is most useful if a catcher of the exception
			 * wants to throw a different exception class (derived from C_Exception).
			 * Note that there is no default value for the parameter for not to introdude
			 * a default constructor that might accidentally be used by derived classes.
			 */
			public:	C_ExceptionBase(C_ErrorRecords *errorRecords);

			/**
			 * @brief Constructs an exception object and adds one error record to it.
			 * This constructor works identically to the constructor that takes only
			 * the pErrorRecords parameter, but also adds one error record to the exception,
			 * for convenience.
			 * For description of the parameters, see the documentation for the member
			 * function M_AddError.
			 */
			public:	C_ExceptionBase(C_ErrorRecords *pErrorRecords, const C_ErrorLocationData &errorLocation,
			   const C_BaseErrorMessage &messageId, const C_Variant &p1 = C_Variant(), const C_Variant &p2 = C_Variant(),
			   const C_Variant &p3 = C_Variant(), const C_Variant &p4 = C_Variant(), const C_Variant &p5 = C_Variant(),
			   const C_Variant &p6 = C_Variant(), const C_Variant &p7 = C_Variant(), const C_Variant &p8 = C_Variant());

			/**
			 * @brief Constructs an exception object and adds one error record to it.
			 * This constructor works identically to the constructor that takes only
			 * the pErrorRecords parameter, but also adds one error record to the exception,
			 * for convenience.
			 * For description of the parameters, see the documentation for the member
			 * function M_AddError.
			 */
			public:	C_ExceptionBase(const C_ErrorLocationData &errorLocation, const C_BaseErrorMessage &messageId,
			   const C_Variant &p1 = C_Variant(), const C_Variant &p2 = C_Variant(), const C_Variant &p3 = C_Variant(),
			   const C_Variant &p4 = C_Variant(), const C_Variant &p5 = C_Variant(), const C_Variant &p6 = C_Variant(),
			   const C_Variant &p7 = C_Variant(), const C_Variant &p8 = C_Variant());

			/**
			 * @brief Copy constructor.
			 * The copy constructor takes care of the reference count to the associated
			 * error record container. If a derived class explicitly defines a copy
			 * constructor, it must copy-construct also the base class.
			 * Never throws an exception.

			 * @param ex (in)
			 * The source C_Exception object.
			 */
			public:	C_ExceptionBase(const C_ExceptionBase &ex);

			/**
			 * @brief Assignment operator.
			 * The assignment operator takes care of the reference counts to the associated
			 * error record containers. There should be seldom any use in C_Exception
			 * assignment because the real exception classes are typically only derived from
			 * C_Exception. If a derived class explicitly defines an assignment operator,
			 * it must call the assignment operator of the base class in order to
			 * manage the reference counting of the error record container.
			 * Never throws an exception.

			 * @param ex (in)
			 * The source C_Exception object.
			 */
			public: C_ExceptionBase &operator=(const C_Exception &ex);

			public: C_ExceptionBase();
			/**
			 * @brief Destructor
			 * The destructor takes care of the reference count to the associated
			 * error record container. Note that if this was the last exception object
			 * that referred to the error record container, the error record container
			 * deletes itself. If the exception was not handed before that (i.e. the
			 * M_SetHandled() was not called) the error record container will dump its
			 * contents. The destructor of a derived class must NOT release the error
			 * record container because this is done by the destructor of the base class.
			 * Should never throw an exception.
			 */
			public: virtual ~C_ExceptionBase();

			/**
			 * @brief Add a new error record to the exception, Returns pointer to it.
			 * The C_Exception contains a list of error records. This function creates
			 * a new error record entry to the beginning of the list. The created record
			 * now becomes the error record number 0.
			 *
			 * @param errorLocation (in) Error location.
			 * @param messageId (in) Error message identifier. For errors defined in CSCommon_ErrorMsg.h,
			 * this is in format C_NumericErrMsg(messageId). And for example for errors defined RTDB_ErrorMsg.h this is in format
			 * RTDB::C_RTDBNumericErrMsg(messageId).
			 * @param p1 (in-opt) Parameters for the error message.
			 * @param p2 (in-opt) Parameters for the error message.
			 * @param p3 (in-opt) Parameters for the error message.
			 * @param p4 (in-opt) Parameters for the error message.
			 * @param p5 (in-opt) Parameters for the error message.
			 * @param p6 (in-opt) Parameters for the error message.
			 * @param p7 (in-opt) Parameters for the error message.
			 * @param p8 (in-opt) Parameters for the error message.
			 */
			public: C_ErrorRecord *M_AddError(const C_ErrorLocationData &errorLocation, const C_BaseErrorMessage &messageId,
			   const C_Variant &p1 = C_Variant(), const C_Variant &p2 = C_Variant(), const C_Variant &p3 = C_Variant(),
			   const C_Variant &p4 = C_Variant(), const C_Variant &p5 = C_Variant(), const C_Variant &p6 = C_Variant(),
			   const C_Variant &p7 = C_Variant(), const C_Variant &p8 = C_Variant());

			/**
			 * Add a traceback record to the exception. Call this function if you
			 * do not have any specialized error message for the error situation.
			 * This function is typically used in a catch() block.
			 *
			 * @param errorLocation (in)
			 * The error location information. Usually given as the macro
			 * Mia_THIS_LOCATION.
			 *
			 * @param p1 (in-opt) Parameters for the error message.
			 * @param p2 (in-opt) Parameters for the error message.
			 * @param p3 (in-opt) Parameters for the error message.
			 * @param p4 (in-opt) Parameters for the error message.
			 * @param p5 (in-opt) Parameters for the error message.
			 * @param p6 (in-opt) Parameters for the error message.
			 * @param p7 (in-opt) Parameters for the error message.
			 * @param p8 (in-opt) Parameters for the error message.
			 *
			 * @see C_ErrorLocationData
			 */
			public: C_ErrorRecord *M_AddTraceback(const C_ErrorLocationData &errorLocation, const C_Variant &p1 = C_Variant(),
				const C_Variant &p2 = C_Variant(), const C_Variant &p3 = C_Variant(), const C_Variant &p4 = C_Variant(),
			   const C_Variant &p5 = C_Variant(), const C_Variant &p6 = C_Variant(), const C_Variant &p7 = C_Variant(),
			   const C_Variant &p8 = C_Variant());

			public: void M_Throw()
			{
				THROW();
			}
			/**
			 * @brief Add a traceback record to the exception and throw. The function adds a
			 * traceback record with M_AddTraceback and rethrows the exception by
			 * the throw statement.
			 * @param errorLocation Location of the error. Given by the macro Mia_THIS_LOCATION.
			 * @param p1 (in-opt) Parameters for the error message.
			 * @param p2 (in-opt) Parameters for the error message.
			 * @param p3 (in-opt) Parameters for the error message.
			 * @param p4 (in-opt) Parameters for the error message.
			 * @param p5 (in-opt) Parameters for the error message.
			 * @param p6 (in-opt) Parameters for the error message.
			 * @param p7 (in-opt) Parameters for the error message.
			 * @param p8 (in-opt) Parameters for the error message.
			 *
			 * @see C_ErrorLocationData
			 */

			public: void M_RethrowTraceback(const C_ErrorLocationData &errorLocation, const C_Variant &p1 = C_Variant(),
			   const C_Variant &p2 = C_Variant(), const C_Variant &p3 = C_Variant(), const C_Variant &p4 = C_Variant(),
			   const C_Variant &p5 = C_Variant(), const C_Variant &p6 = C_Variant(), const C_Variant &p7 = C_Variant(),
			   const C_Variant &p8 = C_Variant());

			/**
			 * Tell that the exception has been handled.
			 * Unless this function is called, and the last exception object that refers
			 * to the same error record container goes out of scope, the destructor of the
			 * error records container will dump the error records (to somewhere).
			 * Adding a new error record causes the "handled" flag to be unset.
			 */
			public:	void M_SetHandled();

			/**
			 * Get pointer to an error record.
			 *
			 * @param iRecordNum (in)
			 * The error record number, between 0..N-1, where N is the value returned
			 * by M_GetNumErrorRecords(). The record 0 is the one that was added latest.
			 *
			 * @return Pointer to error record.
			 *
			 * @exception C_Exception if iRecordNum is out of range.
			 */
			public:	C_ErrorRecord *M_GetErrorRecord(int iRecordNum) const;

			/**
			 * Get the number of error records in the containter.
			 *
			 * @return The number of error records.
			 */
			public:	int M_GetNumErrorRecords() const;

			/**
			 * Print the contents of the exception. The function calls the
			 * virtual function M_SpecificInfo to print the contents of the
			 * derived class, and the M_Dump member function of the C_ErrorRecords
			 * to print the contents of the error records.
			 * The M_Dump function is meant to be used only during development phase.
			 * In the retail system, the contents of the error records should be
			 * enough for reporting the error.
			 *
			 * @param os (use) The output stream where to print the contents.
			 */
			public: virtual void M_Dump(std::ostream &os) const;

			/**
			 * Type conversion operator.
			 * Because the C_Exception class is mostly just a wrapper to a pointer
			 * to the error record container, this type conversion may be useful some
			 * times.
			 * @return Pointer to the error record container.
			 */
			public: operator C_ErrorRecords *() const;

			/**
			 * Deletes the object that was created with
			 * C_ExClassRegistry::M_CreateExceptionObject
			 */
			public: void M_DeleteCreated();

			public: bool M_HasErrorNumber(uint errnumtosearch) const;

			protected: C_ErrorRecords *m_ErrorRecords;
			protected: int m_Handled;
			protected: C_DateTime m_Time;
			public: virtual std::string M_ToString() { return m_Message;}
			protected: std::string m_Message;
		};

		//CLASS C_ErrorLocationData ****************************************************
		/**
		 * @brief The class contains information of the program location and version. This
		 * class is usually used with the help of the Mia_THIS_LOCATION macro.
		 * <p>
		 * File to include: CSCommon_Exception.h.
		 * \ingroup MiaDriverCore
		 */
		// -----------------------------------------------------------------------------
		class Mia_EXPORT C_ErrorLocationData
		{
			public:
			/**
			 Constructor.
			 @param sRoutineName (in)
			 A character string that should contain the full name of the current
			 function. The types of the parameters can be included to the name for
			 distinguishing overloaded functions. The namespace name should also be
			 prefixed to the function name. For example:

			 <pre>
			 void C_MyClass::M_MyFun(int x)
			 {
			 static const char szThisRoutine[] = "RTDB::C_MyClass::M_MyFun(int)";
			 }
			 </pre>

			 The value of szThisRoutine is meant to be presented only to human
			 readers so it is not necessary to use exactly uniform format in every case.

			 @param sSourceFileName (in)
			 The source file name. Can be taken with the __FILE__ macro.

			 @param iSourceFileLine (in)
			 The source file line. Can be taken with the __LINE__ macro.

			 @param sCompilationTime (in)
			 The compilation time string. Can be taken with the __DATE__ and __TIME__
			 macros.

			 @param sSourceFileTime (in)
			 The source file time string. Can be taken with
			 the __TIMESTAMP__ macro (this seems to be in MSVC only)

			 @param sSourceCodeControlVersion (in)
			 A string that identifies the controlled version of the source file.
			 Usually this string can be taken from the RCS_id variable that was defined
			 as follows (when Visual SourceSafe is used for version management):
			 static const char RCS_Id[] = "$Archive: /CSCommon/src/base/CSCommon_Exception.h $ $Revision: 38 $";

			 */
			public:C_ErrorLocationData(const std::string &sRoutineName, const std::string &sSourceFileName, int iSourceFileLine,
			   const std::string &sCompilationTime, const std::string &sSourceFileTime,
			   const std::string &sSourceCodeControlVersion = "");

			/**
			 Displays the data contents to an output stream.
			 @param os (use)
			 The output stream.
			 The Windows NT language identifier. Zero value means the default language
			 defined for the system. The implementation does not currently use this parameter.
			 */
			void M_Dump(std::ostream &os) const;

			C_DateTime m_OccurrenceTime;
			std::string m_RoutineName;
			std::string m_SourceFileName;
			int m_SourceFileLine;
			std::string m_CompilationTime;
			std::string m_SourceFileTime;
			std::string m_SourceCodeControlVersion;
			std::string m_Environment;
		};

		class Mia_EXPORT C_ErrorRecord: public C_ErrorLocationData
		{
			public: C_ErrorRecord(const C_ErrorLocationData &locationData, const C_BaseErrorMessage &messageId,
			   const C_Variant &p1 = C_Variant(), const C_Variant &p2 = C_Variant(), const C_Variant &p3 = C_Variant(),
			   const C_Variant &p4 = C_Variant(), const C_Variant &p5 = C_Variant(), const C_Variant &p6 = C_Variant(),
			   const C_Variant &p7 = C_Variant(), const C_Variant &p8 = C_Variant());

			public:	C_ErrorRecord(const C_ErrorRecord &errorRecord);
			public:	void M_Dump(std::ostream &os) const;
			private:	C_Variant m_Parameter[8];
		};

		/*Template implementation*/
		template<class t_Type>
		C_AutoPointer<t_Type>::C_AutoPointer(t_Type *ptr, bool autoRelease)
		{
			m_Pointer = ptr;
			m_AutoRelease = autoRelease;
		}
		template<class t_Type>
		C_AutoPointer<t_Type>::C_AutoPointer(const C_AutoPointer<t_Type> &p)
		{
			m_Pointer = p.m_Pointer;
			m_AutoRelease = false;
		}
		template<class t_Type>
		void C_AutoPointer<t_Type>::operator=(C_AutoPointer &p)
		{
			m_Pointer = p.m_Pointer;
			p.m_Pointer = NULL;
			m_AutoRelease = p.m_AutoRelease;
			p.m_AutoRelease = false;
		}
		template<class t_Type>
		t_Type *C_AutoPointer<t_Type>::operator=(t_Type *ptr)
		{
			return m_Pointer = ptr;
		}
		template<class t_Type>
		t_Type &C_AutoPointer<t_Type>::operator*() const
		{
			return *m_Pointer;
		}
		template<class t_Type>
		t_Type *C_AutoPointer<t_Type>::operator->() const
		{
			return m_Pointer;
		}
		template<class t_Type>
		t_Type *C_AutoPointer<t_Type>::M_Pointer() const
		{
			return m_Pointer;
		}
		template<class t_Type>
		C_AutoPointer<t_Type>::operator t_Type *() const
		{
			return m_Pointer;
		}

		template<class t_Type>
		void C_AutoPointer<t_Type>::M_Free()
		{
			t_Type *ptr = m_Pointer;
			m_Pointer = NULL;
			if (ptr) delete ptr;
		}
		template<class t_Type>
		C_AutoPointer<t_Type>::~C_AutoPointer()
		{
				if (m_AutoRelease && m_Pointer) delete m_Pointer;
		}

		Mia_EXPORT size_t G_Read7BitEncodedInt(const void* x, size_t maxbytes);

		Mia_EXPORT size_t G_Read7BitEncodedInt(const void* x, size_t *readbytes, size_t maxbytes);

		Mia_EXPORT size_t G_Write7BitEncodedInt(void* x, size_t value, size_t maxbytes);
	} /* namespace Mia */
} /* namespace ABB */

#endif /* WS_BASE_H_ */
