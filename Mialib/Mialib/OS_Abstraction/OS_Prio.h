/*! @addtogroup OS_Abstraction
 *  @{
 */
/******************************************************************************
*                                                                             *
*                   COPYRIGHT (C) 2009    ABB OY, DRIVES                      *
*                                                                             *
*******************************************************************************
*                                                                             *
*                                                                             *
*                     Daisy Assistant Panel SW                                *
*                                                                             *
*                                                                             *
*                  Subsystem:   OS Abstraction                                *
*                                                                             *
*                   Filename:   OS_Prio.h                                     *
*                       Date:   2015-12-01                                    *
*                                                                             *
*                 Programmer:   Sami Riikonen                                 *
*                     Target:   STM32/PC                                      *
*                                                                             *
*******************************************************************************/
/*!
 *  @file OS_Prio.h
 *  @par File description:
 *    Operating system specific configuration file for OS Abstraction layer.
 *    This file contains defines for task priorities.
 *    This file is for uCos-II.
 *
 *  @par Related documents:
 *
 *  @note
 */
/******************************************************************************/
#ifndef OS_PRIO_INC  /* Allow multiple inclusions */
#define OS_PRIO_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

// Max number of OS tasks, this is used by OS to allocate task control block table
#define BU_MAX_OS_TASKS                   42

// Task priority definitions. There are max 32 tasks.
// Last two are reserved for uCOS-II internal tasks (Idle and Statistics)
// so priorities up to 29 are usable.
#define BU_MUTEX_PIP_PRIO                  2   // Prio which will be set temporarily to thread owning FS-mutex

#define BU_TASK_START_PRIO                 3   // Startup task

#define BU_TASK_KEYPAD_PRIO                4   // Key task

#define BU_TASK_COMM_PRIO                  5   // Communication task
#define BU_TASK_CONTROL_PRIO               6   // Control task
#define BU_TASK_CPM_RT_PRIO                7   // Parameter RT reading task task high priority is wanted for timestamp accurazy
#define BU_TASK_UI_PRIO                    8   // UI task
#define BU_TASK_SCRIPT_PRIO                9   // Script task
#define BU_TASK_RS_PRIO                    10  // RS (resource) task
#define BU_TASK_WD_PRIO                    11  // Watchdog task
#define BU_TASK_TX_PRIO                    12  // Tx task priority of Phloem master.
#define BU_TASK_RX_PRIO                    13  // Rx task priority of Phloem master.
#define BU_TASK_PC_RS485_UART_PRIO         14  // PC simulator's RS485 uart task, not used in STM32
#define BU_TASK_PC_DEBUG_UART_PRIO         15  // PC simulator's debug uart task, not used in STM32
#define BU_TASK_BTPSKRNL_HCIRX_PRIO        16  // Bluetooth kernel HCIRX task, not used in PC simulator
#define BU_TASK_BTPSKRNL_MAIN_PRIO         17  // Bluetooth kernel main task, not used in PC simulator
#define BU_TASK_BTPSKRNL_TIMER_PRIO        18  // Bluetooth kernel timer task, not used in PC simulator
#define BU_TASK_BT_SOCKET_PRIO             18  // PC simulator's Bluetooth socket task, , not used in STM32
#define BU_TASK_BT_PRIO                    19  // Bluetooth task
#define USB_HIGHEST_BASE_PRIO              20  // Highest priority for USB stack tasks
// Priorities 21 and 22 are used by USB stack so they are already reserved
#define USB_HIGH_BASE_PRIO                 23  // High priority for USB stack tasks
#define BU_TASK_PEG_PRIO                   24  // PEG main task
#define BU_TASK_PEG_TIMER_PRIO             25  // PEG timer task
#define BU_TASK_DU_PRIO                    26  // Display update task
#define BU_TASK_POLL_PRIO                  27  // Poll task (panel bus)
#define USB_NORMAL_BASE_PRIO               28  // Not used (normal priority for USB stack tasks)
#define USB_LOW_BASE_PRIO                  28  // Not used (low priority for USB stack tasks)
#define USB_LOWEST_BASE_PRIO               28  // Not used (lowest priority for USB stack tasks)
#define BU_TASK_W32SIMMSG_PRIO             29  // PC simulator's msg pump task, not used in STM32.

#define BU_TASK_CPM_COMM_PRIO              31   // CPM Communication task

#define BU_TASK_CPM_COMM_MIA_0             32   // Communication task
#define BU_TASK_CPM_COMM_MIA_1             33   // Communication task
#define BU_TASK_CPM_COMM_MIA_2             34   // Communication task
#define BU_TASK_CPM_COMM_MIA_3             35   // Communication task

#define BU_TASK_APP_PRIO                   36   // Application task

// These have defines in ucos_ii.h, here just to remind that these are used.
//#define BU_TASK_STATISTIC_PRIO             BU_MAX_OS_TASKS + 1
//#define BU_TASK_IDLE_PRIO                  BU_MAX_OS_TASKS + 2

//-----------------------------------------------------------------------------
// 3) DATA STRUCTURES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 4) GLOBAL VARIABLE DECLARATIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 5) GLOBAL FUNCTION  DECLARATIONS
//-----------------------------------------------------------------------------

#endif  /* of OS_PRIO_INC */

/*! @} */ /* EOF, no more */
