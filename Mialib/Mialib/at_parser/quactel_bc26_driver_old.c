#include "modem_driver.h"
#include "os_support.h"
#include <string.h>

#define MAX_SEND 110
#define MAX_RECEIVE 110

#define MODULE_NAME "bc26_driver_old"

#define DUMP_DATA 0

typedef enum { CERTIFICATE_NOT_CHECKED = 0, CERTIFICATE_SAME, CERTIFICATE_NOT_SAME } CaCertificate;

typedef enum { MODEM_INIT = 0, MODEM_ERROR, MODEM_DATA } ModemState;

typedef struct quactel_modem {

	char         *receive_data;
	int           receive_len;
	const char   *send_data;
	int           send_len;
	CaCertificate caCerficateOk;
	ModemState    state;
	char          connected;
	int           socketId;

} quactel_modem;

static void  gen_result_hadler( const struct AT_Handler * parent, const char * result)
{
	network_modem *modem = (network_modem*)(parent->modem);

	if(strstr(result, "ERROR")) {
		quactel_modem *quactel = (quactel_modem*)(modem->driver);

		MAKE_ERROR(modem, ERROR_RESULT, "ERROR found from AT command!\n");
		quactel->state = MODEM_ERROR;
	}

	modem->cmdDone = 1;
}

static void ignorant_result_hadler( const struct AT_Handler * parent, const char * result)
{
	network_modem *modem = (network_modem*)(parent->modem);

	DBG_PR("Ignored result result: |%s|\n", result);

	modem->cmdDone = 1;
}


static void gen_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem   = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	DBG_PR("cmd_at responce: |%s|\n", responce);
	ERR_PR("ERROR no responce should come from AT command %s\n", parent->cmd->cmd);

	MAKE_ERROR(modem, ERROR_RESPONCE, "%s RESPONCE not expected: %s\n", parent->cmd->cmd, responce);

	quactel->state = MODEM_ERROR;
}

static void ignorant_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem   = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	DBG_PR("cmd_at responce: |%s|\n", responce);

	quactel->state = MODEM_ERROR;
}


CREATE_ATCommand(cmd_AT, "AT", gen_result_hadler, gen_responce_hadler);

CREATE_ATCommand(cmd_AT_CGDCONT_REQ, "AT+CGDCONT?", ignorant_result_hadler, ignorant_responce_hadler);

//CREATE_ATCommand(cmd_AT_CGDCONT, "AT+CGDCONT=1", ignorant_result_hadler, gen_responce_hadler);

CREATE_ATCommand(cmd_ATV1, "ATV1", gen_result_hadler, gen_responce_hadler);

CREATE_ATCommand(cmd_AT_IFC, "AT+IFC=0,0", gen_result_hadler, gen_responce_hadler);


static void AT_APN_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);

	char buffer[180];
	snprintf(buffer, ELEMENTS(buffer), "AT+QGACT=1,1,\"%s\",\"%s\",\"%s\"", modem->user_apn, modem->user_name, modem->user_password);
	buffer[ELEMENTS(buffer) - 1] = 0;
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static const AT_Command cmd_AT_APN = { "AT+QGACT=1,1,", AT_APN_cmd, 0, gen_result_hadler, ignorant_responce_hadler, 0, NULL };

CREATE_ATCommand(cmd_AT_APN_DEACTIVATE, "AT+QGACT=0,1", ignorant_result_hadler, ignorant_responce_hadler);

CREATE_ATCommand(cmd_AT_RESET, "AT+QRESET=1", ignorant_result_hadler, ignorant_responce_hadler);

static const char * forwardCommas(const char *text, unsigned int how_many)
{
	while(*text && text)
	{
		if(!how_many)
			return text;

		if(*text == ',')
			how_many--;

		text++;
	}

	return NULL;
}

static void MICCID_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);
	const char *letter;

	DBG_PR("MICCID responce is |%s|\n", responce);

	//*MICCID: 89860615090017648014

	letter = strstr(responce, "*MICCID: ");
	if(!letter)
	{
		return;
	}
	letter += 9;

	strncpy(modem->iccd, letter, ELEMENTS(modem->iccd));

	modem->iccd[ELEMENTS(modem->iccd) - 1] = 0;
}

