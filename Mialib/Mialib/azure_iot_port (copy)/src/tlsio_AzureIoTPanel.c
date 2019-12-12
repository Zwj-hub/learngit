/*! @addtogroup azure iot port
 *  @{
 */
/******************************************************************************
*                                                                             *
*                   COPYRIGHT (C) 2018    ABB Beijing, DRIVES                 *
*                                                                             *
*******************************************************************************
*                                                                             *
*                                                                             *
*                     Daisy Assistant Panel SW                                *
*                                                                             *
*                                                                             *
*                  Subsystem:   azure iot port                                *
*                                                                             *
*                   Filename:   tlsio_AzureIoTPanel.c                         *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file
 *  @par File description:
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include "tlsio_AzureIoTPanel.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/optionhandler.h"
#include "azure_c_shared_utility/tlsio_options.h"
#include "azure_c_shared_utility/macro_utils.h"

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio.h"

#include "WS_Socket.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------
/** Expose tlsio state for test proposes.
*/
#define TLSIO_MIA_STATE_VALUES \
	TLSIO_MIA_STATE_CLOSED,    \
	TLSIO_MIA_STATE_OPENING,   \
	TLSIO_MIA_STATE_OPEN,      \
	TLSIO_MIA_STATE_CLOSING,   \
	TLSIO_MIA_STATE_ERROR,     \
	TLSIO_MIA_STATE_NULL
DEFINE_ENUM(TLSIO_MIA_STATE, TLSIO_MIA_STATE_VALUES);

#define MAX_TLS_OPENING_RETRY 10
#define MAX_TLS_CLOSING_RETRY 10
#define RECEIVE_BUFFER_SIZE 256

#define CallErrorCallback() do { if (tlsio_instance->cb.on_io_error != NULL) (void)tlsio_instance->cb.on_io_error(tlsio_instance->cb.on_io_error_context); } while((void)0,0)
#define CallOpenCallback(status) do { if (tlsio_instance->cb.on_io_open_complete != NULL) (void)tlsio_instance->cb.on_io_open_complete(tlsio_instance->cb.on_io_open_complete_context, status); } while((void)0,0)
#define CallCloseCallback() do { if (tlsio_instance->cb.on_io_close_complete != NULL) (void)tlsio_instance->cb.on_io_close_complete(tlsio_instance->cb.on_io_close_complete_context); } while((void)0,0)
#define CON_IS_OPEN(a) (((a)->socket) && (a)->socketInterface && ((a)->socketInterface->M_Socket_IsReady((a)->socket) == 3))


//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------
static CONCRETE_IO_HANDLE tlsio_create(void* io_create_parameters);
static void tlsio_destroy(CONCRETE_IO_HANDLE tls_io);
static int tlsio_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context);
static int tlsio_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
static int tlsio_send(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
static void tlsio_dowork(CONCRETE_IO_HANDLE tls_io);
static int tlsio_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value);
static OPTIONHANDLER_HANDLE tlsio_retrieveoptions(CONCRETE_IO_HANDLE tls_io);

static const IO_INTERFACE_DESCRIPTION tlsio_mia_interface_description =
{
	tlsio_retrieveoptions,
	tlsio_create,
	tlsio_destroy,
	tlsio_open,
	tlsio_close,
	tlsio_send,
	tlsio_dowork,
	tlsio_setoption
};

typedef struct MiaTLSCB
{
	ON_IO_OPEN_COMPLETE on_io_open_complete;
	void* on_io_open_complete_context;

	ON_BYTES_RECEIVED on_bytes_received;
	void* on_bytes_received_context;

	ON_IO_ERROR on_io_error;
	void* on_io_error_context;

	ON_IO_CLOSE_COMPLETE on_io_close_complete;
	void* on_io_close_complete_context;

}MiaTLSCB;

typedef struct MiaTLS_tag
{
	MiaTLSCB cb;
	uint16_t port;
	char host[128];
	TLSIO_MIA_STATE state;
	int countTry;
	TLSIO_OPTIONS options;
	uint8_t	   reserved;
	t_ISocket	 socketInterface;
	void*		 socket;
} MiaTLS;
static MiaTLS miaTls;

//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------
static CONCRETE_IO_HANDLE tlsio_create(void* io_create_parameters)
{
	MiaTLS* tlsio_instance = &miaTls;
	TLSIO_CONFIG* tlsio_config = (TLSIO_CONFIG*)io_create_parameters;

	if (io_create_parameters == NULL)
	{
		LogError("Invalid TLS parameters.");
		return NULL;
	}

	if (miaTls.reserved)
	{
		LogError("No free TLS context");
		return NULL;
	}

	memset(tlsio_instance, 0, sizeof(*tlsio_instance));

	tlsio_instance->socketInterface = G_Socket_CreateSecure();
	//tlsio_instance->socketInterface = G_Socket_CreateUnsecure();

	if (!tlsio_instance->socketInterface)
	{
		return NULL;
	}

	tlsio_instance->socket = tlsio_instance->socketInterface->M_Socket_New((char*)tlsio_config->hostname, tlsio_config->port);

	if (!tlsio_instance->socket)
	{
		return NULL;
	}

	tlsio_instance->reserved = 1;

	//tlsio_options_initialize(&tlsio_instance->options, TLSIO_OPTION_BIT_NONE);
	//tlsio_options_initialize(&tlsio_instance->options, TLSIO_OPTION_BIT_TRUSTED_CERTS);
	tlsio_options_initialize(&tlsio_instance->options, TLSIO_OPTION_BIT_x509_ECC_CERT);
	//tlsio_options_initialize(&tlsio_instance->options, TLSIO_OPTION_BIT_x509_RSA_CERT);

	tlsio_instance->state = TLSIO_MIA_STATE_CLOSED;

	return (CONCRETE_IO_HANDLE)tlsio_instance;
}


