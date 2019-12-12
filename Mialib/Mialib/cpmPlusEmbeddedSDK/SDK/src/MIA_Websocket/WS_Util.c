/*
 * WS_Util.c
 *
 *  Created on: Sep 29, 2014
 *      Author: Tuan Vu
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include "WS_Error.h"
#include "WS_Util.h"

#ifndef __WS_UTIL_C__ // What is this?
#define __WS_UTIL_C__
#ifdef __linux__
	#include <errno.h>
	#include <openssl/sha.h>
#elif _WIN32
	#include <Windows.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif



static const char m_k_Encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			     "abcdefghijklmnopqrstuvwxyz0123456789+/";

#if _WIN32
	char * G_StringCopy(char * destination, const char * source, size_t num) 
	{
		strncpy_s(destination, num + 1, source, num);
		return destination;
	}
#else
	#define G_StringCopy(destination, source, num) strncpy(destination, source, num)
#endif

int G_Strnlen(const char *text, int maxlen)
{  
	int i = 0;
	while(i < maxlen && text[i])
		i++;
	
	return i;
}
	
	
// Private function. Depends OS 
// Declared here because this are private to G_ThreadLock_Free functions
void * G_ThreadLock_New_OS(void);
void G_ThreadLock_Free_OS(void * l);
int G_ThreadLock_Lock_OS(void * l);
int G_ThreadLock_Unlock_OS(void * l);

int G_Random()
{
	static char init = 0;
	if (!init)
	{
		srand((int)time(0));
		init = 1;
	}
#ifdef DO_NOT_RANDOMIZE
	return 0;
#else
	if(RAND_MAX >= (uint32_t) -1)
	{
		return rand();
	}
	else
	{
		return rand() ^ (rand() << 10) ^ (rand() << 20); 
	}
#endif
}

int G_Random_String(char* string, int length)
{
	int i;
	static int hex = sizeof(m_k_Encode);
	for (i = 0; i < length; i++)
	{
		string[i] = m_k_Encode[(G_Random()%hex)];
	}
	string[length] = '\0';
	return 0;
}

struct t_SHAContext {
	union {
		unsigned char		b8[20];
		unsigned int		b32[5];
	} h;
	union {
		unsigned char		b8[8];
		unsigned long int b64[1];
	} c;
	union {
		unsigned char		b8[64];
		unsigned int		b32[16];
	} m;
	unsigned char			count;
};

#ifdef _WIN32
#define LITTLE_ENDIAN 1
#define BIG_ENDIAN 2
#define BYTE_ORDER LITTLE_ENDIAN
#endif
/* sanity check */
#if !defined(BYTE_ORDER) || !defined(LITTLE_ENDIAN) || !defined(BIG_ENDIAN)
# define unsupported 1
#elif BYTE_ORDER != BIG_ENDIAN
# if BYTE_ORDER != LITTLE_ENDIAN
#  define unsupported 1
# endif
#endif

#ifndef unsupported

#define bzero(a,b) memset(a, 0, b)

/* constant table */
static const unsigned int _K[] = { 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 };
#define	K(t)	_K[(t) / 20]

#define	F0(b, c, d)	(((b) & (c)) | ((~(b)) & (d)))
#define	F1(b, c, d)	(((b) ^ (c)) ^ (d))
#define	F2(b, c, d)	(((b) & (c)) | ((b) & (d)) | ((c) & (d)))
#define	F3(b, c, d)	(((b) ^ (c)) ^ (d))

#define	S(n, x)		(((x) << (n)) | ((x) >> (32 - n)))

#define	H(n)	(ctxt->h.b32[(n)])
#define	COUNT	(ctxt->count)
#define	BCOUNT	(ctxt->c.b64[0] / 8)
#define	W(n)	(ctxt->m.b32[(n)])

