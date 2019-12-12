#include "modem_driver.h"
#include "os_support.h"
#include <string.h>
#include "build_configuration.h"
#include <unistd.h>
#define MAX_SEND    450//110
#define MAX_RECEIVE 450//110
#define MAX_MODEM_TCP_SNDBUF 4096

#define MODULE_NAME "bc26_driver"

#define DUMP_DATA 0

#define DATA_TIMEOUT 10

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
	char          sendOnGoing;
	unsigned int  dataAvailable;
	char          apn[33];
	int           modemSndBufLen;
} quactel_modem;

static int setApnAndSaveBands(network_modem * This);


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

CREATE_ATCommand(cmd_AT_QICFG_DATA_FORMAT, "AT+QICFG=\"dataformat\",0,1", gen_result_hadler, gen_responce_hadler);

#if 0

CREATE_ATCommand(cmd_ATV1, "ATV1", gen_result_hadler, gen_responce_hadler);

CREATE_ATCommand(cmd_AT_IFC, "AT+IFC=0,0", gen_result_hadler, gen_responce_hadler);

CREATE_ATCommand(cmd_AT_CGDCONT_REQ, "AT+CGDCONT?", ignorant_result_hadler, ignorant_responce_hadler);

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
#endif

CREATE_ATCommand(cmd_AT_CFUN0, "AT+CFUN=0", ignorant_result_hadler, ignorant_responce_hadler);

CREATE_ATCommand(cmd_AT_QCSEARFCN, "AT+QCSEARFCN", ignorant_result_hadler, ignorant_responce_hadler);
CREATE_ATCommand(cmd_AT_CPSMS0, "AT+CPSMS=0", ignorant_result_hadler, ignorant_responce_hadler);         //zwj
//CREATE_ATCommand(cmd_AT_RESET, "AT+QRST=1", ignorant_result_hadler, ignorant_responce_hadler);

static char * forwardCommas(const char *text, unsigned int how_many)
{
	while(*text && text)
	{
		if(!how_many)
			return (char*)text;

		if(*text == ',')
			how_many--;

		text++;
	}

	return NULL;
}

static void QCCID_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);
	const char *data;
	const char tag[] = "+QCCID:";
	DBG_PR("QCCID responce is |%s|\n", responce);

	data = strstr(responce, tag);

	if(data)
	{
		data = data + strlen(tag);
	}
	else
	{
		return;
	}

	strncpy(modem->iccd, data, ELEMENTS(modem->iccd));

	modem->iccd[ELEMENTS(modem->iccd) - 1] = 0;
}

CREATE_ATCommand(cmd_AT_QCCID, "AT+QCCID", gen_result_hadler, QCCID_responce_hadler);

static void CGMR_response_handler(const struct AT_Handler * parent, const char * response)
{
	network_modem *modem = (network_modem*)(parent->modem);
	const char *data;
	const char tag[] = "Revision: ";
	DBG_PR("CGMR response is |%s|\n", response);

	data = strstr(response, tag);

	if(data)
	{
		data = data + strlen(tag);
	}
	else
	{
		return;
	}

	strncpy(modem->cgmr, data, ELEMENTS(modem->cgmr));

	modem->cgmr[ELEMENTS(modem->cgmr) - 1] = 0;
}

CREATE_ATCommand(cmd_AT_CGMR, "AT+CGMR", gen_result_hadler, CGMR_response_handler);

static void IMEI_response_handler(const struct AT_Handler * parent, const char * response)
{
	network_modem *modem = (network_modem*)(parent->modem);
	const char *data;
	const char tag[] = "+CGSN: ";
	DBG_PR("IMEI response is |%s|\n", response);

	data = strstr(response, tag);

	if(data)
	{
		data = data + strlen(tag);
	}
	else
	{
		return;
	}

	strncpy(modem->imei, data, ELEMENTS(modem->imei));

	modem->imei[ELEMENTS(modem->imei) - 1] = 0;
}

CREATE_ATCommand(cmd_AT_IMEI, "AT+CGSN=1", gen_result_hadler, IMEI_response_handler);


static void  signal_quolatity_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
    network_modem *modem = (network_modem*)(parent->modem);
    DBG_PR("CEQ responce is |%s|\n", responce);
}
CREATE_ATCommand(cmd_AT_CSQ, "AT+CSQ", gen_result_hadler, signal_quolatity_responce_hadler);

