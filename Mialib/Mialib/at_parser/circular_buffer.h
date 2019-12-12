
#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_

struct circ_buf {
	char *buf;
	int head;
	int tail;
};

// Elements in buffer
#define CIRC_CNT(head,tail,size) (((head) - (tail)) & ((size)-1))

// Space is circular buffer
#define CIRC_SPACE(head,tail,size) CIRC_CNT((tail),((head)+1),(size))

#ifdef GCC

// Elements before end of buffer
#define CIRC_CNT_TO_END(head,tail,size) \
	({int end = (size) - (tail); \
	  int n = ((head) + end) & ((size)-1); \
	  n < end ? n : end;})

// Space before end
#define CIRC_SPACE_TO_END(head,tail,size) \
	({int end = (size) - 1 - (head); \
	  int n = (end + (tail)) & ((size)-1); \
	  n <= end ? n : end+1;})

#else 

static int CIRC_CNT_TO_END(int head, int tail, int size)
{
	int end = (size) - (tail); 
	int n = ((head) + end) & ((size)-1); 
	
	return n < end ? n : end;
}

static int CIRC_SPACE_TO_END(int head, int tail, int size)
{
	int end = (size) - 1 - (head);
	int n = (end + (tail)) & ((size)-1);
	return n <= end ? n : end+1;
}
#endif
		
#endif
