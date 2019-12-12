

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>     /* exit, EXIT_FAILURE */

#include "os_support.h"
#include "modem_driver.h"
#include "cert.h"

static int uart_fd;


int set_interface_attribs (int fd, int speed)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		printf("error %d from tcgetattr", errno);
		return -1;
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
					// no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 0;            // 0.1 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
					// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	//tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	//tty.c_cflag &= ~CRTSCTS;
	tty.c_cflag |= CRTSCTS;

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
	{
		printf("error %d from tcsetattr", errno);
		return -1;
	}
	return 0;
}

static int writeUart(const char * data, unsigned int len)
{
	FILE *fp;
	fp = fopen("out.txt", "a+");
	int wlen;

	wlen = fwrite(data, sizeof(char), len, fp);

	wlen = write (uart_fd, data, len);

	if(len != wlen) {
		printf("Write uart failed\n");
		exit(2);
	}

	fclose(fp);

	return wlen;
}

static int readUart(char * data, unsigned int len)
{
	FILE *fp;
	fp = fopen("in.txt", "a+");
	int rlen;

	rlen = read(uart_fd, data, len);

	if(rlen >= 0) {
		fwrite(data, sizeof(char), rlen, fp);
	}
	else {
		printf("Read uart failed\n");
		exit(2);
	}

	fclose(fp);

	return rlen;
}



static void open_port(const char * portname)
{
	int rlen,i;
	char buffer[30];

	int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);

	if (fd < 0) {
		printf("error %d opening %s: %s\n", errno, portname, strerror (errno));
		exit(1);
	}

	set_interface_attribs (fd, B115200);  // set speed to 115,200 bps, 8n1
       //set_blocking (fd, 0);                // set no blocking

	uart_fd = fd;

	for(i=0; i < 5; i++)
	{
		writeUart("AT\r\n", 4);

		usleep (100 * 1000);

		rlen = readUart(buffer, ELEMENTS(buffer)-1);

		if(rlen)
		{
			buffer[rlen] = 0;
			printf("Modem respond %s\n", buffer);
			return;
		}
		else
		{
			//printf("Modem does not respond\n");
		}

		usleep (1000 * 1000);
	}

	printf("Modem does not respond\n");
	exit(4);
}

#if 0

void unsolicited_result(const struct AT_Handler * parent, const char * result)
{
	printf("unsolicited_result: |%s|\n", result);
}


CREATE_ATHandler(test_handler, writeUart, readUart, unsolicited_result);


static int AT_test_done = 0;

void  cmd_AT_result_hadler( const struct AT_Handler * parent, const char * result)
{
	printf("cmd_at result: |%s|\n", result);

	if(strstr(result, "ERROR")) {
		printf("ERROR founf from AT command!\n");
		exit(5);
	}

	AT_test_done = 1;
}

void cmd_AT_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	printf("cmd_at responce: |%s|\n", responce);
	printf("ERROR no responce should come from AT command %s\n", parent->cmd->cmd);
	exit(6);
}
CREATE_ATCommand(cmd_AT, "AT", cmd_AT_result_hadler, cmd_AT_responce_hadler);

static void gen_result_hadler(const struct AT_Handler * parent, const char * result)
{
	AT_test_done = 1;

	printf("Result for command %s is |%s|\n", parent->cmd->cmd, result);

	if(strstr(result, "ERROR")) {
		printf("ERROR found from result |%s|\n", result);
		exit(5);
	}
}

static void version_handler(const struct AT_Handler * parent, const char * responce)
{
	printf("Version is |%s|\n", responce);
}

CREATE_ATCommand(cmd_AT_CGMR, "AT+CGMR", gen_result_hadler, version_handler);

static void module_handler(const struct AT_Handler * parent, const char * responce)
{
	printf("Module is |%s|\n", responce);
}

CREATE_ATCommand(cmd_AT_CGMM, "AT+CGMM", gen_result_hadler, module_handler);


CREATE_ATCommand(cmd_AT_SELINT,  "AT#SELINT=2", gen_result_hadler, cmd_AT_responce_hadler);
CREATE_ATCommand(cmd_AT_CMEE,    "AT+CMEE=2", gen_result_hadler, cmd_AT_responce_hadler);
CREATE_ATCommand(cmd_AT_AUTOBND, "AT#AUTOBND=2", gen_result_hadler, cmd_AT_responce_hadler);


