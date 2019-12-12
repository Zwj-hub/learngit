#include "modem_driver.h"
#include <string.h>
#include "os_support.h"
#include <time.h>
#include <stdio.h>

#define MODULE_NAME "modem_driver"

#ifndef SIGNAL_QUALITY_CHECK_INTERVAL
#define SIGNAL_QUALITY_CHECK_INTERVAL 20
#endif

#ifndef CGREG_CMD
#define CGREG_CMD "CGREG"
#endif

#ifndef SIGNAL_QUALITY_CMD
#define SIGNAL_QUALITY_CMD "CSQ"
#endif

#ifndef RX_SIGNAL_MAX 
#define	RX_SIGNAL_MAX 31
#endif

#define WAIT_REGISTERING_CNT 50 // 5 * 50 = 250s

#define INVALID_TIMEZONE 0xffffff

uint8_t g_modem_output = 0xff; // Contol modem printing at runtime

struct tm *localtime_r(const time_t *timep, struct tm *result);

static void gen_result_hadler( const struct AT_Handler * parent, const char * result)
{
	network_modem *modem = (network_modem*)(parent->modem);

	DBG_PR("cmd_at result: |%s|\n", result);

	modem->cmdDone = 1;

	if(strstr(result, "ERROR")) {
		MAKE_ERROR(modem, ERROR_RESULT, "ERROR found from AT command!");
		ERR_PR("cmd:%s res:|%s|\n",parent->cmd->cmd, result);
	}
}

#if 1
static void ignorant_result_hadler( const struct AT_Handler * parent, const char * result)
{
	network_modem *modem = (network_modem*)(parent->modem);

	DBG_PR("Ignored result result: |%s|\n", result);

	modem->cmdDone = 1;
}
#endif

static void gen_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);

	ERR_PR("ERROR no responce |%s| should come from AT command %s\n", responce, parent->cmd->cmd);

	MAKE_ERROR(modem, ERROR_RESPONCE, "%s RESPONCE not expected: %s\n", parent->cmd->cmd, responce);
}

static void ati_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
    //DBG_PR("%s\n", responce);       //zwj
    printf("%s\n", responce);
}


CREATE_ATCommand(cmd_ATI, "ATI", gen_result_hadler, ati_responce_hadler);
CREATE_ATCommand(cmd_AT, "AT", gen_result_hadler, gen_responce_hadler);
CREATE_ATCommand(cmd_AT_CMEE,    "AT+CMEE=2", gen_result_hadler, gen_responce_hadler);


static void sim_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);

	DBG_PR("AT+CPIN? responce is |%s|\n", responce);

	if(strstr(responce, "BUSY") || strstr(responce, "Busy") || strstr(responce, "busy")) {
		modem->error = ERROR_SIM;
		return;
	}

	if(strstr(responce, "SIM PIN")) {
		modem->error = ERROR_SIM_PIN;
		return;
	}
	
	if(strstr(responce, "SIM PUK")) {
		modem->error = ERROR_SIM_PUK;
		return;
	}
	
	if(!strstr(responce, "READY")) {
		MAKE_ERROR(modem, ERROR_SIM, "SIM: %s\n", responce);
		return;
	}
	modem->error = 0;
	modem->error_str[0] = 0;
}

CREATE_ATCommand(cmd_AT_CPIN, "AT+CPIN?", gen_result_hadler, sim_responce_hadler);

CREATE_ATCommand(cmd_AT_CREG, "AT+" CGREG_CMD "=0", gen_result_hadler, gen_responce_hadler);
static void AT_CPIN_cmd(const struct AT_Handler * parent)
{
	network_modem *modem = (network_modem*)(parent->modem);

	char * pin = NULL;
	
	if(modem->pinCb)
	{
		modem->pinCb(CB_PIN_GET, &pin);
	}

	if(pin && strlen(pin) < 40 && strlen(pin) > 3 )
	{
		char buffer[200];
		snprintf(buffer, ELEMENTS(buffer), "AT+CPIN=\"%s\"", pin);
		buffer[ELEMENTS(buffer) - 1] = 0;
		parent->write_AT_cmd(buffer, strlen(buffer));
	}
	else
	{
		ERR_PR("SIM card PIN not available\n");
		parent->write_AT_cmd("AT", 2);
	}
}

static const AT_Command cmd_AT_CPIN_UNLOCK = { "AT+CPIN=", AT_CPIN_cmd, 0, ignorant_result_hadler, gen_responce_hadler, 0, NULL };