#define	PUTBYTE(x)	{ \
	ctxt->m.b8[(COUNT % 64)] = (x);		\
	COUNT++;				\
	COUNT %= 64;				\
	ctxt->c.b64[0] += 8;			\
	if (COUNT % 64 == 0)			\
		G_SHA1_Step(ctxt);		\
	}

#define	PUTPAD(x)	{ \
	ctxt->m.b8[(COUNT % 64)] = (x);		\
	COUNT++;				\
	COUNT %= 64;				\
	if (COUNT % 64 == 0)			\
		G_SHA1_Step(ctxt);		\
	}

#ifdef __linux__
static void G_SHA1_Step __P((struct t_SHAContext *));
#endif
static void G_SHA1_Step(struct t_SHAContext *ctxt)
{
	unsigned int	a, b, c, d, e, tmp;
	size_t t, s;

#if BYTE_ORDER == LITTLE_ENDIAN
	struct t_SHAContext tctxt;

	memcpy(&tctxt.m.b8[0], &ctxt->m.b8[0], 64);
	ctxt->m.b8[0] = tctxt.m.b8[3]; ctxt->m.b8[1] = tctxt.m.b8[2];
	ctxt->m.b8[2] = tctxt.m.b8[1]; ctxt->m.b8[3] = tctxt.m.b8[0];
	ctxt->m.b8[4] = tctxt.m.b8[7]; ctxt->m.b8[5] = tctxt.m.b8[6];
	ctxt->m.b8[6] = tctxt.m.b8[5]; ctxt->m.b8[7] = tctxt.m.b8[4];
	ctxt->m.b8[8] = tctxt.m.b8[11]; ctxt->m.b8[9] = tctxt.m.b8[10];
	ctxt->m.b8[10] = tctxt.m.b8[9]; ctxt->m.b8[11] = tctxt.m.b8[8];
	ctxt->m.b8[12] = tctxt.m.b8[15]; ctxt->m.b8[13] = tctxt.m.b8[14];
	ctxt->m.b8[14] = tctxt.m.b8[13]; ctxt->m.b8[15] = tctxt.m.b8[12];
	ctxt->m.b8[16] = tctxt.m.b8[19]; ctxt->m.b8[17] = tctxt.m.b8[18];
	ctxt->m.b8[18] = tctxt.m.b8[17]; ctxt->m.b8[19] = tctxt.m.b8[16];
	ctxt->m.b8[20] = tctxt.m.b8[23]; ctxt->m.b8[21] = tctxt.m.b8[22];
	ctxt->m.b8[22] = tctxt.m.b8[21]; ctxt->m.b8[23] = tctxt.m.b8[20];
	ctxt->m.b8[24] = tctxt.m.b8[27]; ctxt->m.b8[25] = tctxt.m.b8[26];
	ctxt->m.b8[26] = tctxt.m.b8[25]; ctxt->m.b8[27] = tctxt.m.b8[24];
	ctxt->m.b8[28] = tctxt.m.b8[31]; ctxt->m.b8[29] = tctxt.m.b8[30];
	ctxt->m.b8[30] = tctxt.m.b8[29]; ctxt->m.b8[31] = tctxt.m.b8[28];
	ctxt->m.b8[32] = tctxt.m.b8[35]; ctxt->m.b8[33] = tctxt.m.b8[34];
	ctxt->m.b8[34] = tctxt.m.b8[33]; ctxt->m.b8[35] = tctxt.m.b8[32];
	ctxt->m.b8[36] = tctxt.m.b8[39]; ctxt->m.b8[37] = tctxt.m.b8[38];
	ctxt->m.b8[38] = tctxt.m.b8[37]; ctxt->m.b8[39] = tctxt.m.b8[36];
	ctxt->m.b8[40] = tctxt.m.b8[43]; ctxt->m.b8[41] = tctxt.m.b8[42];
	ctxt->m.b8[42] = tctxt.m.b8[41]; ctxt->m.b8[43] = tctxt.m.b8[40];
	ctxt->m.b8[44] = tctxt.m.b8[47]; ctxt->m.b8[45] = tctxt.m.b8[46];
	ctxt->m.b8[46] = tctxt.m.b8[45]; ctxt->m.b8[47] = tctxt.m.b8[44];
	ctxt->m.b8[48] = tctxt.m.b8[51]; ctxt->m.b8[49] = tctxt.m.b8[50];
	ctxt->m.b8[50] = tctxt.m.b8[49]; ctxt->m.b8[51] = tctxt.m.b8[48];
	ctxt->m.b8[52] = tctxt.m.b8[55]; ctxt->m.b8[53] = tctxt.m.b8[54];
	ctxt->m.b8[54] = tctxt.m.b8[53]; ctxt->m.b8[55] = tctxt.m.b8[52];
	ctxt->m.b8[56] = tctxt.m.b8[59]; ctxt->m.b8[57] = tctxt.m.b8[58];
	ctxt->m.b8[58] = tctxt.m.b8[57]; ctxt->m.b8[59] = tctxt.m.b8[56];
	ctxt->m.b8[60] = tctxt.m.b8[63]; ctxt->m.b8[61] = tctxt.m.b8[62];
	ctxt->m.b8[62] = tctxt.m.b8[61]; ctxt->m.b8[63] = tctxt.m.b8[60];
#endif

	a = H(0); b = H(1); c = H(2); d = H(3); e = H(4);

	for (t = 0; t < 20; t++)
	{
		s = t & 0x0f;
		if (t >= 16) W(s) = S(1, W((s+13) & 0x0f) ^ W((s+8) & 0x0f) ^ W((s+2) & 0x0f) ^ W(s));

		tmp = S(5, a) + F0(b, c, d) + e + W(s) + K(t);
		e = d; d = c; c = S(30, b); b = a; a = tmp;
	}
	for (t = 20; t < 40; t++)
	{
		s = t & 0x0f;
		W(s) = S(1, W((s+13) & 0x0f) ^ W((s+8) & 0x0f) ^ W((s+2) & 0x0f) ^ W(s));
		tmp = S(5, a) + F1(b, c, d) + e + W(s) + K(t);
		e = d; d = c; c = S(30, b); b = a; a = tmp;
	}
	for (t = 40; t < 60; t++)
	{
		s = t & 0x0f;
		W(s) = S(1, W((s+13) & 0x0f) ^ W((s+8) & 0x0f) ^ W((s+2) & 0x0f) ^ W(s));
		tmp = S(5, a) + F2(b, c, d) + e + W(s) + K(t);
		e = d; d = c; c = S(30, b); b = a; a = tmp;
	}
	for (t = 60; t < 80; t++)
	{
		s = t & 0x0f;
		W(s) = S(1, W((s+13) & 0x0f) ^ W((s+8) & 0x0f) ^ W((s+2) & 0x0f) ^ W(s));
		tmp = S(5, a) + F3(b, c, d) + e + W(s) + K(t);
		e = d; d = c; c = S(30, b); b = a; a = tmp;
	}

	H(0) = H(0) + a;
	H(1) = H(1) + b;
	H(2) = H(2) + c;
	H(3) = H(3) + d;
	H(4) = H(4) + e;

	bzero(&ctxt->m.b8[0], 64);
}

