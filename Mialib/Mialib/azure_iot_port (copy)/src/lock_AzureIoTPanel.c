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
*                   Filename:   lock_AzureIoTPanel.c                         *
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
#include "azure_c_shared_utility/lock.h"
#include "azure_c_shared_utility/xlogging.h"

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
extern void * G_ThreadLock_New_OS(void);
extern int G_ThreadLock_Lock_OS(void * mutex);
extern int G_ThreadLock_Unlock_OS(void * mutex);
extern void G_ThreadLock_Free_OS(void * mutex);

LOCK_HANDLE Lock_Init(void)
{
	return (LOCK_HANDLE)G_ThreadLock_New_OS();
}

LOCK_RESULT Lock_Deinit(LOCK_HANDLE handle)
{
	G_ThreadLock_Free_OS(handle);
	return LOCK_OK;
}

LOCK_RESULT Lock(LOCK_HANDLE handle)
{
	G_ThreadLock_Lock_OS(handle);
	return LOCK_OK;
}

LOCK_RESULT Unlock(LOCK_HANDLE handle)
{
	G_ThreadLock_Unlock_OS(handle);
	return LOCK_OK;
}
/*! @} */ /* EOF, no more */
