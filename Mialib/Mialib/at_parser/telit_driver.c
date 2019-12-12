#include "modem_driver.h"
#include "os_support.h"
#include <string.h>

#define MAX_SEND 1000

#define MODULE_NAME "telit_driver"



typedef enum { CERTIFICATE_NOT_CHECKED = 0, CERTIFICATE_SAME, CERTIFICATE_NOT_SAME } CaCertificate;

typedef enum { MODEM_INIT = 0, MODEM_ERROR, MODEM_DATA } ModemState;

typedef struct telit_modem {

	char * receive_data;
	int receive_len;
	const char * send_data;
	int send_len;
	CaCertificate caCerficateOk;
	ModemState state;


} telit_modem;

static void  gen_result_hadler( const struct AT_Handler * parent, const char * result)
{
	network_modem *modem = (network_modem*)(parent->modem);

	//DBG_PR("cmd_at result: |%s|\n", result);

	if(strstr(result, "ERROR")) {
		telit_modem *telit = (telit_modem*)(modem->driver);

		MAKE_ERROR(modem, ERROR_RESULT, "ERROR found from AT command!\n");
		telit->state = MODEM_ERROR;
	}

	modem->cmdDone = 1;
}

static void ignorant_result_hadler( const struct AT_Handler * parent, const char * result)
{
	network_modem *modem = (network_modem*)(parent->modem);

	//DBG_PR("Ignored result result: |%s|\n", result);

	modem->cmdDone = 1;
}


static void gen_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);
	telit_modem *telit = (telit_modem*)(modem->driver);

	DBG_PR("cmd_at responce: |%s|\n", responce);
	ERR_PR("ERROR no responce should come from AT command %s\n", parent->cmd->cmd);

	MAKE_ERROR(modem, ERROR_RESPONCE, "%s RESPONCE not expected: %s\n", parent->cmd->cmd, responce);

	telit->state = MODEM_ERROR;
}

/*
static void version_handler(const struct AT_Handler * parent, const char * responce)
{
	DBG_PR("Version is |%s|\n", responce);
}

CREATE_ATCommand(cmd_AT_CGMR, "AT+CGMR", gen_result_hadler, version_handler);
*/

static void module_handler(const struct AT_Handler * parent, const char * responce)
{
	DBG_PR("Module is |%s|\n", responce);
}

CREATE_ATCommand(cmd_AT_ENHRST, "AT#ENHRST=1,0", gen_result_hadler, module_handler);

//CREATE_ATCommand(cmd_AT_CGMM, "AT+CGMM", gen_result_hadler, module_handler);

//CREATE_ATCommand(cmd_AT_WS64_12, "AT+WS46=12", gen_result_hadler, gen_responce_hadler);

//CREATE_ATCommand(cmd_AT_STIA_2_1, "AT#STIA=2,1", gen_result_hadler, gen_responce_hadler);

//CREATE_ATCommand(cmd_AT_PLMNMODE_1, "AT#PLMNMODE=1", gen_result_hadler, gen_responce_hadler);


CREATE_ATCommand(cmd_AT_SSLSECCFG_require_cert, "AT#SSLSECCFG=1,0,1", gen_result_hadler, gen_responce_hadler);

CREATE_ATCommand(cmd_AT_SSLSECCFG_no_cert, "AT#SSLSECCFG=1,0,0", gen_result_hadler, gen_responce_hadler);

CREATE_ATCommand(cmd_AT_SSLEN_ENABLE, "AT#SSLEN=1,1", gen_result_hadler, gen_responce_hadler);

CREATE_ATCommand(cmd_AT_SSLEN_DISABLE, "AT#SSLEN=1,0", ignorant_result_hadler, gen_responce_hadler);

CREATE_ATCommand(cmd_AT_CPUMODE3, "at#cpumode=3", gen_result_hadler, gen_responce_hadler);