static void sim_responce_hadler(const struct AT_Handler * parent, const char * responce)
{	
	printf("CPIN Q responce is |%s|\n", responce);

	if(!strstr(responce, "READY")) {
		printf("SIM is locked\n");
		exit(17);
	}
}

CREATE_ATCommand(cmd_AT_CPIN, "AT+CPIN?", gen_result_hadler, sim_responce_hadler);

CREATE_ATCommand(cmd_AT_QSS, "AT#QSS=1", gen_result_hadler, cmd_AT_responce_hadler);

CREATE_ATCommand(cmd_AT_CREG, "AT+CREG=1", gen_result_hadler, cmd_AT_responce_hadler);

static int cregisted = 0;
static void  CREG_Q_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	int stat;
	int mode;

	printf("CREQ Q responce is |%s|\n", responce);

	sscanf(responce,"+CREG: %d,%d",&mode, &stat);

	printf("Mode %d Stat %d\n", mode, stat);

	if(stat == 1) {
		printf("Modem is registered to Network! Fuck yeah!\n");
		cregisted = 1;
	}
}

CREATE_ATCommand(cmd_AT_CREG_Q, "AT+CREG?", gen_result_hadler, CREG_Q_responce_hadler);


CREATE_ATCommand(cmd_AT_WS64_12, "AT+WS46=12", gen_result_hadler, cmd_AT_responce_hadler);

static void  signal_quolatity_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	printf("signal quolity responce is |%s|\n", responce);
}

CREATE_ATCommand(cmd_AT_CSQ, "AT+CSQ", gen_result_hadler, signal_quolatity_responce_hadler);

CREATE_ATCommand(cmd_AT_STIA_2_1, "AT#STIA=2,1", gen_result_hadler, cmd_AT_responce_hadler);

CREATE_ATCommand(cmd_AT_PLMNMODE_1, "AT#PLMNMODE=1", gen_result_hadler, cmd_AT_responce_hadler);

//Mobile APN=cmnet
//#define OPERATOR_APN "\"cmnet\""
//Unicom APN=none
//#define OPERATOR_APN "\"\""
#define OPERATOR_APN "\"UNINET\""
// AT+CGDCONT=1,"IP",UNINET
// AT+CGDCONT=1,"IP","3gnet"

CREATE_ATCommand(cmd_AT_CGDCONT, "AT+CGDCONT=1,\"IP\"," OPERATOR_APN, gen_result_hadler, cmd_AT_responce_hadler);

static void  connection_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	printf("Connection responce is |%s|\n", responce);
}

CREATE_ATCommand(cmd_AT_CGDCONT_Q, "AT+CGDCONT?", gen_result_hadler, connection_responce_hadler);

CREATE_ATCommand(cmd_AT_SSLSECCFG, "AT#SSLSECCFG=1,0,0", gen_result_hadler, cmd_AT_responce_hadler);


static void  sgact_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	printf("sgact responce is |%s|\n", responce);
}

#define USERNAME "vain"
#define PASSWORD "vain"
// AT#SGACT=1,1,vain,vain
CREATE_ATCommand(cmd_AT_SGACT, "AT#SGACT=1,1," USERNAME "," PASSWORD, gen_result_hadler, sgact_responce_hadler);

#define NUMBER "*99#"
// ATD*99# +++
CREATE_ATCommand(cmd_AT_D, "ATD" NUMBER, gen_result_hadler, cmd_AT_responce_hadler);



static void sslen_result_hadler(const struct AT_Handler * parent, const char * result)
{
	AT_test_done = 1;

	printf("Result for command %s is |%s|\n", parent->cmd->cmd, result);

	if(strstr(result, "SSL already activated")) {
		return;
	}


	if(strstr(result, "ERROR")) {
		printf("ERROR found from result |%s|\n", result);
		exit(5);
	}
}

CREATE_ATCommand(cmd_AT_SSLEN, "AT#SSLEN=1,1", sslen_result_hadler, cmd_AT_responce_hadler);

