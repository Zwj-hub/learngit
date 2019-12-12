#include "modem_driver.h"
#include "os_support.h"
#include <string.h>
#include "base64.h"

#define MODULE_NAME "HuaweiEm909"

#ifdef _WIN32
#pragma warning(push, 4)
#pragma warning(disable: 4127)
#pragma warning(disable: 4996)
#pragma warning(disable: 4100)
#endif

#define IS_ASCII(a) (((a) > 31) && ((a) < 126))

typedef enum { CERTIFICATE_NOT_CHECKED = 0, CERTIFICATE_SAME, CERTIFICATE_NOT_SAME } CaCertificate;

typedef enum { MODEM_INIT = 0, MODEM_ERROR, MODEM_DATA } ModemState;

#define BASE64_BUFFER 512

typedef struct internal_modem {

	char         *receive_data;
	int           receive_len;
	const char   *send_data;
	int           send_len;
	CaCertificate caCerficateOk;
	ModemState    state;
	char          connected;
	char          antModeWrong;
	char          buffer[BASE64_BUFFER];

} internal_modem;

#define MAX_SEND    (3*BASE64_BUFFER/4 - 70)
#define MAX_RECEIVE MAX_SEND // Max receive must be smaller than uart fifo

static void  gen_result_hadler( const struct AT_Handler * parent, const char * result)
{
	network_modem *modem = (network_modem*)(parent->modem);

	if(strstr(result, "ERROR")) {
		internal_modem *iModem = (internal_modem*)(modem->driver);

		MAKE_ERROR(modem, ERROR_RESULT, "ERROR found from AT command! %s |%s|\n",parent->cmd->cmd, result);
		ERR_PR("Resp: %s\n", result);
		iModem->state = MODEM_ERROR;
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
	internal_modem *iModem = (internal_modem*)(modem->driver);

	DBG_PR("cmd_at responce: |%s|\n", responce);
	ERR_PR("ERROR no responce should come from AT command %s\n", parent->cmd->cmd);

	MAKE_ERROR(modem, ERROR_RESPONCE, "%s RESPONCE not expected: %s\n", parent->cmd->cmd, responce);

	iModem->state = MODEM_ERROR;
}

static void ignorant_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	//network_modem *modem   = (network_modem*)(parent->modem);
	//internal_modem *iModem = (internal_modem*)(modem->driver);

	DBG_PR("cmd_at responce: |%s|\n", responce);
}


CREATE_ATCommand(cmd_AT_CURC, "AT^CURC=0", gen_result_hadler, gen_responce_hadler);

CREATE_ATCommand(cmd_AT, "AT", ignorant_result_hadler, gen_responce_hadler);

CREATE_ATCommand(cmd_ATZ, "ATZ0", gen_result_hadler, gen_responce_hadler);

CREATE_ATCommand(cmd_AT_RESET, "AT^RESET", gen_result_hadler, gen_responce_hadler);

static void version_handler(const struct AT_Handler * parent, const char * responce)
{
	DBG_PR("Version is |%s|\n", responce);
}

CREATE_ATCommand(cmd_AT_CGMR, "AT+CGMR", gen_result_hadler, version_handler);


static void module_handler(const struct AT_Handler * parent, const char * responce)
{
	DBG_PR("Module is |%s|\n", responce);
}

CREATE_ATCommand(cmd_AT_CGMM, "AT+CGMM", gen_result_hadler, module_handler);


static void clock_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	DBG_PR("Clock is |%s|\n", responce);
}

CREATE_ATCommand(cmd_AT_CCLK, "AT+CCLK?", gen_result_hadler, clock_responce_hadler);

CREATE_ATCommand(cmd_ATV1, "ATV1", gen_result_hadler, gen_responce_hadler);


static void IPINIT_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);

// AT^IPINIT="1234","card","card"
// AT^IPINIT=<APN>[,<user_name>[,<password>[,<ip_addr>[,<auth_type>]]]]

	char buffer[200];
	snprintf(buffer, ELEMENTS(buffer), "AT^IPINIT=\"%s\",\"%s\",\"%s\"", modem->user_apn, modem->user_name, modem->user_password);
	buffer[ELEMENTS(buffer) - 1] = 0;
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static const AT_Command cmd_AT_IPINIT = { "AT^IPINIT", IPINIT_cmd, 0, gen_result_hadler, ignorant_responce_hadler, 0, NULL };