static void  CREG_Q_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	int stat = 1000;
	int mode = 1000;
	network_modem *modem = (network_modem*)(parent->modem);

	DBG_PR(CGREG_CMD " Q responce is |%s|\n", responce);

	sscanf(responce,"+" CGREG_CMD ": %d,%d",&mode, &stat);

	DBG_PR("Mode %d Stat %d\n", mode, stat);

/*
0 not registered, MT is not currently searching a new operator to register to
1 registered, home network
2 not registered, but MT is currently searching a new operator to register to
3 registration denied
4 unknown (e.g. out of GERAN/UTRAN/E-UTRAN coverage)
5 registered, roaming
6 registered for "SMS only", home network (applicable only when indicates E-UTRAN)
7 registered for "SMS only", roaming (applicable only when indicates E-UTRAN)
8 attached for emergency bearer services only (see NOTE 2) (not applicable)
9 registered for "CSFB not preferred", home network (applicable only when indicates E-UTRAN)
10 registered for "CSFB not preferred", roaming (applicable only when indicates E-UTRAN)

*/
	if(stat >= 6) {
		MAKE_ERROR(modem, ERROR_CREG_REGISTERING_FAILED, "Registering network failed (%d)\n", stat);
	}
	else if(stat == 4) {
		MAKE_ERROR(modem, ERROR_CREG_REGISTERING_FAILED, "Registering failed. No network\n");
	}
	else if(stat == 3) {
		MAKE_ERROR(modem, ERROR_CREG_REGISTERING_FAILED, "SIM Registering denied\n");
	}
	else if(stat == 0) {
		MAKE_ERROR(modem, ERROR_CREG_REGISTERING_FAILED, "SIM Registering failed\n");
	}

	else if(stat == 1 || stat == 5) {
		modem->registed_network = 1;
	}
}

CREATE_ATCommand(cmd_AT_CREG_Q, "AT+" CGREG_CMD "?", gen_result_hadler, CREG_Q_responce_hadler);


static int unsolicited_result(struct AT_Handler * parent, const char * responce)
{
	return impl_modem_unsolicited((network_modem*) parent->modem, responce);
}

static void  signal_quolatity_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	int snr  = -1;
	int error = 99;
	network_modem *modem = (network_modem*)(parent->modem);
	static int last_report = -1;
	sscanf(responce,"+" SIGNAL_QUALITY_CMD ": %d,%d",&snr, &error);
  //  printf("CSQ response:%s\n",responce);
	if(snr < 0)
		return;
/*
 * +CSQ:<rssi>,<ber>
where
<rssi> - received signal strength indication
0 - (-113) dBm or less
1 - (-111) dBm
2..30 - (-109)dBm..(-53)dBm / 2 dBm per step
31 - (-51)dBm or greater
99 - not known or not detectable*
 *
*/
	if(snr && snr < 99) 
	{
		if(snr >= RX_SIGNAL_MAX - 2)
		{
			modem->signal_quality = 100;
		}
		else
		{
			modem->signal_quality = 110 - MIN( (RX_SIGNAL_MAX - snr + 2) * (100 / RX_SIGNAL_MAX + 1), 109);
			modem->signal_quality = MIN(99, modem->signal_quality);
		}
	}
	else
	{
		modem->signal_quality = 0;
	}
	
	if(last_report != modem->signal_quality)
	{
		time_t now = time(NULL);
		struct tm nowTm;
		
		last_report = modem->signal_quality;
		
		localtime_r(&now, &nowTm);

		if(snr < 99) {
			LIGHT_DBG_PR("Radio Signal Quality is %d == %d%%. UTC %02d:%02d\n",
			snr,
			modem->signal_quality,
			nowTm.tm_hour,
			nowTm.tm_min
			);
		}
		else {
			ERR_PR("Radio Signal not detectable. UTC %02d:%02d\n",
			nowTm.tm_hour,
			nowTm.tm_min
			);
			LIGHT_DBG_PR("Check antenna location and connection\n");
		}

		if(modem->error)
			LIGHT_DBG_PR("Modem %s. %s\n", ERROR_TO_STR(modem->error), modem->error_str);
	}
}

CREATE_ATCommand(cmd_AT_CSQ, "AT+" SIGNAL_QUALITY_CMD, gen_result_hadler, signal_quolatity_responce_hadler);


static void  operator_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);

	const char *letter = responce;
	char *target       = modem->network;
	int limit = ELEMENTS(modem->network);

	DBG_PR("Operator responce is |%s|\n", responce);

	while(*letter) {
		if(*letter++ == '\"' )
		 break;
	}

	while(*letter) {

		if(*letter == '\"' )
			break;

		if(--limit <= 0)
			break;

		*target++ = *letter++;
	}

	*target = 0;

	if(!modem->network[0])
	{
		strcpy(modem->network, "N/A");
	}
}

