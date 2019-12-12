/*! @addtogroup wolfSSLv23_client_method
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

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include "CL_Serialiser.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

int CL_SerialiserCommandImpl(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * param);

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

#define GET_PARSER(a) (*(CL_SerialiserCmdType*)((a)->param)) // Serialiser implemantation call function must be first in struct

int CL_SerialiserBlockStarts(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * blockParam)
{
	int ret = GET_PARSER(ser)(ser, type, blockParam);
	if (ret)
		return ret;

	ser->level++;

	return ret;
}

int CL_SerialiserBlockEnds(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * blockParam)
{
	int ret;

	if (ser->level)
	{
		ser->level--;
	}

	ret = GET_PARSER(ser)(ser, type, blockParam);
	
	return ret;
}

int CL_SerialiserChild(CL_Serialiser *ser, CL_SerialiserChildTypes type, const void * childParam)
{
	return GET_PARSER(ser)(ser, type, childParam);
}

int CL_SerialiserIndent(CL_Serialiser *ser, char indent, uint8_t deepnes)
{
	int i;
	
	for (i = 0; i < deepnes * ser->level; i++)
	{
		int err = 0;
		CL_SERIALISER_ADD_TEXT(ser, err, "%c", indent);
		if (err)
			return err;
	}
	return 0;
}

/*! @} */ /* EOF, no more */
