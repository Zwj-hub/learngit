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
*                   Filename:   CL_ABBAbilitySerialiser.h                     *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Christer Ekholm                               *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file CL_ABBAbilitySerialiser.h
 *  @par File description:
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
#ifndef CL_ABB_ABILITY_SERIAL_INC  /* Allow multiple inclusions */
#define CL_ABB_ABILITY_SERIAL_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include "CL_Serialiser.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

#define SERIALISER_ELEMENTS(a) (sizeof(a)/sizeof((a)[0]))

typedef enum CL_ABBAbilitySerialiserType {
	CL_ABB_ABILITY_CREATE_DESCRIPTION = 10,
	CL_ABB_ABILITY_CREATE_TELEMETRICS,
	CL_ABB_ABILITY_CREATE_DEVICEREGISTRATION,

} CL_ABBAbilitySerialiserType;

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------
typedef struct DeviceD
{
	const char *fw;
	const char *guid;
	const char *sn;
	const char *type;

}DeviceD;

typedef struct ParameterD
{
	const char *name;

}ParameterD;

typedef struct ParserVar
{
	uint8_t     addMeasSemicolon;
	uint8_t     telemetricsHeaderDone;
	DeviceD     deviceD;
	ParameterD  parameterD;
}ParserVar;

typedef struct CL_ABBAbilitySerial
{
	CL_SerialiserCmdType  parser;
	CL_ABBAbilitySerialiserType outType;

}CL_ABBAbilitySerial;



//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

int ABBABbilityDeviceRegistration(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * param);
int ABBAbilityTelemetry(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * parameter);
const CL_ABBAbilitySerial * ABBAbilityGetSerialiserHandle(CL_ABBAbilitySerialiserType type);

#endif /* of CL_ABB_ABILITY_SERIAL_INC */
/*! @} */ /* EOF, no more */
