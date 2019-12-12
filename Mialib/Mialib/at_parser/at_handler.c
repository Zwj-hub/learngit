
#include "os_support.h"
#include "at_handler.h"
#include <string.h>

#define LINE_FEED "\r\n"

#include <stdio.h>

#define MODULE_NAME "at_handler"

#define LINE_BUFFER_LEN LINE_BUFFER_FIFO_LEN


#define line_buffer_count(circ)   \
	(CIRC_CNT((circ).head, (circ).tail, LINE_BUFFER_FIFO_LEN))

#define line_buffer_space(circ)      \
	(CIRC_SPACE((circ).head, (circ).tail, LINE_BUFFER_FIFO_LEN))

#define line_buffer_count_to_end(circ)   \
	(CIRC_CNT_TO_END((circ).head, (circ).tail, LINE_BUFFER_FIFO_LEN))

#define line_buffer_space_to_end(circ)   \
	(CIRC_SPACE_TO_END((circ).head, (circ).tail, LINE_BUFFER_FIFO_LEN))

#define line_buffer_add_head(circ, value)   \
	((circ).head = ((circ).head + (value)) & (LINE_BUFFER_FIFO_LEN - 1))

#define line_buffer_add_tail(circ, value)   \
	((circ).tail = ((circ).tail + (value)) & (LINE_BUFFER_FIFO_LEN - 1))


static void copy_to_buffer(struct circ_buf * buf, const char * from, uint16_t count)
{
	while(count) {
		uint16_t space = line_buffer_space_to_end(*buf);
		uint16_t copy = MIN(space, count);

		if(!copy) {
			ERR_PR("Line buffer full");
			buf->head = buf->tail = 0;
			return;
		}

		memcpy(&buf->buf[buf->head], from, copy);
		line_buffer_add_head(*buf, copy);
		count -= copy;
		from  += copy;
	}
}

static uint16_t copy_fom_buffer(struct circ_buf * buf, char *to, uint16_t count)
{
	uint32_t copied = 0;

	while(count) {
		uint16_t avail = line_buffer_count_to_end(*buf);
		uint16_t copy = MIN(avail, count);

		if(!copy)
			break;

		memcpy(to, &buf->buf[buf->tail], copy);
		line_buffer_add_tail(*buf, copy);
		count -= copy;
		to += copy;
		copied += copy;
	}

	return copied;
}

static uint16_t seach_char(struct circ_buf * buf, char character)
{
	int i;
	uint16_t search_end   = line_buffer_count_to_end(*buf);
	uint16_t search_start = line_buffer_count(*buf) - search_end;

	// Search character from end of FIFO
	for(i = 0; i < search_end; i++) {
		if(buf->buf[buf->tail + i] == character) {
			return 1;
		}
	}

	// Search character from start of fifo
	for(i = 0; i < search_start; i++) {
		if(buf->buf[i] == character) {
			return 1;
		}
	}

	return 0;
}

static void flush_fifo(struct circ_buf * buf)
{
	buf->head = buf->tail = 0;
}

static int fifo_empty(struct circ_buf * buf)
{
	return buf->head == buf->tail;
}


static int16_t line_copy_fom_buffer(struct circ_buf * buf,
					char *to, uint16_t count)
{
	uint16_t copied = 0;
	int found = seach_char(buf, '\n');

	if(!found)
		return -1;

	while(1)
	{
		char byte = buf->buf[buf->tail];
		line_buffer_add_tail(*buf, 1);

		if(copied + 1 >= count) {
			*to = 0;
			ERR_PR("Line len was too long %d\n", copied);
			return 0;
		}

		if(byte == '\n') {
			*to++ = 0;
			return copied;
		}

		if(byte != '\0' && byte != '\r') {
			*to++ = byte;
			copied++;
		}
	}
}



static void addToFifo(AT_Handler * This)
{
	char buffer[20];
	int len;

	do {
		len  = This->read_AT_cmd(buffer,
			MIN(ELEMENTS(buffer), line_buffer_space(This->line_buf)));

		if(len) {
			// Add to FIFO
			copy_to_buffer(&This->line_buf, buffer, len);
		}

	} while(len);

}

static int16_t getLine(AT_Handler * This, char * lineBuffer, uint16_t lineBufferLen)
{
	int16_t lineLen;

	if(lineBufferLen < 2) {
		ERR_PR("Too short input buffer %d\n", lineBufferLen);
		return -2;
	}

	addToFifo(This);

	while(1) {
		lineLen = line_copy_fom_buffer(&This->line_buf, lineBuffer, lineBufferLen);

		if(lineLen > 0) // zero length lines are filtered out
			return lineLen;

		if(lineLen < 0 && !line_buffer_space(This->line_buf))
		{
			char buffer[64];
			snprintf(buffer,MIN(ELEMENTS(buffer),line_buffer_count_to_end(This->line_buf)),"%s",&This->line_buf.buf[This->line_buf.tail]);
			buffer[ELEMENTS(buffer) - 1] = 0;
			
			ERR_PR("Line buff full. Flush it |%s|\n", buffer);
			
			while(!fifo_empty(&This->line_buf))
			{
				flush_fifo(&This->line_buf);	
				SLEEP(5);
				addToFifo(This);
				lineLen = line_copy_fom_buffer(&This->line_buf, lineBuffer, lineBufferLen);
				if(lineLen >= 0)
					return -1; // discard partial line
			}
		}

		if(lineLen < 0)
			return lineLen;
	}
}


