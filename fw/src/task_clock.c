/*! *******************************************************************************************************
* Copyright (c) 2026 K. Sz. Horvath
*
* All rights reserved
*
* \file task_clock.c
*
* \brief Clock task
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/
#include <stdio.h>
#include <string.h>
#include "rtc.h"
#include "types.h"
#include "buttons.h"
#include "buzzer.h"
#include "lcd.h"
#include "housekeeping.h"

// Own include
#include "task_clock.h"


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
#define BATTERY_ICON_WIDTH    (40u)  //!< Width of the battery icon in pixels
#define BATTERY_ICON_HEIGHT   (15u)  //!< Height of the battery icon in pixels


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/
//! \brief States of the clock settings state machine
//! \note  Must start from 0 and increase by 1.
typedef enum
{
  STATE_SET_HOUR = 0u,
  STATE_SET_MINUTES,
  STATE_SET_SECONDS,
  STATE_SET_YEAR,
  STATE_SET_MONTH,
  STATE_SET_DAY,
  STATE_SET_WEEKDAY
} E_STATE_SET;


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/
//! \brief Current time
static RTC_TimeTypeDef gsTimeStructure;

//! \brief Current date
static RTC_DateTypeDef gsDateStructure;


//--------------------------------------------------------------------------------------------------------/
// Static function declarations
//--------------------------------------------------------------------------------------------------------/
static void RefreshDisplay( void );
static void PrintBatteryLevel( U16 u16X, U16 u16Y );
static void PrintClock( U16 u16X, U16 u16Y );
static void PrintDate( U16 u16X, U16 u16Y );
static void CheckButtons( void );


//--------------------------------------------------------------------------------------------------------/
// Static functions
//--------------------------------------------------------------------------------------------------------/
/*! *******************************************************************
 * \brief  Refresh the display contents
 * \param  -
 * \return -
 *********************************************************************/
static void RefreshDisplay( void )
{
  // Display the battery level in the upper right corner
  PrintBatteryLevel( LCD_WIDTH - BATTERY_ICON_WIDTH - 1u, 0u );
  // Display clock in the middle of the screen
  PrintClock( (LCD_WIDTH/2u)-(8u*16u/2u), (LCD_HEIGHT/2u)-(26u/2u) );
  // Display the date in the upper left corner
  PrintDate( 0u, 0u );
}

/*! *******************************************************************
 * \brief  Prints the battery level/icon to a given position
 * \param  u16X: horizontal coordinate of the upper left corner
 * \param  u16Y: vertical coordinate of the upper left corner
 * \return -
 *********************************************************************/
