// #include "CSCommon_StabileHeaders.h" -- not using

//static const char RCS_Id[] = "$Archive: /CSCommon/src/base/CSCommon_Exception.cpp $ $Revision: 41 $";

/*==============================================================================

 CSCommonException.cpp

 CSCommon - Common Classes
 (c) ABB Industry Oy
 
 
 Description:

 Support classes for error handling.

 History:

 22-JAN-1998/Jari Kulmala/WO#1241
 Original

 16-AUG-2001/Jari Kulmala/QC#9059CS-B0003
 Improvement: Huge performance boost to cache the environment information

 10-OCT-2003/Jari Kulmala/QC#RTDB--A0057 (SPE 03/061)
 Re-org: M_S_GetErrorRecordsInstance() instead of M_S_GetErrorsList()

 20-JAN-2007/Jari Kulmala/CR#07-0028
 Feature: conditional code for CSCommon_SAFESTRING_IMPLEMENTATION

 17-NOV-2009/Jari Kulmala/CR#08-0442
 Feature: Include the creation time of the process and the command line
 string to the exception environment information.

 14-DEC-2009/Jari Kulmala/CR#09-0231
 ReOrg: separate constructor that takes at most one error message parameter.

 06-JUL-2010/Jari Kulmala/CR#10-0179
 Feature: include CSCommon version check to C_ErrorLocationData
 constructor starting from version 4.3

 09-JAN-2012/Jari Kulmala/WI#CPM-278
 Feature change: The CSCommon error messages no longer contain the "SCC Version" information
 (the information is not available in TFS version control)

 13-NOV-2012/Jari Kulmala/WI#CPM-7580
 ReOrg: Use of new C_ExString and t_ticsvalue
 ==============================================================================*/

#define CSCommon_EXCEPTION_CPP 1

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <assert.h>
#include <sstream>
#include "Mia_Exception.h"
#include "Mia_Base.h"

#if _WIN32
#include <Windows.h>
#include <crtdbg.h>
#endif

#ifdef __linux__
#include <unistd.h>
#endif

using std::set_terminate;
using std::wostream;
using std::clog;
using std::endl;
using std::wcerr;

#define JKULOG 0

namespace ABB
{
	namespace Mia
	{
#ifdef _WIN32
#if _MSC_VER < 1310

		template<> const C_SafeString<tchar, std::char_traits<tchar>, C_ExAllocator<tchar> >::size_type
		C_SafeString<tchar, std::char_traits<tchar>, C_ExAllocator<tchar> >::npos =
		(C_SafeString<tchar, std::char_traits<tchar>, C_ExAllocator<tchar> >::size_type)-1;

#endif
#endif
		//static const char s_szThisFile[] = __FILE__;

		extern const tchar Exception_e_paramrange[] =
		Mia_TEXT("EBad '%1' parameter value '%2'\1")
		Mia_TEXT("iCSCommmonException_e_paramrange\1");

#define JKU_DEB1 0

		inline uint64 G_GetCurrentProcessId()
		{
#ifdef _WIN32
			return GetCurrentProcessId();
#elif defined(OSAL_OS)
			return 42;
#else
			return getpid();
#endif
		}

		/*------------------------------------------------------------------------------
		 ------------------------------------------------------------------------------*/
		std::string G_GetProcName()
		{
			char tmp[40];
#ifdef __linux__
			int len = sprintf(tmp, Mia_TEXT("%ld"), (long) G_GetCurrentProcessId());
#elif _WIN32
			int len = sprintf(tmp, "%ld", (long)G_GetCurrentProcessId());
#elif defined(OSAL_OS)
			int len = sprintf(tmp, "%d", 42); // XXX TODO
#endif
			return std::string(tmp, len);
		}

		C_ExceptionBase::C_ExceptionBase(C_ErrorRecords *errorRecords)
		{
			if (errorRecords)
			{
				m_ErrorRecords = errorRecords;
			} else
			{
				m_ErrorRecords = DBG_NEW C_ErrorRecords();
			}

			m_Handled = 0;
			m_Time = C_DateTime::M_S_Now();
		}

		C_ExceptionBase::C_ExceptionBase(C_ErrorRecords *pErrorRecords, const C_ErrorLocationData &errorLocation,
		   const C_BaseErrorMessage &messageId, const C_Variant &p1, const C_Variant &p2, const C_Variant &p3,
		   const C_Variant &p4, const C_Variant &p5, const C_Variant &p6, const C_Variant &p7, const C_Variant &p8)
		{
			if (pErrorRecords)
			{
				m_ErrorRecords = pErrorRecords;
			} else
			{
				m_ErrorRecords = DBG_NEW C_ErrorRecords();
			}

			m_ErrorRecords->push_back(new C_ErrorRecord(errorLocation, messageId, p1, p2, p3, p4, p5, p6, p7, p8));
			m_Handled = 0;
			m_Time = C_DateTime::M_S_Now();
		}

