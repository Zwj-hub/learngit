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
*                   Filename:   ABBA_EventMsg.h                               *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
*  @file ABBA_EventMsg.h
*  @par File description:
*    ABBA event msg genertaion
*
*  @par Related documents:
*
*  @note
*/
/******************************************************************************/
#ifndef ABBA_EVENTMSG_H_
#define ABBA_EVENTMSG_H_

#include <stdint.h>
#include <time.h>

#include "iothub_message.h"
#include "ABBALib.h"

int ABBA_EventMsg(ABBA_Data * This, IOTHUB_MESSAGE_HANDLE *out, const char *evtMsg, const char *srcMsg, const char *severity, const struct tm *devTimestamp);

#endif