CREATE_ATCommand(cmd_AT_COPS, "AT+COPS?", gen_result_hadler, operator_responce_hadler);


static void  cclk_responce_handler(const struct AT_Handler * parent, const char * responce)
{
	network_modem *modem = (network_modem*)(parent->modem);

	struct tm t = { 0 };
	t.tm_isdst = -1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
	int timezone;

	DBG_PR("CCLK responce is |%s|\n", responce);

	//+CCLK: 2019/1/18,4:9:3GMT+8

	int howMany = sscanf(responce,
		"+CCLK: %d/%d/%d,%d:%d:%dGMT%d",
		&t.tm_year,
		&t.tm_mon,
		&t.tm_mday,
		&t.tm_hour,
		&t.tm_min,
		&t.tm_sec,
		&timezone
	);

	if(howMany >= 6)
	{
		// set time
		if(t.tm_year > 2100 || t.tm_year <= 2018  || !t.tm_mon)
		{
			LIGHT_DBG_PR("Bad time from modem. %s\n", responce);
			return;
		}
		t.tm_year -= 1900; // Year starts from 1900
		t.tm_mon  -= 1;    // Month index starts from 0

		modem->epoch = mktime(&t);

		if(howMany >= 7)
		{
			modem->timezone = timezone * 3600;
		}
	}
	else
	{
		LIGHT_DBG_PR("Bad time from modem %s\n", responce);
	}
}

CREATE_ATCommand(cmd_AT_CCCLK, "AT+CCLK?", gen_result_hadler, cclk_responce_handler);


int modem_do_command(network_modem *This, const AT_Command *cmd, int timeout)
{
	network_modem *modem = This;
	This->cmdDone = 0;

	if(AT_handler_cmd(This->at_handler, cmd)) {
		MAKE_ERROR(modem, ERROR_CMD_FAILED, "cmd failed %s\n", cmd->cmd);
		return -ERROR_CMD_FAILED;
	}

	while(!This->cmdDone && timeout--){
		SLEEP(10);
		AT_handler_poll(This->at_handler);
		if(!timeout) {
			MAKE_ERROR(modem, ERROR_MODEM_TIMEOUT, "cmd timeout %s\n", cmd->cmd);
			This->at_handler->state = AT_HANDLER_IDLE;
			return -ERROR_MODEM_TIMEOUT;
		}
	}

	SLEEP(1);

	return This->cmdDone;
}


#define doIt(name) modem_do_command(This, &name, 300)
#define doIt_ret_if_error(name) if(doIt(name) <= 0 || This->error) return -1;

#define doIt_long(name) modem_do_command(This, &name, 6000)
#define doIt_long_ret_if_error(name) if(doIt_long(name) <= 0 || This->error) return -1;

static int modem_wakeup(network_modem *This)
{
	int i;
	int first = 1; 
	int tryCount = WAIT_REGISTERING_CNT;

	This->error = 0;
	doIt_ret_if_error(cmd_AT);
	doIt_ret_if_error(cmd_ATI);
	
	#define PIN_CB(a,b) if(This->pinCb){This->pinCb(a, b);}
	
	This->error = 0;
	doIt(cmd_AT_CPIN);
	if(This->error)
	{
		SLEEP(4000);
	}
	
	i = 0;
	while(1)
	{
		This->error = 0;
		doIt(cmd_AT_CPIN);
		
		switch(This->error)
		{
		case 0:
			if(first)
			{
				PIN_CB(CB_PIN_NOT_LOCKED, NULL);
			}
			else
			{
				PIN_CB(CB_PIN_OK, NULL);
			}
			break;

		case ERROR_SIM:
			break;
			
		case ERROR_SIM_PIN:
			if(first)
			{
				This->error = 0;
				doIt_ret_if_error(cmd_AT_CPIN_UNLOCK);
				This->error = ERROR_SIM_PIN;
				first = 0;
			}
			else
			{
				PIN_CB(CB_MIA_PIN_BAD, NULL);
				MAKE_ERROR(This, ERROR_SIM_PIN, "SIM PIN wrong\n");
				return -1;
			}
			break;
			
		case ERROR_SIM_PUK:
			PIN_CB(CB_PIN_PUK_REQUIRED, NULL);
			MAKE_ERROR(This, ERROR_SIM_PUK, "SIM PUK required\n");
			return -1;

		default:
			break;
		}
		
		if(This->error == 0 || This->error == ERROR_SIM_PUK)
			break;
		
		This->error = 0;
		SLEEP(1000);
		if(i++ > 15) 
		{
			MAKE_ERROR(This, ERROR_SIM, "SIM card locked\n");
			return -1;
		}
	}

	This->network[0] = 0;

	doIt_ret_if_error(cmd_AT_CSQ);

	if(!This->registed_network)
	{
		LIGHT_DBG_PR("Registering to network\n");
		
		doIt(cmd_AT_CREG_Q);
		This->error = 0;

		while(!This->registed_network && tryCount-- > 0) {
			This->error = 0;
			SLEEP(5000);
			doIt(cmd_AT_CREG_Q);
		}
		if(This->error)
		{
			MAKE_ERROR(This, ERROR_CREG_REGISTERING_TIMEOUT, "Modem register timeout\n");
			return -1;
		}

		doIt_ret_if_error(cmd_AT_COPS);
		LIGHT_DBG_PR("Registering Done to %s\n", This->network);
	}
	else
	{
		doIt_ret_if_error(cmd_AT_COPS);
	}

	doIt_ret_if_error(cmd_AT_CSQ);
	doIt_ret_if_error(cmd_AT_CREG);
	doIt_ret_if_error(cmd_AT_CMEE);
	
	return 0;
}

