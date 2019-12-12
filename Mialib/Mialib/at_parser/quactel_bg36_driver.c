#include "modem_driver.h"
#include "os_support.h"
#include <string.h>
#include "build_configuration.h"      //zwj
#define MAX_SEND 1200
#define MAX_RECEIVE 400

#define MODULE_NAME "bg36_driver"

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

//CREATE_ATCommand(cmd_ATZ, "ATZ", gen_result_hadler, gen_responce_hadler);
CREATE_ATCommand(cmd_AT, "AT", gen_result_hadler, gen_responce_hadler);

/*
static void version_handler(const struct AT_Handler * parent, const char * responce)
{
	DBG_PR("Version is |%s|\n", responce);
}

CREATE_ATCommand(cmd_AT_CGMR, "AT+CGMR", gen_result_hadler, version_handler);
*/

/*
static void module_handler(const struct AT_Handler * parent, const char * responce)
{
	DBG_PR("Module is |%s|\n", responce);
}

CREATE_ATCommand(cmd_AT_CGMM, "AT+CGMM", gen_result_hadler, module_handler);


static void QIACT_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	DBG_PR("IP is |%s|\n", responce);
}

CREATE_ATCommand(cmd_AT_QIACT_Q, "AT+QIACT?", gen_result_hadler, QIACT_responce_hadler);
*/

static void clock_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	DBG_PR("Clock is |%s|\n", responce);
}

CREATE_ATCommand(cmd_AT_CCLK, "AT+CCLK?", gen_result_hadler, clock_responce_hadler);

CREATE_ATCommand(cmd_AT_QIACT, "AT+QIACT=1", ignorant_result_hadler, gen_responce_hadler);

CREATE_ATCommand(cmd_ATV1, "ATV1", gen_result_hadler, gen_responce_hadler);

CREATE_ATCommand(cmd_AT_IFC, "AT+IFC=0,0", gen_result_hadler, gen_responce_hadler);


static void AT_APN_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);

	char buffer[180];
	if(modem->user_apn[0])
	{
		snprintf(buffer, ELEMENTS(buffer), "AT+CGDCONT=1,\"IP\",\"%s\"", modem->user_apn);
	}
	else
	{
		snprintf(buffer, ELEMENTS(buffer), "AT+CGDCONT=1");
	}
	buffer[ELEMENTS(buffer) - 1] = 0;
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static const AT_Command cmd_AT_APN = { "AT+CGDCONT=", AT_APN_cmd, 0, gen_result_hadler, gen_responce_hadler, 0, NULL };


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

static void QCCID_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);
	const char *letter;

	DBG_PR("QCCID responce is |%s|\n", responce);

	letter = strstr(responce, "+QCCID: ");
	if(!letter)
	{
		return;
	}
	letter += 8;

	strncpy(modem->iccd, letter, ELEMENTS(modem->iccd));

	modem->iccd[ELEMENTS(modem->iccd) - 1] = 0;
}

CREATE_ATCommand(cmd_AT_QCCID, "AT+QCCID", gen_result_hadler, QCCID_responce_hadler);

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

CREATE_ATCommand(cmd_AT_QSSLSTATE, "AT+QSSLSTATE=4", gen_result_hadler, QSSLSTATE_responce_hadler);


static void ATQICSGP_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);
	//quactel_modem *quactel = (quactel_modem*)(modem->driver);

	char buffer[180];
	snprintf(buffer, ELEMENTS(buffer), "AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",0", modem->user_apn, modem->user_name, modem->user_password); // XXX check authentication mode
	buffer[ELEMENTS(buffer) - 1] = 0;
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static const AT_Command cmd_AT_QICSGP = { "AT+QICSGP=", ATQICSGP_cmd, 0, gen_result_hadler, gen_responce_hadler, 0, NULL };

static void QSSLOPEN_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);

	char buffer[200];
	snprintf(buffer, ELEMENTS(buffer), "AT+QSSLOPEN=1,1,4,\"%s\",%u,0",
		modem->server_addr,
		modem->server_port);
	buffer[ELEMENTS(buffer) - 1] = 0;
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static const AT_Command cmd_AT_QSSLOPEN = { "AT+QSSLOPEN", QSSLOPEN_cmd, 0, gen_result_hadler, gen_responce_hadler, 0, NULL };

CREATE_ATCommand(cmd_AT_QSSLCLOSE, "AT+QSSLCLOSE=4", ignorant_result_hadler, gen_responce_hadler);

