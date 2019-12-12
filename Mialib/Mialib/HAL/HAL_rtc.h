/*! @addtogroup HAL
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
*                  Subsystem:   HAL RTC                                       *
*                                                                             *
*                   Filename:   HAL_rtc.h                                     *
*                       Date:   2009-03-06                                    *
*                                                                             *
*                 Programmer:   ?Janne Mäkihonka / Espotel Oy Nivala          *
*                     Target:   ?                                             *
*                                                                             *
*******************************************************************************/
/*!
 *  @file HAL_rtc.h
 *  @par File description:
 *    [detailed description of the file]
 *
 *  @par Related documents:
 *    List here...
 *
 *  @note
 *    START HERE!!!
 */
/******************************************************************************/
#ifndef HAL_RTC_INC  /* Allow multiple inclusions */
#define HAL_RTC_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------

#include "GENERIC_defines.h"
#include "GENERIC_errno.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

typedef enum {
    RTC_SET_DATE, RTC_GET_DATE,
    RTC_SET_TIME, RTC_GET_TIME,
    RTC_GET_DIVIDER, RTC_SET_HOUR_MODE
    //RTC_SET_ALARM (???)
} E_RtcIoctl;

// RTC Time and Data format setting Binary or BCD data format
#define RTC_TIME_FORMAT   RTC_Format_BIN
#define RTC_DATE_FORMAT   RTC_Format_BIN

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------

typedef struct {
    uint8 u8Sec;
    uint8 u8Min;
    uint8 u8Hour;
    uint8 u8AmPm;
} T_RtcTime;

typedef struct {
    uint8 u8WeekDay;  // 0 not known, 1 mon, 2 tue....
    uint8 u8Day;
    uint8 u8Month;
    uint16 u16Year;
} T_RtcDate;

typedef struct {
    uint64 u64CpuClk;
    uint64 u64Rtc; 	 		// RTC (32768Hz)
} T_RtcOscMeas;

//-----------------------------------------------------------------------------
// 4) GLOBAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

/******************************************************************************
 *                                                                            *
 *  Function:        HAL_RtcInit                                              *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Inits the rtc device
 *
 *    [detailed description]
 *
 *  @param[in]
 *
 *  @param[out]
 *
 *  @return
 *    zero on success, HAL_E_Status error code on error (used error codes added here when implemented !!!)
 *
 *  @note
 *    [any note about the function you might have]
 *
 *  @par Syntax
 *    [an example how to call the function]
 *
 *  @par Inputs
 *    [list and explain read variables]
 *
 *  @par Outputs
 *    [list and explain changed variables]
 *
 *  @par Calls
 *    [ist and explain function calls]
 *
 ******************************************************************************/
HAL_E_Status HAL_RtcInit(void);


/******************************************************************************
 *                                                                            *
 *  Function:        HAL_RtcIoctl                                             *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    [brief description]
 *
 *    [detailed description]
 *
 *  @param[in] eCmd
 *    the ioctl command
 *
 *  @param[inout] u32Value
 *    Value, in or out depending on ioctl.
 *
 *  @return
 *    zero on success, HAL_E_Status error code on error (used error codes added here when implemented !!!)
 *
 *  @note
 *    [any note about the function you might have]
 *
 *  @par Syntax
 *    [an example how to call the function]
 *
 *  @par Inputs
 *    [list and explain read variables]
 *
 *  @par Outputs
 *    [list and explain changed variables]
 *
 *  @par Calls
 *    [ist and explain function calls]
 *
 ******************************************************************************/
HAL_E_Status HAL_RtcIoctl( E_RtcIoctl eCmd, uint32* pu32Value);

/******************************************************************************
 *                                                                            *
 *  Function:        HAL_RtcStartOscMeas                                      *
 *                                                                            *
 ******************************************************************************/
HAL_E_Status HAL_RtcStartOscMeas(void);

/******************************************************************************
 *                                                                            *
 *  Function:        HAL_RtcLapOscMeas                                        *
 *                                                                            *
 ******************************************************************************/
HAL_E_Status HAL_RtcLapOscMeas(T_RtcOscMeas* ptMeasRes);

/******************************************************************************
 *                                                                            *
 *  Function:        HAL_RtcStopOscMeas                                      *
 *                                                                            *
 ******************************************************************************/
