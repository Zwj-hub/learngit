/*! @addtogroup AzureIoTApp
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
*                   Filename:   ParameterList.h                               *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file ParameterList.h
 *  @par File description:
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
#ifndef PARAMETERLIST_INC  /* Allow multiple inclusions */
#define PARAMETERLIST_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include "CL_Parameter.h"
#include "CL_Serialiser.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

#define CREATE_CL_ParameterList(name, size)    \
	CL_Parameter name ## _parameters[size]  = { {""} };    \
	CL_ParameterList name = {                  \
		name ## _parameters,                   \
		size                                   \
	}

#define CL_ParameterListSize(paraList)  ((paraList)->listCnt)

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------

typedef struct CL_ParameterList
{
	CL_Parameter *list;
	uint16_t listCnt;

}CL_ParameterList;

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

// Add new parameter to list.
int CL_ParameterListAddParam(CL_ParameterList *paraList, CL_Parameter *param);

// Get parameter from list using Id as key
CL_Parameter * CL_ParameterListGetParamByID(CL_ParameterList *paraList, ParameterID id);

// get paramet using index 0..amount of valid parameters
CL_Parameter * CL_ParameterListGetParamByIndex(CL_ParameterList *paraList, uint16_t index);

int CL_ParameterListSerialise(CL_ParameterList *paraList, CL_Serialiser * serialiser);


#endif /* of PARAMETERLIST_INC */
/*! @} */ /* EOF, no more */