/*------------------------------------------------------------*/

void G_SHA1_Init(struct t_SHAContext *ctxt)
{
	bzero(ctxt, sizeof(struct t_SHAContext));
	H(0) = 0x67452301;
	H(1) = 0xefcdab89;
	H(2) = 0x98badcfe;
	H(3) = 0x10325476;
	H(4) = 0xc3d2e1f0;
}

void G_SHA1_Pad(struct t_SHAContext *ctxt)
{
	size_t padlen;		/*pad length in bytes*/
	size_t padstart;

	PUTPAD(0x80);

	padstart = COUNT % 64;
	padlen = 64 - padstart;
	if (padlen < 8) 
	{
		memset(&ctxt->m.b8[padstart], 0, padlen);
		COUNT += (unsigned char)padlen;
		COUNT %= 64;
		G_SHA1_Step(ctxt);
		padstart = COUNT % 64;	/* should be 0 */
		padlen = 64 - padstart;	/* should be 64 */
	}
	memset(&ctxt->m.b8[padstart], 0, padlen - 8);
	COUNT += (unsigned char)(padlen - 8);
	COUNT %= 64;
#if BYTE_ORDER == BIG_ENDIAN
	PUTPAD(ctxt->c.b8[0]); PUTPAD(ctxt->c.b8[1]);
	PUTPAD(ctxt->c.b8[2]); PUTPAD(ctxt->c.b8[3]);
	PUTPAD(ctxt->c.b8[4]); PUTPAD(ctxt->c.b8[5]);
	PUTPAD(ctxt->c.b8[6]); PUTPAD(ctxt->c.b8[7]);
#else
	PUTPAD(ctxt->c.b8[7]); PUTPAD(ctxt->c.b8[6]);
	PUTPAD(ctxt->c.b8[5]); PUTPAD(ctxt->c.b8[4]);
	PUTPAD(ctxt->c.b8[3]); PUTPAD(ctxt->c.b8[2]);
	PUTPAD(ctxt->c.b8[1]); PUTPAD(ctxt->c.b8[0]);
#endif
}

