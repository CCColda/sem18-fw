/*! *******************************************************************************************************
* Copyright (c) 2023-2026 K. Sz. Horvath
*
* All rights reserved
*
* \file buttons.c
*
* \brief Layer for reading and debouncing push buttons
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/
#include <string.h>
#include "main.h"
#include "types.h"
//#include "error_handler.h"

// Own include
#include "buttons.h"


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
#define BOUNCE_PERIOD_MS      (50u)  //!< Button bounce time period in msec
#define REPEAT_WAIT_MS       (400u)  //!< Time before repetition starts in msec
#define REPEAT_MS             (50u)  //!< Time between two repetitions in msec


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/
//! \brief GPIO pin descriptor
typedef struct
{
  GPIO_TypeDef* psGPIOPort;  //!< Pointer to the GPIO port 
  U16           u16GPIOPin;  //!< GPIO pin index
  GPIO_PinState ePolarity;   //!< GPIO_PIN_RESET: active low; GPIO_PIN_SET: active high
} S_GPIO_PIN;


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/
//! \brief Lookup table for button GPIO pins
//! \note  Should comply with E_BUTTONS_INDEX
static const S_GPIO_PIN casPins[ NUM_BUTTONS ] = 
{ 
  { BUTTON_SW1_GPIO_Port,       BUTTON_SW1_Pin,       GPIO_PIN_SET   }, 
  { BUTTON_SW2_GPIO_Port,       BUTTON_SW2_Pin,       GPIO_PIN_RESET }, 
  { BUTTON_SW3_GPIO_Port,       BUTTON_SW3_Pin,       GPIO_PIN_RESET },
  { BUTTON_SW4_UP_GPIO_Port,    BUTTON_SW4_UP_Pin,    GPIO_PIN_RESET },
  { BUTTON_SW4_DOWN_GPIO_Port,  BUTTON_SW4_DOWN_Pin,  GPIO_PIN_RESET },
  { BUTTON_SW4_LEFT_GPIO_Port,  BUTTON_SW4_LEFT_Pin,  GPIO_PIN_RESET },
  { BUTTON_SW4_RIGHT_GPIO_Port, BUTTON_SW4_RIGHT_Pin, GPIO_PIN_RESET },
  { BUTTON_SW4_PUSH_GPIO_Port,  BUTTON_SW4_PUSH_Pin,  GPIO_PIN_RESET }
};

//! \brief Long pressed buttons generate repeated events, or not
static volatile BOOL gbRepeatedPress;

//! \brief States of each buttons
volatile E_BUTTONS_STATE gaeButtonsState[ NUM_BUTTONS ];

//! \brief Debounce timers
volatile U32 gau32ButtonsTimer[ NUM_BUTTONS ];

//! \brief Button events since last check
volatile E_BUTTONS_EVENT gaeButtonsEvent[ NUM_BUTTONS ];


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


//--------------------------------------------------------------------------------------------------------/
// Interface functions
//--------------------------------------------------------------------------------------------------------/
/*! *******************************************************************
 * \brief  Initialize layer
 * \param  -
 * \return -
 *********************************************************************/
void Buttons_Init( void )
{
  U32 u32Index;
  
  memset( (void*)gau32ButtonsTimer, 0, sizeof( gau32ButtonsTimer ) );
  for( u32Index = 0u; u32Index < NUM_BUTTONS; u32Index++ )
  {
    gaeButtonsState[ u32Index ] = BUTTON_INACTIVE;
    gaeButtonsEvent[ u32Index ] = BUTTON_NOEVENT;
  }
  gbRepeatedPress = FALSE;
}

/*! *******************************************************************
 * \brief  Callback from msec timer routine
 * \param  -
 * \return -
 *********************************************************************/
