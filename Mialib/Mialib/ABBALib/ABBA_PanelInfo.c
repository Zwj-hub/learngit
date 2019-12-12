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
*                   Filename:   ABBA_PanelInfo.c                              *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
*  @file ABBA_PanelInfo.c
*  @par File description:
*    device provisioning info model state machine
*
*  @par Related documents:
*
*  @note
*/
/******************************************************************************/
#include "GENERIC_defines.h"
#include "ABBA_PanelInfo.h"

#define SIGNAL_LOW 0
#define SIGNAL_WEAK 30
#define SIGNAL_NORMAL 80

typedef enum signal_val
{
	NO_SIG = 0,
	SIG_WEAK,
	SIG_MED,
	SIG_STRONG
}SigVal;

const char* const sigStr[] =
{
	"No Signal",
	"Weak",
	"Medium",
	"Strong"
};

extern int G_Socket_get_network(const char ** network, uint16 * signalPercent);
extern int G_Socket_get_ccid(char * buffer, unsigned int buffer_len);

void ABBA_get_network_operator_and_signal(const char **netOperator, const char **sigName)
{
	uint16 signalPercent;
	G_Socket_get_network(netOperator, &signalPercent);
	//signal range define
	if (signalPercent <= SIGNAL_LOW)
	{
		*sigName = sigStr[NO_SIG];
	}
	else if (signalPercent > SIGNAL_LOW && signalPercent <= SIGNAL_WEAK)
	{
		*sigName = sigStr[SIG_WEAK];
	}
	else if (signalPercent > SIG_WEAK && signalPercent <= SIGNAL_NORMAL)
	{
		*sigName = sigStr[SIG_MED];
	}
	else
	{
		*sigName = sigStr[SIG_STRONG];
	}
}

void ABBA_get_panelICCID(char * buffer, unsigned int buffer_len)
{
	G_Socket_get_ccid( buffer, buffer_len);
}
