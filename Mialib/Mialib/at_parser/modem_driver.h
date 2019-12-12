#include "at_handler.h"
#include <stdio.h>

#define MAKE_ERROR(modem, value, ...) \
	do{\
	modem->error = value; \
	modem->diagnose_error = value; \
	snprintf(modem->error_str, ELEMENTS(modem->error_str), __VA_ARGS__);\
	modem->error_str[ELEMENTS(modem->error_str) -1] = 0; \
	 ERR_PR("error line %d: ", __LINE__); \
	 ERR_PR("%s\n", modem->error_str); \
	}while(0)

#define ERROR_OK       0
#define ERROR_RESULT   1
#define ERROR_RESPONCE 2
#define ERROR_SIM      3
#define ERROR_CREG_RESPONCE_INVALID    4
#define ERROR_CREG_REGISTERING_FAILED  5
#define ERROR_MODEM_TIMEOUT            6
#define ERROR_CMD_FAILED               7
#define ERROR_SERVER_DOES_NOT_RESPOND  8
#define ERROR_SIM_PIN                  9
#define ERROR_SIM_PUK                  10
#define ERROR_CREG_REGISTERING_TIMEOUT 11

#define ERROR_TO_STR(a) \
	a == 0 ? "OK" : \
	a == 1 ? "ERROR RESULT" : \
	a == 2 ? "ERROR RESPONCE" : \
	a == 3 ? "ERROR SIM" : \
	a == 4 ? "ERROR NETWORK REGISTER" : \
	a == 5 ? "ERROR NETWORKING" : \
	a == 6 ? "ERROR TIMEOUT" : \
	a == 7 ? "ERROR INTERNAL" : \
	a == 8 ? "ERROR SERVER" : \
	a == 9  ? "ERROR SIM LOCKED" : \
	a == 10 ? "ERROR SIM PUK REQUIRED" : \
	a == 11 ? "ERROR REGISTER TIMEOUT" : \
	"UNKWNOW ERROR"
	
typedef enum { 

	CB_PIN_GET          = 0,
	CB_PIN_OK           = 1,
	CB_MIA_PIN_BAD      = 2,
	CB_PIN_PUK_REQUIRED = 3,
	CB_PIN_NOT_LOCKED   = 4

} Modem_Pin_CB_action;
	
struct network_modem;

typedef struct network_modem {

	int registed_network;
	char network[32];
	int signal_quality;
	const char *user_name;
	const char *user_password;
	const char *user_apn;
	unsigned int server_port;
	const char  *server_addr;
	unsigned int (*getCACertificate)(char *buffer, unsigned int bufferLen, unsigned int offset);
	int error;
	char error_str[40];
	AT_Handler * at_handler;
	void(*pinCb)(int action, void * param);
	int (*modem_wakeup)(struct network_modem *This);
	void * driver;
	int cmdDone;
	unsigned int rx_amount;
	unsigned int tx_amount;
	char iccd[32];
	char cgmr[16];
	char imei[16];
	char crypted;
	unsigned int signal_time;
	unsigned int epoch;
	signed int timezone;
	int diagnose_error;

} network_modem;

#define CREATE_network_modem(name, user_name_, user_password_, user_apn_, server_port_, server_addr_, getCertificate_) \
	CREATE_ATHandler(name##_modem_AT_handler, NULL, NULL, NULL); \
static network_modem name = { 0, "", 0, user_name_, user_password_, user_apn_, server_port_, server_addr_, getCertificate_, 0, "", &name##_modem_AT_handler }
/*
	static network_modem name = {           \
	.user_name = user_name_,          \
	.user_password = user_password_,   \
	.user_apn = user_apn_,              \
	.server_port = server_port_,         \
	.server_addr = server_addr_,          \
	.getCACertificate = getCertificate_,   \
	.at_handler = &name##_modem_AT_handler  \
	}
*/

// Initialise modem to default
int modem_init(network_modem *This,
	int (*write)(const char *buffer, unsigned int len),
	int (*read)(char *buffer, unsigned int len)
	);

// Write data to network
int modem_write(network_modem *This, const char *buffer, int len);

// Read data from netwok	
int modem_read(network_modem *This, char *buffer, int len);

#define CMD_MODEM_INIT          0 // Reset modem
#define CMD_MODEM_CONNECT       1 // Connect modem to server
#define CMD_MODEM_TIME_UTC      2 // Get UTC time
#define CMD_MODEM_TIME_TZ       3 // Get timezone
#define CMD_MODEM_POWER_CHANGE  4 // Reset modem state when power changed

int modem_cmd(network_modem *This, int cmd, void * value); // Do CMD_MODEM_* command
int modem_get_error(network_modem *This, const char *error[]); // Read error status

// Private
int modem_do_command(network_modem *This, const AT_Command *cmd, int timeout);


// Driver must implemt this

int  impl_modem_init(network_modem *This);
int  impl_modem_write(network_modem *This, const char *buffer, int len);
int  impl_modem_read(network_modem *This, char *buffer, int len);
int  impl_modem_unsolicited(network_modem *This, const char * responce);
int  impl_modem_connect(network_modem *This, int disconnect);