static void QCGDEFCONT_Q_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);
	char *letter;
	int len;

	DBG_PR("QCGDEFCONT? responce is |%s|\n", responce);

	letter = strstr(responce, "+QCGDEFCONT:");
	if(!letter)
	{
		return;
	}

	quactel->apn[0] = 0;

	letter = forwardCommas(letter, 1);

	if(!letter)
	{
		return; // no apn
	}

	letter++; // Delete first "
	strncpy(quactel->apn, letter, ELEMENTS(quactel->apn));
	quactel->apn[ELEMENTS(quactel->apn) - 1] = 0;

	letter = forwardCommas(quactel->apn, 1);
	if(letter)
	{
		// delete username and password by terminating comma
		*--letter = 0;
	}

	len = strlen(quactel->apn);
	if(len > 0)
	{
		quactel->apn[len - 1] = 0; // Delete last "
	}
}

CREATE_ATCommand(cmd_AT_QCGDEFCONT_Q, "AT+QCGDEFCONT?", gen_result_hadler, QCGDEFCONT_Q_responce_hadler);
static void QCGDEFCONT_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);

	char buffer[200];
	snprintf(
		buffer,
		ELEMENTS(buffer),
        "AT+QCGDEFCONT=\"IP\",\"%s\",\"%s\",\"%s\"",
		modem->user_apn,
		modem->user_name,
		modem->user_password
	);
	buffer[ELEMENTS(buffer) - 1] = 0;
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static const AT_Command cmd_AT_QCGDEFCONT = { "AT+QCGDEFCONT", QCGDEFCONT_cmd, 0, gen_result_hadler, gen_responce_hadler, 0, NULL };

static void QSSLSTATE_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	// |+QSSLSTATE: 4,"SSLClient","139.217.12.206",443,65344,2,1,4,0,"usbat",1|  // 2 == connected
	const char  * conStat = forwardCommas(responce, 5);

	if(conStat && conStat[0] == '2')
		quactel->connected = 1;
	else if(conStat && conStat[0] == '1')
		quactel->connected = 0;
	else
		quactel->connected = 2;

}

CREATE_ATCommand(cmd_AT_QISTATE, "AT+QISTATE=1,3", gen_result_hadler, QSSLSTATE_responce_hadler);

static void QIOPEN_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);

	char buffer[200];
	snprintf(buffer, ELEMENTS(buffer), "AT+QIOPEN=1,3,\"TCP\",\"%s\",%u,0,0",
		modem->server_addr,
		modem->server_port);
	buffer[ELEMENTS(buffer) - 1] = 0;
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static const AT_Command cmd_AT_QIOPEN = { "AT+QIOPEN", QIOPEN_cmd, 0, gen_result_hadler, gen_responce_hadler, 0, NULL };

CREATE_ATCommand(cmd_AT_QICLOSE, "AT+QICLOSE=3", ignorant_result_hadler, gen_responce_hadler);

static void QISENDEX_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	char buffer[256];
	char *printer;
	int send = 0;

	quactel->sendOnGoing = 1;
	sprintf(buffer, "AT+QISENDEX=3,%d,", quactel->send_len);
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

static const AT_Command cmd_AT_QISENDEX = { "AT+QISENDEX=", QISENDEX_cmd, 0, gen_result_hadler, gen_responce_hadler, 0, NULL };

static void QITXCACHE_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);
	unsigned int sent;

	if ((sscanf(responce, "+QITXCACHE: %u", &sent) == 1))
	{
		quactel->modemSndBufLen = sent;
	}
	else
	{
		quactel->modemSndBufLen = MAX_MODEM_TCP_SNDBUF;
	}
}

CREATE_ATCommand(cmd_AT_QITXCACHE, "AT+QITXCACHE=3,0", gen_result_hadler, QITXCACHE_responce_hadler);

CREATE_ATCommand(cmd_AT_CPSMS, "AT+CPSMS=0", gen_result_hadler, gen_responce_hadler);
CREATE_ATCommand(cmd_AT_QSCLK, "AT+QSCLK=0", gen_result_hadler, gen_responce_hadler);