static void SSLD_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);

	char buffer[200];
	snprintf(buffer, ELEMENTS(buffer), "AT#SSLD=1,%u,\"%s\",0,1,5000",
		modem->server_port,
		modem->server_addr);
	buffer[ELEMENTS(buffer) - 1] = 0;
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static const AT_Command cmd_AT_SSLD = { "AT#SSLD=1,", SSLD_cmd, 0, gen_result_hadler, gen_responce_hadler, 0, NULL };


//CREATE_ATCommand(cmd_AT_ATE0, "ATE0", gen_result_hadler, gen_responce_hadler);

static void  sslsend_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	int timeout = 200;
	network_modem *modem = (network_modem*)(parent->modem);
	telit_modem *telit = (telit_modem*)(modem->driver);
	unsigned int echo_len = telit->send_len;

	if(responce[0] != '>') {
		DBG_PR("sslsend responce discarded %c %d\n", responce[0], responce[1]);
		return;
	}

	SLEEP(5);

	// Flush input
	if(1) {
		char buffer[20];
		while(parent->read_AT_cmd(buffer, 20));
	}

	//DBG_PR("sslsend: sending %d bytes data\n",  telit->send_len);
	if(parent->write_AT_cmd(telit->send_data, telit->send_len) != telit->send_len) {
		telit->send_len = -1;
		MAKE_ERROR(modem, ERROR_RESPONCE, "Write out failed\n");
		return;
	}

	SLEEP(5);

	while(echo_len) { // Read echo manually

		int read_len = MIN(echo_len, 20);
		char buffer[20];
		read_len = parent->read_AT_cmd(buffer, read_len);
		echo_len -= read_len;
		if(!read_len) {
			SLEEP(10);
			timeout--;
		}
		if(read_len < 0 || timeout < 0) {
			telit->send_len = -1;
			DBG_PR("Echo error %u %d %d\n", echo_len, read_len, timeout);
			MAKE_ERROR(modem, ERROR_RESPONCE, "Send echo not received\n");
			return;
		}
	}
}

static void SSLSENDEXT_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);
	telit_modem *telit = (telit_modem*)(modem->driver);

	char buffer[80];
	sprintf(buffer, "AT#SSLSENDEXT=1,%d,600", telit->send_len);
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static const AT_Command cmd_AT_SSLSEND = { "AT#SSLSENDEXT=1", SSLSENDEXT_cmd, '>', gen_result_hadler, sslsend_responce_hadler, 0, NULL };


static void  sslrcv_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	unsigned int len;
	unsigned int read = 0;
	int timeout = 1000;
	network_modem *modem = (network_modem*)(parent->modem);
	telit_modem *telit = (telit_modem*)(modem->driver);

	if(strstr(responce, "#SSLRECV:")) {
		sscanf(responce,"#SSLRECV: %u", &len);

		//DBG_PR("sslreceive responce is |%s|. read len is %u\n", responce, len);

		telit->receive_len = -1;

		while(read < len) {
			int read_len = AT_handler_read(
				modem->at_handler,
				&telit->receive_data[read],
				len - read);

			if(read_len > 0) {
				//DBG_PR("SSL received %d\n", read_len);
				read += read_len;
			}

			if(read < len)
				SLEEP(10); // Block until all is read

			if(timeout-- < 0) {
				telit->state = MODEM_ERROR;
				DBG_PR("SSL receive failed %d\n", read_len);
				return;
			}

		}
		telit->receive_len = read;
	}
}