static void  sslsend_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	int timeout = 6;
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	//DBG_PR("resp %s\n", responce);

	if(!quactel->send_data || quactel->send_len < 1)
		return;

	if(responce[0] != '>') {
		DBG_PR("sslsend responce discarded %c %d\n", responce[0], responce[1]);
		return;
	}

	if(0 && quactel->send_len < 20)
	{
		char buffer[160];
		char *ptr = buffer;
		int i;

		for( i = 0; i < quactel->send_len; i++)
			ptr += sprintf(ptr,"%d, ", quactel->send_data[i]);

		DBG_PR("sslsend data %d: %s\n", quactel->send_len, buffer);
	}


	if(parent->write_AT_cmd(quactel->send_data, quactel->send_len - 1) != quactel->send_len - 1) {
		MAKE_ERROR(modem, ERROR_RESPONCE, "Write out failed\n");
		quactel->send_len = -1;
		return;
	}

	SLEEP(20);

	while(1) { // Read echo manually

		int read_len = 100;
		char buffer[100];
		read_len = parent->read_AT_cmd(buffer, read_len);
		if(!read_len) {
			timeout--;
		}
		else
		{
			timeout = 2;
		}
		SLEEP(5);

		if(read_len < 0) {
			MAKE_ERROR(modem, ERROR_RESPONCE, "Send echo not received\n");
			quactel->send_len = -1;
			return;
		}
		else if(timeout < 0) {
			break;
		}
	}

	if(parent->write_AT_cmd(&quactel->send_data[quactel->send_len - 1], 1) != 1) {
		MAKE_ERROR(modem, ERROR_RESPONCE, "Final Write out failed\n");
		quactel->send_len = -1;
	}

	for(timeout = 0; timeout < 10; timeout++) {
		int read_len = 1;
		char buffer[1];

		read_len = parent->read_AT_cmd(buffer, read_len);

		if(read_len > 0)
			break;
		SLEEP(1);
	}

	quactel->send_data = NULL;

}

static void QSSLSEND_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	char buffer[80];
	sprintf(buffer, "AT+QSSLSEND=4,%d", quactel->send_len);
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static const AT_Command cmd_AT_SSLSEND = { "AT+QSSLSEND=", QSSLSEND_cmd, '>', gen_result_hadler, sslsend_responce_hadler, 0, NULL };

static void  sslrcv_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	unsigned int len;
	unsigned int read = 0;
	int timeout = 1000;
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	if(strstr(responce, "+QSSLRECV:")) {
		sscanf(responce,"+QSSLRECV: %u", &len);

		DBG_PR("sslreceive responce is |%s|. read len is %u\n", responce, len);

		while(read < len) {
			int read_len = AT_handler_read(
				modem->at_handler,
				&quactel->receive_data[read],
				len - read);

			if(read_len > 0) {
				if(DUMP_DATA)
				{
					int j;
					DBG_PR("SSL received %d\n", read_len);
					for(j = 0; j < read_len; j++)
						DBG_PR_RAW("%c\n", quactel->receive_data[read + j]);
				}
				read += read_len;
			}

			if(read < len)
				SLEEP(1); // Block until all is read

			if(timeout-- < 0) {
				quactel->state = MODEM_ERROR;
				ERR_PR("SSL receive failed %d\n", read_len);
				quactel->receive_len = -1;
				return;
			}

		}
		quactel->receive_len = read;
	}
}


static void QSSLRECV_cmd(const struct AT_Handler * parent)
{
	char buffer[80];
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	sprintf(buffer, "AT+QSSLRECV=4, %d", quactel->receive_len);
	parent->write_AT_cmd(buffer, strlen(buffer));
	quactel->receive_len = -42;
}

static const AT_Command cmd_AT_SSLRECV = { "AT+QSSLRECV=", QSSLRECV_cmd, 0, gen_result_hadler, sslrcv_responce_hadler, 0, NULL };


static void QIOPEN_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);

	char buffer[200];
	snprintf(buffer, ELEMENTS(buffer), "AT+QIOPEN=1,4,\"TCP\",\"%s\",%u,0,0",
		modem->server_addr,
		modem->server_port);
	buffer[ELEMENTS(buffer) - 1] = 0;
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static const AT_Command cmd_AT_QIOPEN = { "AT+QIOPEN", QIOPEN_cmd, 0, gen_result_hadler, gen_responce_hadler, 0, NULL };

CREATE_ATCommand(cmd_AT_QICLOSE, "AT+QICLOSE=4", ignorant_result_hadler, gen_responce_hadler);

CREATE_ATCommand(cmd_AT_QISTATE, "AT+QISTATE=1,4", gen_result_hadler, QSSLSTATE_responce_hadler);