static void tlsio_destroy(CONCRETE_IO_HANDLE tlsio_handle)
{
	MiaTLS* tlsio_instance = (MiaTLS*)tlsio_handle;

	if (tlsio_handle == NULL)
	{
		LogError("NULL TLS handle.");
		return;
	}

	if ((tlsio_instance->state == TLSIO_MIA_STATE_OPENING) ||
		(tlsio_instance->state == TLSIO_MIA_STATE_OPEN) ||
		(tlsio_instance->state == TLSIO_MIA_STATE_CLOSING))
	{
		LogError("TLS destroyed with a SSL connection still active.");
	}

	tlsio_options_release_resources(&tlsio_instance->options);

	tlsio_instance->socketInterface->M_Socket_Free(&tlsio_instance->socket);

	tlsio_instance->state = TLSIO_MIA_STATE_CLOSED;

	miaTls.reserved = 0;
}

static int tlsio_open(
	CONCRETE_IO_HANDLE tlsio_handle,
	ON_IO_OPEN_COMPLETE on_io_open_complete,
	void* on_io_open_complete_context,
	ON_BYTES_RECEIVED on_bytes_received,
	void* on_bytes_received_context,
	ON_IO_ERROR on_io_error,
	void* on_io_error_context)
{
	int result;
	MiaTLS* tlsio_instance = (MiaTLS*)tlsio_handle;

	if (tlsio_handle == NULL)
	{
		LogError("NULL TLS handle.");
		return __FAILURE__;
	}


	tlsio_instance->cb.on_io_open_complete = on_io_open_complete;
	tlsio_instance->cb.on_io_open_complete_context = on_io_open_complete_context;

	tlsio_instance->cb.on_bytes_received = on_bytes_received;
	tlsio_instance->cb.on_bytes_received_context = on_bytes_received_context;

	tlsio_instance->cb.on_io_error = on_io_error;
	tlsio_instance->cb.on_io_error_context = on_io_error_context;

	if ((tlsio_instance->state == TLSIO_MIA_STATE_ERROR) ||
		(tlsio_instance->state == TLSIO_MIA_STATE_OPENING) ||
		(tlsio_instance->state == TLSIO_MIA_STATE_OPEN) ||
		(tlsio_instance->state == TLSIO_MIA_STATE_CLOSING))
	{
		LogError("Try to open a connection with an already opened TLS.");
		return __FAILURE__;
	}

	result = tlsio_instance->socketInterface->M_Socket_Connect(tlsio_instance->socket);

	if (result != 0)
	{
		tlsio_instance->state = TLSIO_MIA_STATE_ERROR;

		if (on_io_open_complete != NULL)
		{
			(void)on_io_open_complete(on_io_open_complete_context, IO_OPEN_ERROR);
		}
		if (on_io_error != NULL)
		{
			(void)on_io_error(on_io_error_context);
		}
	}
	else
	{
		tlsio_instance->state = TLSIO_MIA_STATE_OPENING;
	}

	tlsio_dowork(tlsio_handle);

	return result;
}

static int tlsio_close(CONCRETE_IO_HANDLE tlsio_handle, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* on_io_close_complete_context)
{
	int result;
	MiaTLS* tlsio_instance = (MiaTLS*)tlsio_handle;

	if (tlsio_handle == NULL)
	{
		LogError("NULL TLS handle.");
		return __FAILURE__;
	}

	tlsio_instance->cb.on_io_close_complete = on_io_close_complete;
	tlsio_instance->cb.on_io_close_complete_context = on_io_close_complete_context;

	if ((tlsio_instance->state == TLSIO_MIA_STATE_CLOSED) || (tlsio_instance->state == TLSIO_MIA_STATE_ERROR))
	{
		tlsio_instance->state = TLSIO_MIA_STATE_CLOSED;
		result = 0;
	}
	else if ((tlsio_instance->state == TLSIO_MIA_STATE_OPENING) || (tlsio_instance->state == TLSIO_MIA_STATE_CLOSING))
	{
		LogError("Try to close the connection with an already closed TLS.");
		tlsio_instance->state = TLSIO_MIA_STATE_ERROR;
		result = __FAILURE__;
	}
	else
	{
		result = tlsio_instance->socketInterface->M_Socket_Free(&tlsio_instance->socket);
		tlsio_instance->state = TLSIO_MIA_STATE_CLOSED;
		result = 0;
	}

	return result;
}

