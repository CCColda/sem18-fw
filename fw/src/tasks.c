/*! *******************************************************************************************************
* Copyright (c) 2026 K. Sz. Horvath
*
* All rights reserved
*
* \file tasks.c
*
* \brief Module responsible for running the tasks
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/
#include "types.h"
#include "task_menu.h"
#include "task_clock.h"

// Own include
#include "tasks.h"


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/
//! \brief Initializer function of the currently running task
static PFN_TASK_FUNCTION gpfTaskInitializer;

//! \brief Initializer function of the currently running task
static PFN_TASK_FUNCTION gpfTaskMainCycle;


//--------------------------------------------------------------------------------------------------------/
// Static function declarations
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Static functions
//--------------------------------------------------------------------------------------------------------/
/*! *******************************************************************
 * \brief
 * \param
 * \return
 *********************************************************************/

/*! *******************************************************************
 * \brief
 * \param
 * \return
 *********************************************************************/


//--------------------------------------------------------------------------------------------------------/
// Interface functions
//--------------------------------------------------------------------------------------------------------/
/*! *******************************************************************
 * \brief  Initialize module
 * \param  -
 * \return -
 *********************************************************************/
void Tasks_Init( void )
{
  // Default task: clock
  gpfTaskInitializer = Task_Clock_Init;
  gpfTaskMainCycle = Task_Clock_Cycle;

  // Initialize special menu task
  Task_Menu_Init();

  // Initialize default task
  gpfTaskInitializer();
}

/*! *******************************************************************
 * \brief  Main cycle
 * \param  -
 * \return -
 *********************************************************************/
void Tasks_Cycle( void )
{
  // Call the currently running task
  gpfTaskMainCycle();
}

/*! *******************************************************************
 * \brief  Changes the task to a new one
 * \param  pfTaskInitializer: pointer to the task initializer function
 * \param  pfTaskInitializer: pointer to the task main cycle function
 * \return -
 *********************************************************************/
void Tasks_Change( PFN_TASK_FUNCTION pfTaskInitializer, PFN_TASK_FUNCTION pfTaskMainCycle )
{
  gpfTaskInitializer = pfTaskInitializer;
  gpfTaskMainCycle = pfTaskMainCycle;

  // Initialize task
  gpfTaskInitializer();
}

/*! *******************************************************************
 * \brief  Exits the current task
 * \param  -
 * \return -
 *********************************************************************/
void Tasks_EndTask( void )
{
  // If current task is the menu task
  if( Task_Menu_Cycle == gpfTaskMainCycle )
  {
    // Return to the clock task
    gpfTaskInitializer = Task_Clock_Init;
    gpfTaskMainCycle = Task_Clock_Cycle;
  }
  else  // Anything else
  {
    // Return to the menu
    gpfTaskInitializer = Task_Menu_Init;
    gpfTaskMainCycle = Task_Menu_Cycle;
  }
  // Initialize task
  gpfTaskInitializer();
}

/*! *******************************************************************
 * \brief
 * \param
 * \return
 *********************************************************************/


//-----------------------------------------------< EOF >--------------------------------------------------/