static int check_result(const char *resp)
{
	const char * valid_resp[] = {
		"OK",
		"CONNECT",
		"NO CARRIER",
		"BUSY",
		"NO ANSWER",
		"CONNECT",
		"+CME ERROR:",
		"+CMS ERROR:",
		"ERROR",
		//"SEND FAIL",
		//"SEND OK",
		"+QSSLURC: \"recv\""
	};

	int i,j;

	for(i = 0; i < ELEMENTS(valid_resp); i++) {
		j = 0;
		while(1)
		{
			if(resp[j] != valid_resp[i][j]) {
				break;
			}


			j++;
			if(valid_resp[i][j] == 0)
				return 1; // Match found
		}
	}

	return 0; // No match
}

static int read_unsolicited_results(AT_Handler * This, const char *urc)
{
	if(This->unsolicited_result)
			return This->unsolicited_result(This, urc);
	else
		return 0;
}


// Public functions

void AT_handler_init(AT_Handler * This)
{
	This->cmd           = NULL;
	This->state         = AT_HANDLER_IDLE;
	flush_fifo(&This->line_buf);
}

void AT_handler_no_echo(AT_Handler * This)
{
	if (This->state == AT_HANDLER_WAITING_ECHO)
	{
		This->state = AT_HANDLER_WAITING_RESULT;
	}
}

unsigned int AT_handler_read(AT_Handler * This, char *buffer, unsigned int len)
{
	addToFifo(This);
	return copy_fom_buffer(&This->line_buf, buffer, len);
}

AT_Handler_state AT_handler_poll(AT_Handler * This)
{
	int ret;

	char lineBuffer[LINE_BUFFER_LEN];

	ret = getLine(This, lineBuffer, LINE_BUFFER_LEN);

	if(ret <= 0) {
	
		if(This->state == AT_HANDLER_WAITING_RESULT && This->cmd->channel_open) { // If no \r\n after channel open character then force.

			if(This->line_buf.buf[This->line_buf.tail] == This->cmd->channel_open) {
				char buff[2] = { 0, 0 };
				buff[0] = This->cmd->channel_open;
				This->line_buf.buf[This->line_buf.tail] = '\n';
				//flush_fifo(&This->line_buf);
				This->cmd->responce_hadler(This, buff);
				return This->state;
			}
		}

		return This->state;
	}

	if(read_unsolicited_results(This, lineBuffer)) {
		return This->state;
	}

	switch (This->state)
	{
	case AT_HANDLER_IDLE:
		break;

	case AT_HANDLER_WAITING_ECHO:
	{
		DBG_PR("received echo |%s|\n", lineBuffer);
		This->state = AT_HANDLER_WAITING_RESULT;
		break;
	}
	case AT_HANDLER_WAITING_RESULT:
	{

		if(check_result(lineBuffer)) {
			DBG_PR("received result |%s|\n", lineBuffer);
			This->state = AT_HANDLER_IDLE;
			This->cmd->result_hadler(This, lineBuffer);
			return This->state;
		}

		DBG_PR("received responce |%s|\n",lineBuffer);

		if(This->cmd->responce_hadler) {
			This->cmd->responce_hadler(This, lineBuffer);
		}
		break;
	}
	}

	return This->state;
}

int16_t AT_handler_cmd(AT_Handler * This, const AT_Command * cmd)
{
	if(This->state != AT_HANDLER_IDLE) {
		return -1;
	}

	This->cmd = cmd;
	This->state = AT_HANDLER_WAITING_ECHO;
	addToFifo(This);
	flush_fifo(&This->line_buf);
	if(This->cmd->cmd_func) {
		This->cmd->cmd_func(This);
	}
	else if (This->cmd->cmd) {
		This->write_AT_cmd(This->cmd->cmd, strlen(This->cmd->cmd));
	}
	else {
		ERR_PR("Missing cmd and cmd func\n");
		This->state = AT_HANDLER_IDLE;
		return -2;
	}

	if(This->state == AT_HANDLER_WAITING_ECHO) // This->cmd->cmd_func may go directly to responce
	{
		This->write_AT_cmd(LINE_FEED, strlen(LINE_FEED));
	}

	return 0;
}
