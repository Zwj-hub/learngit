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
*                   Filename:   OSAL_Semaphore.h                              *
*                       Date:   2009-02-02                                    *
*                                                                             *
*                 Programmer:   Kimmo Lamminpää                               *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file OSAL_Semaphore.h
 *  @par File description:                                                      
 *    OS Abstraction layer semaphore header file.
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
#ifndef OSAL_SEM_INC  /* Allow multiple inclusions */
#define OSAL_SEM_INC

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
 *  Function:        OSAL_eSemCreate                                          *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Creates semaphore
 *
 *  @param[out] ptSem
 *    Created semaphore
 *  
 *  @return 
 *    Creation status
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eSemCreate(OSAL_T_Semaphore *ptSem, OSAL_T_SemCount tCount);

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_eSemDelete                                          *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Deletes semaphore
 *
 *  @param[in] ptSem
 *    Semaphore
 *  
 *  @return 
 *    Deletion status
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eSemDelete(OSAL_T_Semaphore *ptSem);

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_eSemPost                                            *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Semaphore post/signal operation 
 *
 *  @param[in,out] ptSem 
 *    Semaphore
 *  
 *  @return 
 *    Operation status
 * 
 ******************************************************************************/
OSAL_E_Status OSAL_eSemPost(OSAL_T_Semaphore *ptSem);

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_eSemPend                                            *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Semaphore pend/wait operation
 *
 *  @param[in,out] ptSem 
 *    Semaphore
 *  
 *  @return 
 *    Operation status
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eSemPend(OSAL_T_Semaphore *ptSem);

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_eSemPendTimeOut                                     *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Semaphore pend/wait operation with timeout
 *
 *  @param[in,out] ptSem 
 *    Semaphore
 *  
 *  @return 
 *    Operation status
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eSemPendTimeOut(OSAL_T_Semaphore *ptSem, uint16 timeout ); 

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_tSemAccept                                          *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Semaphore accept operation
 *
 *    This function checks the semaphore to see if a resource is available or,
 *    if an event occurred. 
 *
 *  @param[in,out] ptSem 
 *    Semaphore
 *  
 *  @return
 *    Semaphore count
 *        >  0     if the resource is available or the event did not occur 
 *                 the semaphore is decremented to obtain the resource.
 *        == 0     if the resource is not available or the event did not occur
 *                 error
 * 
 ******************************************************************************/
OSAL_T_SemCount OSAL_tSemAccept(OSAL_T_Semaphore *ptSem);

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_vSemCreate                                          *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Creates semaphore. If fails calls OSAL_vAssert function.
 *
 *  @param[out] ptSem
 *    Created semaphore
 *  
 *  @return 
 *    void
 *
 ******************************************************************************/
void OSAL_vSemCreate(OSAL_T_Semaphore *ptSem, OSAL_T_SemCount tCount);

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_vSemDelete                                          *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Deletes semaphore. If fails calls OSAL_vAssert function.
 *
 *  @param[in] ptSem
 *    Semaphore
 *  
 *  @return 
 *    void
 *
 ******************************************************************************/
void OSAL_vSemDelete(OSAL_T_Semaphore *ptSem);

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_vSemPost                                            *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Semaphore post/signal operation. If fails calls OSAL_vAssert function.
 *
 *  @param[in,out] ptSem 
 *    Semaphore
 *  
 *  @return 
 *    void
 * 
 ******************************************************************************/
void OSAL_vSemPost(OSAL_T_Semaphore *ptSem);

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_vSemPend                                            *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Semaphore pend/wait operation. If fails calls OSAL_vAssert function.
 *
 *  @param[in,out] ptSem 
 *    Semaphore
 *  
 *  @return 
 *    void
 *
 ******************************************************************************/
void OSAL_vSemPend(OSAL_T_Semaphore *ptSem);


/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_u16SemQuery                                         *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Returns current semaphore count. If fails calls OSAL_vAssert function.
 *
 *  @param[in] ptSem 
 *    Semaphore
 *  
 *  @return 
 *    Semaphore count
 *
 ******************************************************************************/
uint16 OSAL_u16SemQuery(OSAL_T_Semaphore *ptSem);

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_eSemAbort                                           *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Aborts waiting for all penders of a semaphore.
 *
 *  @param[in] ptSem
 *    Semaphore
 *
 *  @return
 *    Operation status
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eSemAbort(OSAL_T_Semaphore *ptSem);

#endif  /* of OSAL_SEM_INC */

/*! @} */ /* EOF, no more */