static void SSLRECV_cmd(const struct AT_Handler * parent)
{
	char buffer[80];
	network_modem *modem = (network_modem*)(parent->modem);
	telit_modem *telit = (telit_modem*)(modem->driver);

	sprintf(buffer, "AT#SSLRECV=1,%d,10", telit->receive_len);
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static const AT_Command cmd_AT_SSLRECV = { "AT#SSLRECV=1,", SSLRECV_cmd, 0, gen_result_hadler, sslrcv_responce_hadler, 0, NULL };


//AT#SSLSECCFG=1,0,1,1
//OK
// <cert_format> = 1, PEM format is selected

static void SSLSECDATA_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);
	char buffer[30];
	unsigned int len = 0;

	while(1)
	{
		char tmp[20];
		int get = modem->getCACertificate(tmp, ELEMENTS(tmp), len);
		len += get;
		if(!get)
			break;
	}

	sprintf(buffer, "AT#SSLSECDATA=1,1,1,%u",len);
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static void  sslsecdata_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);
	//telit_modem *telit = (telit_modem*)(modem->driver);
	const char terminate = 26;

	if(responce[0] == '>' && responce[1] == 0) {

		int written = 0;

		while(1) {
			char buffer[80];
			unsigned int len = modem->getCACertificate(buffer, ELEMENTS(buffer), written);

			if(!len)
				break;

			parent->write_AT_cmd(buffer, len);
			written += len;
		}

		parent->write_AT_cmd(&terminate, 1);
		DBG_PR("New ceritificate written\n");
	}
}

static const AT_Command cmd_AT_SSLSECDATA = { "AT#SSLSECDATA=1,1,1,<size>", SSLSECDATA_cmd, '>', gen_result_hadler, sslsecdata_responce_hadler, 0, NULL };



static void SSLSECDATA_CHECK_responce_handler(const struct AT_Handler * parent, const char * responce)
{
	int SSid, CAcertIsSet, CertiIsSet, PrivKeyIsSet;

	sscanf(responce, "#SSLSECDATA: %d,%d,%d,%d",
	&SSid,
	&CertiIsSet,
	&CAcertIsSet,
	&PrivKeyIsSet
	);

	DBG_PR("Certificate check |%s|. Server serti is set %d\n", responce, CAcertIsSet);
}

CREATE_ATCommand(cmd_AT_SSLSECDATA_CHECK, "AT#SSLSECDATA?", gen_result_hadler, SSLSECDATA_CHECK_responce_handler);


static void  SSLSECDATA_COMPARE_result_hadler( const struct AT_Handler * parent, const char * result)
{
	network_modem *modem = (network_modem*)(parent->modem);
	telit_modem *telit = (telit_modem*)(modem->driver);

	DBG_PR("cmd_at result: |%s|\n", result);

	if(strstr(result, "ERROR")) {
		MAKE_ERROR(modem, ERROR_RESULT, "ERROR found from AT command!\n");
	}

	if(telit->caCerficateOk == CERTIFICATE_NOT_CHECKED) {
		telit->caCerficateOk = CERTIFICATE_NOT_SAME;
	}

	modem->cmdDone = 1;
}

static void SSLSECDATA_COMPARE_responce_handler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);
	telit_modem *telit = (telit_modem*)(modem->driver);

	if(telit->caCerficateOk != CERTIFICATE_NOT_CHECKED)
		return;

	if(responce[0] == 'M' && responce[1] == 'I' && responce[2] == 'I' )
	{
		char buffer[20];

		unsigned int len = modem->getCACertificate(buffer, ELEMENTS(buffer), 28);

		telit->caCerficateOk = CERTIFICATE_NOT_SAME;

		if(len && len != ELEMENTS(buffer)) {
			MAKE_ERROR(modem, ERROR_RESPONCE, "CA certificate read failed\n");
		}

		if(!memcmp(buffer, responce, ELEMENTS(buffer))) {
			telit->caCerficateOk = CERTIFICATE_SAME;
			DBG_PR("Cartificate are same\n");
		}
		else {
			DBG_PR("Certificate are not same %c %c %c\n",
				buffer[0],
				buffer[1],
				buffer[2]
			);
		}
	}

}

CREATE_ATCommand(cmd_AT_SSLSECDATA_COMPARE, "AT#SSLSECDATA=1,2,1", SSLSECDATA_COMPARE_result_hadler, SSLSECDATA_COMPARE_responce_handler);