CREATE_ATCommand(cmd_AT_SSLDIS, "AT^SSLEN=1,0", ignorant_result_hadler, ignorant_responce_hadler);

CREATE_ATCommand(cmd_AT_SSLEN, "AT^SSLEN=1,1,1", gen_result_hadler, ignorant_responce_hadler);
CREATE_ATCommand(cmd_AT_SSLEN_U, "AT^SSLEN=0,1,1", gen_result_hadler, ignorant_responce_hadler);

CREATE_ATCommand(cmd_AT_CMGD_1, "AT+CMGD=0,4", gen_result_hadler, ignorant_responce_hadler);
CREATE_ATCommand(cmd_AT_CMGD_0, "AT+CMGD=0,3", gen_result_hadler, ignorant_responce_hadler);

CREATE_ATCommand(cmd_AT_SLEEPCFG_0, "AT^SLEEPCFG=1,3", gen_result_hadler, ignorant_responce_hadler);
CREATE_ATCommand(cmd_AT_SLEEPCFG_1, "AT^SLEEPCFG=0,3000", gen_result_hadler, ignorant_responce_hadler);
CREATE_ATCommand(cmd_AT_WAKEUPCFG,  "AT^WAKEUPCFG=1,3,4", gen_result_hadler, ignorant_responce_hadler);

CREATE_ATCommand(cmd_AT_ANTMODE, "AT^ANTMODE=1", gen_result_hadler, ignorant_responce_hadler);


static void sslstat_responce_handler(const struct AT_Handler * parent, const char * responce)
{
	internal_modem *iModem = (internal_modem*)(((network_modem*)(parent->modem))->driver);

	if(!strstr(responce, "connection opened"))
	{
		ERR_PR("Bad socket state %s\n", responce);
		iModem->state = MODEM_ERROR;
	}
}

CREATE_ATCommand(cmd_AT_SSLSTAT, "AT^SSLSTAT=1", gen_result_hadler, sslstat_responce_handler);
CREATE_ATCommand(cmd_AT_SSLSTAT_U, "AT^SSLSTAT=0", gen_result_hadler, sslstat_responce_handler);

static void antmode_responce_handler(const struct AT_Handler * parent, const char * responce)
{
	internal_modem *iModem = (internal_modem*)(((network_modem*)(parent->modem))->driver);

	if(strstr(responce, "^ANTMODE: 0,"))
	{
		ERR_PR("Bad ANTMODE %s\n", responce);
		iModem->antModeWrong = 1;
	}
}

CREATE_ATCommand(cmd_AT_ANTMODE_Q, "AT^ANTMODE?", gen_result_hadler, antmode_responce_handler);

CREATE_ATCommand(cmd_AT_SSLC,    "AT^SSLC=1", ignorant_result_hadler, ignorant_responce_hadler);
CREATE_ATCommand(cmd_AT_SSLC_UN, "AT^SSLC=0", ignorant_result_hadler, ignorant_responce_hadler);

static void  iccid_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);
	const char *letter;

	DBG_PR("ICCD responce is |%s|\n", responce);

	letter = strstr(responce, "^ICCID: ");
	if(!letter)
	{
		return;
	}
	letter += 8;

	strncpy(modem->iccd, letter, ELEMENTS(modem->iccd));

	modem->iccd[ELEMENTS(modem->iccd) - 1] = 0;
}

CREATE_ATCommand(cmd_AT_ICCID, "AT^ICCID?", gen_result_hadler, iccid_responce_hadler);


static char parseUCD2(const char *ascii)
{
	char tmp[5] = { 0 };
	unsigned int letter;
	memcpy(tmp, ascii, 4);
	sscanf(tmp,"%x", &letter);
	return (char)letter;
}

static void  eons_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);

	const char *letter = responce;
	char *target       = modem->network;
	int limit = ELEMENTS(modem->network);

	DBG_PR("EONS responce is |%s|\n", responce);

	letter = strstr(responce, ",\"");
	if(!letter)
	{
		return;
	}
	letter += 2;

	while(*letter && *letter != '\"' ) {

		if(!--limit)
			break;

		*target++ = parseUCD2(letter);
		letter += 4;

	}
	*target = 0;
}

