/*! *******************************************************************************************************
* Copyright (c) 2023-2025 K. Sz. Horvath
*
* All rights reserved
*
* \file buttons.h
*
* \brief Layer for reading and debouncing push buttons
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

#ifndef BUTTONS_H
#define BUTTONS_H

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/
#include "types.h"

//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
#define NUM_BUTTONS       (8u)  //!< Number of buttons present


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/
//! \brief Each push buttons have a state like this
typedef enum
{
  BUTTON_INACTIVE,  //!< The button is not pressed (debounced)
  BUTTON_BOUNCING,  //!< The button is pressed and currently bouncing
  BUTTON_ACTIVE,    //!< The button is pressed (debounced)
  BUTTON_RELEASING  //!< The button is released and currently bouncing
} E_BUTTONS_STATE;

//! \brief Button events
typedef enum
{
  BUTTON_NOEVENT,  //!< No events since last check
  BUTTON_PRESSED,  //!< The button was pressed since last check
  BUTTON_RELEASED  //!< The button was released since last check
} E_BUTTONS_EVENT;

//! \brief Index of each button
typedef enum
{
  BUTTON_SW1 = 0u,
  BUTTON_SW2,
  BUTTON_SW3,
  BUTTON_SW4_UP,
  BUTTON_SW4_DOWN,
  BUTTON_SW4_LEFT,
  BUTTON_SW4_RIGHT,
  BUTTON_SW4_PUSH
} E_BUTTONS_INDEX;


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/
extern volatile E_BUTTONS_STATE gaeButtonsState[ NUM_BUTTONS ];


//--------------------------------------------------------------------------------------------------------/
// Interface functions
//--------------------------------------------------------------------------------------------------------/
// System functions
void Buttons_Init( void );
void Buttons_TimerIT( void );

// API functions
E_BUTTONS_EVENT Buttons_GetEvent( E_BUTTONS_INDEX eButton );
void Buttons_SetRepeatedPresses( E_BUTTONS_INDEX eButton, BOOL bSet );


#endif  // BUTTONS_H

//-----------------------------------------------< EOF >--------------------------------------------------/