static int readByte(network_modem *modem)
{
	while(1)
	{
		char tmp;
		int timeout = 30;

		int read_len = AT_handler_read(
					modem->at_handler,
					&tmp,
					1);

		if(read_len == 1)
		{
			return (unsigned char)tmp;
		}
		else if(!read_len)
		{
			if(timeout-- > 0)
			{
				SLEEP(1);
				continue;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			return -2;
		}
	}
}

static void  qird_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	unsigned int len = 0;
	unsigned int read = 0;
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	if(strstr(responce, "+QIRD: ")) {

		sscanf(responce,"+QIRD: %u", &len);
		DBG_PR("QIRD responce is |%s|. read len is %u\n", responce, len);

		while(read < len) {
			int first  = readByte(modem);
			int second = readByte(modem);
			char tmp[3] = { first, second, 0 };
			unsigned int data;

			if(first < 0 || second < 0 || 1 != sscanf(tmp,"%x", &data))
			{
				MAKE_ERROR(modem, ERROR_RESPONCE, "%s RESPONCE parse failed: %s\n", parent->cmd->cmd, responce);
				quactel->receive_len = -16;
				return;
			}
			quactel->receive_data[read++] = data;
		}

		quactel->receive_len = read;
	}
}


static void QIRD_cmd(const struct AT_Handler * parent)
{
	char buffer[80];
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	sprintf(buffer, "AT+QIRD=3,%d", quactel->receive_len);
	parent->write_AT_cmd(buffer, strlen(buffer));
	quactel->receive_len = -42;
}

static const AT_Command cmd_AT_QIRD = { "AT+QIRD=3,", QIRD_cmd, 0, gen_result_hadler, qird_responce_hadler, 0, NULL };



#define doIt(name, timeout) modem_do_command(This, &name, timeout)

#define doIt_ret_if_error(name) if(doIt(name, 100) <= 0 || This->error) \
	do{ quactel->state = MODEM_ERROR; return -1;}while(0)

#define doIt_long_ret_if_error(name) if(doIt(name, 4000) <= 0 || This->error) \
	do{ quactel->state = MODEM_ERROR; return -1;}while(0)

static int waitAt(network_modem * This)
{
	//network_modem *modem = This;
	quactel_modem *quactel = (quactel_modem*)(This->driver);

	int waitWakeup = 20;

	while(1)
	{
        doIt(cmd_AT, 100);

		if(!This->error)
			break;

		This->error = 0;

		if(waitWakeup-- < 0)
			doIt_ret_if_error(cmd_AT);
	}

	return 0;
}

static int setup_connection(network_modem * This)
{
	int i, j;
	network_modem *modem = This;
	quactel_modem *quactel = (quactel_modem*)(This->driver);

    LIGHT_DBG_PR("Setup connection\n");

    //doIt_ret_if_error(cmd_AT_QCGDEFCONT_Q);   //zwj
    doIt(cmd_AT_QCGDEFCONT_Q,100);
	if(strcmp(quactel->apn, modem->user_apn))
	{
        printf("quactel->apn=%s,modem->user_apn=%s\n",quactel->apn,modem->user_apn);
		setApnAndSaveBands(This);
	}
	else
	{
        //doIt_ret_if_error(cmd_AT_QCGDEFCONT); //zwj  // Force username and password
        doIt(cmd_AT_QCGDEFCONT,100);
	}

    //doIt_ret_if_error(cmd_AT_QCGDEFCONT_Q);    //zwj
      doIt(cmd_AT_QCGDEFCONT_Q,100);
	if(strcmp(quactel->apn, modem->user_apn))
	{
		LIGHT_DBG_PR("Modem apn set failed\n");
	}

	if(This->modem_wakeup(This))
	{
		This->error = 0;
		This->error = 0;

		This->error = ERROR_CREG_REGISTERING_FAILED;

		return -1;
	}

	quactel->sendOnGoing = 0;

	quactel->connected = 0;
	for(j = 0; j < 3 && quactel->connected != 1; j++)
	{
		if(modem->crypted)
		{
			MAKE_ERROR(modem, ERROR_CMD_FAILED, "crypted connection is not supported\n");
			return -5;
		}
		else
		{
			doIt_long_ret_if_error(cmd_AT_QICLOSE);
			SLEEP(200); // URC: CLOSE OK
			waitAt(modem);
			doIt_long_ret_if_error(cmd_AT_QIOPEN);
		}

		quactel->connected = 0;
		for( i = 0; i < 82 && quactel->connected == 0; i++)
		{
			SLEEP(500);
			quactel->connected = 2;

			doIt_ret_if_error(cmd_AT_QISTATE);
		}
	}

	if(quactel->connected != 1)
	{
		return -1;
	}

	doIt_ret_if_error(cmd_AT_QICFG_DATA_FORMAT);

	quactel->state = MODEM_DATA;

	return 0;
}

