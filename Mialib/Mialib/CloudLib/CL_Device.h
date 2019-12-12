/*! @addtogroup CloudLib
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
*                   Filename:   CL_Device.h                                   *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file CL_Device.h
 *  @par File description:
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
#ifndef CL_DEVICE_INC  /* Allow multiple inclusions */
#define CL_DEVICE_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include "CL_ParameterList.h"
#include "CL_Serialiser.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

#define MAX_GUID_SIZE       39
#define MAX_SN_SIZE         39
#define MAX_FWVER_SIZE      11
#define MAX_DEVICETYPE_SIZE 18

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------


typedef struct CL_Device
{
	char guid[MAX_GUID_SIZE];
	char sn[MAX_SN_SIZE];
	char fwVersion[MAX_DEVICETYPE_SIZE];
	char type[MAX_DEVICETYPE_SIZE];
	CL_ParameterList *paraList;

} CL_Device;

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

int CL_DeviceInit(CL_Device * dev, const char *guid, const char *sn, const char *fwVersion, const char *type);

int CL_DeviceSetGUID(CL_Device *dev, const char *guid);
int CL_DeviceSetSN(CL_Device *dev, const char *sn);
int CL_DeviceSetType(CL_Device *dev, const char *type);
int CL_DeviceSetFWVersion(CL_Device *dev, const char *fwVersion);

int CL_DeviceSetParameters(CL_Device *dev, CL_ParameterList *paraList);

int CL_DeviceSerialise(CL_Device * dev, CL_Serialiser * serialiser);

#endif /* of CL_DEVICE_INC */
/*! @} */ /* EOF, no more */