#define IP_ADDR     "\"52.187.47.30\""
#define REMOTE_PORT "8000"

CREATE_ATCommand(cmd_AT_SSLD, "AT#SSLD=1," REMOTE_PORT "," IP_ADDR ",0,1", gen_result_hadler, cmd_AT_responce_hadler);

static void  sslsend_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	const char hello[] = "Hello, World!\n";
	const char terminate = 26;
	printf("sslsend responce is |%s|\n", responce);

	parent->write_AT_cmd(hello, strlen(hello));
	parent->write_AT_cmd(&terminate, 1);
}


const AT_Command cmd_AT_SSLSEND = { "AT#SSLSEND=1", NULL, '>', gen_result_hadler, sslsend_responce_hadler, 0, NULL };


static void  sslrcv_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	char buffer[100] = {0};
	int len;
	printf("sslreceive responce is |%s|\n", responce);
	usleep(100 * 1000);
	len = parent->read_AT_cmd(buffer, 100);
	buffer[len] = 0;
	printf("Received |%s|\n", buffer);
}

const AT_Command cmd_AT_SSLRECV = { "AT#SSLRECV=1,100", NULL, '>', gen_result_hadler, sslrcv_responce_hadler, 0, NULL };


//AT#SSLSECCFG=1,0,1,1
//OK
// <cert_format> = 1, PEM format is selected

static void SSLSECDATA_cmd(const struct AT_Handler * parent)
{
	char buffer[30];
	sprintf(buffer, "AT#SSLSECDATA=1,1,1,%d",(int)strlen(server_sertificate));
	parent->write_AT_cmd(buffer, strlen(buffer));
}

static void  sslsegdata_responce_hadler(const struct AT_Handler * parent, const char * responce)
{
	const char terminate = 26;
	if(responce[0] == '>' && responce[1] == 0) {
		printf("sslsegdata responce is |%s|\n", responce);
		parent->write_AT_cmd(server_sertificate, strlen(server_sertificate));
		usleep(1000 * 1000);
		parent->write_AT_cmd(&terminate, 1);
	}
}

const AT_Command cmd_AT_SSLSECDATA = { "AT#SSLSECDATA=1,1,1,<size>", SSLSECDATA_cmd, '>', gen_result_hadler, sslsegdata_responce_hadler, 0, NULL };


#if 0
#AT#SSLSECCFG=1,0,


# AT#SSLCFG=<SSId>,<cid>,<pktSize>,<maxTo>,<defTo>,<txTo>

# AT#SSLD=1,<remotePort>,<remoteHost>,0,1

# AT#SSLSEND and AT#SSLRECV

#endif

// AT#SSLSEND=<SSId>[,<Timeout>]


static int do_command(const AT_Command * cmd, int * ready)
{
	AT_Handler_state state_old = 99;

	*ready = 0;

	if(AT_handler_cmd(&test_handler, cmd)) {
		printf("AT_handler_cmd failed %s\n", cmd->cmd);
		exit(4);
	}

	while(!*ready){

		AT_Handler_state state = AT_handler_poll(&test_handler);

		if(state != state_old) {
			state_old = state;
		}
	}

	return *ready;
}

#define doIt(name) do_command(&name, &AT_test_done);

int main(int argc, char *argv[])
{
	int i;
	open_port("/dev/ttyUSB0");

	doIt(cmd_AT);
	doIt(cmd_AT_SELINT);
	doIt(cmd_AT_CMEE);
	//doIt(cmd_AT_CGMR);
	//doIt(cmd_AT_CGMM);
	//doIt(cmd_AT_WS64_12);
	//doIt(cmd_AT_AUTOBND);
	doIt(cmd_AT_CPIN);
	//doIt(cmd_AT_QSS);
	doIt(cmd_AT_CREG);

	while(!cregisted) {
		doIt(cmd_AT_CREG_Q);
		usleep (500 * 1000);
		doIt(cmd_AT_CSQ);
		usleep (500 * 1000);
	}

	doIt(cmd_AT_SSLEN);
	//doIt(cmd_AT_SSLSECDATA)
	doIt(cmd_AT_SSLSECCFG);

	doIt(cmd_AT_CSQ);
	doIt(cmd_AT_STIA_2_1);
	doIt(cmd_AT_PLMNMODE_1);

	doIt(cmd_AT_CGDCONT);
	doIt(cmd_AT_CGDCONT_Q);
	//doIt(cmd_AT_D);
	doIt(cmd_AT_SGACT);
	doIt(cmd_AT_SSLD);

	for(i = 0; i < 30; i++) {
		doIt(cmd_AT_SSLSEND);
		doIt(cmd_AT_SSLRECV);
		usleep (1000 * 1000);
	}


	printf("Test done\n");
}

