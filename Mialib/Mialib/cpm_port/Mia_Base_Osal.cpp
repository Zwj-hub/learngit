/*
 * Mia_Base_OS.cpp
 *
 *  Created on:
 *      Author:
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>

#include "Mia_Base.h"
#include "build_configuration.h"      //zwj
extern "C" void DLOG_vPrintF (char * format, ...);
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef NDEBUG
	#define C_THREAD_DEBUG 1
	#include <assert.h>
	#define ASSERT(a) assert(a)
#else
	#define ASSERT(a)
	#define C_THREAD_DEBUG 0
#endif

char g_debug_output_c_thread = 1;

#define MIA_DEBUG_PRINT(...) do{ if(C_THREAD_DEBUG && g_debug_output_c_thread) printf(__VA_ARGS__); }while(0)

extern "C" {

#include "OSAL_Time.h"
#include "OS_Prio.h"
#include "OSAL.h"
#include "OSAL_TaskManagement.h"
#include "OSAL_Semaphore.h"

}


typedef struct MiaThread
{
	const char * nameMatch;
	OSAL_T_STK * taskStack;
	uint16       taskStackLen;

} MiaThread;


#define CREATE_Thread(name, _nameMatch, _taskSize)\
	static OSAL_T_STK name##_stack[_taskSize];\
	static const struct MiaThread name = { _nameMatch, name##_stack, _taskSize};

CREATE_Thread(th_websocket, "C_MiaDriverBasePrivate",    1 * 1024 );
CREATE_Thread(th_reconnect, "C_IReconnectHandler",       1 * 1024 );
CREATE_Thread(th_device,    "C_Device_",                 1 * 1024 );
CREATE_Thread(th_receive,   "C_WebsocketWrapperPrivate", 1 * 1024 );

static const MiaThread * const th_array[] =
{
		&th_reconnect,
		&th_device,
		&th_websocket,
		//&th_service,
		&th_receive,
		NULL // List terminator
};


#ifdef BU_TASK_CPM_PLUS_0
static const uint8 priorities[] = { BU_TASK_CPM_PLUS_0, BU_TASK_CPM_PLUS_1, BU_TASK_CPM_PLUS_2, BU_TASK_CPM_PLUS_3 };
const uint8 *th_array_priority  = priorities;
#else
const uint8 *th_array_priority = NULL;
#endif

void Mia_Base_Osal_Init(const uint8 * EnoughTaskIdsForMia) // Currently 4 needed
{
	th_array_priority = EnoughTaskIdsForMia;
}


typedef struct TH_handle
{
	void  * thread_handle;
} TH_handle;
// Make separate list for reservation for saving RAM
static TH_handle th_array_handles[sizeof(th_array) / sizeof(th_array[0])] = { { NULL } };

namespace ABB
{
	namespace Mia
	{
		C_OsalPrinter G_OsalPrinter;
		C_OsalPrinter G_OsalPrinterDebug;
		C_OsalPrinter G_OsalPrinterInfo;
		C_OsalPrinter G_OsalPrinterWebsocket;

		int C_OsalPrinter::C_OsalPrinterBuf::sync()
		{
			if(!*m_quiet)
			{
				CONSOLE_PRINTER("%s", str().c_str());
			}
			str("");
			return 0;
		}

		/* C_Thread OS dependency */

		C_Thread::C_Thread(const std::string &routineName) :
				m_Running(false),
				m_Stopped(true),
				m_Loop(false),
				m_IntervalMs(0),
				m_Thread(NULL),
				m_RoutineName(routineName)
		{
			if(C_THREAD_DEBUG)
			{
				static int thread_counter; // Count created threads
				thread_counter++;
				MIA_DEBUG_PRINT("MIA: create thread %d %s\n", thread_counter, m_RoutineName.c_str() );
			}
		}

		C_Thread::~C_Thread()
		{
			M_Die_OS();
		}

