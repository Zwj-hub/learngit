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
*                   Filename:   CL_Serialiser.h                               *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Christer Ekholm                               *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file CL_Serialiser.h
 *  @par File description:
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
#ifndef CL_SERIALISER_INC  /* Allow multiple inclusions */
#define CL_SERIALISER_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include "CL_String.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

#define CREATE_CL_Serialiser(name, clstring, serialiserParam)\
	CL_Serialiser name = {  \
		clstring,            \
		serialiserParam,      \
		0                      \
	}

#define CL_THIS_IS_BLOCK(a) a ## _START, a ## _END 

typedef enum
{
	CL_SER_PARAMETER_MEAS,
	CL_SER_DEVICE_FW_VER,
	CL_SER_DEVICE_GUID,
	CL_SER_DEVICE_SN,
	CL_SER_DEVICE_TYPE,
	CL_THIS_IS_BLOCK(CL_SER_DEVICE),
	CL_THIS_IS_BLOCK(CL_SER_MGR),
	CL_SER_PARAMETER_MEAS_TIMESTAMP,
	CL_SER_PARAMETER_MEAS_VALUE,
	CL_THIS_IS_BLOCK(CL_SER_PARAMETER_MEAS),
	CL_SER_PARAMETER_ID,
	CL_SER_PARAMETER_NAME,
	CL_SER_PARAMETER_TYPE,
	CL_THIS_IS_BLOCK(CL_SER_PARAMETER),
	CL_THIS_IS_BLOCK(CL_SER_PARAMETER_LIST),
	CL_SER_HUB_DEVICE_ID,
	CL_SER_HUB_URL

} CL_SerialiserChildTypes;

#define CL_SERIALISER_DO_BLOCK(serialiser, blockType, func, func_paramter, ret)             \
	do{                                                                                       \
		ret = CL_SerialiserBlockStarts(serialiser, blockType ## _START, NULL); if(ret) break;  \
		ret = func(func_paramter, serialiser); if(ret) break;                                   \
		ret = CL_SerialiserBlockEnds(serialiser, blockType ## _END, NULL); if(ret) break;        \
	}while(0);


#define CL_SERIALISER_ADD(serialiser, childType, paramater)\
	CL_SerialiserChild(serialiser, childType, paramater)

#define CL_SERIALISER_ADD_TEXT(ser,err, ...)\
	CL_StringAddText(ser->string, err, __VA_ARGS__)

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------

typedef struct CL_Serialiser
{
	CL_String *string;
	void *param;
	uint16_t level;

} CL_Serialiser;

// This parameter must be first in Serialiser context
typedef int(*CL_SerialiserCmdType)(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * param);

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

int CL_SerialiserIndent(CL_Serialiser *ser, char indent, uint8_t deepnes);

int CL_SerialiserBlockStarts(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * blockParam);

int CL_SerialiserBlockEnds(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * blockParam);

int CL_SerialiserChild(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * childParam);


#endif /* of CL_SERIALISER_INC */
/*! @} */ /* EOF, no more */