void G_SHA1_Loop(struct t_SHAContext *ctxt, const unsigned char *input, size_t len)
{
	size_t gaplen;
	size_t gapstart;
	size_t off;
	size_t copysiz;

	off = 0;

	while (off < len)
	{
		gapstart = COUNT % 64;
		gaplen = 64 - gapstart;

		copysiz = (gaplen < len - off) ? gaplen : len - off;
		memcpy(&ctxt->m.b8[gapstart], &input[off], copysiz);
		COUNT += (unsigned char)copysiz;
		COUNT %= 64;
		ctxt->c.b64[0] += (unsigned long)copysiz * 8;
		if (COUNT % 64 == 0)	G_SHA1_Step(ctxt);
		off += copysiz;
	}
}

void G_SHA1_Result(struct t_SHAContext *ctxt, void *digest0)
{
	unsigned char *digest;

	digest = (unsigned char *)digest0;
	G_SHA1_Pad(ctxt);
#if BYTE_ORDER == BIG_ENDIAN
	memcpy(digest, &ctxt->h.b8[0], 20);
#else
	digest[0] = ctxt->h.b8[3]; digest[1] = ctxt->h.b8[2];
	digest[2] = ctxt->h.b8[1]; digest[3] = ctxt->h.b8[0];
	digest[4] = ctxt->h.b8[7]; digest[5] = ctxt->h.b8[6];
	digest[6] = ctxt->h.b8[5]; digest[7] = ctxt->h.b8[4];
	digest[8] = ctxt->h.b8[11]; digest[9] = ctxt->h.b8[10];
	digest[10] = ctxt->h.b8[9]; digest[11] = ctxt->h.b8[8];
	digest[12] = ctxt->h.b8[15]; digest[13] = ctxt->h.b8[14];
	digest[14] = ctxt->h.b8[13]; digest[15] = ctxt->h.b8[12];
	digest[16] = ctxt->h.b8[19]; digest[17] = ctxt->h.b8[18];
	digest[18] = ctxt->h.b8[17]; digest[19] = ctxt->h.b8[16];
#endif
}

#endif /*unsupported*/
/*
 * This should look and work like the libcrypto implementation
 */