HAL_E_Status HAL_RtcStopOscMeas(void);

/******************************************************************************
 *                                                                            *
 *  Function:        HAL_RtcReadOscMeas                                       *
 *                                                                            *
 ******************************************************************************/
HAL_E_Status HAL_RtcReadOscMeas(T_RtcOscMeas* ptMeas);

/******************************************************************************
 *                                                                            *
 *  Function:        HAL_RtcConfigAlarm	                                      *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Enables real time clock alarm interrupt
 *
 *    [detailed description]
 *
 *  @param[in] 
 *		None
 *
 *  @param[inout] 
 *		None
 *
 *  @return 
 *		None
 *
 *  @note
 *    [any note about the function you might have]
 *
 *  @par Syntax
 *    [an example how to call the function]
 *
 *  @par Inputs
 *    [list and explain read variables]
 *
 *  @par Outputs
 *    [list and explain changed variables]
 *
 *  @par Calls
 *    [ist and explain function calls]
 *
 ******************************************************************************/
void HAL_RtcConfigAlarm(void);

/******************************************************************************
 *                                                                            *
 *  Function:        HAL_RtcDeConfigAlarm                                     *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Disables real time clock alarm interrupt
 *
 ******************************************************************************/
void HAL_RtcDeConfigAlarm(void);

/******************************************************************************
 *                                                                            *
 *  Function:        HAL_RtcAlarmIntHndlr                                     *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Real time clock alarm interrupt handler routine
 *
 *    [detailed description]
 *
 *  @param[in] 
 *		None
 *
 *  @param[inout] 
 *		None
 *
 *  @return 
 *		None
 *
 *  @note
 *    [any note about the function you might have]
 *
 *  @par Syntax
 *    [an example how to call the function]
 *
 *  @par Inputs
 *    [list and explain read variables]
 *
 *  @par Outputs
 *    [list and explain changed variables]
 *
 *  @par Calls
 *    [ist and explain function calls]
 *
 ******************************************************************************/
void HAL_RtcAlarmIntHndlr(void);

/******************************************************************************
 *                                                                            *
 *  Function:        HAL_RtcStartAlarmDelay                                   *
 *                                                                            *
 ******************************************************************************/
/*!
 *  @brief
 *    Start wakeup alarm delay for wakeup interrupt
 *
 *  @param[in] u16Seconds
 *		Delay in seconds
 *
 ******************************************************************************/
void HAL_RtcStartAlarmDelay(uint16 u16Seconds);

/******************************************************************************
 *                                                                            *
 *  Function:        HAL_RtcSetAlarmIntFlag                                   *
 *                                                                            *
 ******************************************************************************/
void HAL_RtcSetAlarmIntFlag(boolean bValue);

/******************************************************************************
 *                                                                            *
 *  Function:        HAL_RtcGetAlarmIntFlag                                   *
 *                                                                            *
 ******************************************************************************/
boolean HAL_RtcGetAlarmIntFlag(void);

/******************************************************************************
 *                                                                            *
 *  Function:        HAL_RtcGetDivider                                             *
 *                                                                            *
 ******************************************************************************/
 /*!
 *  @brief
 *    Return RTC clock divider
 *
 *    Asynchronous clock divider value: *pu32Value[22 : 16]
 *    Synchronous clock divider value:  *pu32Value[14 :  0]
 *
 *  @param[in] 
 *		None
 *
 *  @param[inout] 
 *		Value of RTC divider
 *
 *  @return 
 *		ER_OK when *pu32Value > 0, HAL_E_Status error code on error: ERINVAL
 *
 *  @note
 *    [any note about the function you might have]
 *
 *  @par Syntax
 *    [an example how to call the function]
 *
 *  @par Inputs
 *    [list and explain read variables]
 *
 *  @par Outputs
 *    [list and explain changed variables]
 *
 *  @par Calls
 *    [ist and explain function calls]
 *
 ******************************************************************************/
HAL_E_Status HAL_RtcGetDivider(uint32* pu32Value);

/******************************************************************************
 *                                                                            *
 *  Function:        HAL_RtcGetCounter                                        *
 *                                                                            *
 ******************************************************************************/
HAL_E_Status HAL_RtcGetCounter(uint64* pu64Value);

#endif  /* of HAL_RTC_INC */

/*! @} */ /* EOF, no more */