static void SGACT_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);

	char buffer[80];
	snprintf(buffer, ELEMENTS(buffer), "AT#SGACT=1,1,\"%s\",\"%s\"",
		modem->user_name,
		modem->user_password);
	parent->write_AT_cmd(buffer, strlen(buffer));
}
static void  SGACT_EN_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	DBG_PR("SGACT_EN responce is |%s|\n", responce);
}

const AT_Command cmd_AT_SGACT_EN = { "AT#SGACT=1,1,\"%s\",\"%s\"", SGACT_cmd, 0, ignorant_result_hadler, SGACT_EN_responce_hadler, 0, NULL };

static void CGDCONT_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);
	char buffer[80];

	snprintf(buffer, ELEMENTS(buffer), "AT+CGDCONT=1,\"IP\",\"%s\"", modem->user_apn);
	parent->write_AT_cmd(buffer, strlen(buffer));
}

const AT_Command cmd_AT_CGDCONT = { "AT+CGDCONT=1,\"IP\",\"%s\"", CGDCONT_cmd, 0, gen_result_hadler, gen_responce_hadler, 0, NULL };

static void  connection_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	DBG_PR("CGDCONT? responce is |%s|\n", responce);
}

CREATE_ATCommand(cmd_AT_CGDCONT_Q, "AT+CGDCONT?", gen_result_hadler, connection_responce_hadler);

CREATE_ATCommand(cmd_AT_AUTOBND, "AT#AUTOBND=2", gen_result_hadler, gen_responce_hadler);
CREATE_ATCommand(cmd_AT_SELINT,  "AT#SELINT=2", gen_result_hadler, gen_responce_hadler);



#define doIt(name, timeout) modem_do_command(This, &name, timeout)

#define doIt_ret_if_error(name) if(doIt(name, 300) <= 0 || This->error) \
	do{ telit->state = MODEM_ERROR; return -1;}while(0)

#define doIt_long_ret_if_error(name) if(doIt(name, 6000) <= 0 || This->error) \
	do{ telit->state = MODEM_ERROR; return -1;}while(0)


static int setup_connection(network_modem * This)
{
	telit_modem *telit = (telit_modem*)(This->driver);

	LIGHT_DBG_PR("Setup connection\n");

	doIt_ret_if_error(cmd_AT_ENHRST);
	SLEEP(5000);

	doIt_ret_if_error(cmd_AT_SELINT);
	doIt_ret_if_error(cmd_AT_AUTOBND);

	doIt_ret_if_error(cmd_AT_CPUMODE3);

	//doIt_ret_if_error(cmd_AT_WS64_12);

	//doIt_ret_if_error(cmd_AT_STIA_2_1);
	//doIt_ret_if_error(cmd_AT_PLMNMODE_1);

	if(This->modem_wakeup(This)) return -1;

	doIt_ret_if_error(cmd_AT_CGDCONT);
	doIt_ret_if_error(cmd_AT_CGDCONT_Q);
	doIt_long_ret_if_error(cmd_AT_SGACT_EN);

	doIt_ret_if_error(cmd_AT_SSLEN_DISABLE);
	doIt_ret_if_error(cmd_AT_SSLEN_ENABLE);

	if(1) // Scope for buffer
	{
		char buffer[30];
		if(This->getCACertificate(buffer, ELEMENTS(buffer), 0) > 0) // If any certificate installed
		{
			doIt_ret_if_error(cmd_AT_SSLSECCFG_require_cert);

			doIt_ret_if_error(cmd_AT_SSLSECDATA_COMPARE);

			if(telit->caCerficateOk != CERTIFICATE_SAME) {
				doIt_ret_if_error(cmd_AT_SSLSECDATA);
			}
		}
		else
		{
			doIt_ret_if_error(cmd_AT_SSLSECCFG_no_cert);
		}
		doIt_ret_if_error(cmd_AT_SSLSECDATA_CHECK);
	}

	doIt_long_ret_if_error(cmd_AT_SSLD);

	telit->state = MODEM_DATA;

	return 0;

}

