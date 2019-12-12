/*! @addtogroup CouldLib
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
*                  Subsystem:   CouldLib                                      *
*                                                                             *
*                   Filename:   CL_Manager.h                                  *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file CL_Manager.h
 *  @par File description:
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
#ifndef CL_MANAGER_INC  /* Allow multiple inclusions */
#define CL_MANAGER_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include "CL_Device.h"
#include "CL_MeasBuff.h"
#include "CL_Parameter.h"
#include "CL_ABBAbilitySerialiser.h"
#include <stdint.h>

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

#define CREATE_CL_Manager(name, devices, measures, deviceID, hubURL)   \
	CREATE_CL_MeasBuff(name ## _measbuff, measures);   \
	CL_Device name ## _dev[devices];                   \
	CL_Manager name = {                                \
		name ## _dev,                                  \
		devices,                                       \
		&name ## _measbuff,                            \
		deviceID,                                      \
		hubURL                                         \
	}

#define CL_ManagerGetDeviceSize(mgr) ((mgr)->deviceCnt)
#define MAX_IOTHUB_DEVICE_ID_SIZE 32
#define MAX_IOTHUB_URL_SIZE 64
//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------

typedef struct CL_Manger
{
	CL_Device *devices;
	uint16_t deviceCnt;
	CL_MeasBuff *measBuff;
	char hubDeviceID[MAX_IOTHUB_DEVICE_ID_SIZE];
	char hubURL[MAX_IOTHUB_URL_SIZE];
}CL_Manager;

typedef uint8_t CL_DeviceIndex;

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

// Add device to manager
int CL_MgrSetDevice(CL_Manager *mgr, CL_Device * device, CL_DeviceIndex index);

// Get reference to device
CL_Device * CL_MgrGetDevice(CL_Manager *mgr, uint8_t index);

// Add measurement 
int CL_MgrAddMeas(CL_Manager *mgr, CL_ParameterId parameterId, void * value, CL_Timestamp timestamp);

// Delete all measurements
int CL_MgrDelMeas(CL_Manager *mgr);

// Read configuration from server
int CL_MgrPull(CL_Manager *mgr, const char *inData);

// Get measurement data to string
int CL_MgrPush(CL_Manager *mgr, char *output, uint16_t outputlen, CL_ABBAbilitySerialiserType type);

int CL_MgrMeasCount(CL_Manager *mgr, uint16_t *meas, uint16_t *free);

#endif /* of CL_MANAGER_INC */
/*! @} */ /* EOF, no more */
