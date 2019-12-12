/*! @addtogroup azure_iot
*  @{
*/
/******************************************************************************
*                                                                             *
*                   COPYRIGHT (C) 2018    ABB DRIVES                          *
*                                                                             *
*******************************************************************************
*                                                                             *
*                                                                             *
*                           Assistant Panel SW                                *
*                                                                             *
*                                                                             *
*                  Subsystem:   azure_iot                                     *
*                                                                             *
*                   Filename:   AzureOSConfig.h                               *
*                       Date:   2018-09-11                                    *
*                                                                             *
*                 Programmer:  Henri HAN                                      *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
*  @file AzureOSConfig.h
*  @par File description:
*    azure iot config for IoT hub connection
*
*  @par Related documents:
*
*  @note
*/
/******************************************************************************/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AZUREOSCONFIG_H
#define __AZUREOSCONFIG_H

/* String containing Hostname, Device Id & Device Key in the format:                         */
#ifdef _WIN32
//#define AZUREDEVICECONNECTIONSTRING "HostName=PanelIoTHub.azure-devices.net;DeviceId=paneltestdev;SharedAccessKey=NWCnbRbg7J/Ji6xQR9+eini591BDktz5+sle2Cgk2RA="
//#define AZUREDEVICECONNECTIONSTRING "HostName=PanelIoTHub.azure-devices.net;DeviceId=wolfsslpanel;x509=true"
#define AZUREDEVICECONNECTIONSTRING "HostName=PanelIoTHub.azure-devices.net;DeviceId=iotpanel001;x509=true"
//#define AZUREDEVICECONNECTIONSTRING "HostName=PanelIoTHub.azure-devices.net;DeviceId=iotpanel001_ecc;x509=true"
#else
#define AZUREDEVICECONNECTIONSTRING "HostName=PanelIoTHub.azure-devices.net;DeviceId=IoTPanelBlack;SharedAccessKey=eNnBzcVbFT5bCuAbCK358lc9s9Zh0eaxrpG8uZszses="
#endif

extern void DLOG_vPrintF(char * format, ...);
extern char g_Azure_debug_level;

#ifdef MIA_DEBUG
#define _AZURE_PRINTF_MSG_(level, ...) do{ if(g_Azure_debug_level >= level){DLOG_vPrintF(__VA_ARGS__);}}while(0)
#else
#define _AZURE_PRINTF_MSG_(level, ...) do{ if((0)){DLOG_vPrintF(__VA_ARGS__);}}while(0)
#endif

#define AZURE_PRINTF_ERR(...)   _AZURE_PRINTF_MSG_(1,__VA_ARGS__) // Error
#define AZURE_PRINTF_INFO(...)  _AZURE_PRINTF_MSG_(2,__VA_ARGS__) // Start stop
#define AZURE_PRINTF_STATE(...) _AZURE_PRINTF_MSG_(3,__VA_ARGS__) // DPS, IOT hub, Registering
#define AZURE_PRINTF_TRACE(...) _AZURE_PRINTF_MSG_(4,__VA_ARGS__) // ALL messages



#endif /* __AZUREOSCONFIG_H */
