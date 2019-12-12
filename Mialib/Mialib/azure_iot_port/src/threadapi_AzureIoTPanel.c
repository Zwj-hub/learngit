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
*                   Filename:   threadapi_AzureIoTPanel.c                     *
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
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/threadapi.h"
#include "OSAL_Time.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------
void ThreadAPI_Sleep(unsigned int milliseconds)
{
	while(milliseconds >= 1000)
	{
		OSAL_eTimeDelay(1);
		milliseconds -= 1000;
	}

	if (milliseconds)
	{
		OSAL_eTimeDelayMS(milliseconds);
	}
}

THREADAPI_RESULT ThreadAPI_Create(THREAD_HANDLE* threadHandle, THREAD_START_FUNC func, void* arg)
{
	(void)threadHandle;
	(void)func;
	(void)arg;
	LogError("multi-threading create not supported");
	return THREADAPI_ERROR;
}

THREADAPI_RESULT ThreadAPI_Join(THREAD_HANDLE threadHandle, int* res)
{
	(void)threadHandle;
	(void)res;
	LogError("multi-threading join not supported");
	return THREADAPI_ERROR;
}

void ThreadAPI_Exit(int res)
{
	(void)res;
	LogError("multi-threading exit not supported");
}
/*! @} */ /* EOF, no more */
