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
*                   Filename:   tlsio_AzureIoTPanel.h                         *
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

#ifndef TLSIO_AZURE_IOT_PANEL_H
#define TLSIO_AZURE_IOT_PANEL_H

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

#include "azure_c_shared_utility/xio.h"
//#include "azure_c_shared_utility/umock_c_prod.h"               //zwj
#include "umock_c/umock_c_prod.h"
/**@brief Return the tlsio table of functions.
*
*  @param void.
*
*  @return The tlsio interface (IO_INTERFACE_DESCRIPTION).
*/
MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION*, tlsio_mia_get_interface_description);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLSIO_AZURE_IOT_PANEL_H */