#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
#define RESOURCE OS_CPU_SR  cpu_sr = 0u
#define CRITICAL_LOCK()   OS_ENTER_CRITICAL()
#define CRITICAL_UNLOCK() OS_EXIT_CRITICAL()
#else
#define RESOURCE
#define CRITICAL_LOCK()
#define CRITICAL_UNLOCK()
#endif



		void C_Thread::M_Die_OS()
		{
			RESOURCE;
			CRITICAL_LOCK();

			if(m_Thread)
			{
				if(((TH_handle*)m_Thread)->thread_handle) {
					void *handle = ((TH_handle*)m_Thread)->thread_handle;
					((TH_handle*)m_Thread)->thread_handle = NULL;
					m_Thread = NULL;

					CRITICAL_UNLOCK();
					MIA_DEBUG_PRINT("MIA: thread %d %s die\n", m_ThreadCounter, m_RoutineName.c_str());
					OSAL_eTaskDelete(&handle); // No return if suicide
					CRITICAL_LOCK();
				}
				m_Thread = NULL;
			}
			CRITICAL_UNLOCK();
		}

		int C_Thread::M_Start_OS()
		{
			static short thread_counter = 0; // Count started threads
			TH_handle *handle = NULL;
			int i;

			if(m_Thread)
			{
				return 14; // Thread already running
			}

			thread_counter++;

			m_ThreadCounter = thread_counter;

			MIA_DEBUG_PRINT("MIA: thread %d %s start\n", thread_counter, m_RoutineName.c_str() );


			for(i = 0; th_array[i]; i++)
			{
				if(!th_array_handles[i].thread_handle && !m_RoutineName.find(th_array[i]->nameMatch) && th_array_priority)
				{
					m_Thread = &th_array_handles[i];
					break;
				}
			}

			if(!m_Thread) {
				ASSERT(0);
				MIA_DEBUG_PRINT("MIA: thread %d %s allocation failed\n", thread_counter, m_RoutineName.c_str() );
				return 1;
			}

			handle = (TH_handle*) m_Thread;
			const MiaThread * th = th_array[handle - th_array_handles];
			const uint8 priority = th_array_priority[handle - th_array_handles];

			OSAL_E_Status stat = OSAL_eTaskCreate(
					 (void (*) (void*))C_Thread::M_S_Routine,
					 this,
					 th->taskStack,
					 priority,
					 th->taskStackLen,
					 &handle->thread_handle
			);

			if(stat != OSAL_OK)
			{
				((TH_handle*)m_Thread)->thread_handle = 0;
				m_Thread = NULL;
			}

			ASSERT(stat == OSAL_OK);

			return stat == OSAL_OK ? 0 : 42;
		}

		void C_Thread::M_S_Sleep(int intervalMs)
		{
			if(intervalMs <= 0)
				return;

			while(intervalMs >= 1000)
				{ OSAL_eTimeDelay(1); intervalMs -= 1000; }

			if(intervalMs)
				OSAL_eTimeDelayMS(max(intervalMs, 5)); // TICS is 200 thus 5 is minimium delay
		}

		C_Event::C_Event(bool initialState) : m_Event(initialState ? (void*)1 : (void*)0)
		{

		}

		C_Event::~C_Event()
		{

		}

		bool C_Event::M_Wait(int timeout)
		{
			int i = 0;
			while (!m_Event)
			{
				C_Thread::M_S_Sleep(5);
				i += 5;
				if (timeout && i > timeout)
				{
					return false;
				}
			}
			return true;
		}

		void C_Event::M_Set()
		{
			m_Event = (void*)1;
		}

		void C_Event::M_Unset()
		{
			m_Event = (void*)0;
		}

		C_ThreadLock::C_ThreadLock()
		{
			m_Lock = malloc(sizeof(OSAL_T_Semaphore));

			OSAL_vSemCreate((OSAL_T_Semaphore*)m_Lock, 1);
		}

		C_ThreadLock::~C_ThreadLock()
		{
			OSAL_vSemDelete((OSAL_T_Semaphore*)m_Lock);
			free(m_Lock);
		}
		void C_ThreadLock::M_Lock()
		{
			OSAL_E_Status stat;
			do
			{
				stat = OSAL_eSemPendTimeOut((OSAL_T_Semaphore*)m_Lock, 10000);

				if(stat != OSAL_OK)
				{
					MIA_DEBUG_PRINT("MIA: locking takes long!\n");
				}

			}while(stat != OSAL_OK);
		}

		void C_ThreadLock::M_Unlock()
		{
			OSAL_vSemPost((OSAL_T_Semaphore*)m_Lock);
		}

		bool C_ThreadLock::M_TryLock(int timeout)
		{
			OSAL_E_Status stat = OSAL_eSemPendTimeOut((OSAL_T_Semaphore*)m_Lock, timeout);

			return stat == OSAL_OK;
		}

		/* C_DateTime OS dependency */

		C_DateTime::C_DateTime(const std::string &data, const std::string &format)
		{
			printf("C_DateTime::C_DateTime not implemeted\n");
			while(1){};
		}

		const C_DateTime C_DateTime::M_S_Now()
		{
			return C_DateTime(time(NULL),0);
		}

	}
}



extern "C" {

#ifndef WIN32

/*
Managing locks in multithreaded applications
A thread-safe function protects shared resources from concurrent access using locks.
There are functions you can reimplement that enable you to manage the locking mechanisms and
so prevent the corruption of shared data such as the heap.
The function prototypes for these functions are:
*/
/*
_mutex_initialize()
This function accepts a pointer to a 32-bit word and initializes it as a valid mutex.
By default, _mutex_initialize() returns zero for a nonthreaded application. Therefore,
in a multithreaded application, _mutex_initialize() must return a nonzero value on success so that,
at runtime, the library knows that it is being used in a multithreaded environment.
This function must be supplied if you are using mutexes.
*/

#define USED __attribute__((used))

static char mutexInited = 0;

USED int _mutex_initialize(OSAL_T_Semaphore **m)
{
	static OSAL_T_Semaphore os_lock;

	*m = &os_lock;

	return 1;
}

/*
_mutex_acquire()
This function causes the calling thread to obtain a lock on the supplied mutex.
_mutex_acquire() returns immediately if the mutex has no owner. If the mutex is owned by another thread,
_mutex_acquire() must block it until it becomes available.
This function must be supplied if you are using mutexes.
*/
USED void _mutex_acquire(OSAL_T_Semaphore **m)
{
//	if(OSRunning)      //zwj
	{
		if(!mutexInited){
			OSAL_vSemCreate(*m, 1);
			mutexInited = 1;
		}

		OSAL_vSemPend(*m);
	}
}


/*
_mutex_release()
This function causes the calling thread to release the lock on a mutex acquired by _mutex_acquire().
The mutex remains in existence, and can be relocked by a subsequent call to mutex_acquire().
_mutex_release() assumes that the mutex is owned by the calling thread.
This function must be supplied if you are using mutexes.
*/
USED void _mutex_release(OSAL_T_Semaphore **m)
{
    //if(OSRunning)
	{
		OSAL_vSemPost(*m);
	}
}

/*
_mutex_free()
This function causes the calling thread to free the supplied mutex.
Any operating system resources associated with the mutex are freed. The mutex is destroyed and cannot be reused.
_mutex_free() assumes that the mutex is owned by the calling thread.
This function is optional. If you do not supply this function, the C library does not attempt to call it.
*/
USED void _mutex_free(OSAL_T_Semaphore **m)
{
    //if(OSRunning)
	{
		OSAL_vSemDelete(*m);
	}
}

#endif
}