static int make_disconnect(network_modem * This)
{
	telit_modem *telit = (telit_modem*)(This->driver);

	LIGHT_DBG_PR("Reboot modem\n");

	doIt_ret_if_error(cmd_AT_ENHRST);

	telit->state = MODEM_INIT;

	return 0;
}


static inline int init(network_modem * This)
{
	network_modem *modem = This;
	static telit_modem telit_mem; // Only singel modem supported

	modem->driver = &telit_mem;

	memset(&telit_mem, 0, sizeof(telit_mem));

	return 0;
}

static inline int m_write(network_modem * This, const char *buffer, unsigned int len)
{
	unsigned int send = 0;
	network_modem *modem = This;
	telit_modem *telit = (telit_modem*)(modem->driver);

	if(telit->state != MODEM_DATA)
	{
		return -1;
	}

	while(send < len)
	{
		int required = len - send;
		int transfer = MIN(required, MAX_SEND);

		telit->send_data = &buffer[send];
		telit->send_len  = transfer;

		doIt_long_ret_if_error(cmd_AT_SSLSEND);

		send += transfer;
	}

	if(telit->send_len < 0)
		telit->state = MODEM_ERROR;

	return send;
}

static inline int m_read(network_modem * This, char *buffer, unsigned int len)
{
	network_modem *modem = This;
	telit_modem *telit = (telit_modem*)(modem->driver);

	len = MIN(MAX_SEND, len);

	if(telit->state != MODEM_DATA)
		return -1;

	if(!len)
		return 0;

	telit->receive_data = buffer;
	telit->receive_len  = len;

	doIt_long_ret_if_error(cmd_AT_SSLRECV);

	if(telit->receive_len < 0)
		telit->state = MODEM_ERROR;

	return telit->receive_len;
}

static inline int unsolicited(network_modem *This, const char * responce)
{
	if (!This || !responce)
	{
		return 0;
	}

	network_modem *modem = This;
	telit_modem *telit = (telit_modem*)(modem->driver);

	DBG_PR("unsolicited: %s\n", responce);

	if (strstr(responce, "NO CARRIER") || strstr(responce, "DISCONNECTED")) {

		DBG_PR("Server disconnected\n");

		telit->state = MODEM_ERROR;

		return 1;
	}
	return 0;
}

static inline int telit_connect(network_modem *This, int discon)
{
	network_modem *modem = This;
	telit_modem *telit = (telit_modem*)(modem->driver);

	if(discon)
	{
		switch(telit->state)
		{
		case MODEM_DATA:
			return make_disconnect(This);
		default:
			return 0;
		}
	}
	else
	{
		telit->state = MODEM_INIT;

		return setup_connection(This);
/*
		switch(telit->state)
		{
		case MODEM_DATA:
			return 0;
		default:
			return setup_connection(This);
		}
*/
	}
}


#ifndef USE_TELIT_DRIVER
#define USE_TELIT_DRIVER 0
#endif

#if USE_TELIT_DRIVER
int impl_modem_init(network_modem *This) { return init(This); }
int impl_modem_write(network_modem *This, const char *buffer, int len) { return m_write(This, buffer, len); }
int impl_modem_read(network_modem *This, char *buffer, int len) { return m_read(This, buffer, len); }
int impl_modem_unsolicited(network_modem *This, const char * responce) { return unsolicited(This, responce); }
int impl_modem_connect(network_modem *This, int disconnect) { return telit_connect(This, disconnect); };
#else
void avoid_warnings_telit(void) // Fake call to make compiler happy
{
	if((USE_TELIT_DRIVER))
	{
		init(NULL);
		m_read(NULL, NULL, 0);
		m_write(NULL, NULL, 0);
		unsolicited(NULL, NULL);
		telit_connect(NULL, 0);
	}
}
#endif
