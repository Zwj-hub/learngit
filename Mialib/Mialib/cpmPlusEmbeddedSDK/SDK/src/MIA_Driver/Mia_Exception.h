/*==============================================================================

Mia_Exception.h
(c) ABB Industry Oy

Description:

   The base class and the suppport classes for error handling. All definitions
   reside in the name space ABB::Mia.


History:

   20-APRIL-2016/TUAN VU
      Original

===================================================================================================*/

#ifndef _MIA_EXCEPTION_H
#define _MIA_EXCEPTION_H 1

#include <exception>
#include <list>
#include <algorithm>

extern "C"
{
	#include "WS_Error.h"
}
#include "Mia_Base.h"

/**
 * \defgroup MiaDriverException cpmPlus Embedded SDK - Exceptions
 */

namespace ABB
{
namespace Mia
{
	/**
	 * @brief t_MiaErrorCode Collections of common drive error code
	 * \ingroup MiaDriverException
	 */
	enum t_MiaErrorCode
	{
		g_MiaError_InvalidDateTime = 0x100,
		g_MiaError_InvalidState,
		g_MiaError_ServerError,
		g_MiaError_Timeout,
		g_MiaError_UnableToSend,
		g_MiaError_PropertyNotFound,
		g_MiaError_SSLError,
		g_MiaError_SocketError,
		g_MiaError_ClassNotFound
	};

	/**
	 * @brief Exception base of Driver's exception
	 * \ingroup MiaDriverException
	 */
	class C_Exception : public C_ExceptionBase
	{
		public: C_Exception(
			const C_ErrorLocationData     &errorLocation,
			const C_BaseErrorMessage		&messageId,
			const C_Variant            	&p1 = C_Variant(),
			const C_Variant            	&p2 = C_Variant(),
			const C_Variant            	&p3 = C_Variant(),
			const C_Variant            	&p4 = C_Variant(),
			const C_Variant            	&p5 = C_Variant(),
			const C_Variant            	&p6 = C_Variant(),
			const C_Variant            	&p7 = C_Variant(),
			const C_Variant            	&p8 = C_Variant());

		public: C_Exception(
			const C_ErrorLocationData     &errorLocation,
			const std::string 				&messageId,
			const C_Variant            	&p1 = C_Variant(),
			const C_Variant            	&p2 = C_Variant(),
			const C_Variant            	&p3 = C_Variant(),
			const C_Variant            	&p4 = C_Variant(),
			const C_Variant            	&p5 = C_Variant(),
			const C_Variant            	&p6 = C_Variant(),
			const C_Variant            	&p7 = C_Variant(),
			const C_Variant            	&p8 = C_Variant());

		public: virtual ~C_Exception();

		public: std::string M_GetMessage() { return m_Message;}

		public: virtual std::string M_ToString() { return m_Message;}

		public: static bool M_S_HandleException(
				const C_ErrorLocationData     &errorLocation,
				const int			 				&errorCode,
				const C_Variant            	&p1 = C_Variant(),
				const C_Variant            	&p2 = C_Variant(),
				const C_Variant            	&p3 = C_Variant(),
				const C_Variant            	&p4 = C_Variant(),
				const C_Variant            	&p5 = C_Variant(),
				const C_Variant            	&p6 = C_Variant(),
				const C_Variant            	&p7 = C_Variant(),
				const C_Variant            	&p8 = C_Variant());
	};

	/**
	 * @brief Driver is in a invalid state
	 * \ingroup MiaDriverException
	 */
	class C_InvalidStateException : public C_Exception
	{
		public: C_InvalidStateException(
				const C_ErrorLocationData     &errorLocation,
				const std::string 				&currentState,
				const std::string 				&expectedState,
				const C_Variant            	&p1 = C_Variant(),
				const C_Variant            	&p2 = C_Variant(),
				const C_Variant            	&p3 = C_Variant(),
				const C_Variant            	&p4 = C_Variant(),
				const C_Variant            	&p5 = C_Variant(),
				const C_Variant            	&p6 = C_Variant(),
				const C_Variant            	&p7 = C_Variant(),
				const C_Variant            	&p8 = C_Variant()) :
				C_Exception(errorLocation,
					C_BaseErrorMessage(g_MiaError_InvalidState, "Expected state " + expectedState + " while Current State = " + currentState),
					p1,
					p2,
					p3,
					p4,
					p5,
					p6,
					p7,
					p8)
		{
			m_CurrentState = currentState;
			m_ExpectedState = expectedState;
			m_Message = "C_InvalidStateException";
		}

