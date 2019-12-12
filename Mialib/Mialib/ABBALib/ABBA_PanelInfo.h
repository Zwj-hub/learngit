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
*                   Filename:   ABBA_PanelInfo.h                              *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
*  @file ABBA_PanelInfo.h
*  @par File description:
*    Parse ABBA server configurations
*
*  @par Related documents:
*
*  @note
*/
/******************************************************************************/
#ifndef ABBA_PANELINFO_H_
#define ABBA_PANELINFO_H_

void ABBA_get_network_operator_and_signal(const char **netOperator, const char **sigName);
void ABBA_get_panelICCID(char * buffer, unsigned int buffer_len);

#endif