CREATE_ATCommand(cmd_AT_QCFG_SCRAMBLE, "AT+QCFG=\"nbsibscramble\",0", gen_result_hadler, gen_responce_hadler);
CREATE_ATCommand(cmd_AT_QCFG_NWSCANSEQ, "AT+QCFG=\"nwscanseq\",03", gen_result_hadler, gen_responce_hadler);
CREATE_ATCommand(cmd_AT_QCFG_IOTOPMODE, "AT+QCFG=\"iotopmode\",1", gen_result_hadler, gen_responce_hadler);
CREATE_ATCommand(cmd_AT_QCFG_BAND, "AT+QCFG=\"band\"," BAND_SELECTION, gen_result_hadler, gen_responce_hadler);
CREATE_ATCommand(cmd_AT_QCFG_NWSCANMODE, "AT+QCFG=\"nwscanmode\",3", gen_result_hadler, gen_responce_hadler);

static void  qisend_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	#define MAX_AT_ONCE 128
	int timeout = 10;
	int echo_len = 0;
	int echo_len_t = 0;
	int send_data = 0;
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	if(!quactel->send_data || quactel->send_len < 1)
		return;

	DBG_PR("resp %s\n", responce);

	if(responce[0] != '>') {
		DBG_PR("qisend responce discarded %c %d\n", responce[0], responce[1]);
		return;
	}

	if(0 && quactel->send_len < 20)
	{
		char buffer[160];
		char *ptr = buffer;
		int i;

		for( i = 0; i < quactel->send_len; i++)
			ptr += sprintf(ptr,"%d, ", quactel->send_data[i]);

		DBG_PR("qisend data %d: %s\n", quactel->send_len, buffer);
	}

	while(send_data < quactel->send_len)
	{
		int send = MIN((quactel->send_len - send_data), MAX_AT_ONCE);

		if(parent->write_AT_cmd(&quactel->send_data[send_data], send) != send ) {
			MAKE_ERROR(modem, ERROR_RESPONCE, "Write out failed\n");
			quactel->send_len = -1;
			return;
		}

		send_data += send;
		echo_len = 0;
		timeout = 10;

		while(echo_len < send) { // Read echo manually
			int read_len = send - echo_len;
			char buffer[MAX_AT_ONCE];
			read_len = parent->read_AT_cmd(buffer, read_len);
			if(!read_len) {
				timeout--;
				SLEEP(10);
			}
			else
			{
				timeout = 2;
			}

			if(read_len < 0) {
				MAKE_ERROR(modem, ERROR_RESPONCE, "Send echo not received\n");
				quactel->send_len = -1;
				quactel->send_data = NULL;
				return;
			}
			else if(timeout < 0) {
				DBG_PR("qisend echo timeout %d \n", echo_len);
				break;
			}
			else {
				echo_len += read_len;
				echo_len_t += read_len;
			}
		}
	}

	if(echo_len_t != quactel->send_len)
	{
		DBG_PR("qisend echo len %d \n", echo_len_t);
	}

	quactel->send_data = NULL;
}

static void QISEND_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	char buffer[80];
	sprintf(buffer, "AT+QISEND=4,%d", quactel->send_len);
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static const AT_Command cmd_AT_QISEND = { "AT+QSSLSEND=", QISEND_cmd, '>', gen_result_hadler, qisend_responce_hadler, 0, NULL };

static void  qird_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	unsigned int len = 0;
	unsigned int read = 0;
	int timeout = 1000;
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	if(strstr(responce, "+QIRD: ")) {
		sscanf(responce,"+QIRD: %u", &len);

		DBG_PR("sslreceive responce is |%s|. read len is %u\n", responce, len);

		while(read < len) {
			int read_len = AT_handler_read(
				modem->at_handler,
				&quactel->receive_data[read],
				len - read);

			if(read_len > 0) {
				if(DUMP_DATA)
				{
					int j;
					DBG_PR("QIRD received %d\n", read_len);
					for(j = 0; j < read_len; j++)
						DBG_PR_RAW("%c\n", quactel->receive_data[read + j]);
				}

				read += read_len;
			}

			if(read < len)
				SLEEP(1); // Block until all is read

			if(timeout-- < 0) {
				quactel->state = MODEM_ERROR;
				ERR_PR("SSL receive failed %d\n", read_len);
				quactel->receive_len = -1;
				return;
			}

		}
		quactel->receive_len = read;
	}
}


static void QIRD_cmd(const struct AT_Handler * parent)
{
	char buffer[80];
	network_modem *modem = (network_modem*)(parent->modem);
	quactel_modem *quactel = (quactel_modem*)(modem->driver);

	sprintf(buffer, "AT+QIRD=4,%d", quactel->receive_len);
	parent->write_AT_cmd(buffer, strlen(buffer));
	quactel->receive_len = -42;
}

static const AT_Command cmd_AT_QIRD = { "AT+QIRD=4,", QIRD_cmd, 0, gen_result_hadler, qird_responce_hadler, 0, NULL };


#define doIt(name, timeout) modem_do_command(This, &name, timeout)

#define doIt_ret_if_error(name) if(doIt(name, 100) <= 0 || This->error) \
	do{ quactel->state = MODEM_ERROR; return -1;}while(0)