static void PrintBatteryLevel( U16 u16X, U16 u16Y )
{
  E_HOUSEKEEPING_CHARGERSTATE eChargerState = Housekeeping_GetChargerState();
  U8 u8ChargeLevelPercent = Housekeeping_GetBatteryPercentage();
  char acString[] = "Full";
  
  // Draw battery siluette
  LCD_DrawLine( u16X, u16Y, u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y, WHITE );  // up
  LCD_DrawLine( u16X, u16Y + BATTERY_ICON_HEIGHT - 1u, u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y + BATTERY_ICON_HEIGHT - 1u, WHITE );  // down
  LCD_DrawLine( u16X, u16Y, u16X, u16Y + BATTERY_ICON_HEIGHT - 1u, WHITE );  // left
  LCD_DrawLine( u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y, u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y + BATTERY_ICON_HEIGHT/4u, WHITE );  // right upper
  LCD_DrawLine( u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y + (BATTERY_ICON_HEIGHT*3u)/4u, u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y + BATTERY_ICON_HEIGHT - 1u, WHITE );  // right lower
  LCD_DrawLine( u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y + BATTERY_ICON_HEIGHT/4u, u16X + BATTERY_ICON_WIDTH - 1u, u16Y + BATTERY_ICON_HEIGHT/4u, WHITE );  // right top
  LCD_DrawLine( u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y + (BATTERY_ICON_HEIGHT*3u)/4u, u16X + BATTERY_ICON_WIDTH - 1u, u16Y + (BATTERY_ICON_HEIGHT*3u)/4u, WHITE );  // right bottom
  LCD_DrawLine( u16X + BATTERY_ICON_WIDTH - 1u, u16Y + BATTERY_ICON_HEIGHT/4u, u16X + BATTERY_ICON_WIDTH - 1u, u16Y + (BATTERY_ICON_HEIGHT*3u)/4u, WHITE );  // right
  
  // If USB power is not connected...
  if( CHARGER_STATE_NONE == eChargerState )
  {
    // Fill battery siluette according to the charge level
    LCD_DrawFilledRectangle( u16X + 1u, u16Y + 1u, u16X + 1u + (BATTERY_ICON_WIDTH - 3u - 5u)*((U32)u8ChargeLevelPercent)/100u, u16Y + BATTERY_ICON_HEIGHT - 2u, GREEN );
    LCD_DrawFilledRectangle( u16X + 1u + (BATTERY_ICON_WIDTH - 2u - 5u)*((U32)u8ChargeLevelPercent)/100u, u16Y + 1u, u16X + BATTERY_ICON_WIDTH - 2u - 5u, u16Y + BATTERY_ICON_HEIGHT - 2u, BLACK );
    
    // Print percentage
    snprintf( acString, sizeof( acString ), "%02u", u8ChargeLevelPercent );
    LCD_PrintString( u16X + BATTERY_ICON_WIDTH/2u - 10u, u16Y + BATTERY_ICON_HEIGHT/2u - 4u, acString, LCD_FONT_7x10, WHITE, BLACK );
  }
  else if( CHARGER_STATE_CHARGING == eChargerState )  // charging in progress
  {
    // Clear battery siluette
    LCD_DrawFilledRectangle( u16X + 1u, u16Y + 1u, u16X + (BATTERY_ICON_WIDTH - 2u - 5u), u16Y + BATTERY_ICON_HEIGHT - 2u, BLACK );
    // Write "++" into siluette
    snprintf( acString, sizeof( acString ), "++" );
    LCD_PrintString( u16X + BATTERY_ICON_WIDTH/2u - 10u, u16Y + BATTERY_ICON_HEIGHT/2u - 4u, acString, LCD_FONT_7x10, WHITE, BLACK );
  }
  else  // fully charged
  {
    // Fill battery siluette
    LCD_DrawFilledRectangle( u16X + 1u, u16Y + 1u, u16X + (BATTERY_ICON_WIDTH - 2u - 5u), u16Y + BATTERY_ICON_HEIGHT - 2u, WHITE );
    // Write "Full" into siluette
    snprintf( acString, sizeof( acString ), "Full" );
    LCD_PrintString( u16X + BATTERY_ICON_WIDTH/2u - 17u, u16Y + BATTERY_ICON_HEIGHT/2u - 4u, acString, LCD_FONT_7x10, BLACK, WHITE );
  }
}

/*! *******************************************************************
 * \brief  Prints the clock to a given position
 * \param  u16X: horizontal coordinate of the upper left corner
 * \param  u16Y: vertical coordinate of the upper left corner
 * \return -
 *********************************************************************/
static void PrintClock( U16 u16X, U16 u16Y )
{
  char acString[] = "00:00:00";
  
  // Display time format : hh:mm:ss
  snprintf( acString,sizeof( acString ),"%02u:%02u:%02u",gsTimeStructure.Hours, gsTimeStructure.Minutes, gsTimeStructure.Seconds );
  LCD_PrintString( u16X, u16Y, acString, LCD_FONT_16x26, WHITE, BLACK );
}

/*! *******************************************************************
 * \brief  Prints the date to a given position
 * \param  u16X: horizontal coordinate of the upper left corner
 * \param  u16Y: vertical coordinate of the upper left corner
 * \return -
 *********************************************************************/
