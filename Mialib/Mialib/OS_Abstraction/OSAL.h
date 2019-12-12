/*! @addtogroup OS_Abstraction
 *  @{
 */
/******************************************************************************
*                                                                             *
*                   COPYRIGHT (C) 2009    ABB OY, DRIVES                      *
*                                                                             *
*******************************************************************************
*                                                                             *
*                                                                             *
*                     Daisy Assistant Panel SW                                *
*                                                                             *
*                                                                             *
*                  Subsystem:   OS Abstraction                                *
*                                                                             *
*                   Filename:   OSAL.h                                        *
*                       Date:   2009-01-28                                    *
*                                                                             *
*                 Programmer:   Kimmo Lamminpää                               *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file OSAL.h
 *  @par File description:
 *    OS Abstraction layer main header file.
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
#ifndef OSAL_INC  /* Allow multiple inclusions */
#define OSAL_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------

#include "GENERIC_defines.h"
#ifndef WIN32
//#include "includes.h"
#include <define.h>
#else
#include "PC/includes.h"
#endif

#ifdef HAVE_MPU_SUPPORT
//#include "sp_core.h"                                  /* use: uC/OS-MPU extension                 */
//#include "sp_osapi.h"                                 /* use: uC/OS-II wrapper functions          */
#endif

#include <semaphore.h>             //zwj
#include <time.h>
//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

typedef enum {
	OSAL_OK = 0,
	OSAL_NULL_POINTER,
	OSAL_OUT_OF_MEM,
	OSAL_OS_ERR,
	OSAL_ILLEGAL_EVENT_ID,
	OSAL_ILLEGAL_EVENT_QUEUE,
    OSAL_ILLEGAL_EVENT_PTR,
	OSAL_EVENT_QUEUE_EMPTY,
	OSAL_EVENT_WRITE_PAST_END,
	OSAL_EVENT_READ_PAST_END,
	OSAL_EVENT_NULL_REF,
	OSAL_SEM_CREATE_FAIL,
	OSAL_SEM_DELETE_FAIL,
	OSAL_SEM_POST_FAIL,
	OSAL_SEM_PEND_FAIL,
	OSAL_SEM_PEND_TIMEOUT,
    OSAL_EXISTS
} OSAL_E_Status;

#define OSAL_ENABLE_ASSERT

// Assert macros
#ifdef OSAL_ENABLE_ASSERT
#define OSAL_ASSERT(bCondition) OSAL_vAssert(bCondition)
#define OSAL_ASSERT_EQUAL(n32Value1, n32Value2) OSAL_vAssertEqual(n32Value1, n32Value2)
#define OSAL_ASSERT_NOT_EQUAL(n32Value1, n32Value2) OSAL_vAssertNotEqual(n32Value1, n32Value2)
#else
#define OSAL_ASSERT(bCondition)
#define OSAL_ASSERT_EQUAL(n32Value1, n32Value2)
#define OSAL_ASSERT_NOT_EQUAL(n32Value1, n32Value2)
#endif

#define OSAL_NUM_OF_ELEMENTS(array)  (sizeof(array)/sizeof(array[0]))        // Macro computes Number Of Elements for any kind of arrays

typedef OS_STK OSAL_T_STK;

typedef INT8U OSAL_T_PRIO;

typedef INT32U OSAL_T_STK_SIZE;

typedef INT16U OSAL_T_SemCount;

typedef struct {
    OS_EVENT *ptEvent;
    sem_t sem_id;
} OSAL_T_Semaphore;

//struct timespec ts_init;

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 4) GLOBAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_eInit                                               *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Initializes the operating system
 *
 *  @return
 *    Initialisation result
 *
 *  @note
 *
 *  @par Inputs
 *    No inputs
 *
 *  @par Outputs
 *    No outputs
 *
 *  @par Calls
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eInit(void);


/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_ePostInit                                           *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Initializes the rest of operating system after OS is running
 *
 *  @return
 *    Initialisation result
 *
 ******************************************************************************/
OSAL_E_Status OSAL_ePostInit(void);


/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_eStart                                              *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Starts the operating system
 *
 *  @return
 *    Operation result
 *
 *  @note
 *
 *  @par Inputs
 *    No inputs
 *
 *  @par Outputs
 *    No outputs
 *
 *  @par Calls
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eStart(void);


/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_eSysTickInit                                        *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *     Initializes the operating system
 *
 *  @return
 *    Operation result
 *
 *  @note
 *
 *  @par Inputs
 *    No inputs
 *
 *  @par Outputs
 *    No outputs
 *
 *  @par Calls
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eSysTickInit(void);


/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_eStatInit                                           *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *
 *
 *  @return
 *    Operation result
 *
 *  @note
 *
 *  @par Inputs
 *    No inputs
 *
 *  @par Outputs
 *    No outputs
 *
 *  @par Calls
 *
 ******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
OSAL_E_Status OSAL_eStatInit(void);
#ifdef __cplusplus
}
#endif
/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_vAssert                                             *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *     This function is used to check conditions that must be TRUE. Conditions
 *     checked with this function must be such that there is no reason to
 *     continue if the check fails.
 *
 *  @param bCondition
 *     If FALSE, assert causes system reset
 *
 *  @return
 *    void
 *
 ******************************************************************************/
void OSAL_vAssert(boolean bCondition);


/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_vAssertEqual                                        *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *     This function is used to check that two given values are equal. Conditions
 *     checked with this function must be such that there is no reason to
 *     continue if the check fails.
 *
 *  @param n32Value1
 *     Value to be checked
 *
 *  @param n32Value2
 *     Value to be checked
 *
 *  @return
 *    void
 *
 ******************************************************************************/
void OSAL_vAssertEqual(int32 n32Value1, int32 n32Value2);


/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_vAssertNotEqual                                     *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *     This function is used to check that two given values are not equal.
 *     Conditions checked with this function must be such that there is no
 *     reason to continue if the check fails.
 *
 *  @param n32Value1
 *     Value to be checked
 *
 *  @param n32Value2
 *     Value to be checked
 *
 *  @return
 *    void
 *
 ******************************************************************************/
void OSAL_vAssertNotEqual(int32 n32Value1, int32 n32Value2);


#endif  /* of OSAL_INC */

/*! @} */ /* EOF, no more */