#define doIt_long_ret_if_error(name) if(doIt(name, 4000) <= 0 || This->error) \
	do{ quactel->state = MODEM_ERROR; return -1;}while(0)


static int setup_connection(network_modem * This)
{
	int i,j;
	network_modem *modem = This;
	quactel_modem *quactel = (quactel_modem*)(This->driver);

	LIGHT_DBG_PR("Setup connection\n");

	doIt_ret_if_error(cmd_AT_QICSGP);
	doIt_long_ret_if_error(cmd_AT_QIACT);

	quactel->connected = 0;
	for(j = 0; j < 3 && quactel->connected != 1; j++)
	{
		if(modem->crypted)
		{
			doIt_long_ret_if_error(cmd_AT_QSSLCLOSE);
			doIt_long_ret_if_error(cmd_AT_QSSLOPEN);
		}
		else
		{
			doIt_long_ret_if_error(cmd_AT_QICLOSE);
			doIt_long_ret_if_error(cmd_AT_QIOPEN);
		}

		quactel->connected = 0;
		for( i = 0; i < 120 && quactel->connected == 0; i++)
		{
			SLEEP(500);
			quactel->connected = 2;
			if(modem->crypted)
			{
				doIt_ret_if_error(cmd_AT_QSSLSTATE);
			}
			else
			{
				doIt_ret_if_error(cmd_AT_QISTATE);
			}
		}
	}

	doIt_ret_if_error(cmd_AT_CCLK);

	if(quactel->connected == 1)
		quactel->state = MODEM_DATA;

	return 0;

}

static int make_disconnect(network_modem * This)
{
	quactel_modem *quactel = (quactel_modem*)(This->driver);

	LIGHT_DBG_PR("Reboot modem\n");

	//doIt_ret_if_error(cmd_AT_ENHRST);

	quactel->state = MODEM_INIT;

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

	doIt(cmd_AT_QCCID, 100);
	This->error = 0;

	if (This->modem_wakeup(This))
	{
		This->error = 0;
		doIt_ret_if_error(cmd_AT_IFC);
		doIt_ret_if_error(cmd_ATV1);

		doIt_ret_if_error(cmd_AT_APN);

		doIt_ret_if_error(cmd_AT_QCFG_SCRAMBLE);
		doIt_ret_if_error(cmd_AT_QCFG_NWSCANSEQ);
		doIt_ret_if_error(cmd_AT_QCFG_IOTOPMODE);
		doIt_ret_if_error(cmd_AT_QCFG_BAND);
		doIt_ret_if_error(cmd_AT_QCFG_NWSCANMODE);

		This->error = ERROR_CREG_REGISTERING_FAILED;

		return -1;
	}

	doIt_ret_if_error(cmd_AT_QCCID);

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
				doIt(cmd_AT_SSLSEND, 2000);
			}
			else
			{
				doIt(cmd_AT_QISEND, 2000);
			}

			if(!This->error)
				break;
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
		doIt_long_ret_if_error(cmd_AT_SSLRECV);
	}
	else
	{
		doIt_long_ret_if_error(cmd_AT_QIRD);
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

	if (strstr(responce, "NO CARRIER") || strstr(responce, "DISCONNECTED") || strstr(responce, "+QSSLOPEN: 4,1")) {

		DBG_PR("unsolicited: %s\n", responce);

		quactel->connected = 2;

		DBG_PR("Server disconnected\n");

		quactel->state = MODEM_ERROR;
		return 1;
	}

	if (!strstr(responce, "+QSSLURC: ")) {
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
		switch(quactel->state)
		{
		case MODEM_DATA:
			return make_disconnect(This);
		default:
			return 0;
		}
	}
	else
	{
		quactel->state = MODEM_INIT;

		return setup_connection(This);
	}
}


#ifndef USE_QUACTEL_BG36_DRIVER
#define USE_QUACTEL_BG36_DRIVER 0
#endif

#if USE_QUACTEL_BG36_DRIVER
int impl_modem_init(network_modem *This) { return init(This); }
int impl_modem_write(network_modem *This, const char *buffer, int len) { return m_write(This, buffer, len); }
int impl_modem_read(network_modem *This, char *buffer, int len) { return m_read(This, buffer, len); }
int impl_modem_unsolicited(network_modem *This, const char * responce) { return unsolicited(This, responce); }
int impl_modem_connect(network_modem *This, int disconnect) { return m_connect(This, disconnect); };
#else
void avoid_warnings_quactelBG36___(void) // Fake call to make compiler happy
{
	if((USE_QUACTEL_BG36_DRIVER))
	{
		init(NULL);
		m_read(NULL, NULL, 0);
		m_write(NULL, NULL, 0);
		unsolicited(NULL, NULL);
		m_connect(NULL, 0);
	}
}
#endif