		public: virtual ~C_InvalidStateException(){};
		private: std::string m_CurrentState;
		private: std::string m_ExpectedState;
	};

	static std::string G_ReplaceAll(std::string str, const std::string& from, const std::string& to)
	{
		 size_t start_pos = 0;
		 while((start_pos = str.find(from, start_pos)) != std::string::npos) {
			  str.replace(start_pos, from.length(), to);
			  start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
		 }
		 return str;
	}

	/**
	 * @brief Server has encoutered an error and is unable to complete the request.
	 * \ingroup MiaDriverException
	 */
	class C_ServerException : public C_Exception
	{
		public: C_ServerException(
			const C_ErrorLocationData     &errorLocation,
			const std::string 				&serverMessage,
			const C_Variant            	&p1 = C_Variant(),
			const C_Variant            	&p2 = C_Variant(),
			const C_Variant            	&p3 = C_Variant(),
			const C_Variant            	&p4 = C_Variant(),
			const C_Variant            	&p5 = C_Variant(),
			const C_Variant            	&p6 = C_Variant(),
			const C_Variant            	&p7 = C_Variant(),
			const C_Variant            	&p8 = C_Variant()) :
			C_Exception(errorLocation, C_BaseErrorMessage(g_MiaError_ServerError, serverMessage), p1, p2, p3, p4, p5, p6, p7, p8)
		{
			m_DetailMessage = G_ReplaceAll(serverMessage, "\\r\\n", "\r\n");

			m_Message = "C_ServerException.\n>  ServerMessage: " + m_DetailMessage + ".";
		}

		public: virtual ~C_ServerException(){};
		private: std::string m_DetailMessage;

	};

	/**
	 * @brief The given property is not found
	 * \ingroup MiaDriverException
	 */
	class C_PropertyNotFoundException : public C_Exception
	{
		public: C_PropertyNotFoundException(
			const C_ErrorLocationData     &errorLocation,
			const std::string 				&propertyName,
			const C_Variant            	&p1 = C_Variant(),
			const C_Variant            	&p2 = C_Variant(),
			const C_Variant            	&p3 = C_Variant(),
			const C_Variant            	&p4 = C_Variant(),
			const C_Variant            	&p5 = C_Variant(),
			const C_Variant            	&p6 = C_Variant(),
			const C_Variant            	&p7 = C_Variant(),
			const C_Variant            	&p8 = C_Variant()) :
			C_Exception(errorLocation, C_BaseErrorMessage(g_MiaError_PropertyNotFound, propertyName), p1, p2, p3, p4, p5, p6, p7, p8)

		{
			m_PropertyName = propertyName;
			m_Message = "C_PropertyNotFoundException, Property= " + m_PropertyName;
		}

		public: virtual ~C_PropertyNotFoundException(){};
		public: const std::string &M_GetProperty() { return m_PropertyName;}
		private: std::string m_PropertyName;
	};

	/**
	 * @brief Exception related to encrypted SSL connection
	 * \ingroup MiaDriverException
	 */
	class C_ConnectionException : public C_Exception
	{
		public: C_ConnectionException(
			const C_ErrorLocationData     &errorLocation,
			const std::string 				&sslMessage,
			const C_Variant            	&p1 = C_Variant(),
			const C_Variant            	&p2 = C_Variant(),
			const C_Variant            	&p3 = C_Variant(),
			const C_Variant            	&p4 = C_Variant(),
			const C_Variant            	&p5 = C_Variant(),
			const C_Variant            	&p6 = C_Variant(),
			const C_Variant            	&p7 = C_Variant(),
			const C_Variant            	&p8 = C_Variant()) :
			C_Exception(errorLocation, C_BaseErrorMessage(g_MiaError_SSLError, sslMessage), p1, p2, p3, p4, p5, p6, p7, p8)
		{
			m_Message = ("C_ConnectionException");
		}

		public: virtual ~C_ConnectionException(){};
	};