CREATE_ATCommand(cmd_AT_MICCID, "AT*MICCID", gen_result_hadler, MICCID_responce_hadler);

static void QSOC_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);
	int socketId;

	// +QSOC=0

	DBG_PR("QSOC responce is |%s|\n", responce);

	if(sscanf(responce,"+QSOC=%d", &socketId) == 1)
	{
		quactel->socketId = socketId;
	}
}


CREATE_ATCommand(cmd_AT_QSOC, "AT+QSOC=1,1,1", gen_result_hadler, QSOC_responce_hadler);

static void QSOCON_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	char buffer[200];
	snprintf(buffer, ELEMENTS(buffer), "AT+QSOCON=%d,%u,\"%s\"",
		quactel->socketId,
		modem->server_port,
		modem->server_addr
		);
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static const AT_Command cmd_AT_QSOCON = { "AT+QSOCON=", QSOCON_cmd, 0, gen_result_hadler, gen_responce_hadler, 0, NULL };

static void QSODIS_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	char buffer[200];
	snprintf(buffer, ELEMENTS(buffer), "AT+QSODIS=%d",
		quactel->socketId
	);

	parent->write_AT_cmd(buffer, strlen(buffer));
}
static const AT_Command cmd_AT_QSODIS = { "AT+QSODIS=", QSODIS_cmd, 0, ignorant_result_hadler, gen_responce_hadler, 0, NULL };



static void QSOSEND_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	char buffer[256];
	char *printer = buffer;
	int send = 0;

	sprintf(buffer, "AT+QSOSEND=%d,%d,",quactel->socketId, quactel->send_len);

	parent->write_AT_cmd(buffer, strlen(buffer));

	while(send < quactel->send_len)
	{
		int i;
		int chunk = MIN(64, quactel->send_len - send);
		printer = buffer;
		for(i = 0; i < chunk; i++)
		{
			const unsigned char * value = (const unsigned char *)&quactel->send_data[send + i];
			printer += sprintf(printer, "%02X", *value);
		}
		parent->write_AT_cmd(buffer, chunk * 2);
		send += chunk;
	}
}

static const AT_Command cmd_AT_QSOSEND = { "AT+QSOSEND=", QSOSEND_cmd, 0, gen_result_hadler, gen_responce_hadler, 0, NULL };

static void  QSORCV_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	unsigned int len = 0;
	unsigned int read = 0;
	//unsigned int socket = 0;
	//int timeout = 1000;
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);
	char search_string[20];

	sprintf(search_string, "%d,", quactel->socketId);

#if 0
AT+QSORF=0,5

0,40.125.172.173,443,5,160303086a,507

OK

AT+QSORF=0,110

data_buf is NUll

ERROR

#endif

	if(strstr(responce, "data_buf is NUll") == responce)
	{
		quactel->receive_len = 0;
	}
	else if(strstr(responce, search_string) == responce)
	{

		const char * dataStart = forwardCommas(responce, 3);

		DBG_PR("QSORF responce is |%s|\n", responce);

		if(sscanf(dataStart,"%u,", &len) != 1)
		{
			quactel->receive_len = -56;
			return;
		}

		dataStart = forwardCommas(dataStart, 1);

		while(read < len) {
			char tmp[3] = { 0 };
			unsigned int data;
			tmp[0] = dataStart[read*2];
			tmp[1] = dataStart[read*2+1];
			if(1 != sscanf(tmp,"%x", &data))
			{
				MAKE_ERROR(modem, ERROR_RESPONCE, "%s RESPONCE parse failed: %s\n", parent->cmd->cmd, responce);
				quactel->receive_len = -16;
				return;
			}
			quactel->receive_data[read++] = data;
		}
		quactel->receive_len = read;
	}
	else if(strlen(responce) > 5)
	{
		quactel->receive_len = -12;
	}
}

static void  QSORCV_result_hadler( const struct AT_Handler * parent, const char * result)
{
	network_modem *modem = (network_modem*)(parent->modem);
/*
	if(strstr(result, "ERROR")) {
		//quactel_modem *quactel = (quactel_modem*)(modem->driver);

		//quactel->receive_len = 0;
	}
*/
	modem->cmdDone = 1;
}

