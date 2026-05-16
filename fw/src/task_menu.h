/*! *******************************************************************************************************
* Copyright (c) 2023-2026 K. Sz. Horvath
*
* All rights reserved
*
* \file task_menu.h
*
* \brief Menu system
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

#ifndef MENU_H
#define MENU_H

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/
//! \brief Menu action function pointer type
typedef void (*PFN_MENU_ACTION)( void );

//! \brief Boolean check function pointer type
typedef BOOL (*PFN_MENU_CHECK)( U8* );

//! \brief Menu item actions
typedef enum
{
  MENU_ACTION_SUBMENU,          //!< Menu item opens up a (sub)menu
  MENU_ACTION_WIDGET_FUNCTION,  //!< Menu item is a function call widget
  MENU_ACTION_WIDGET_TASK,      //!< Menu item runs a task
  MENU_ACTION_WIDGET_ONOFF,     //!< Menu item opens an on/off widget
  MENU_ACTION_WIDGET_NUMSET,    //!< Menu item opens a number setting widget
  MENU_ACTION_WIDGET_FILE       //!< Menu item opens a file selection widget
} E_MENUITEM_ACTION;

//! \brief Function call widget parameters
typedef struct
{
  BOOL                  bOpenNewPage;      //!< If TRUE, it opens a new page, otherwise it just runs the action function
  const char* const     pu8WidgetString;   //!< String displayed when the widget is activated
  const PFN_MENU_ACTION pfnAction;         //!< Function to be executed when activated
  const PFN_MENU_ACTION pfnActionOnEntry;  //!< Function to be called when the widget is activated
  const PFN_MENU_ACTION pfnActionOnExit;   //!< Function to be called when the widget is deactivated
} S_MENU_WIDGET_FUNCTION;

//! \brief Function call widget parameters
typedef struct
{
  const void*           pfnInitFunction;       //!< Pointer to the task initializer
  const void*           pfnMainCycleFunction;  //!< Pointer to the task main cycle
} S_MENU_WIDGET_TASK;

//! \brief On/off widget parameters
typedef struct
{
  const char* const     pu8WidgetString;     //!< String displayed when the widget is activated
  BOOL* const           pbBoolToChange;      //!< Pointer to the BOOL to be set
  const PFN_MENU_ACTION pfExitWidgetAction;  //!< Function to be called when the value is saved
} S_MENU_WIDGET_ONOFF;

//! \brief Number setting widget parameters
typedef struct
{
  const char*           pu8WidgetString;     //!< String displayed when the widget is activated
  U32* const            pu32NumberToChange;  //!< Pointer to the number to be set
  BOOL                  bDecimal;            //!< TRUE: decimal (BCD) value; FALSE: hex value
  U32                   u32FixPoint;         //!< Position of the decimal/binary point; 0 means integer number
  U32                   u32Min;              //!< Minimum value
  U32                   u32Max;              //!< Maximum value
  const PFN_MENU_ACTION pfExitWidgetAction;  //!< Function to be called when the value is saved
} S_MENU_WIDGET_NUMSET;

//! \brief File selection widget parameters
typedef struct
{
  U8* const             pu8FilePathBuffer;       //!< Pointer to the file path buffer
  U32                   u32FilePathBufferSize;   //!< Max. size of the path
  const PFN_MENU_CHECK  pfnFileCheckerFunction;  //!< Function that checks the validity of the selected file
  const PFN_MENU_ACTION pfExitWidgetAction;      //!< Function to be called when the value is saved
} S_MENU_WIDGET_FILESELECT;

//! \brief Item in a menu
typedef struct
{
  const char*           pu8ItemString;  //!< Pointer to the menu item string
  BOOL                  bSelectable;    //!< Can be selected, or not
  E_MENUITEM_ACTION     eAction;        //!< Action type of the menu item
  const void* const     pAction;        //!< Pointer to the action
} S_MENU_ITEM;

//! \brief Menu header
typedef struct
{
  U32                   u32NumItems;          //!< Number of items in the menu
  const PFN_MENU_ACTION pfnActionOnEntry;     //!< Function to be called when entering the menu
  const PFN_MENU_ACTION pfnActionOnExit;      //!< Function to be called when exiting the menu
  S_MENU_ITEM           asMenuItems[];        //!< Array of menu items
} S_MENU;


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Interface functions
//--------------------------------------------------------------------------------------------------------/
// System functions
void Task_Menu_Init( void );
void Task_Menu_Cycle( void );


#endif  // MENU_H

//-----------------------------------------------< EOF >--------------------------------------------------/