unsigned char * G_SHA1(const unsigned char *d, size_t n, unsigned char *md)
{
#if !defined(__linux__) && !defined(OSAL_OS) && !_WIN32
	SHA1(d, n, md);
#else
	struct t_SHAContext c;
	struct t_SHAContext *ctx = &c;

	G_SHA1_Init(ctx);
	G_SHA1_Loop(ctx, d, n);
	G_SHA1_Result(ctx, (void *)md);
#endif
	return md;
}


typedef struct _ThreadLockPrivate
{
	void * m_Mutex;

} *t_ThreadLockPrivate;


t_ThreadLock G_ThreadLock_New()
{
	t_ThreadLock t = (struct _ThreadLock*) malloc(sizeof(struct _ThreadLock));

	if (!t)
	{
		G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		return 0;
	}
	memset(t, 0, sizeof(struct _ThreadLock));

	t->m_Private = (struct _ThreadLockPrivate*) malloc(sizeof(struct _ThreadLockPrivate));
	if (!t->m_Private)
	{
		G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		free(t);
		return NULL;
	}
	memset(t->m_Private, 0, sizeof(struct _ThreadLockPrivate));

	t->m_Private->m_Mutex = G_ThreadLock_New_OS();

	if (!t->m_Private->m_Mutex)
	{
		G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		free(t->m_Private);
		free(t);
		return NULL;
	}

	return t;
}

int G_ThreadLock_Lock(t_ThreadLock lock)
{
	int ret = G_ThreadLock_Lock_OS(lock->m_Private->m_Mutex);
	if (ret == 0)
	{
		 return 0;
	}
	else
	{
#ifdef __linux__
		ret = errno;
#endif
		G_SetLastErrorExact(G_ERROR_UNKOWN, ret);
		return -1;
	}
}

int G_ThreadLock_Unlock(t_ThreadLock lock)
{
	int ret = G_ThreadLock_Unlock_OS(lock->m_Private->m_Mutex);
	if (ret == 0)
	{
		 return 0;
	}
	else
	{
#ifdef __linux__
		ret = errno;
#endif
		G_SetLastErrorExact(G_ERROR_UNKOWN, ret);
		return -1;
	}
}

void G_ThreadLock_Free(t_ThreadLock *lock)
{
	if (*lock)
	{
		if ((*lock)->m_Private)
		{
			G_ThreadLock_Free_OS((*lock)->m_Private->m_Mutex);
			free ((*lock)->m_Private);
		}
		free(*lock);
		*lock = 0;
	}
}

t_List G_List_New()
{
	t_List l = (t_List) malloc(sizeof(struct _List));
	if (!l)
	{
		G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		return 0;
	}

	memset(l, 0, sizeof(struct _List));

	l->m_Lock = G_ThreadLock_New();
	if (!l->m_Lock)
	{
		G_SetLastError(G_ERROR_UNINITIALIZED);
		return 0;
	}
	return l;
}

int G_List_AppendFront(t_List list, void* data)
{
	t_ListIterator iter = (t_ListIterator) malloc(sizeof(struct _ListIterator));
	if (!iter)
	{
		G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		return -1;
	}
	memset(iter, 0, sizeof(struct _ListIterator));

	iter->m_Data = data;

	G_ThreadLock_Lock(list->m_Lock);

	if (list->m_Size == 0)
	{
		list->m_Head = iter;
		list->m_Tail = iter;
	} else
	{
		iter->m_Next = list->m_Head;
		list->m_Head = iter;
	}
	list->m_Size++;

	G_ThreadLock_Unlock(list->m_Lock);

	return 0;
}

int G_List_AppendBack(t_List list, void* data)
{
	t_ListIterator iter = (t_ListIterator) malloc(sizeof(struct _ListIterator));
	if (!iter)
	{
		G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		return -1;
	}
	memset(iter, 0, sizeof(struct _ListIterator));
	iter->m_Data = data;

	G_ThreadLock_Lock(list->m_Lock);
	if (list->m_Size == 0)
	{
		list->m_Head = iter;
		list->m_Tail = iter;
	} else
	{
		list->m_Tail->m_Next = iter;
		list->m_Tail = iter;
	}
	list->m_Size++;

	G_ThreadLock_Unlock(list->m_Lock);
	return 0;
}

