/*
 * Mia_Base_OS.cpp
 *
 *  Created on:
 *      Author:
 */

// This file handles OS dependencies to Windows and Linux systems. Do not add new OS here. Just create new OS file like  Mia_Base_OSAL.cpp
// Perfect world would have Mia_Base_Linux.cpp and Mia_Base_Windows.cpp. Christer is lazy and does not refactor original ifdefs to separate files.

#include "Mia_Base.h"
#include "Mia_Exception.h"

#ifdef __linux__
	#include <unistd.h>
	#include <signal.h>
	#include <sys/eventfd.h>
	#include <poll.h>
#else
	#include <iomanip>
	#include <Windows.h>
	#include <iostream>
#endif // __linux__


#ifndef NDEBUG
	#define C_THREAD_DEBUG 1
#else
	#define C_THREAD_DEBUG 0
#endif

#define MIA_DEBUG_PRINT(...) if (C_THREAD_DEBUG) do { printf(__VA_ARGS__); } while(0)


namespace ABB
{
	namespace Mia
	{
		#ifdef _WIN32
			extern "C" char* strptime(const char* s, const char* f, struct tm* tm) 
			{
				std::istringstream input(s);
				input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
				input >> std::get_time(tm, f);
				if (input.fail()) 
				{
					return nullptr;
				}
				return (char*)(s + (int)input.tellg());
			}
		#endif

		C_Event::C_Event(bool initialState) 
		{
			#ifdef _WIN32
				m_Event = CreateEvent(
					NULL,               
					TRUE,						// manual-reset event
					initialState,			// initial state 
					NULL);
			#elif __linux__
				m_Event = eventfd(0, 0);
			#endif
		}

		C_Event::~C_Event()
		{
			#if __linux__
				close(m_Event);
			#elif _WIN32
				CloseHandle(m_Event);
			#endif
		}

		bool C_Event::M_Wait(int timeout)
		{
			#ifdef _WIN32
				return WaitForSingleObject(m_Event, timeout) == WAIT_OBJECT_0;
			#elif __linux__
				ssize_t s;
				uint64_t u;
				struct pollfd fds;
				fds.fd = (int)m_Event;
				fds.events = POLLIN;
				fds.revents = 0;

				if (!timeout)
				{
					while(poll(&fds, 1, 10000) != 1){}

					read((int)m_Event, &u, sizeof(u)); // Remove pending event
					return true;
				} else
				{
					s = poll(&fds, 1, timeout);
					if(s == 1) // event available
					{
						read((int)m_Event, &u, sizeof(u)); // Remove pending event
						return true;
					}
					return false;
				}
			#else
			#error no OS
			#endif
		}

		void C_Event::M_Set()
		{
			#ifdef _WIN32
				SetEvent(m_Event);
			#elif __linux__
				uint64_t u = 1;
				write((int)m_Event, &u, sizeof(u));
			#endif
		}

		void C_Event::M_Unset()
		{
			#ifdef _WIN32
				ResetEvent(m_Event);
			#elif __linux__
				while(1) // Flush all events
				{
					struct pollfd fds;
					fds.fd = (int)m_Event;
					fds.events = POLLIN;
					fds.revents = 0;
					if(poll(&fds, 1, 0) == 1) // Check if event available
					{
						uint64_t u;
						read((int)m_Event, &u, sizeof(u)); // flush event
					}
					else
					{
						return; // no events available
					}
				}
			#endif
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
			if (C_THREAD_DEBUG)
			{
				static int thread_counter; // Count created threads
				thread_counter++;
				MIA_DEBUG_PRINT("[C_Thread::C_Thread]: Create thread %d %s\n", thread_counter, m_RoutineName.c_str() );
			}
		}

		C_Thread::~C_Thread()
		{
			#ifdef __linux__
				free(m_Thread);
			#elif _WIN32
				CloseHandle(m_Thread);
				m_Thread = INVALID_HANDLE_VALUE;
			#endif
		}

		void C_Thread::M_Die_OS()
		{
			MIA_DEBUG_PRINT("[C_Thread::M_Die_OS]: Thread %d %s die\n", m_ThreadCounter, m_RoutineName.c_str() );
			#ifdef __linux__
				free(m_Thread);
				m_Thread = NULL;
			#elif _WIN32
				CloseHandle(m_Thread);
				m_Thread = INVALID_HANDLE_VALUE;
			#endif
		}

		int C_Thread::M_Start_OS()
		{
			static short thread_counter = 0; // Count started threads
			thread_counter++;
			m_ThreadCounter = thread_counter;

			MIA_DEBUG_PRINT("[C_Thread::M_Start_OS]: Thread %d %s start\n", thread_counter, m_RoutineName.c_str() );

			#ifdef __linux__
				if(!m_Thread)
				{
					m_Thread = (pthread_t*) malloc(sizeof(pthread_t));
				}

				if(!m_Thread) {
					MIA_DEBUG_PRINT("[C_Thread::M_Start_OS]: Thread %d %s allocation failed\n", thread_counter, m_RoutineName.c_str() );
					return 1;
				}

				int ret = pthread_create((pthread_t*)m_Thread, 0, C_Thread::M_S_Routine, this);
				return ret == 0;
			#elif _WIN32
				m_Thread = CreateThread(NULL, 0, M_S_Routine, this, 0, NULL);
				return m_Thread != INVALID_HANDLE_VALUE;
			#endif
		}