		C_ExceptionBase::C_ExceptionBase(const C_ErrorLocationData &errorLocation, const C_BaseErrorMessage &messageId,
		   const C_Variant &p1, const C_Variant &p2, const C_Variant &p3, const C_Variant &p4, const C_Variant &p5,
		   const C_Variant &p6, const C_Variant &p7, const C_Variant &p8)
		{
			m_ErrorRecords = DBG_NEW C_ErrorRecords();
			m_ErrorRecords->push_back(new C_ErrorRecord(errorLocation, messageId, p1, p2, p3, p4, p5, p6, p7, p8));
			m_Handled = 0;
			m_Time = C_DateTime::M_S_Now();
		}

		C_ErrorRecord::C_ErrorRecord(const C_ErrorLocationData &locationData, const C_BaseErrorMessage &messageId,
		   const C_Variant &p1, const C_Variant &p2, const C_Variant &p3, const C_Variant &p4, const C_Variant &p5,
		   const C_Variant &p6, const C_Variant &p7, const C_Variant &p8) :
			C_ErrorLocationData(locationData)
		{
			if (p1.M_IsValid()) m_Parameter[0] = p1;
			if (p2.M_IsValid()) m_Parameter[1] = p2;
			if (p3.M_IsValid()) m_Parameter[2] = p3;
			if (p4.M_IsValid()) m_Parameter[3] = p4;
			if (p5.M_IsValid()) m_Parameter[4] = p5;
			if (p6.M_IsValid()) m_Parameter[5] = p6;
			if (p7.M_IsValid()) m_Parameter[6] = p7;
			if (p8.M_IsValid()) m_Parameter[7] = p8;
		}

		C_ErrorRecord::C_ErrorRecord(const C_ErrorRecord &errorRecord) :
			C_ErrorLocationData(errorRecord)
		{
			for (int i = 0; i < 8; i++)
			{
				m_Parameter[i] = errorRecord.m_Parameter[i];
			}
		}

		C_ExceptionBase::C_ExceptionBase(const C_ExceptionBase &ex)
		{
			m_ErrorRecords = DBG_NEW C_ErrorRecords();
			for (C_ErrorRecords::iterator iter = m_ErrorRecords->begin(); iter != m_ErrorRecords->end(); iter++)
			{
				C_ErrorRecord * er = *iter;
				m_ErrorRecords->push_back(new C_ErrorRecord(*er));
			}

			m_Handled = 0;
			m_Time = ex.m_Time;
		}

		C_ExceptionBase &C_ExceptionBase::operator=(const C_Exception &ex)
		{
			m_ErrorRecords = DBG_NEW C_ErrorRecords();
			for (C_ErrorRecords::iterator iter = m_ErrorRecords->begin(); iter != m_ErrorRecords->end(); iter++)
			{
				C_ErrorRecord * er = *iter;
				m_ErrorRecords->push_back(new C_ErrorRecord(*er));
			}
			m_Handled = 0;

			m_Time = ex.m_Time;
			return *this;
		}

		C_ExceptionBase::C_ExceptionBase() : m_Handled(0)
		{
			m_ErrorRecords = DBG_NEW C_ErrorRecords();
			m_Time = C_DateTime::M_S_Now();
		}

		C_ExceptionBase::~C_ExceptionBase()
		{
			if (!m_Handled) M_Dump(MIA_OUT_WARNING_STREAM);

			for (C_ErrorRecords::iterator iter = m_ErrorRecords->begin(); iter != m_ErrorRecords->end(); iter++)
			{
				C_ErrorRecord * er = *iter;
				delete er;
			}

			m_ErrorRecords->clear();

			delete m_ErrorRecords;
		}

		void C_ErrorRecord::M_Dump(std::ostream &os) const
		{
			C_ErrorLocationData::M_Dump(os);
			bool dump = false;
			if (m_Parameter[0].M_IsValid())
			{
				dump = true;
				os << " Parameters: (";
			}
			for (int i = 0; i < 8; i++)
			{
				if (m_Parameter[i].M_IsValid())
				{
					os << m_Parameter[i];
					if (i < 7 && m_Parameter[i + 1].M_IsValid()) os << ",";
				}
			}
			if (dump)
			{
				os << ")";
			}
			os << std::endl;
		}

		C_ErrorRecord *C_ExceptionBase::M_AddError(const C_ErrorLocationData &errorLocation,
		   const C_BaseErrorMessage &messageId, const C_Variant &p1, const C_Variant &p2, const C_Variant &p3,
		   const C_Variant &p4, const C_Variant &p5, const C_Variant &p6, const C_Variant &p7, const C_Variant &p8)
		{
			C_ErrorRecord *er = DBG_NEW C_ErrorRecord(errorLocation, messageId, p1, p2, p3, p4, p5, p6, p7, p8);
			m_ErrorRecords->push_back(er);
			m_Handled = 0;
			return er;
		}

		C_ErrorRecord *C_ExceptionBase::M_AddTraceback(const C_ErrorLocationData &errorLocation, const C_Variant &p1,
		   const C_Variant &p2, const C_Variant &p3, const C_Variant &p4, const C_Variant &p5, const C_Variant &p6,
		   const C_Variant &p7, const C_Variant &p8)
		{
			C_ErrorRecord *er = DBG_NEW C_ErrorRecord(errorLocation, C_BaseErrorMessage::M_S_AddTrace(), p1, p2, p3, p4, p5,
			   p6, p7, p8);
			m_ErrorRecords->push_back(er);
			m_Handled = 0;
			return er;
		}

