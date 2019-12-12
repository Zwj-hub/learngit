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
*                   Filename:   CL_Meas.h                                     *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file CL_MeasBuff.h
 *  @par File description:
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
#ifndef CL_STRING_INC  /* Allow multiple inclusions */
#define CL_STRING_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h> // for snprintf
#include "CL_Error.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------
#define CREATE_CL_String(name, buffer, bufferLen)\
	CL_String name = { \
		buffer,         \
		bufferLen,       \
		0                 \
	}

#define CL_StringAddTextError(string, err)\
	{err = -OUTPUT_BUFFER_FULL; string->buffer[string->len - 1] = 0; string->wr = string->len; }

#define CL_StringAddText(string, err, ... )                                                      \
		do{                                                                                       \
			int len = snprintf(&string->buffer[string->wr], string->len - string->wr, __VA_ARGS__);\
			string->wr += len;                                                                      \
			if (string->wr >= string->len || len < 0) CL_StringAddTextError(string, err);            \
		} while (0);

#define CL_StringLen(string) ((string)->wr)
//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------
typedef struct CL_String
{
	char * buffer;
	uint16_t len;
	uint16_t wr;

}CL_String;



//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------


#endif /* of CL_STRING_INC */
/*! @} */ /* EOF, no more */
