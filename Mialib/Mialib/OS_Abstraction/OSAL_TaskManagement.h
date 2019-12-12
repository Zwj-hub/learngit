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
*                   Filename:   OSAL_TaskManagement.h                         *
*                       Date:   2009-01-28                                    *
*                                                                             *
*                 Programmer:   Kimmo Lamminpää                               *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file OSAL_TaskManagement.h
 *  @par File description:                                                      
 *    OS Abstraction layer task management header file.
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
#ifndef OSAL_TM_INC  /* Allow multiple inclusions */
#define OSAL_TM_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------

#include "OSAL.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

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
 *  Function:        OSAL_eTaskCreate                                         *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Create a new process
 *
 *  @param[in] task 
 *    Task body
 *  
 *  @param[in] p_arg 
 *    Parameter for the task body
 *  
 *  @param[in] ptos 
 *    Pointer to the bottom of the task's stack 
 *  
 *  @param[in] prio 
 *    Task priority
 *  
 *  @param[in] stk_size 
 *    Stack size
 *  
 *  @param[out] handle 
 *    Handle of the process (can be NULL)
 *  
 *  @return 
 *    Operation status
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eTaskCreate(void (*task)(void *p_arg),
			         	       void              *p_arg, 
					     	   OSAL_T_STK        *ptos, 
							   OSAL_T_PRIO        prio,
							   OSAL_T_STK_SIZE    stk_size,
                               void             **handle);

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_eTaskChill                                          *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Suspend the current task until Idle task gets running time
 *
 *  @return 
 *    Operation status
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eTaskChill(void);

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_eTaskDelete                                         *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Destroy the process
 *
 *  @param[in] handle 
 *    Handle of the process (got from OSAL_eTaskCreate())
 *  
 *  @return 
 *    Operation status
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eTaskDelete(void **handle);

#endif  /* of OSAL_TM_INC */

/*! @} */ /* EOF, no more */
