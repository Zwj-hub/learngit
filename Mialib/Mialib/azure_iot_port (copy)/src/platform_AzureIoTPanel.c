/*! @addtogroup azure iot port
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
*                  Subsystem:   azure iot port                                *
*                                                                             *
*                   Filename:   platform_AzureIoTPanel.c                      *
*                       Date:                                                 *
*                                                                             *
*                 Programmer:   Henri HAN                                     *
*                     Target:   All                                           *
*                                                                             *
*******************************************************************************/
/*!
 *  @file 
 *  @par File description:
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdint.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "azure_c_shared_utility/platform.h"
#include "tlsio_AzureIoTPanel.h"

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 4) LOCAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------
/**
  * @brief  Function for Initializing the platform
  * @param  None
  * @retval int Ok/Error (0/1)
  */
int platform_init(void)
{
	return 0;
}

/** @brief Function for de-initializing the platform
 *  @param None
 *  @retval None
 */
void platform_deinit(void)
{
	/* TODO: Insert here any platform specific one time de-initialization code like.
	This has to undo what has been done in platform_init. */
}

/**
  * @brief  Function for getting default tls io
  * @param  None
  * @retval IO_INTERFACE_DESCRIPTION*
  */
const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
	return tlsio_mia_get_interface_description();
}

STRING_HANDLE platform_get_platform_info(void)
{
	return STRING_construct("(IoTPanel)");
}
/*! @} */ /* EOF, no more */