		void C_ExceptionBase::M_RethrowTraceback(const C_ErrorLocationData &errorLocation, const C_Variant &p1,
		   const C_Variant &p2, const C_Variant &p3, const C_Variant &p4, const C_Variant &p5, const C_Variant &p6,
		   const C_Variant &p7, const C_Variant &p8)
		{
			C_ErrorRecord *er = DBG_NEW C_ErrorRecord(errorLocation, C_BaseErrorMessage::M_S_AddTrace(), p1, p2, p3, p4, p5, p6, p7, p8);
			m_ErrorRecords->push_back(er);
			m_Handled = 0;
			THROW(;);
		}

		void C_ExceptionBase::M_SetHandled()
		{
			m_Handled = 1;
		}

		int C_ExceptionBase::M_GetNumErrorRecords() const
		{
			return (int)m_ErrorRecords->size();
		}

		C_ExceptionBase::operator C_ErrorRecords *() const
		{
			return m_ErrorRecords;
		}
		void C_ExceptionBase::M_Dump(std::ostream &os) const
		{
			os << std::endl << "> Exception: " << m_Message << std::endl;
			os << "> Exception time: " << m_Time << "." << std::endl;
			os << "> Exception traces: " <<std::endl;
			for (C_ErrorRecords::iterator iter = m_ErrorRecords->begin(); iter != m_ErrorRecords->end(); iter++)
			{
				C_ErrorRecord * er = *iter;
				er->M_Dump(os);
			}
			os << std::endl;
		}

		C_Exception::C_Exception(const C_ErrorLocationData &errorLocation, const C_BaseErrorMessage &messageId,
		   const C_Variant &p1, const C_Variant &p2, const C_Variant &p3, const C_Variant &p4, const C_Variant &p5,
		   const C_Variant &p6, const C_Variant &p7, const C_Variant &p8) :
			C_ExceptionBase(errorLocation, messageId, p1, p2, p3, p4, p5, p6, p7, p8)
		{

		}

		C_Exception::C_Exception(const C_ErrorLocationData &errorLocation, const std::string &messageId,
		   const C_Variant &p1, const C_Variant &p2, const C_Variant &p3, const C_Variant &p4, const C_Variant &p5,
		   const C_Variant &p6, const C_Variant &p7, const C_Variant &p8) :
			C_ExceptionBase(errorLocation, C_BaseErrorMessage(G_ERROR_UNKOWN, messageId), p1, p2, p3, p4, p5, p6, p7, p8)
		{

		}

		C_Exception::~C_Exception()
		{
			//MIA_OUT_DEBUG<<"Exception: " << m_Message << ". Stack trace:" << std::endl;
		}

		bool C_Exception::M_S_HandleException(const C_ErrorLocationData &errorLocation, const int &errorCode,
		   const C_Variant &p1, const C_Variant &p2, const C_Variant &p3, const C_Variant &p4, const C_Variant &p5,
		   const C_Variant &p6, const C_Variant &p7, const C_Variant &p8)
		{

			switch (errorCode)
			{
				case G_ERROR_SSL_UNABLE_LOAD_CERTIFICATE:
				{
					THROW(C_ConnectionException(errorLocation, "Unable to load certificate", p1, p2, p3, p4, p5, p6, p7, p8));
				}
				case G_ERROR_CONNECTION_REFUSED:
				case G_ERROR_SERVER_ERROR:
				case G_ERROR_UNABLE_CONNECT:
				case G_ERROR_UNABLE_OPEN_SOCKET:
				case G_ERROR_SSL_UNABLE_CONNECT_SOCKET:
				case G_ERROR_UNABLE_RECEIVE:
				{
					THROW(C_ConnectionException(errorLocation, "Unable to connect socket", p1, p2, p3, p4, p5, p6, p7, p8));
				}

				case G_ERROR_UNAUTHORIZED:
				{
					THROW(C_UnauthorizedException(errorLocation, p1, p2, p3, p4, p5, p6, p7, p8));
				}

				case G_ERROR_INVALID_CONNECTION_STRING:
				{
					THROW(C_BadConnectionStringException(errorLocation, "", p1, p2, p3, p4, p5, p6, p7, p8));
				}

				case G_ERROR_INVALID_HTTP_RESPONSE:
				{
					THROW(C_ServerException(errorLocation, "Invalid server message response", p1, p2, p3, p4, p5, p6, p7, p8));
				}

				case G_ERROR_TIME_OUT:
				{
					THROW(C_TimeoutException(errorLocation, p1, p2, p3, p4, p5, p6, p7, p8));
				}
				default:
				{
					THROW(C_UnknownException(errorLocation, errorCode, p1, p2, p3, p4, p5, p6, p7, p8));
				}
			}
			
			NO_THROW(return true;);
		}
	} /* end namespace Mia */
} /* end namespace ABB */