static void QSORCV_cmd(const struct AT_Handler * parent)
{
	char buffer[80];
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	sprintf(buffer, "AT+QSORF=%d,%d",quactel->socketId, quactel->receive_len);
	parent->write_AT_cmd(buffer, strlen(buffer));
	quactel->receive_len = 0;
}

static const AT_Command cmd_AT_QIRD = { "AT+QSORF=,", QSORCV_cmd, 0, QSORCV_result_hadler, QSORCV_responce_hadler, 0, NULL };


#define doIt(name, timeout) modem_do_command(This, &name, timeout)

#define doIt_ret_if_error(name) if(doIt(name, 100) <= 0 || This->error) \
	do{ quactel->state = MODEM_ERROR; return -1;}while(0)

#define doIt_long_ret_if_error(name) if(doIt(name, 4000) <= 0 || This->error) \
	do{ quactel->state = MODEM_ERROR; return -1;}while(0)


static int setup_connection(network_modem * This)
{
	int j;
	network_modem *modem = This;
	quactel_modem *quactel = (quactel_modem*)(This->driver);

	LIGHT_DBG_PR("Setup connection\n");

	quactel->socketId = -1;
	doIt(cmd_AT_QSOC, 100);

	if(quactel->socketId < 0)
	{
		int i;
		MAKE_ERROR(modem, ERROR_CMD_FAILED, "Cannot create socket\n");

		for( i = 0; i < 4; i++)
		{
			quactel->socketId = i;
			doIt(cmd_AT_QSODIS, 100);
			This->error = 0;
		}
		quactel->socketId = -1;
		doIt_ret_if_error(cmd_AT_QSOC);
		if(quactel->socketId < 0)
		{
			return -7;
		}
	}


	quactel->connected = 0;
	for(j = 0; j < 3 && quactel->connected != 1; j++)
	{
		if(modem->crypted)
		{
			MAKE_ERROR(modem, ERROR_CMD_FAILED, "crypted connection is not supported\n");
			return -2;
		}
		else
		{
			doIt_long_ret_if_error(cmd_AT_QSOCON);
			doIt_ret_if_error(cmd_AT_CGDCONT_REQ);
			quactel->connected = 1;
		}
	}

	quactel->state = MODEM_DATA;

	return 0;
}

static int make_disconnect(network_modem * This)
{
	quactel_modem *quactel = (quactel_modem*)(This->driver);

	LIGHT_DBG_PR("Reboot modem\n");

	quactel->state = MODEM_INIT;

	doIt(cmd_AT_RESET, 100);

	return 0;
}


static inline int init(network_modem * This)
{
	if (!This)
	{
		return -1;
	}

	network_modem *modem = This;
	static quactel_modem quactel_mem; // Only singel modem supported
	int waitWakeup = 20;
	quactel_modem *quactel = &quactel_mem;

	modem->driver = &quactel_mem;

	memset(&quactel_mem, 0, sizeof(quactel_mem));

	LIGHT_DBG_PR("Modem init\n");

	while (1)
	{
		doIt(cmd_AT, 100);

		if (!This->error)
			break;

		This->error = 0;

		if (waitWakeup-- < 0)
			doIt_ret_if_error(cmd_AT);
	}

	doIt_ret_if_error(cmd_AT_APN_DEACTIVATE);
	doIt_ret_if_error(cmd_AT_APN);


	doIt(cmd_AT_MICCID, 100);
	This->error = 0;

	if (This->modem_wakeup(This))
	{
		This->error = 0;
		doIt_ret_if_error(cmd_AT_IFC);
		doIt_ret_if_error(cmd_ATV1);

		doIt_ret_if_error(cmd_AT_APN);
		//doIt_ret_if_error(cmd_AT_CGDCONT);

		This->error = ERROR_CREG_REGISTERING_FAILED;

		return -1;
	}

	doIt_ret_if_error(cmd_AT_MICCID);

	//doIt_ret_if_error(cmd_AT_CGDCONT);

	doIt_ret_if_error(cmd_AT_CGDCONT_REQ);

	quactel_mem.state = MODEM_INIT;

	return 0;
}