static int make_disconnect(network_modem * This)
{
	quactel_modem *quactel = (quactel_modem*)(This->driver);

	LIGHT_DBG_PR("Reboot modem\n");

	quactel->state = MODEM_INIT;
	quactel->sendOnGoing = 0;
	This->registed_network = 0;

	doIt(cmd_AT_QICLOSE, 100);
	This->error = 0;
	doIt(cmd_AT_CFUN0, 1000);
	This->error = 0;
	This->at_handler->write_AT_cmd("AT+QRST=1\r\n", strlen("AT+QRST=1\r\n"));
	SLEEP(2000);
	This->error = 0;

	return 0;
}

static int setApnAndSaveBands(network_modem * This)
{
	//network_modem *modem = This;
	quactel_modem *quactel = (quactel_modem*)(This->driver);

	LIGHT_DBG_PR("Modem apn set\n");
	doIt_ret_if_error(cmd_AT_QCGDEFCONT); // Set default context
	doIt_ret_if_error(cmd_AT_QCSEARFCN); // Reset saved band info
	make_disconnect(This); // reboot modem

	return waitAt(This);
}

static int checkModemSendBuf(network_modem * This, int required)
{
	int transfer;
	network_modem *modem = This;
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	if (modem->cgmr[10] > '0' || modem->cgmr[11] >= '6')
	{
		doIt_ret_if_error(cmd_AT_QITXCACHE);
		transfer = MIN(required, (quactel->modemSndBufLen - 512));
		transfer = MIN(transfer, MAX_SEND);
	}
	else
	{
		transfer = MIN(required, MAX_SEND);
		SLEEP(200); // Max outputspeed 3kb/s
	}

	return transfer;
}

static inline int init(network_modem * This)
{
	network_modem *modem = This;
	static quactel_modem quactel_mem; // Only singel modem supported

	modem->driver = &quactel_mem;

	memset(&quactel_mem, 0, sizeof(quactel_mem));

    LIGHT_DBG_PR("Modem init..\n");


	if(waitAt(This))
		return -34;
    doIt(cmd_AT_CPSMS0, 100);
    //SLEEP(3000);
    doIt(cmd_AT_QCCID, 100);
    //SLEEP(3000);
    doIt(cmd_AT_CGMR, 100);
   // SLEEP(3000);
    doIt(cmd_AT_IMEI, 100);
   // SLEEP(3000);
    doIt(cmd_AT_CSQ, 100);
	if (modem->cgmr[10] > '0' || modem->cgmr[11] >= '3')
	{
		//set modem no sleep
        doIt(cmd_AT_CPSMS, 300);
        doIt(cmd_AT_QSCLK, 300);
	}

	quactel_mem.state = MODEM_INIT;

	return 0;
}