CREATE_ATCommand(cmd_AT_EONS, "AT^EONS=1", gen_result_hadler, eons_responce_hadler);


static void SSLO_cmd(const struct AT_Handler * parent)
{
	// AT^SSLO=1,"192.168.63.41",9900,1,60
	// AT^SSLO=<SSL_id>,<remote_IP>,<remote_port>[,<mode>,<TimeOut>]

	network_modem *modem = (network_modem*)(parent->modem);

	char buffer[200];
	snprintf(buffer, ELEMENTS(buffer), "AT^SSLO=%d,\"%s\",%u,0,60",
		modem->crypted ? 1 : 0,
		modem->server_addr,
		modem->server_port);
	buffer[ELEMENTS(buffer) - 1] = 0;
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static const AT_Command cmd_AT_SSLO = { "AT^SSLO", SSLO_cmd, 0, gen_result_hadler, gen_responce_hadler, 0, NULL };

static void SSLTX_cmd(const struct AT_Handler * parent)
{
	// AT^SSLTX=1,"aGVsbG8=",60
	const char * sendPre;
	const char * const sendPost = "\",60\r\n";
	int timeout = 600;
	network_modem *modem = (network_modem*)(parent->modem);
	internal_modem *iModem = (internal_modem*)(((network_modem*)(parent->modem))->driver);
	int result;
	unsigned int echoLen;

	sendPre = modem->crypted ? "AT^SSLTX=1,\"" : "AT^SSLTX=0,\"";

	DBG_PR("SSLTX_cmd: Sending %d\n", iModem->send_len);

	if(!iModem->send_data || iModem->send_len < 1)
	{
		ERR_PR("sslsend bad params");
		return;
	}

	result = base64encode(iModem->send_data, iModem->send_len, iModem->buffer, ELEMENTS(iModem->buffer));

	if(result < 1)
	{
		ERR_PR("sslsend coding failed");
		return;
	}
/*
	if(result != strlen(iModem->buffer))
	{
		ERR_PR(" SSL TX internal faul\n");
		result = strlen(iModem->buffer);
	}
*/
	parent->write_AT_cmd(sendPre, strlen(sendPre));
	parent->write_AT_cmd(iModem->buffer, result);
	parent->write_AT_cmd(sendPost, strlen(sendPost));
	SLEEP(1); // Give modem time to reply echo

	echoLen =  result + strlen(sendPre) + strlen(sendPost);
	while(echoLen > 0)
	{
		char buffer[180];
		int read_len = parent->read_AT_cmd(buffer, MIN(ELEMENTS(buffer), echoLen));
		if(!read_len) {
			SLEEP(5);
			timeout--;
		}

		echoLen -= read_len;

		if(read_len < 0 || timeout < 0) {
			MAKE_ERROR(modem, ERROR_RESPONCE, "Send echo not received %d %d\n", read_len, timeout);
			iModem->send_len = -1;
			return;
		}
	}

	modem->tx_amount += iModem->send_len;

	AT_handler_no_echo(modem->at_handler);
}

static const AT_Command cmd_AT_SSLTX = { "AT^SSLTX", SSLTX_cmd, 0, gen_result_hadler, gen_responce_hadler, 0, NULL };

static void  sslrcv_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	/*
	^SSLRX: 0
	TIMEOUT
	OK
	*/
    unsigned int len = (uint32_t)-1;
    uint16_t read = 0;
	int timeout = 1000;
	network_modem *modem = (network_modem*)(parent->modem);
	internal_modem *iModem = (internal_modem*)(((network_modem*)(parent->modem))->driver);

	sscanf(responce,"^SSLRX: %u", &len);

    if(len ==  (uint32_t)-1) {
		ERR_PR("sslreceive parse failed |%s|\n", responce);
		return;
	}

	DBG_PR("sslreceive responce is |%s|. data len is %u\n", responce, len);

	modem->rx_amount += len;

	if(len == 0)
	{
		//DBG_PR("Receiving TIMEOUT\n");
		iModem->receive_len = 0;
		// read timeout
		while(read < 9)
		{
			int read_len = AT_handler_read(
				modem->at_handler,
				&iModem->buffer[read],
				9 - read);

			if(read_len < 0 || timeout-- < 0)
			{
				ERR_PR("Read TIMEOUT failed\n");
				return;
			}
            read += (uint16_t)read_len;
			if(!read_len)
				SLEEP(1);
		}
		return;

	}
	else if(len <= MAX_RECEIVE)
	{
		// read data
		while(1)
		{
			int read_len = AT_handler_read(
				modem->at_handler,
				&iModem->buffer[read],
				1);

			if(read_len > 0)
			{
				timeout = 1000;

				iModem->buffer[read + read_len] = 0;

				if(iModem->buffer[read] == '\n')
				{
					size_t outLen = len;
					int result = base64decode(iModem->buffer, read - 1, (unsigned char*)iModem->receive_data, &outLen);

					//DBG_PR("SSL received %s\n", iModem->buffer);

					if(!result && outLen == len)
					{
						char * c = iModem->receive_data;

						if(outLen > 10 &&
							IS_ASCII(*c++) &&
							IS_ASCII(*c++) &&
							IS_ASCII(*c++) &&
							IS_ASCII(*c++) &&
							IS_ASCII(*c++) &&
							IS_ASCII(*c++)
						)
						{
							DBG_PR("SSL received %lu decoded:\n%s\n", (unsigned long)outLen, iModem->receive_data);
						}

						iModem->receive_len = outLen;
						return;
					}
					else
					{
						ERR_PR("SSL received decode failed %d %lu %u\n", result, (unsigned long)outLen, len);
						iModem->receive_len = -12;
						return;
					}
				}
                read += (uint16_t)read_len;
			}
			else
			{
				SLEEP(1); // Block until all is read
			}

			if(timeout-- < 0 || read_len < 0)
			{
				iModem->state = MODEM_ERROR;
				ERR_PR("SSL receive failed %d %d\n", read_len, timeout);
				iModem->receive_len = -1;
				return;
			}
		}

	}
	else
	{
		// error
		ERR_PR("SSL receive parse failed %u\n", len);
		iModem->receive_len = -123;
		return;
	}
}