static inline int m_write(network_modem * This, const char *buffer, unsigned int len)
{
	unsigned int send      = 0;
	network_modem *modem   = This;
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	if(quactel->state != MODEM_DATA)
	{
		return -1;
	}

	while(send < len)
	{
		int i;
		int required = len - send;
		int transfer = MIN(required, MAX_SEND);

		quactel->send_data = &buffer[send];
		quactel->send_len  = transfer;

		for( i = 0; i < 3; i++)
		{
			quactel->send_data = &buffer[send];
			quactel->send_len  = transfer;

			This->error = 0;
			modem->error_str[0] = 0;

			if(modem->crypted)
			{
				MAKE_ERROR(modem, ERROR_CMD_FAILED, "crypted connection is not supported\n");
				return -5;
			}
			else
			{
				doIt(cmd_AT_QSOSEND, 2000);
			}

			if(!This->error)
			{
				break;
			}
			else
			{
				SLEEP(1000);
			}
		}

		if(This->error)
		{
			quactel->state = MODEM_ERROR;
			return -20;
		}

		send += transfer;
	}

	if(quactel->send_len < 0)
		quactel->state = MODEM_ERROR;
	else
		quactel->state = MODEM_DATA;

	return send;
}

static inline int m_read(network_modem * This, char *buffer, unsigned int len)
{
	network_modem *modem = This;
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	len = MIN(MAX_SEND, len);

	if(quactel->state != MODEM_DATA)
		return -1;

	if(!len)
		return 0;

	quactel->receive_data = buffer;
	quactel->receive_len  = MIN(len, MAX_RECEIVE);

	if(modem->crypted)
	{
		MAKE_ERROR(modem, ERROR_CMD_FAILED, "crypted connection is not supported\n");
	}
	else
	{
		//doIt_long_ret_if_error(cmd_AT_QIRD);
		doIt(cmd_AT_QIRD, 100);
		if(This->error)
		{
			This->error = 0;
			return 0;
		}
	}

	if(quactel->receive_len < 0)
		quactel->state = MODEM_ERROR;
	else
		quactel->state = MODEM_DATA;

	return quactel->receive_len;
}

static inline int unsolicited(network_modem *This, const char * responce)
{
	if (!This || !responce)
	{
		return 0;
	}

	network_modem *modem = This;
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	if (strstr(responce, "NO CARRIER") || strstr(responce, "DISCONNECTED")) {

		DBG_PR("unsolicited: %s\n", responce);

		quactel->connected = 2;

		DBG_PR("Server disconnected\n");

		quactel->state = MODEM_ERROR;
		return 1;
	}

	if (!strstr(responce, "+QSONMI")) {
		return 0;
	}

	DBG_PR("unsolicited: %s\n", responce);

	return 1;
}

static inline int m_connect(network_modem *This, int discon)
{
	network_modem *modem = This;
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	if(discon)
	{
		return make_disconnect(This);
	}
	else
	{
		quactel->state = MODEM_INIT;

		return setup_connection(This);
	}
}


#ifndef USE_QUACTEL_BC26_DRIVER_OLD
#define USE_QUACTEL_BC26_DRIVER_OLD 0
#endif

#if USE_QUACTEL_BC26_DRIVER_OLD
int impl_modem_init(network_modem *This) { return init(This); }
int impl_modem_write(network_modem *This, const char *buffer, int len) { return m_write(This, buffer, len); }
int impl_modem_read(network_modem *This, char *buffer, int len) { return m_read(This, buffer, len); }
int impl_modem_unsolicited(network_modem *This, const char * responce) { return unsolicited(This, responce); }
int impl_modem_connect(network_modem *This, int disconnect) { return m_connect(This, disconnect); };
#else
void avoid_warnings_quactelBC26_old___(void) // Fake call to make compiler happy
{
	if((USE_QUACTEL_BC26_DRIVER_OLD))
	{
		init(NULL);
		m_read(NULL, NULL, 0);
		m_write(NULL, NULL, 0);
		unsolicited(NULL, NULL);
		m_connect(NULL, 0);
	}
}
#endif
