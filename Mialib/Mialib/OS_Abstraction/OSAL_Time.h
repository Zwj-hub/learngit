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
*                   Filename:   OSAL_Time.h                                   *
*                       Date:   2009-01-28                                    *
*                                                                             *
*                 Programmer:   Kimmo Lamminpää                               *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file OSAL_Time.h
 *  @par File description:                                                      
 *    OS Abstraction layer time management header file.
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
#ifndef OSAL_TIME_INC  /* Allow multiple inclusions */
#define OSAL_TIME_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------

#include "OSAL.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

/*!
 * @brief
 *   OS Abstraction layer time counter type definition
 */
typedef struct
{
	/*! Milliseconds since system startup */
	uint32    u32MilliSeconds;

	/*! Microseconds */
	uint32    u32MicroSeconds;

}OSAL_T_TimeCounter;

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
 *  Function:        OSAL_eTimeInit                                           *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    OSAL Time initialization
 *
 *  @return 
 *    Operation status
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eTimeInit(void);

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_eTimeDelay                                          *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Delay until given time in seconds has passed 
 *
 *  @param[in] u8Seconds 
 *    Time in seconds
 *  
 *  @return 
 *    Operation status
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eTimeDelay(uint8 u8Seconds);

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_eTimeDelayMS                                        *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Delay until given time in milliseconds has passed 
 *
 *  @param[in] u16MSeconds 
 *    Time in milliseconds
 *  
 *  @return 
 *    Operation status
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eTimeDelayMS(uint16 u16MSeconds);

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_eTimeDelayUS                                        *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Delay until given time in microseconds has passed 
 *
 *  @param[in] u16USeconds 
 *    Time in microseconds
 *  
 *  @return 
 *    Operation status
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eTimeDelayUS(uint16 u16USeconds);

/******************************************************************************
 *                                                                            *
 *  Function:        OSAL_eTimeGetCounter                                     *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Get time counter value 
 *
 *  @param[out] ptTimeCounter 
 *    Time counter value
 *  
 *  @return 
 *    Operation status
 *
 ******************************************************************************/
OSAL_E_Status OSAL_eTimeGetCounter(OSAL_T_TimeCounter* ptTimeCounter);

#endif  /* of OSAL_TIME_INC */

/*! @} */ /* EOF, no more */
