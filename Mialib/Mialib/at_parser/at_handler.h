#ifndef AT_HANDLER_H_
#define AT_HANDLER_H_

#include <stdint.h>

#include "circular_buffer.h"

#ifndef LINE_BUFFER_FIFO_LEN
	#define LINE_BUFFER_FIFO_LEN 1024 // Must be 2^n
#endif


#define ELEMENTS(a) (sizeof(a) / sizeof((a)[0]))

#ifndef MIN
	#define MIN(a,b)         ((a)<(b)?(a):(b))
	#define MAX(a,b)         ((a)>(b)?(a):(b))
#endif

struct AT_Command;
struct AT_Handler;


typedef struct AT_Command
{
	const char *cmd;
	void (*cmd_func)(
	const struct AT_Handler * parent);
	char   channel_open;
	void        (*result_hadler)(
	const struct AT_Handler * parent,
	const char * result);

	void        (*responce_hadler)(
	const struct AT_Handler * parent,
	const char * responce);

	uint16_t  timeout_t;

	const void * user_con;

} AT_Command;

#define CREATE_ATCommand(name, cmd, result_handler, responce_hander) \
	static const AT_Command name = { cmd, NULL, 0, result_handler, responce_hander, 0, NULL };


typedef enum AT_Handler_state { AT_HANDLER_IDLE = 0, AT_HANDLER_WAITING_ECHO, AT_HANDLER_WAITING_RESULT } AT_Handler_state;


typedef struct AT_Handler
{
	const AT_Command * cmd;
	AT_Handler_state state;
	int (*write_AT_cmd)(const char * data, unsigned int len);
	int (*read_AT_cmd)(char * data, unsigned int len);
	int (*unsolicited_result)(
	struct AT_Handler * parent,
	const char * result);

	struct circ_buf line_buf;
	void * modem;

} AT_Handler;

#define CREATE_ATHandler(name, write, read, unsolicited_handler) \
	static char name##_buffer[LINE_BUFFER_FIFO_LEN]; \
	static AT_Handler name = { NULL, AT_HANDLER_IDLE, write, read, unsolicited_handler, { name##_buffer, 0, 0, }}


void AT_handler_init(AT_Handler * This);

int16_t AT_handler_cmd(AT_Handler * This, const AT_Command * cmd);

AT_Handler_state AT_handler_poll(AT_Handler * This);

unsigned int AT_handler_read(AT_Handler * This, char *buffer, unsigned int len);

void AT_handler_no_echo(AT_Handler * This);

#endif
