/*! @addtogroup azure iot port
 *  @{
 */
/******************************************************************************
*                                                                             *
*                   COPYRIGHT (C) 2018    ABB Beijing, DRIVES                 *
*                                                                             *
*******************************************************************************
*                                                                             *
*                                                                             *
*                     Daisy Assistant Panel SW                                *
*                                                                             *
*                                                                             *
*                  Subsystem:   azure iot port                                *
*                                                                             *
*                   Filename:   agenttime_AzureIoTPanel.c                     *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
*  @file CL_DebugSerialiser.h
*  @par File description:
*
*  @par Related documents:
*
*  @note
*/
/******************************************************************************/

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <time.h>
#include "azure_c_shared_utility/agenttime.h"
#include "OSAL_Time.h"
#include "OSAL_Semaphore.h"
#include "HAL_rtc.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------
#define YYEAR0 1900                    /* the first year */
#define EEPOCH_YR 1970            /* EPOCH = Jan 1 1970 00:00:00 */
#define SSECS_DAY (24L * 60L * 60L)
#define LLEAPYEAR(year) (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YYEARSIZE(year) (LLEAPYEAR(year) ? 366 : 365)
static uint32 s_lastCheckTime;
static int32  s_timeZone = 100 * 3600; // Invalid
static OSAL_T_Semaphore time_lock;
const int yytab[2][12] = {
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------
static void time_semaphore_init(void)
{
	static boolean init;
	
	if(!init)
	{
		OSAL_vSemCreate(&time_lock, 1);
		init = TRUE;
	}
}

// Returns UTC time
static time_t time_from_epoch(void)
{
	OSAL_T_TimeCounter counter; 

	static time_t s_lastTime;

	OSAL_eTimeGetCounter(&counter);

	if(!s_lastCheckTime || (counter.u32MilliSeconds - s_lastCheckTime > 6000))
	{
		struct tm timeinfo;
		T_RtcDate rDate = { 0 };
		T_RtcTime rTime = { 0 };

		HAL_RtcIoctl( RTC_GET_DATE, (uint32*) &rDate); // RTC is in local time
		HAL_RtcIoctl( RTC_GET_TIME, (uint32*) &rTime);
		
		timeinfo.tm_year = rDate.u16Year - 1900;
		timeinfo.tm_mon  = rDate.u8Month - 1;
		timeinfo.tm_mday = rDate.u8Day;
		timeinfo.tm_hour = rTime.u8Hour;
		timeinfo.tm_min  = rTime.u8Min;
		timeinfo.tm_sec  = rTime.u8Sec;
		timeinfo.tm_isdst = -1;

		s_lastCheckTime = counter.u32MilliSeconds;

		return s_lastTime = mktime(&timeinfo) - s_timeZone;
	}
	else 
	{
		return s_lastTime + ((counter.u32MilliSeconds - s_lastCheckTime) / 1000);
	}
}

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------
/** @brief gmtime conversion for ARM compiler
*   @param  time_t *timer : File pointer to time_t structure
*   @retval struct tm * : date in struct tm format
*/
struct tm *gmtimeMDK(register const time_t *timer)
{
	static struct tm br_time;
	register struct tm *timep = &br_time;
	time_t time = *timer;
	register unsigned long dayclock, dayno;
	int year = EEPOCH_YR;

	dayclock = (unsigned long)time % SSECS_DAY;
	dayno = (unsigned long)time / SSECS_DAY;

	timep->tm_sec = dayclock % 60;
	timep->tm_min = (dayclock % 3600) / 60;
	timep->tm_hour = dayclock / 3600;
	timep->tm_wday = (dayno + 4) % 7;       /* day 0 was a thursday */
	while (dayno >= YYEARSIZE(year)) 
	{
		dayno -= YYEARSIZE(year);
		year++;
	}
	timep->tm_year = year - YYEAR0;
	timep->tm_yday = dayno;
	timep->tm_mon = 0;
	while (dayno >= yytab[LLEAPYEAR(year)][timep->tm_mon]) {
		dayno -= yytab[LLEAPYEAR(year)][timep->tm_mon];
		timep->tm_mon++;
	}
	timep->tm_mday = dayno + 1;
	timep->tm_isdst = 0;
	return timep;
}

/**
 * @brief implementation of get_time
 * @param time_t* p pointer to the time_t structure
 * @retval time_t return time_t structure
 */
time_t get_time(time_t* p)
{
	time_t time_n;

	time_semaphore_init();
	OSAL_vSemPend(&time_lock);
	
	time_n = time_from_epoch();
	
	OSAL_vSemPost(&time_lock);

	if(p) 
		*p = time_n;
	
	return time_n;
}

/**
 * @brief implementation of get_gmtime
 * @param time_t* currentTime pointer to the time_t structure
 * @retval tm* return tm structure
 */
struct tm* get_gmtime(time_t* currentTime)
{
	return gmtimeMDK(currentTime);
}

/**
 * @brief implementation of get_mktime
 * @param tm* cal_time pointer to the tm structure
 * @retval time_t return time_t structure
 */
time_t get_mktime(struct tm* cal_time)
{
	return mktime(cal_time);
}

/**
 * @brief implementation of get_ctime
 * @param time_t* timeToGet pointer to the time_t structure
 * @retval char* return value
 */
char* get_ctime(time_t* timeToGet)
{
	return ctime(timeToGet);
}

/**
 * @brief implementation of get_difftime
 * @param time_t stopTime end time
 * @param time_t startTime starting time
 * @retval double difference
 */
double get_difftime(time_t stopTime, time_t startTime)
{
	return difftime(stopTime, startTime);
}
/*! @} */ /* EOF, no more */