void* G_List_RemoveFront(t_List list)
{
	t_ListIterator iter;
	void* data;
	G_ThreadLock_Lock(list->m_Lock);
	if (!list->m_Size)
	{
		G_SetLastError(G_ERROR_LIST_EMPTY);
		G_ThreadLock_Unlock(list->m_Lock);
		return 0;
	}

	if (list->m_Size == 1)
	{
		iter = list->m_Head;
		list->m_Head = 0;
		list->m_Tail = 0;
	} else
	{
		iter = list->m_Head;
		list->m_Head = iter->m_Next;
	}
	list->m_Size --;
	G_ThreadLock_Unlock(list->m_Lock);

	data = iter->m_Data;
	free(iter);
	return data;
}

void* G_List_RemoveBack(t_List list)
{
	t_ListIterator iter;
	void* data;
	G_ThreadLock_Lock(list->m_Lock);

	if (!list->m_Size)
	{
		G_SetLastError(G_ERROR_LIST_EMPTY);
		G_ThreadLock_Unlock(list->m_Lock);
		return 0;
	}

	if (list->m_Size == 1)
	{
		iter = list->m_Head;
		list->m_Head = 0;
		list->m_Tail = 0;
	} else
	{
		iter = list->m_Head;
		while (iter->m_Next && iter->m_Next != list->m_Tail)
		{
			iter = iter->m_Next;
		}
		list->m_Tail = iter;
		list->m_Tail->m_Next = 0;
	}
	list->m_Size --;
	G_ThreadLock_Unlock(list->m_Lock);

	data = iter->m_Data;
	free(iter);
	return data;
}

void G_List_Free(t_List* list)
{
	if (*list)
	{
		G_ThreadLock_Lock((*list)->m_Lock);
		while ((*list)->m_Size)
		{
			G_List_RemoveFront(*list);
		}
		G_ThreadLock_Unlock((*list)->m_Lock);
		G_ThreadLock_Free(&((*list)->m_Lock));
		free(*list);
	}
}

t_ListIterator G_List_GetIterator(t_List list)
{
	return list->m_Head;
}

unsigned int G_List_GetSize(t_List list)
{
	return list->m_Size;
}

t_ListIterator G_ListIterator_Next(t_ListIterator iter)
{
	if (iter) return iter->m_Next;
	return NULL;
}

void*	G_ListIterator_Data(t_ListIterator iter)
{
	if (iter) return iter->m_Data;
	return NULL;
}

void G_ListIterator_Free(t_ListIterator *iter)
{
	if (iter)
	{
		free(*iter);
		*iter = 0;
	}
}

t_String G_String_New()
{
	t_String s = (t_String) malloc(sizeof(struct _String));

	if (!s)
	{
		G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}

	memset(s, 0, sizeof(struct _String));

	return s;
}

t_String G_String_NewSize(unsigned int length)
{
	t_String s = G_String_New();
	if (!s) return 0;

	s->m_Data = (char*)malloc(length+1);

	if (!s->m_Data)
	{
		free(s);
		G_SetLastError(G_ERROR_NOT_ENOUGH_MEMORY);
		return 0;
	}

	memset(s->m_Data, 0, length+1);
	s->m_Capacity = length;
	s->m_Length = 0;
	return s;
}

t_String G_String_NewString(const t_String string)
{
	char* data = string->m_Data;
	int length = G_Strnlen(data, 1024*1024);
	t_String s = G_String_NewSize(length);
	if (!s) return 0;

	G_StringCopy(s->m_Data, data, length);
	s->m_Data[length] = '\0';
	s->m_Capacity = length;
	s->m_Length = length;

	return s;
}

