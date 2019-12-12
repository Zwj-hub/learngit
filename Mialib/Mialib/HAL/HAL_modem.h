#ifndef HAL_MODEM_INC  /* Allow multiple inclusions */
#define HAL_MODEM_INC
/*! @addtogroup HAL
 *  @{
 */
/******************************************************************************
*                                                                             *
*                   COPYRIGHT (C) 2017    ABB OY, DRIVES                      *
*                                                                             *
*******************************************************************************
*                                                                             *
*                                                                             *
*                      Assistant Panel SW                                     *
*                                                                             *
*                                                                             *
*                  Subsystem:   HAL Modem                                     *
*                                                                             *
*                   Filename:   HAL_modem.h                                   *
*                       Date:   2017                                          *
*                                                                             *
*                 Programmer:   Christer Ekholm                               *
*                     Target:   ALL                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file HAL_modem.h
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


//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------

#include "GENERIC_defines.h"
#include "GENERIC_errno.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

typedef enum { MODEM_POWER_ON, MODEM_POWER_OFF, MODEM_IS_DATA_AVAILABLE, MODEM_IS_ON } ModemCmd;

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

HAL_E_Status HAL_eModemOpen(uint8 u8Fd);
HAL_E_Status HAL_eModemClose(uint8 u8Fd);
HAL_E_Status HAL_eModemWrite(uint8 u8Fd, const void *data, uint16 len);
HAL_E_Status HAL_eModemRead(uint8 u8Fd, void *data, uint16 *len /* in/out */ );
HAL_E_Status HAL_eModemIOCTRL(uint8 u8Fd, ModemCmd eCmd, void * param);


#endif