static void SSLRX_cmd(const struct AT_Handler * parent)
{
	// AT^SSLRX=1,10,60

	char buffer[80];

	network_modem *modem = (network_modem*)(parent->modem);
	internal_modem *iModem = (internal_modem*)(((network_modem*)(parent->modem))->driver);

	sprintf(buffer, "AT^SSLRX=%d,%d,1", modem->crypted ? 1 : 0, iModem->receive_len);
	parent->write_AT_cmd(buffer, strlen(buffer));
	iModem->receive_len = -42;
	DBG_PR("Receiving cmd: %s\n", buffer);
}

static const AT_Command cmd_AT_SSLRECV = { "AT^SSLRX", SSLRX_cmd, 0, gen_result_hadler, sslrcv_responce_hadler, 0, NULL };


#define doIt(name, timeout) modem_do_command(This, &name, timeout)

#define doIt_ret_if_error(name) if(doIt(name, 500) <= 0 || This->error) \
	do{ iModem->state = MODEM_ERROR; return -1;}while(0)

#define doIt_long_ret_if_error(name) if(doIt(name, 5000) <= 0 || This->error) \
	do{ iModem->state = MODEM_ERROR; return -1;}while(0)

static int setup_connection(network_modem * This)
{
	int i;
	internal_modem *iModem = (internal_modem*)(This->driver);

	LIGHT_DBG_PR("Setup connection\n");

	for(i = 0; i < 10; i++)
	{
		This->error = 0;
		This->error_str[0] = 0;
		doIt_ret_if_error(cmd_AT);

		if(This->crypted)
		{
			doIt(cmd_AT_SSLC, 100);
		}
		else
		{
			doIt(cmd_AT_SSLC_UN, 100);
		}
		SLEEP(500);

		This->error = 0;
		doIt(cmd_AT_SSLO, 20000);

		if(This->error)
		{
			if(i >= 5)
			{
				LIGHT_DBG_PR("Ability Server does not respond! Retry %d\n", i - 4);
			}

			if(i == 5)
			{
				doIt_ret_if_error(cmd_AT_SSLDIS);
				SLEEP(3000);
			}

			doIt(cmd_AT_IPINIT, 1500);
			SLEEP(1000);

			if(This->crypted)
			{
				doIt(cmd_AT_SSLEN,  500);
			}
			else
			{
				doIt(cmd_AT_SSLEN_U, 500);
			}
			SLEEP(1000);
		}
		else
		{
			break;
		}
	}

	if(This->error)
	{
		MAKE_ERROR(This, ERROR_SERVER_DOES_NOT_RESPOND, "Ability server does not respond\n");
		return -1;
	}

	iModem->connected = 1;

	doIt_ret_if_error(cmd_AT_CCLK);

	doIt(cmd_AT_CMGD_0, 100);
	doIt(cmd_AT_CMGD_1, 100);
	doIt(cmd_AT_SLEEPCFG_0, 100);
	doIt(cmd_AT_SLEEPCFG_1, 100);
	doIt(cmd_AT_WAKEUPCFG, 100);

	if(iModem->connected == 1)
		iModem->state = MODEM_DATA;

	This->error_str[0] = 0;
	This->error = 0;

	return 0;

}