t_String G_String_NewRaw(const char* data)
{
	int length = G_Strnlen(data, 1024*1024);
	t_String s = G_String_NewSize(length);
	if (!s) return 0;

	G_StringCopy(s->m_Data, data, length);
	s->m_Data[length] = '\0';
	s->m_Capacity = length;
	s->m_Length = length;

	return s;
}

unsigned int G_String_Length(t_String string)
{
	return string->m_Length;
}

unsigned int G_String_Capacity(t_String string)
{
	return string->m_Capacity;
}

int G_String_Resize(t_String string, unsigned int newLength)
{
	newLength = newLength+1;
	if (string)
	{
		unsigned int oldlengh = G_String_Length(string);
		if (oldlengh>=newLength)
		{
			/* resize is not necessary */
			char* data = G_String_Data(string);
			data[newLength] = '\0';
			return 0;
		} else
		{
			free(string->m_Data);
			string->m_Data = (char*) malloc(newLength+1);
			if (!string->m_Data)
			{
				string->m_Length = newLength;
				string->m_Capacity = newLength;
			}
		}
	} else
	{
		G_SetLastError(G_ERROR_UNINITIALIZED);
	}
	return -1;
}

int G_String_Copy(t_String destination, t_String source)
{
	if (destination && source && destination->m_Data && source->m_Data)
	{
		unsigned int length = G_String_Length(source);
		if (length > G_String_Capacity(destination))
		{
			G_String_Resize(destination, length);
		}

		G_StringCopy(G_String_Data(destination), G_String_Data(source), length);
		return 0;
	} else
	{
		G_SetLastError(G_ERROR_UNINITIALIZED);
	}

	return -1;
}

char* G_String_Data(t_String string)
{
	return string->m_Data;
}

void G_String_Free(t_String *data)
{
	t_String s = *data;
	if (s && s->m_Data)
	{
		free(s->m_Data);
		free(s);
		*data = 0;
	}
}

int G_String_Compare(t_String source, t_String comparand)
{
	return strcmp(source->m_Data, comparand->m_Data);
}

int G_String_CompareRaw(t_String source, const char* comparand)
{
	return strcmp(source->m_Data, comparand);
}


void G_String_Append(t_String string, t_String stringToAppend)
{
	char* newstring;
	int newsize = string->m_Length + stringToAppend->m_Length + 1;

	if (string->m_Capacity < newsize)
	{
		/* extend the space */
		newsize = newsize < 2*string->m_Capacity ? newsize : 2*string->m_Capacity;
		newstring = (char*) malloc(newsize+1);
		G_StringCopy(newstring, string->m_Data, string->m_Length);
		free(string->m_Data);
		string->m_Data = newstring;
		string->m_Capacity = newsize + 1;
	}

	G_StringCopy(&string->m_Data[string->m_Length], stringToAppend->m_Data, stringToAppend->m_Length);
	string->m_Length+=stringToAppend->m_Length;
}

void G_String_AppendRaw(t_String string, char* data)
{
	int length = (int)strlen(data);
	int newsize = string->m_Length + length + 1;
	char* newstring;
	if (string->m_Capacity < newsize)
	{
		/* extend the space */

		newsize = newsize < 2*string->m_Capacity ? newsize : 2*string->m_Capacity;
		newstring = (char*) malloc(newsize+1);
		G_StringCopy(newstring, string->m_Data, string->m_Length);
		free(string->m_Data);
		string->m_Data = newstring;
		string->m_Capacity = newsize + 1;
	}

	G_StringCopy(&string->m_Data[string->m_Length], data, length);
	string->m_Length+=length;
}

void G_String_AppendInt(t_String string, int number)
{
	char buffer[32];
#if _WIN32
	sprintf_s(buffer, 32, "%d", number);
#else
	sprintf(buffer, "%d", number);
#endif
	G_String_AppendRaw(string, buffer);
}
#endif