	/**
	 * @brief Exception related to socket connection
	 * \ingroup MiaDriverException
	 */
	class C_SocketException : public C_Exception
	{
		public: C_SocketException(
			const C_ErrorLocationData     &errorLocation,
			const std::string 				&socketErrorMessage,
			const C_Variant            	&p1 = C_Variant(),
			const C_Variant            	&p2 = C_Variant(),
			const C_Variant            	&p3 = C_Variant(),
			const C_Variant            	&p4 = C_Variant(),
			const C_Variant            	&p5 = C_Variant(),
			const C_Variant            	&p6 = C_Variant(),
			const C_Variant            	&p7 = C_Variant(),
			const C_Variant            	&p8 = C_Variant()) :
			C_Exception(errorLocation, C_BaseErrorMessage(g_MiaError_SocketError, socketErrorMessage), p1, p2, p3, p4, p5, p6, p7, p8)
		{
			m_Message = ("C_SocketException");
		}

		public: virtual ~C_SocketException(){};
	};

	/**
	 * @brief Unable to authenticate the client
	 * \ingroup MiaDriverException
	 */
	class C_UnauthorizedException : public C_Exception
	{
		public: C_UnauthorizedException(
			const C_ErrorLocationData     &errorLocation,
			const C_Variant            	&p1 = C_Variant(),
			const C_Variant            	&p2 = C_Variant(),
			const C_Variant            	&p3 = C_Variant(),
			const C_Variant            	&p4 = C_Variant(),
			const C_Variant            	&p5 = C_Variant(),
			const C_Variant            	&p6 = C_Variant(),
			const C_Variant            	&p7 = C_Variant(),
			const C_Variant            	&p8 = C_Variant()) :
			C_Exception(errorLocation, C_BaseErrorMessage(G_ERROR_UNAUTHORIZED), p1, p2, p3, p4, p5, p6, p7, p8)
		{
			m_Message = ("C_UnauthorizedException");
		}

		public: virtual ~C_UnauthorizedException(){};
	};

	/**
	 * @brief Given connectionstring is invalid
	 * \ingroup MiaDriverException
	 */
	class C_BadConnectionStringException : public C_Exception
	{
		public: C_BadConnectionStringException(
			const C_ErrorLocationData     &errorLocation,
			const std::string 				&connectionString,
			const C_Variant            	&p1 = C_Variant(),
			const C_Variant            	&p2 = C_Variant(),
			const C_Variant            	&p3 = C_Variant(),
			const C_Variant            	&p4 = C_Variant(),
			const C_Variant            	&p5 = C_Variant(),
			const C_Variant            	&p6 = C_Variant(),
			const C_Variant            	&p7 = C_Variant(),
			const C_Variant            	&p8 = C_Variant()) :
			C_Exception(errorLocation, C_BaseErrorMessage(G_ERROR_INVALID_CONNECTION_STRING, connectionString), p1, p2, p3, p4, p5, p6, p7, p8)
		{
			m_Message = "C_BadConnectionStringException, " + connectionString + ". " ;
		}

		public: virtual ~C_BadConnectionStringException(){};
	};

	/**
	 * @brief Given connectionstring is invalid
	 * \ingroup MiaDriverException
	 */
	class C_InvalidVariantTypeException : public C_Exception
	{
		public: C_InvalidVariantTypeException(
			const C_ErrorLocationData     &errorLocation,
			const C_Variant            	&p1 = C_Variant(),
			const C_Variant            	&p2 = C_Variant(),
			const C_Variant            	&p3 = C_Variant(),
			const C_Variant            	&p4 = C_Variant(),
			const C_Variant            	&p5 = C_Variant(),
			const C_Variant            	&p6 = C_Variant(),
			const C_Variant            	&p7 = C_Variant(),
			const C_Variant            	&p8 = C_Variant()) :
			C_Exception(errorLocation, C_BaseErrorMessage(G_ERROR_INVALID_VARIANT_TYPE),p1, p2, p3, p4, p5, p6, p7, p8)
		{
			m_Message = "C_InvalidVariantTypeException " ;
		}

		public: virtual ~C_InvalidVariantTypeException(){};
	};

	/**
	 * @brief Given connectionstring is invalid
	 * \ingroup MiaDriverException
	 */
	class C_InvalidJsonFormatException : public C_Exception
	{
		public: C_InvalidJsonFormatException(
			const C_ErrorLocationData     &errorLocation,
			const std::string 				&message,
			const C_Variant            	&p1 = C_Variant(),
			const C_Variant            	&p2 = C_Variant(),
			const C_Variant            	&p3 = C_Variant(),
			const C_Variant            	&p4 = C_Variant(),
			const C_Variant            	&p5 = C_Variant(),
			const C_Variant            	&p6 = C_Variant(),
			const C_Variant            	&p7 = C_Variant(),
			const C_Variant            	&p8 = C_Variant()) :
			C_Exception(errorLocation, C_BaseErrorMessage(G_ERROR_INVALID_JSON_FORMAT, message),p1, p2, p3, p4, p5, p6, p7, p8)
		{
			m_Message = "C_InvalidJsonFormatException: " + message ;
		}