static int make_disconnect(network_modem * This)
{
	internal_modem *iModem = (internal_modem*)(This->driver);

	LIGHT_DBG_PR("Reboot modem\n");

	doIt_ret_if_error(cmd_AT_RESET);
	SLEEP(5000);

	iModem->state = MODEM_INIT;

	return 0;
}


static inline int init(network_modem * This)
{
	if (!This)
	{
		return -1;
	}

	network_modem *modem = This;
	static internal_modem modem_internals; // Only single modem supported
	internal_modem *iModem = &modem_internals;
	int waitWakeup = 60;

	modem->driver = &modem_internals;

	memset(&modem_internals, 0, sizeof(modem_internals));

	while (1)
	{
		doIt(cmd_AT, 100);

		if (!This->error)
		{
			doIt_ret_if_error(cmd_AT_ANTMODE_Q);

			if (modem_internals.antModeWrong)
			{
				LIGHT_DBG_PR("Fix ANTMODE to 1\n");
				doIt_ret_if_error(cmd_AT_ANTMODE);
				modem_internals.antModeWrong = 0;
				SLEEP(10000); // Wait reboot
				continue;
			}
			doIt(cmd_AT_ICCID, 1000);
			break;
		}

		This->error = 0;

		if (waitWakeup-- < 0)
			doIt_ret_if_error(cmd_AT);
	}

	doIt_ret_if_error(cmd_AT_CURC);

	doIt_ret_if_error(cmd_ATZ);
	doIt_ret_if_error(cmd_ATV1);

	doIt_ret_if_error(cmd_AT_CGMR);
	doIt_ret_if_error(cmd_AT_CGMM);

	if (This->modem_wakeup(This)) return -1;

	doIt_ret_if_error(cmd_AT_ICCID);
	doIt_ret_if_error(cmd_AT_EONS);

	return 0;
}

static inline int m_write(network_modem * This, const char *buffer, unsigned int len)
{
	int i;
	const int loopCnt = 4;
	unsigned int send      = 0;
	network_modem *modem   = This;
	internal_modem *iModem = (internal_modem*)(This->driver);

	if(iModem->state != MODEM_DATA)
	{
		return -1;
	}

	while(send < len)
	{
		int required = len - send;
		int transfer = MIN(required, MAX_SEND);

		for( i = 0; i < loopCnt; i++)
		{
			iModem->send_data = &buffer[send];
			iModem->send_len  = transfer;

			doIt(cmd_AT, 10);
			This->error = 0;

			if(doIt(cmd_AT_SSLTX, 2000) <= 0 || This->error)
			{
				This->error = 0;
				modem->error_str[0] = 0;
				iModem->state = MODEM_DATA;
				iModem->send_len = -1;
				SLEEP(2000);
				continue;
			}
			else
			{
				break;
			}
		}
		if(iModem->send_len < 0 || loopCnt == i) {
			MAKE_ERROR(modem, ERROR_CMD_FAILED, "Transmitting fails\n");
			return -30;
		}

		send += transfer;
	}



	if(iModem->send_len < 0)
	{
		iModem->state = MODEM_ERROR;
	}

	return send;
}

