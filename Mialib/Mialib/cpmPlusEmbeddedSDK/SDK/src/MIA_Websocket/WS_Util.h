/*
* WS_Util.h
*
*  Created on: Sep 25, 2014
*      Author: Tuan Vu
*/

#ifndef WS_UTIL_H_
#define WS_UTIL_H_

#include <string.h>

#ifdef _WIN32
	#define _CRTDBG_MAP_ALLOC  
	#include <stdlib.h>  
	#include <crtdbg.h>  
#endif

#ifndef __linux__
	#define snprintf _snprintf
#endif


struct _ListIterator;
typedef struct _ListIterator *t_ListIterator;

struct _ThreadLockPrivate;

typedef struct _ThreadLock
{
	struct _ThreadLockPrivate *m_Private;
} *t_ThreadLock;

t_ThreadLock G_ThreadLock_New(void);

int G_ThreadLock_Lock(t_ThreadLock lock);

int G_ThreadLock_Unlock(t_ThreadLock lock);

void G_ThreadLock_Free(t_ThreadLock *lock);

unsigned char * G_SHA1(const unsigned char *d, size_t n, unsigned char *md);

struct _ListIterator
{
	void* m_Data;
	t_ListIterator m_Next;
} ;

typedef struct _List
{
	t_ListIterator m_Head;
	t_ListIterator m_Tail;
	int m_Size;
	t_ThreadLock m_Lock;
} *t_List;

t_List G_List_New(void);

int G_List_AppendFront(t_List list, void* data);

int G_List_AppendBack(t_List list, void* data);

void* G_List_RemoveFront(t_List list);

void* G_List_RemoveBack(t_List list);

void* G_List_Front(void);

t_ListIterator G_List_GetIterator(t_List list);
t_ListIterator G_ListIterator_Next(t_ListIterator iter);
void*	G_ListIterator_Data(t_ListIterator iter);
void G_ListIterator_Free(t_ListIterator *iter);
void G_List_Free(t_List* list);
unsigned int G_List_GetSize(t_List list);



int G_Random(void);

int G_Random_String(char* string, int length);

typedef struct _String
{
	int m_Length;
	int m_Capacity;
	char *m_Data;
} *t_String;

struct t_SHAContext;

t_String G_String_New(void);

t_String G_String_NewSize(unsigned int length);

t_String G_String_NewString(const t_String string);

t_String G_String_NewRaw(const char* data);

unsigned int G_String_Length(t_String string);

unsigned int G_String_Capacity(t_String string);

void G_String_Append(t_String string, t_String stringToAppend);

void G_String_AppendRaw(t_String string, char* data);

void G_String_AppendInt(t_String string, int number);

int G_String_Resize(t_String string, unsigned int newLength);

int G_String_Copy(t_String destination, t_String source);

char* G_String_Data(t_String string);

void G_String_Free(t_String *data);

int G_String_Compare(t_String source, t_String comparand);

int G_String_CompareRaw(t_String source, const char* comparand);

int G_Strnlen(const char *text, int maxlen);
	
/*
void G_SHA1_Init(struct t_SHAContext *context);
void G_SHA1_Loop(struct t_SHAContext *context, const unsigned char *input, size_t len);
void G_SHA1_Result(struct t_SHAContext *context, void *digest0);*/

#ifdef __MEMORY_DEBUG__

#endif

#endif