static inline int m_write(network_modem * This, const char *buffer, unsigned int len)
{
	unsigned int send      = 0;
	int timeout;
	int timeoutBuffer = 45;
	network_modem *modem   = This;
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	if(quactel->state != MODEM_DATA)
	{
		return -1;
	}

	if(modem->crypted)
	{
		MAKE_ERROR(modem, ERROR_CMD_FAILED, "crypted connection is not supported\n");
		return -5;
	}

	quactel->dataAvailable = 0; // Fail safe. Do one read after sending

	while(send < len)
	{
		int i;
		int required = len - send;
		int transfer = checkModemSendBuf(This, required);

		if (transfer <= 0)
		{
			SLEEP(1000);
			if (timeoutBuffer-- < 0)
			{
				quactel->state = MODEM_ERROR;
				return -128;
			}
			continue;
		}

		for( i = 0; i < 3; i++)
		{
			quactel->send_data = &buffer[send];
			quactel->send_len  = transfer;

			This->error          = 0;
			modem->error_str[0]  = 0;
			quactel->sendOnGoing = 0;
			quactel->state       = MODEM_DATA;

			doIt(cmd_AT_QISENDEX, 100);

			timeout = 4000;
			while(quactel->sendOnGoing && quactel->state == MODEM_DATA && !This->error)
			{
				AT_handler_poll(This->at_handler); // wait for "SEND OK"
				SLEEP(10);
				if(timeout-- < 0)
				{
					//MAKE_ERROR(modem, ERROR_CMD_FAILED, "Send timeout\n");
					//return -128;
					ERR_PR(" \"SEND OK\" timeout \n");
					quactel->sendOnGoing = 0;
				}
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
	network_modem *modem   = This;
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	if(quactel->state != MODEM_DATA)
		return -1;

	if(!len || (GET_TIME_SECONDS() < quactel->dataAvailable + DATA_TIMEOUT))
		return 0;

	quactel->receive_data = buffer;
	quactel->receive_len  = MIN(len, MAX_RECEIVE);
    printf("len=%d,MAX_RECEIVE=%d,quactel->receive_len=%d...!!!\n",len,MAX_RECEIVE,quactel->receive_len);
	if(modem->crypted)
	{
		MAKE_ERROR(modem, ERROR_CMD_FAILED, "crypted connection is not supported\n");
	}
	else
	{
		doIt(cmd_AT_QIRD, 300);
	}

	if(quactel->receive_len < 0 || This->error)
		quactel->state = MODEM_ERROR;
	else
		quactel->state = MODEM_DATA;

	if(!quactel->receive_len)
	{
		quactel->dataAvailable = GET_TIME_SECONDS();
	}

	return quactel->receive_len;
}

static int searchStr(const char *from, const char *what)
{
	int i = 0;
	while(what[i])
	{
		if(from[i] != what[i])
		{
			return 0;
		}
		i++;
	}
	return 1;
}

static inline int unsolicited(network_modem *This, const char * responce)
{
	network_modem *modem = This;
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	if(searchStr(responce, "+QIURC: \"closed\",") ) {
		DBG_PR("unsolicited: %s\n", responce);

		quactel->connected = 2;

		ERR_PR("Server disconnected\n");

		quactel->state = MODEM_ERROR;
		return 1;
	}

	if(searchStr(responce, "+QIURC: \"recv\",")) {
		quactel->dataAvailable = 0;
		DBG_PR("unsolicited: %s\n", responce);
		return 1;
	}

	if(searchStr(responce, "SEND FAIL"))
	{
		DBG_PR("unsolicited: %s\n", responce);
		modem->error = ERROR_CREG_RESPONCE_INVALID;
		quactel->state = MODEM_ERROR;
		return 1;
	}

	if(searchStr(responce, "SEND OK")) {
		quactel->sendOnGoing = 0;
		DBG_PR("unsolicited: %s\n", responce);
		return 1;
	}

	if(searchStr(responce, "+IP:")) {
		DBG_PR("Our IP %s\n", responce);
		return 1;
	}

	return 0;
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


#ifndef USE_QUACTEL_BC26_DRIVER
#define USE_QUACTEL_BC26_DRIVER 0
#endif





#if USE_QUACTEL_BC26_DRIVER
int impl_modem_init(network_modem *This) { return init(This); }
int impl_modem_write(network_modem *This, const char *buffer, int len) { return m_write(This, buffer, len); }
int impl_modem_read(network_modem *This, char *buffer, int len) { return m_read(This, buffer, len); }
int impl_modem_unsolicited(network_modem *This, const char * responce) { return unsolicited(This, responce); }
int impl_modem_connect(network_modem *This, int disconnect) { return m_connect(This, disconnect); };
#else
void avoid_warnings_quactelBC26___(void) // Fake call to make compiler happy
{
	if((USE_QUACTEL_BC26_DRIVER))
	{
		init(NULL);
		m_read(NULL, NULL, 0);
		m_write(NULL, NULL, 0);
		unsolicited(NULL, NULL);
		m_connect(NULL, 0);
	}
}
#endif