static inline int m_read(network_modem * This, char *buffer, unsigned int len)
{
	int i;
	const int loopCnt = 2;
	network_modem *modem = This;
	internal_modem *iModem = (internal_modem*)(This->driver);

	len = MIN(MAX_RECEIVE, len);

	if(iModem->state != MODEM_DATA)
		return -1;

	if(!len)
	{
		if(modem->crypted)
		{
			doIt_ret_if_error(cmd_AT_SSLSTAT);
		}
		else
		{
			doIt_ret_if_error(cmd_AT_SSLSTAT_U);
		}

		if(iModem->state != MODEM_DATA)
			return -1;
		else
			return 0;
	}

	for( i = 0; i < loopCnt; i++)
	{
		iModem->receive_data = buffer;
		iModem->receive_len  = len;

		doIt(cmd_AT, 10);
		This->error = 0;

		if(doIt(cmd_AT_SSLRECV, 2000) <= 0 || This->error)
		{
			This->error = 0;
			modem->error_str[0] = 0;
			iModem->state = MODEM_DATA;
			SLEEP(2000);
			continue;
		}
		else
		{
			break;
		}
	}

	if(iModem->receive_len < 0 || loopCnt == i)
	{
		MAKE_ERROR(modem, ERROR_CMD_FAILED, "Receiving fails\n");
		return -30;
	}

	return iModem->receive_len;
}

static inline int unsolicited(network_modem *This, const char * responce)
{
	if (!This || !responce)
	{
		return 0;
	}

	network_modem *modem = This;
	internal_modem *iModem = (internal_modem*)(This->driver);

	if (strstr(responce, "^SSLRX: "))
	{
		if (modem->at_handler->cmd == &cmd_AT_SSLRECV)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}

	if (strstr(responce, "^ICCID: ") || strstr(responce, "^SSLSTAT: "))
	{
		return 0;
	}

	DBG_PR("unsolicited: %s\n", responce);

	if (
		responce[0] == '+' &&
		responce[1] == 'C' &&
		responce[2] == 'U' &&
		responce[3] == 'S'
	)
	{
			return 1;
	}

	if (strstr(responce, "NO CARRIER") ||
		strstr(responce, "DISCONNECTED") ||
		strstr(responce, "^SYSSTART") ||
		strstr(responce, "^IPSTATE: 2,0,0")
	)
	{

		iModem->connected = 2;

		ERR_PR("Server disconnected %s\n", responce);

		iModem->state = MODEM_ERROR;
		return 1;
	}

	if (responce[0] == '^')
		return 1;
	else
		return 0;
}

static inline int m_connect(network_modem *This, int discon)
{
	//network_modem *modem = This;
	internal_modem *iModem = (internal_modem*)(This->driver);

	if(discon)
	{
		switch(iModem->state)
		{
		case MODEM_DATA:
			return make_disconnect(This);
		default:
			return 0;
		}
	}
	else
	{
		iModem->state = MODEM_INIT;

		return setup_connection(This);
	}
}


#ifndef USE_HUAEWEI_EM909
#define USE_HUAEWEI_EM909 0
#endif

#if USE_HUAEWEI_EM909
int  impl_modem_init(network_modem *This) { return init(This); }
int  impl_modem_write(network_modem *This, const char *buffer, int len) { return m_write(This, buffer, len); }
int  impl_modem_read(network_modem *This, char *buffer, int len) { return m_read(This, buffer, len); }
int impl_modem_unsolicited(network_modem *This, const char * responce) { return unsolicited(This, responce); }
int impl_modem_connect(network_modem *This, int disconnect) { return m_connect(This, disconnect); };
#else
void avoid_warnings_huawei_em909____(void) // Fake call to make compiler happy
{
	if((USE_HUAEWEI_EM909))
	{
		init(NULL);
		m_read(NULL, NULL, 0);
		m_write(NULL, NULL, 0);
		unsolicited(NULL, NULL);
		m_connect(NULL, 0);
	}
}
#endif