void Buttons_TimerIT( void )
{
  U32 u32Index;
  E_BUTTONS_STATE eState;
  U32 u32TimeNowMs = HAL_GetTick();
  
  // Sample the buttons
  for( u32Index = 0u; u32Index < NUM_BUTTONS; u32Index++ )
  {
    eState = gaeButtonsState[ u32Index ];
    // If input is active
    if( casPins[ u32Index ].ePolarity == HAL_GPIO_ReadPin( casPins[ u32Index ].psGPIOPort, casPins[ u32Index ].u16GPIOPin ) )
    {
      // Implement behaviour based on the current state
      switch( eState )
      {
        case BUTTON_INACTIVE:
          // Start debouncing
          gau32ButtonsTimer[ u32Index ] = u32TimeNowMs + BOUNCE_PERIOD_MS;
          gaeButtonsState[ u32Index ] = BUTTON_BOUNCING;
          break;
          
        case BUTTON_BOUNCING:
          if( (I32)( gau32ButtonsTimer[ u32Index ] - u32TimeNowMs ) <= 0 )
          {
            // Timer has expired
            gaeButtonsState[ u32Index ] = BUTTON_ACTIVE;
            // Set event flag
            gaeButtonsEvent[ u32Index ] = BUTTON_PRESSED;
            // Start repetition timer
            gau32ButtonsTimer[ u32Index ] = u32TimeNowMs + REPEAT_WAIT_MS;
          }
          break;
          
        case BUTTON_ACTIVE:
          // Repetition, if enabled
          if( TRUE == gbRepeatedPress )
          {
            // Time's up
            if( (I32)( gau32ButtonsTimer[ u32Index ] - u32TimeNowMs ) <= 0 )
            {
              // Timer has expired, so set event flag
              gaeButtonsEvent[ u32Index ] = BUTTON_PRESSED;
              // Restart repetition timer
              gau32ButtonsTimer[ u32Index ] = u32TimeNowMs + REPEAT_MS;
            }
          }
          break;
          
        case BUTTON_RELEASING:
          // Restart debounce timer
          gau32ButtonsTimer[ u32Index ] = u32TimeNowMs + BOUNCE_PERIOD_MS;
          break;
          
        default:  // Note: this should not happen
          //TODO: implement error handling
          //ErrorHandler_Fatal( __FILE__ , __LINE__, (U32)eState, 0u, 0u );
          break;
      }
    }
    else  // input is inactive
    {
      // Implement behaviour based on the current state
      switch( eState )
      {
        case BUTTON_INACTIVE:
          // Do nothing
          break;
          
        case BUTTON_BOUNCING:
          // Restart debounce timer
          gau32ButtonsTimer[ u32Index ] = u32TimeNowMs + BOUNCE_PERIOD_MS;
          break;
          
        case BUTTON_ACTIVE:
          // Start debouncing
          gau32ButtonsTimer[ u32Index ] = u32TimeNowMs + BOUNCE_PERIOD_MS;
          gaeButtonsState[ u32Index ] = BUTTON_RELEASING;
          break;
          
        case BUTTON_RELEASING:
          if( (I32)( gau32ButtonsTimer[ u32Index ] - u32TimeNowMs ) <= 0 )
          {
            // Timer has expired
            gaeButtonsState[ u32Index ] = BUTTON_INACTIVE;
            // Set event flag
            gaeButtonsEvent[ u32Index ] = BUTTON_RELEASED;
          }
          break;
          
        default:  // Note: this should not happen
          //TODO: implement error handling
          //ErrorHandler_Fatal( __FILE__ , __LINE__, (U32)eState, 0u, 0u );
          break;
      }
    }
  }
}

/*! *******************************************************************
 * \brief  Get last event on given button
 * \param  eButton: index of the button
 * \return Button event code
 * \note   Clears the event
 *********************************************************************/
E_BUTTONS_EVENT Buttons_GetEvent( E_BUTTONS_INDEX eButton )
{
  E_BUTTONS_EVENT eReturn;
  U32 u32PRIMASK = __get_PRIMASK();  // get PRIMASK so we know interrupts were enabled or not
  
  if( (U32)eButton > NUM_BUTTONS )
  {
    //TODO: implement error handling
    //ErrorHandler_Fatal( __FILE__ , __LINE__, (U32)eButton, 0u, 0u );
  }
  
  __disable_irq();                   // disable interrupts

  eReturn = gaeButtonsEvent[ eButton ];
  gaeButtonsEvent[ eButton ] = BUTTON_NOEVENT;

  if( 0 == u32PRIMASK )  // re-enable interrupts only if they were enabled before
  {
    __enable_irq();
  }
  return eReturn;
}

/*! *******************************************************************
 * \brief  Turn repeated keypress events on long pressed buttons on/off.
 * \param  bSet: TRUE: turn on; FALSE: turn off
 * \return -
 *********************************************************************/
void Buttons_SetRepeatedPresses( BOOL bSet )
{
  gbRepeatedPress = bSet;
}

/*! *******************************************************************
 * \brief
 * \param
 * \return
 *********************************************************************/



//-----------------------------------------------< EOF >--------------------------------------------------/