static int read_time(network_modem *This, unsigned int *epoch, signed int *tz)
{
	network_modem *modem = This;

	modem->epoch   = 0;
	modem->timezone = INVALID_TIMEZONE;

	doIt(cmd_AT_CCCLK);

	if(!This->epoch)
	{
		return -6;
	}

	if(epoch)
	{
		*epoch = This->epoch;
	}

	if(tz)
	{
		*tz = This->timezone;
	}

	return 0;

}

static void reset_modem_state(network_modem *This)
{
	This->error            = 0;
	//This->error_str[0]     = 0;  // Done just before registering
	//This->network[0]       = 0;  // Done when data send success
	This->registed_network = 0;
	This->signal_quality   = 0;
	This->signal_time = 0;
}


int modem_init(
	network_modem *This,
	int (*write)(const char *buffer, unsigned int len),
	int (*read)(char *buffer, unsigned int len)
	)
{
	// Setup at handler

	This->at_handler->write_AT_cmd = write;
	This->at_handler->read_AT_cmd  = read;
	This->at_handler->modem        = This;
	This->at_handler->unsolicited_result = unsolicited_result;

	This->modem_wakeup = modem_wakeup;

	reset_modem_state(This);

	return impl_modem_init(This);
}

int modem_cmd(network_modem *This, int cmd, void * value)
{
	// Set param
	int status;
	
	switch(cmd)
	{
		case CMD_MODEM_INIT:
			reset_modem_state(This);
			impl_modem_init(This);
			return 0;

		case CMD_MODEM_POWER_CHANGE:
			reset_modem_state(This);
			return 0;

		case CMD_MODEM_CONNECT:
			reset_modem_state(This);
			status = impl_modem_connect(This, (int)value);
			doIt(cmd_AT_CSQ);
			return status;

		case CMD_MODEM_TIME_UTC:
			return read_time(This, (unsigned int*)value, NULL);

		case CMD_MODEM_TIME_TZ:
			status = read_time(This, NULL, (int*)value);
			return (status == 0 && (*(int*)value) != INVALID_TIMEZONE) ? 0 : -23;


		default:
			return -1;
	}
}

int modem_write(network_modem *This, const char *buffer, int len)
{
	int write_len;

	AT_handler_poll(This->at_handler);

	write_len = impl_modem_write(This, buffer, len);

	if(write_len < 0) 
	{
		ERR_PR("Sending error %d\n", write_len);
	}
	else
	{
		This->error_str[0] = 0;  // Clean error message if writing works.
	}
	
	return write_len;
}

int modem_read(network_modem *This, char *buffer, int len)
{
	int received;
	
	AT_handler_poll(This->at_handler);
	
	received = impl_modem_read(This, buffer, len);

	if(received < 0)
	{
		ERR_PR("receiving error %d\n", received);
		return received;
	}

	if(len && (This->signal_time + SIGNAL_QUALITY_CHECK_INTERVAL < GET_TIME_SECONDS() || This->signal_quality == 0)) 
	{
		doIt(cmd_AT_CSQ);

		if(This->signal_quality && !This->network[0] && This->signal_time)
		{
			LIGHT_DBG_PR("Requesting operator\n");
			modem_do_command(This, &cmd_AT_COPS, 3000);
			LIGHT_DBG_PR("Operator is %s\n", This->network);
		}

		This->signal_time = GET_TIME_SECONDS();
	}

	return received;
}

int modem_get_error(network_modem *This, const char *error[])
{
	*error = This->error_str;

	return This->error;
}