		public: virtual ~C_InvalidJsonFormatException(){};
	};

	/**
	 * @brief Unable to compelete the request within the timout.
	 * \ingroup MiaDriverException
	 */
	class C_TimeoutException : public C_Exception
	{
		public: C_TimeoutException(
			const C_ErrorLocationData     &errorLocation,
			const C_Variant            	&p1 = C_Variant(),
			const C_Variant            	&p2 = C_Variant(),
			const C_Variant            	&p3 = C_Variant(),
			const C_Variant            	&p4 = C_Variant(),
			const C_Variant            	&p5 = C_Variant(),
			const C_Variant            	&p6 = C_Variant(),
			const C_Variant            	&p7 = C_Variant(),
			const C_Variant            	&p8 = C_Variant()) :
			C_Exception(errorLocation, C_BaseErrorMessage(g_MiaError_Timeout), p1, p2, p3, p4, p5, p6, p7, p8)
		{
			m_Message = "C_TimeoutException";
		}

		public: virtual ~C_TimeoutException(){};
	};

	/**
	 * @brief Given class is not found
	 * \ingroup MiaDriverException
	 */
	class C_ClassNotFoundException : public C_Exception
	{
		public: C_ClassNotFoundException(
				const C_ErrorLocationData     &errorLocation,
				const C_Variant            	&p1 = C_Variant(),
				const C_Variant            	&p2 = C_Variant(),
				const C_Variant            	&p3 = C_Variant(),
				const C_Variant            	&p4 = C_Variant(),
				const C_Variant            	&p5 = C_Variant(),
				const C_Variant            	&p6 = C_Variant(),
				const C_Variant            	&p7 = C_Variant(),
				const C_Variant            	&p8 = C_Variant()) :
				C_Exception(errorLocation, C_BaseErrorMessage(g_MiaError_ClassNotFound), p1, p2, p3, p4, p5, p6, p7, p8)
		{
			m_Message = "C_ClassNotFoundException ";
		}

		public: virtual ~C_ClassNotFoundException(){};
	};

	/**
	* @brief Given class is not found
	* \ingroup MiaDriverException
	*/
	class C_DevicePropertyAlreadyUsedException : public C_Exception
	{
	public: C_DevicePropertyAlreadyUsedException(
		const C_ErrorLocationData     &errorLocation,
		const C_Variant            	&p1 = C_Variant(),
		const C_Variant            	&p2 = C_Variant(),
		const C_Variant            	&p3 = C_Variant(),
		const C_Variant            	&p4 = C_Variant(),
		const C_Variant            	&p5 = C_Variant(),
		const C_Variant            	&p6 = C_Variant(),
		const C_Variant            	&p7 = C_Variant(),
		const C_Variant            	&p8 = C_Variant()) :
		C_Exception(errorLocation, C_BaseErrorMessage(g_MiaError_ClassNotFound), p1, p2, p3, p4, p5, p6, p7, p8)
	{
		m_Message = "C_DevicePropertyAlreadyUsedException ";
	}

	public: virtual ~C_DevicePropertyAlreadyUsedException() {};
	};

	/**
	 * @brief Unknown error has happened.
	 * \ingroup MiaDriverException
	 */
	class C_UnknownException : public C_Exception
	{
		public: C_UnknownException(
			const C_ErrorLocationData     &errorLocation,
			int 									errorCode,
			const C_Variant            	&p1 = C_Variant(),
			const C_Variant            	&p2 = C_Variant(),
			const C_Variant            	&p3 = C_Variant(),
			const C_Variant            	&p4 = C_Variant(),
			const C_Variant            	&p5 = C_Variant(),
			const C_Variant            	&p6 = C_Variant(),
			const C_Variant            	&p7 = C_Variant(),
			const C_Variant            	&p8 = C_Variant()) :
			C_Exception(errorLocation, C_BaseErrorMessage(errorCode), p1, p2, p3, p4, p5, p6, p7, p8)
		{
			m_Message = "C_UnknownException";
		}

		public: virtual ~C_UnknownException(){};
	};
}
}


#ifndef CSCommon_INCLUDE_LIST
#define CSCommon_INCLUDE_LIST 1
// Includes list only for source code backward compatibility
#include <list>
#endif



#endif
