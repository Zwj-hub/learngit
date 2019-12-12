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
*                   Filename:   tickcounter_AzureIoTPanel.c                   *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file 
 *  @par File description:
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdint.h>

#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"

#include "OSAL_time.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------
#define MAX_TICK_COUNTS 10
#ifndef ELEMENTS
	#define ELEMENTS(a) (sizeof((a)) / sizeof((a[0])))
#endif 

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------
typedef struct TICK_COUNTER_INSTANCE_TAG
{
	uint32_t start;
	uint8_t reserved;

} TICK_COUNTER_INSTANCE;

//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------
static TICK_COUNTER_INSTANCE tickCounters[MAX_TICK_COUNTS];

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------
TICK_COUNTER_HANDLE tickcounter_create(void)
{
	int i;
	TICK_COUNTER_INSTANCE* result = NULL;
	OSAL_T_TimeCounter timeCounter;
	OSAL_E_Status ostatus = OSAL_eTimeGetCounter(&timeCounter);
	
	if (ostatus)
	{
		LogError("Failed creating tick counter. OSAL error");
		return 0;
	}

	for (i = 0; i < ELEMENTS(tickCounters); i++)
	{
		if (!tickCounters[i].reserved)
		{
			result = &tickCounters[i];
			tickCounters[i].reserved = 1;
			break;
		}
	}

	if (!result)
	{
		LogError("Failed creating tick counter");
		return NULL; // No available counters
	}

	result->start = timeCounter.u32MilliSeconds;

	return result;
}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
	if (!tick_counter)
		return;
#if 1
	for (int i = 0; i < ELEMENTS(tickCounters); i++) // Make robust code and check that pointer is pointing to our list. 
	{
		if (&tickCounters[i] == tick_counter)
		{
			tick_counter->reserved = 0;
			return;
		}
	}
	LogError("Failed deleting tick counter. Not our counter");
#else
	tick_counter->reserved = 0;
#endif
	
}

int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t * current_ms)
{
	uint32_t start, now;
	OSAL_T_TimeCounter timeCounter;
	OSAL_E_Status ostatus = OSAL_eTimeGetCounter(&timeCounter);

	if (tick_counter == NULL || current_ms == NULL)
	{
		LogError("Invalid Arguments.");
		return __FAILURE__;
	}

	start = tick_counter->start;
	now   = timeCounter.u32MilliSeconds;

	*current_ms = now - start;

	return 0;
}

/*! @} */ /* EOF, no more */