#endif

#define IP_ADDR     "52.187.47.30"
#define REMOTE_PORT 8000


static unsigned int getCACertificate(char *buffer, unsigned int bufferLen, unsigned int offset)
{
	const char * reader = server_sertificate;
	int available = ELEMENTS(server_sertificate) - offset;
	int copyAmount = MAX(MIN(available, bufferLen), 0);

	reader += offset;

	memcpy(buffer, reader, copyAmount);

	return copyAmount;
}

#define HELLO_W "Hello, World!\n"

int main(int argc, char *argv[])
{
	int read_len;
	char read_buffer[40];

	CREATE_network_modem(modem, "vain", "vain", "3gnet", REMOTE_PORT, IP_ADDR, getCACertificate);

	open_port("/dev/ttyUSB0");

	modem_init(&modem, writeUart, readUart);

	modem_write(&modem, "",0);

	printf("Modem init %d %s\n", modem.error, modem.error_str);
	printf("Operator %s\n", modem.network);
	printf("Signal quolity %d%%\n", modem.signal_quality);

	while(1) {

		modem_write(&modem, HELLO_W,strlen(HELLO_W));

		SLEEP(1000);

		read_len = modem_read(&modem, read_buffer,ELEMENTS(read_buffer));
		read_buffer[ELEMENTS(read_buffer) - 1] = 0;

		if(read_len > 0)
		{
			read_buffer[read_len] = 0;
			printf("received |%s|\n", read_buffer);
		}
		else if(read_len < 0) {
			printf("Modem error %d %s\n", modem.error, modem.error_str);
			return 1;
		}
	}
}



















#if 0

int main(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "Dhdc:p:r:t:n:")) != -1) {
		switch (opt) {
		case 'D':
			do_defaults = 1;
			break;
		case 'd':
			g_debug_mode++;
			break;
		case 'c':
			sscanf(optarg, "%u",&command);
			break;
		case 'p':
			sscanf(optarg, "%u",&parameter);
			break;
		case 'r':
			readcmd = 1;
			break;
		case 't':
			sscanf(optarg, "%u",&channel);
			break;
		case 'n':
			device = optarg;
			break;
		case 'h':
		default: /* '?' */
			OUTPR(0, "Usage: %s [ -d ] -c command [ -p parameter ] [ -r ] [ -t channel ]\n"
			" [ -r device ] [ -h ] [-D] \n"
			"\n\n"
			" -d                  : add debug verbose. full verbose -ddd \n"
			" -t channel          : index of counter channel. -t 0 - 6 \n"
			" -c command          : counter command. Look HSC_CMD_xxx \n"
			" -p parameter        : uint32_t parameter to timer\n"
			" -r                  : make read command\n"
			" -n device           : set device file. -n %s\n"
			" -D                  : set device to default mode\n"
			,
			argv[0], DEFULT_DEVICE);
			exit(EXIT_FAILURE);
	}
	}



	OUTPR(1, "command   is %u\n", command);
	OUTPR(1, "parameter is %u\n", parameter);
	OUTPR(1, "channel   is %u\n", channel);
	OUTPR(1, "read      is %s\n", readcmd ? "true" : "false");
	OUTPR(1, "device    is %s\n", device);

	device_fd = open(device, O_RDWR | O_NONBLOCK);

	if (device_fd < 0) {
		printf("Cannot open %s\n", device);
		exit(EXIT_FAILURE);
	}

	if(do_defaults)
	{
		set_default();
	}

	if(command) {
		make_command();
	}

	read_cnt();
	close(device_fd);

	return 0;
}

#endif
