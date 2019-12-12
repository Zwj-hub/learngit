/*
 * WS_Error.c
 *
 *  Created on: Sep 25, 2014
 *      Author: Tuan Vu
 */

#include "WS_Error.h"

#ifdef __linux__
	static __thread t_ErrorCode s_ErrorCode;
	static __thread int s_ExactCode;
#else
	static t_ErrorCode s_ErrorCode;
	static int s_ExactCode;
#endif

void G_SetLastErrorExact(t_ErrorCode code, int exactCode)
{
	s_ErrorCode = code;
	s_ExactCode = exactCode;
}

void G_SetLastError(t_ErrorCode code)
{
	s_ErrorCode = code;
}

t_ErrorCode G_GetLastError()
{
	return s_ErrorCode;
}

int G_GetExactCode()
{
	return s_ExactCode;
}

