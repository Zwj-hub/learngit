/*! @addtogroup ABBA Lib
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
*                  Subsystem:   ABBA Lib                                      *
*                                                                             *
*                   Filename:   ABBA_Configuration.h                          *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Christer Ekholm                               *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
*  @file ABBA_Configuration.h
*  @par File description:
*    Parse ABBA server configurations
*
*  @par Related documents:
*
*  @note
*/
/******************************************************************************/
#ifndef ABBA_CONFIGURATION_H_
#define ABBA_CONFIGURATION_H_

#include "ABBALib.h"

int ABBA_ParseConf(ABBA_Data * This, IOTHUB_MESSAGE_HANDLE in);

#endif