static void PrintDate( U16 u16X, U16 u16Y )
{
  char acString[] = "2026:05:12 TUE";
  const char cau8WeekDays[7u][4u] =
  {
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
    "SAT",
    "SUN"
  };
  // Display time format: yyyy,mm,dd
  snprintf( acString,sizeof( acString ),"%04u:%02u:%02u %3s",2000u+gsDateStructure.Year, gsDateStructure.Month, gsDateStructure.Date, cau8WeekDays[ gsDateStructure.WeekDay ] );
  LCD_PrintString( u16X, u16Y, acString, LCD_FONT_11x18, WHITE, BLACK );
}

/*! *******************************************************************
 * \brief  Checks the buttons and act accordingly
 * \param  -
 * \return -
 *********************************************************************/
static void CheckButtons( void )
{
  static U32  u32Timer = 0u;
  static U32  u32BlinkTimer = 0u;
  static BOOL bSetTime = FALSE;
  static E_STATE_SET eState = STATE_SET_HOUR;
  E_BUTTONS_EVENT eButtonTop = Buttons_GetEvent( BUTTON_SW1 );
  E_BUTTONS_EVENT eButtonBottom = Buttons_GetEvent( BUTTON_SW2 );
  U32 u32CurrentTick = HAL_GetTick();
  
  // Top button: adjust/select
  if( BUTTON_PRESSED == eButtonTop )
  {
    // If idle mode
    if( FALSE == bSetTime )
    {
      u32Timer = u32CurrentTick;
    }
    else  // we're setting the time and date
    {
      if( eState < STATE_SET_WEEKDAY )
      {
        eState++;
      }
      else  // everything is set
      {
        bSetTime = FALSE;
        //TODO: short beep
      }
    }
  }
  else if( BUTTON_RELEASED == eButtonTop )
  {
    u32Timer = 0u;
  }
  // Check top button timer
  if( ( 0u != u32Timer )
   && ( FALSE == bSetTime )
   && ( u32CurrentTick - u32Timer > 1000u ) )
  {
    bSetTime = TRUE;
    eState = STATE_SET_HOUR;
    //TODO: short beep
  }
  
  // Bottom button: set
  if( BUTTON_PRESSED == eButtonBottom )
  {
    // If we're setting the time and date
    if( TRUE == bSetTime )
    {
      switch( eState )
      {
        case STATE_SET_HOUR:
          gsTimeStructure.Hours += 1u;
          break;
          
        case STATE_SET_MINUTES:
          gsTimeStructure.Minutes += 1u;
          break;

        case STATE_SET_SECONDS:
          gsTimeStructure.Seconds = 0u;
          if( gsTimeStructure.Seconds >= 30u )
          {
            gsTimeStructure.Minutes += 1u;
          }
          break;

        case STATE_SET_YEAR:
          gsDateStructure.Year += 1u;
          break;

        case STATE_SET_MONTH:
          gsDateStructure.Month += 1u;
          break;

        case STATE_SET_DAY:
          gsDateStructure.Date += 1u;
          break;
          
        case STATE_SET_WEEKDAY:
          gsDateStructure.WeekDay += 1u;
          break;
          
        default:
          //TODO: error handler
          break;
      }
      //Check for validity and adjust format
      if( gsTimeStructure.Minutes >= 60u )
      {
        gsTimeStructure.Minutes -= 60u;
        gsTimeStructure.Hours += 1u;
      }
      if( gsTimeStructure.Hours >= 24u )
      {
        gsTimeStructure.Hours -= 24u;
        gsDateStructure.WeekDay += 1u;
        gsDateStructure.Date += 1u;
      }
      if( gsDateStructure.WeekDay >= 7u )
      {
        gsDateStructure.WeekDay -= 7u;
      }
#warning "FIXME: is this the right way?"
      if( gsDateStructure.Date >= 31u )
      {
        gsDateStructure.Date -= 31u;
        gsDateStructure.Month += 1u;
      }
      if( gsDateStructure.Month >= 12u )
      {
        gsDateStructure.Month -= 12u;
        gsDateStructure.Year += 1u;
      }
      // Update the time/date
      HAL_PWR_EnableBkUpAccess();
      __HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
      HAL_RTC_SetTime( &hrtc, &gsTimeStructure, RTC_FORMAT_BIN );
      HAL_RTC_SetDate( &hrtc, &gsDateStructure, RTC_FORMAT_BIN );
      __HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);
      HAL_PWR_DisableBkUpAccess();
    }
    else
    {
      // Nothing to do
    }
  }
  
  // Blink the current value to be set on screen
  if( TRUE == bSetTime )
  {
    if( u32CurrentTick - u32BlinkTimer >= 1000u )
    {
      u32BlinkTimer = u32CurrentTick;
    }
    if( u32CurrentTick - u32BlinkTimer < 200u )
    {
      // Mask out the value
      switch( eState )
      {
        case STATE_SET_HOUR:
          //LCD_FONT_16x26
          LCD_DrawFilledRectangle( (LCD_WIDTH/2u)-(8u*16u/2u), (LCD_HEIGHT/2u)-(26u/2u), (LCD_WIDTH/2u)-(8u*16u/2u) + 2u*16u, (LCD_HEIGHT/2u)-(26u/2u) + 26u, BLACK );
          break;
          
        case STATE_SET_MINUTES:
          LCD_DrawFilledRectangle( (LCD_WIDTH/2u)-(8u*16u/2u) + 3u*16, (LCD_HEIGHT/2u)-(26u/2u), (LCD_WIDTH/2u)-(8u*16u/2u) + 5u*16u, (LCD_HEIGHT/2u)-(26u/2u) + 26u, BLACK );
          break;

        case STATE_SET_SECONDS:
          LCD_DrawFilledRectangle( (LCD_WIDTH/2u)-(8u*16u/2u) + 6u*16, (LCD_HEIGHT/2u)-(26u/2u), (LCD_WIDTH/2u)-(8u*16u/2u) + 8u*16u, (LCD_HEIGHT/2u)-(26u/2u) + 26u, BLACK );
          break;

        case STATE_SET_YEAR:
          //LCD_FONT_11x18
          LCD_DrawFilledRectangle( 0u + 0u*11u, 0u, 0u + 4u*11u, 18u, BLACK );
          break;

        case STATE_SET_MONTH:
          LCD_DrawFilledRectangle( 0u + 5u*11u, 0u, 0u + 7u*11u, 18u, BLACK );
          break;

        case STATE_SET_DAY:
          LCD_DrawFilledRectangle( 0u + 8u*11u, 0u, 0u + 10u*11u, 18u, BLACK );
          break;
          
        case STATE_SET_WEEKDAY:
          LCD_DrawFilledRectangle( 0u + 11u*11u, 0u, 0u + 14u*11u, 18u, BLACK );
          break;
          
        default:
          //TODO: error handler
          break;
      }
    }
  }
  // Middle button: deep sleep
  if( BUTTON_PRESSED == Buttons_GetEvent( BUTTON_SW3 ) )
  {
    Housekeeping_DeepSleep();
  }
}

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
void Task_Clock_Init( void )
{
  // Clear screen
  LCD_Clear( BLACK );
  
  // We don't need the repeated press function for buttons
  Buttons_SetRepeatedPresses( FALSE );
}

/*! *******************************************************************
 * \brief  Task main cycle
 * \param  -
 * \return -
 *********************************************************************/
void Task_Clock_Cycle( void )
{
  static uint32_t u32Timer = 0u;
  
  // Get the current time from RTC
  HAL_RTC_GetTime( &hrtc, &gsTimeStructure, RTC_FORMAT_BIN );
  // Get the current date from RTC
  HAL_RTC_GetDate( &hrtc, &gsDateStructure, RTC_FORMAT_BIN );
  
  // Refresh display content
  if( HAL_GetTick() - u32Timer > 100u )
  {
    u32Timer = HAL_GetTick();
    RefreshDisplay();
  }

  // Check buttons
  CheckButtons();
}

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

//-----------------------------------------------< EOF >--------------------------------------------------/
