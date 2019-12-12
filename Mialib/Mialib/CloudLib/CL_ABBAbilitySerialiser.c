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

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include "CL_ABBAbilitySerialiser.h"
#include "CL_Serialiser.h"
#include <stdint.h>


//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------
 static const CL_ABBAbilitySerial ABBAbilitySerialiserList[] = {
	{ ABBAbilityTelemetry, CL_ABB_ABILITY_CREATE_TELEMETRICS },
	{ ABBABbilityDeviceRegistration, CL_ABB_ABILITY_CREATE_DEVICEREGISTRATION }
};

//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------


 const CL_ABBAbilitySerial * ABBAbilityGetSerialiserHandle(CL_ABBAbilitySerialiserType type)
{
	int i;
	for (i = 0; i < SERIALISER_ELEMENTS(ABBAbilitySerialiserList); i++) {
		if (type == ABBAbilitySerialiserList[i].outType) {
			return &ABBAbilitySerialiserList[i];
		}
	}
	return NULL;
}


//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------



/*! @} */ /* EOF, no more */