		void C_Thread::M_S_Sleep(int intervalMs)
		{
			#ifdef __linux__
				usleep(1000 * intervalMs);
			#elif _WIN32
				Sleep(intervalMs);
			#else
				#error unknown host
			#endif
		}

		C_ThreadLock::C_ThreadLock()
		{
			#ifdef __linux__
				m_Lock =  malloc(sizeof(pthread_mutex_t));
				pthread_mutex_init((pthread_mutex_t*)m_Lock, NULL);
			#elif _WIN32
				m_Lock = CreateMutex(NULL, FALSE, NULL);
			#endif
		}

		C_ThreadLock::~C_ThreadLock()
		{
			#ifdef __linux__
				pthread_mutex_destroy((pthread_mutex_t*)m_Lock);
				free(m_Lock);
				m_Lock = NULL;
			#elif _WIN32
				CloseHandle(m_Lock);
			#endif
		}
		void C_ThreadLock::M_Lock()
		{
			#ifdef __linux__
				pthread_mutex_lock((pthread_mutex_t*)m_Lock);
			#elif _WIN32
				DWORD result = WaitForSingleObject(m_Lock, INFINITE);
				if (result != WAIT_OBJECT_0)
				{
					std::cerr << "Lock error!!!" << m_Lock << std::endl;
				}
			#endif
		}

		void C_ThreadLock::M_Unlock()
		{
			#ifdef __linux__
				pthread_mutex_unlock((pthread_mutex_t*)m_Lock);
			#elif _WIN32
				ReleaseMutex(m_Lock);
			#endif
		}

		bool C_ThreadLock::M_TryLock(int timeout)
		{
			if (timeout)
			{
				#ifdef __linux__
					while (timeout--)
					{
						if (pthread_mutex_trylock((pthread_mutex_t*)m_Lock) == 0) return true;
						C_Thread::M_S_Sleep(1);
					}
				#elif _WIN32
					DWORD result = WaitForSingleObject(m_Lock, timeout);
					return result == WAIT_OBJECT_0;
				#endif
			} else
			{
				#ifdef __linux__
					if (pthread_mutex_trylock((pthread_mutex_t*)m_Lock) == 0) return true;
				#elif _WIN32
					DWORD result = WaitForSingleObject(m_Lock, 0);
					return result == WAIT_OBJECT_0;
				#endif
			}
			return false;
		}

		/* C_DateTime OS dependency */
		C_DateTime::C_DateTime(const std::string &data, const std::string &format)
		{
			Mia_THIS_ROUTINE("C_DateTime::C_DateTime");
			memset(&m_TimeData, 0, sizeof(m_TimeData));
			struct tm tm;
			if (strptime(data.c_str(), format.c_str(), &tm) != NULL)
			{
				tm.tm_isdst = -1;
				time_t t = mktime(&tm);
				if (t == (time_t)-1)
				{
					THROW(C_ExceptionBase(Mia_THIS_LOCATION, C_BaseErrorMessage(g_MiaError_InvalidDateTime, "Invalid time structure"), data, format));
				}
				m_TimeData.m_FullTime = ((uint64)t << 24) | 0;
			}
		}

		const C_DateTime C_DateTime::M_S_Now()
		{
			#ifdef __linux__
				timespec t;
				clock_gettime(CLOCK_REALTIME, &t);
				return C_DateTime(t.tv_sec, t.tv_nsec / 100);
			#elif _WIN32
				FILETIME ft;
				GetSystemTimeAsFileTime(&ft);
				int64* ret = (int64*)&ft;
				*ret -= 116444736000000000ULL;
				uint64 sec = (uint64)(*ret / (m_k_s_100nsPERSECOND));
				uint32 n100nsec = (uint32)(*ret % (m_k_s_100nsPERSECOND));
				return C_DateTime(sec, n100nsec);
			#endif
		}

		class C_PlatformInit
		{
			public: C_PlatformInit()
			{
				G_PlatformInit();
			}

			public: ~C_PlatformInit()
			{
				G_Platform_DeInit();
			}
		};

		//static C_PlatformInit s_Init;

		void G_PlatformInit()
		{
			#ifdef _WIN32
				WORD versionrequested = MAKEWORD(2, 0);
				WSADATA wsaData;
				if (WSAStartup(versionrequested, &wsaData) != 0)
					throw std::exception("Couldn't initiate WinSock, WSAStartup() failed");
				#ifdef _CRT_DEBUG_
					// Get current flag
					int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

					// Turn on leak-checking bit.
					tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

					// Turn off CRT block checking bit.
					tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;

					// Set flag to the new value.
					_CrtSetDbgFlag(tmpFlag);
				#endif

			#elif __linux__
				sigset_t sigpipe_mask;
				sigemptyset(&sigpipe_mask);
				sigaddset(&sigpipe_mask, SIGPIPE);
				sigset_t saved_mask;
				if (pthread_sigmask(SIG_BLOCK, &sigpipe_mask, &saved_mask) == -1)
				{
					perror("pthread_sigmask");
					exit(1);
				}
			#endif
		}

		void G_Platform_DeInit()
		{
#ifdef _WIN32
			WSACleanup();
#endif
		}
	}
}