static int tlsio_send(CONCRETE_IO_HANDLE tlsio_handle, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
	int result;
	MiaTLS* tlsio_instance = (MiaTLS*)tlsio_handle;

	if (!size)
		return 0;

	if ((tlsio_handle == NULL) || (buffer == NULL))
	{
		LogError("Invalid parameter");
		return __FAILURE__;
	}

	if (tlsio_instance->state != TLSIO_MIA_STATE_OPEN || !tlsio_instance->socket)
	{
		LogError("TLS is not ready to send data");
		return __FAILURE__;
	}

	result = tlsio_instance->socketInterface->M_Socket_Write(tlsio_instance->socket, buffer, size);

	if (result < 1)
	{
		tlsio_instance->state = TLSIO_MIA_STATE_ERROR;
		LogError("TLS send fail");
		CallErrorCallback();
		return __FAILURE__;
	}

	if (on_send_complete != NULL)
	{
		on_send_complete(callback_context, IO_SEND_OK);
	}

	return 0;
}

static void tlsio_dowork(CONCRETE_IO_HANDLE tlsio_handle)
{
	int received;
	char RecvBuffer[RECEIVE_BUFFER_SIZE];

	MiaTLS* tlsio_instance = (MiaTLS*)tlsio_handle;

	if (tlsio_handle == NULL)
	{
		LogError("Invalid parameter");
		return;
	}

	switch (tlsio_instance->state)
	{
	case TLSIO_MIA_STATE_OPENING:
		if (CON_IS_OPEN(tlsio_instance))
		{
			tlsio_instance->state = TLSIO_MIA_STATE_OPEN;
			CallOpenCallback(IO_OPEN_OK);
		}
		else
		{
			tlsio_instance->state = TLSIO_MIA_STATE_ERROR;
			LogError("Timeout for TLS connect.");
			CallOpenCallback(IO_OPEN_CANCELLED);
			CallErrorCallback();
		}
		break;

	case TLSIO_MIA_STATE_OPEN:
		if (!CON_IS_OPEN(tlsio_instance))
		{
			tlsio_instance->state = TLSIO_MIA_STATE_ERROR;
			LogError("SSL closed the connection.");
			CallErrorCallback();
		}
		else
		{
			while(tlsio_instance->socket)
			{
				received = tlsio_instance->socketInterface->M_Socket_Read(tlsio_instance->socket, RecvBuffer, sizeof(RecvBuffer));

				if (received > 0 && tlsio_instance->cb.on_bytes_received != NULL)
				{
					// explictly ignoring here the result of the callback
					(void)tlsio_instance->cb.on_bytes_received(tlsio_instance->cb.on_bytes_received_context, (const unsigned char*)RecvBuffer, received);
				}
				else if (received < 0)
				{
					tlsio_instance->state = TLSIO_MIA_STATE_ERROR;
					LogError("Socket Read failure");
					CallErrorCallback();
				}
				else if (!received)
				{
					break;
				}
			}
		}
		break;
	case TLSIO_MIA_STATE_CLOSING:
		if (!CON_IS_OPEN(tlsio_instance))
		{
			tlsio_instance->state = TLSIO_MIA_STATE_CLOSED;
			CallCloseCallback();
		}
		else
		{
			tlsio_instance->state = TLSIO_MIA_STATE_ERROR;
			LogError("Timeout for close TLS");
			CallErrorCallback();
		}
		break;
	case TLSIO_MIA_STATE_CLOSED:
		;
	case TLSIO_MIA_STATE_ERROR:
		;
	default:
		break;
	}

}

static int tlsio_setoption(CONCRETE_IO_HANDLE tlsio_handle, const char* optionName, const void* value)
{
	MiaTLS* tlsio_instance = (MiaTLS*)tlsio_handle;
	int result;

	if (tlsio_instance == NULL)
	{
		LogError("NULL tlsio");
		return __FAILURE__;
	}

	TLSIO_OPTIONS_RESULT options_result = tlsio_options_set(&tlsio_instance->options, optionName, value);

	if (options_result != TLSIO_OPTIONS_RESULT_SUCCESS)
	{
		LogError("Failed tlsio_options_set");
		result = __FAILURE__;
	}
	else
	{
		result = 0;
	}

	return result;
}

static OPTIONHANDLER_HANDLE tlsio_retrieveoptions(CONCRETE_IO_HANDLE tlsio_handle)
{
	MiaTLS* tlsio_instance = (MiaTLS*)tlsio_handle;

	//OPTIONHANDLER_HANDLE result;
	if (tlsio_instance == NULL)
	{
		LogError("NULL tlsio");
		return NULL;
	}

	return tlsio_options_retrieve_options(&tlsio_instance->options, tlsio_setoption);
}

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------
const IO_INTERFACE_DESCRIPTION* tlsio_mia_get_interface_description(void)
{
	return &tlsio_mia_interface_description;
}
/*! @} */ /* EOF, no more */
