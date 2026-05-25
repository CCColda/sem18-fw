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
#include "spi.h"
#include "types.h"
#include "buttons.h"
#include "buzzer.h"
#include "lcd.h"
#include "housekeeping.h"
#include "tasks.h"

// Own include
#include "task_clock.h"


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
#define BATTERY_ICON_WIDTH      (40u)  //!< Width of the battery icon in pixels
#define BATTERY_ICON_HEIGHT     (15u)  //!< Height of the battery icon in pixels
#define COLOR_INK             (WHITE)  //!< Color of the text/numbers
#define COLOR_BG              (BLACK)  //!< Color of the background
#define SEGMENTS_WIDTH          (32u)  //!< Width of a 7-segment number in pixels


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
  STATE_SET_WEEKDAY,
  STATE_SET_DST,
  //
  NUMBEROF_STATE_SET_ELEMENTS
} E_STATE_SET;


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/
//! \brief Current time
static RTC_TimeTypeDef gsTimeStructure;

//! \brief Current date
static RTC_DateTypeDef gsDateStructure;

//! \brief Is it daylight saving time or not
static BOOL gbDaylightSavingTime;

//! \brief Which parts of the clock should be displayed
//! \note  Indexed by E_STATE_SET values
static BOOL gabDisplayPartOn[ NUMBEROF_STATE_SET_ELEMENTS ];


//--------------------------------------------------------------------------------------------------------/
// Static function declarations
//--------------------------------------------------------------------------------------------------------/
static void RefreshDisplay( void );
static void PrintBatteryLevel( U16 u16X, U16 u16Y );
static void DrawSegment( BOOL bHorizontal, U16 u16X0, U16 u16Y0, U16 u16X1, U16 u16Y1, U16 u16Color );
static void Draw7Segment( U8 u8Number, U16 u16X0, U16 u16Y0, U16 u16WidthPx, U16 u16Color );
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
  PrintClock( (LCD_WIDTH/2u)-(7*SEGMENTS_WIDTH/2u), (LCD_HEIGHT*0.6)-SEGMENTS_WIDTH );
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
  LCD_DrawLine( u16X, u16Y, u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y, COLOR_INK );  // up
  LCD_DrawLine( u16X, u16Y + BATTERY_ICON_HEIGHT - 1u, u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y + BATTERY_ICON_HEIGHT - 1u, COLOR_INK );  // down
  LCD_DrawLine( u16X, u16Y, u16X, u16Y + BATTERY_ICON_HEIGHT - 1u, COLOR_INK );  // left
  LCD_DrawLine( u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y, u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y + BATTERY_ICON_HEIGHT/4u, COLOR_INK );  // right upper
  LCD_DrawLine( u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y + (BATTERY_ICON_HEIGHT*3u)/4u, u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y + BATTERY_ICON_HEIGHT - 1u, COLOR_INK );  // right lower
  LCD_DrawLine( u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y + BATTERY_ICON_HEIGHT/4u, u16X + BATTERY_ICON_WIDTH - 1u, u16Y + BATTERY_ICON_HEIGHT/4u, COLOR_INK );  // right top
  LCD_DrawLine( u16X + BATTERY_ICON_WIDTH - 1u - 5u, u16Y + (BATTERY_ICON_HEIGHT*3u)/4u, u16X + BATTERY_ICON_WIDTH - 1u, u16Y + (BATTERY_ICON_HEIGHT*3u)/4u, COLOR_INK );  // right bottom
  LCD_DrawLine( u16X + BATTERY_ICON_WIDTH - 1u, u16Y + BATTERY_ICON_HEIGHT/4u, u16X + BATTERY_ICON_WIDTH - 1u, u16Y + (BATTERY_ICON_HEIGHT*3u)/4u, COLOR_INK );  // right
  
  // If USB power is not connected...
  if( CHARGER_STATE_NONE == eChargerState )
  {
    // Fill battery siluette according to the charge level
    LCD_DrawFilledRectangle( u16X + 1u, u16Y + 1u, u16X + 1u + (BATTERY_ICON_WIDTH - 3u - 5u)*((U32)u8ChargeLevelPercent)/100u, u16Y + BATTERY_ICON_HEIGHT - 2u, GREEN );
    LCD_DrawFilledRectangle( u16X + 1u + (BATTERY_ICON_WIDTH - 2u - 5u)*((U32)u8ChargeLevelPercent)/100u, u16Y + 1u, u16X + BATTERY_ICON_WIDTH - 2u - 5u, u16Y + BATTERY_ICON_HEIGHT - 2u, COLOR_BG );
    
    // Print percentage
    snprintf( acString, sizeof( acString ), "%02u", u8ChargeLevelPercent );
    LCD_PrintString( u16X + BATTERY_ICON_WIDTH/2u - 10u, u16Y + BATTERY_ICON_HEIGHT/2u - 4u, acString, LCD_FONT_7x10, COLOR_INK, COLOR_BG );
  }
  else if( CHARGER_STATE_CHARGING == eChargerState )  // charging in progress
  {
    // Clear battery siluette
    LCD_DrawFilledRectangle( u16X + 1u, u16Y + 1u, u16X + (BATTERY_ICON_WIDTH - 2u - 5u), u16Y + BATTERY_ICON_HEIGHT - 2u, COLOR_BG );
    // Write "++" into siluette
    snprintf( acString, sizeof( acString ), "++" );
    LCD_PrintString( u16X + BATTERY_ICON_WIDTH/2u - 10u, u16Y + BATTERY_ICON_HEIGHT/2u - 4u, acString, LCD_FONT_7x10, COLOR_INK, COLOR_BG );
  }
  else  // fully charged
  {
    // Fill battery siluette
    LCD_DrawFilledRectangle( u16X + 1u, u16Y + 1u, u16X + (BATTERY_ICON_WIDTH - 2u - 5u), u16Y + BATTERY_ICON_HEIGHT - 2u, COLOR_INK );
    // Write "Full" into siluette
    snprintf( acString, sizeof( acString ), "Full" );
    LCD_PrintString( u16X + BATTERY_ICON_WIDTH/2u - 17u, u16Y + BATTERY_ICON_HEIGHT/2u - 4u, acString, LCD_FONT_7x10, COLOR_BG, COLOR_INK );
  }
}

/*! *******************************************************************
 * \brief  Draws a segment of a 7-segment display to a given position
 * \param  bHorizontal: the segment is horizontal, or not
 * \param  u16X0: horizontal coordinate of the upper left corner
 * \param  u16Y0: vertical coordinate of the upper left corner
 * \param  u16X1: horizontal coordinate of the lower right corner
 * \param  u16Y1: vertical coordinate of the lower right corner
 * \param  u16Color: color of the segment to be drawn
 * \return -
 *********************************************************************/
static void DrawSegment( BOOL bHorizontal, U16 u16X0, U16 u16Y0, U16 u16X1, U16 u16Y1, U16 u16Color )
{
  U16 u16Temp;

  // NOTE: A segment is made up from 3 parts: a rectangle and two triangles at each end.
  //       The triangle has the same height as the width of the rectangle.
  if( u16X1 < u16X0 )
  {
    u16Temp = u16X0;
    u16X0 = u16X1;
    u16X1 = u16Temp;
  }
  if( u16Y1 < u16Y0 )
  {
    u16Temp = u16Y0;
    u16Y0 = u16Y1;
    u16Y1 = u16Temp;
  }
  // If vertical segment
  if( FALSE == bHorizontal )
  {
    // Draw rectangle part
    LCD_DrawFilledRectangle( u16X0, u16Y0+(u16X1-u16X0)/2u, u16X1, u16Y1-(u16X1-u16X0)/2u, u16Color );
    // Draw upper triangle
    LCD_DrawFilledTriangle( (u16X0 + u16X1)>>1u, u16Y0, u16X0, u16Y0+(u16X1-u16X0)/2u, u16X1, u16Y0+(u16X1-u16X0)/2u, u16Color );
    // Draw lower triangle
    LCD_DrawFilledTriangle( u16X0, u16Y1-(u16X1-u16X0)/2u, u16X1, u16Y1-(u16X1-u16X0)/2u, (u16X0 + u16X1)>>1u, u16Y1, u16Color );
  }
  else  // horizontal segment
  {
    // Draw rectangle part
    LCD_DrawFilledRectangle( u16X0+(u16Y1-u16Y0)/2u, u16Y0, u16X1-(u16Y1-u16Y0)/2u, u16Y1, u16Color );
    // Draw left triangle
    LCD_DrawFilledTriangle( u16X0, (u16Y0 + u16Y1)>>1u, u16X0+(u16Y1-u16Y0)/2u, u16Y0, u16X0+(u16Y1-u16Y0)/2u, u16Y1, u16Color );
    // Draw right triangle
    LCD_DrawFilledTriangle( u16X1, (u16Y0 + u16Y1)>>1u, u16X1-(u16Y1-u16Y0)/2u, u16Y0, u16X1-(u16Y1-u16Y0)/2u, u16Y1, u16Color );
  }
}

/*! *******************************************************************
 * \brief  Draws a number in 7-segment style to a given position
 * \param  u8Number: number to be displayed
 * \param  u16X0: horizontal coordinate of the upper left corner
 * \param  u16Y0: vertical coordinate of the upper left corner
 * \param  u16WidthPx: width of the number in pixels
 * \param  u16Color: color of the number
 * \return -
 *********************************************************************/
static void Draw7Segment( U8 u8Number, U16 u16X0, U16 u16Y0, U16 u16WidthPx, U16 u16Color )
{
  /* Note: Segments are encoded in the following way:
   *
   * +--B--+
   * |     |
   * A     C
   * |     |
   * +--G--+
   * |     |
   * F     D
   * |     |
   * +--E--+
   *
   * where the bits are ABCDEFG
   */
  static const U8 cau8Segments[ 10u ] =
  {
    0x7Eu,  // 0
    0x18u,  // 1
    0x37u,  // 2
    0x3Du,  // 3
    0x59u,  // 4
    0x6Du,  // 5
    0x6Fu,  // 6
    0x38u,  // 7
    0x7Fu,  // 8
    0x7Du   // 9
  };

  // Draw segment A
  if( 0u != (cau8Segments[ u8Number ] & 0x40u ) ) DrawSegment( FALSE, u16X0, u16Y0, u16X0+u16WidthPx/4u, u16Y0+u16WidthPx, u16Color );
  // Draw segment B
  if( 0u != (cau8Segments[ u8Number ] & 0x20u ) ) DrawSegment( TRUE, u16X0, u16Y0, u16X0+u16WidthPx, u16Y0+u16WidthPx/4u, u16Color );
  // Draw segment C
  if( 0u != (cau8Segments[ u8Number ] & 0x10u ) ) DrawSegment( FALSE, u16X0+u16WidthPx-u16WidthPx/4u, u16Y0, u16X0+u16WidthPx, u16Y0+u16WidthPx, u16Color );
  // Draw segment D
  if( 0u != (cau8Segments[ u8Number ] & 0x08u ) ) DrawSegment( FALSE, u16X0+u16WidthPx-u16WidthPx/4u, u16Y0+u16WidthPx, u16X0+u16WidthPx, u16Y0+u16WidthPx*2u, u16Color );
  // Draw segment E
  if( 0u != (cau8Segments[ u8Number ] & 0x04u ) ) DrawSegment( TRUE, u16X0, u16Y0+u16WidthPx*2u-u16WidthPx/4u, u16X0+u16WidthPx, u16Y0+u16WidthPx*2u, u16Color );
  // Draw segment F
  if( 0u != (cau8Segments[ u8Number ] & 0x02u ) ) DrawSegment( FALSE, u16X0, u16Y0+u16WidthPx, u16X0+u16WidthPx/4u, u16Y0+u16WidthPx*2u, u16Color );
  // Draw segment G
  if( 0u != (cau8Segments[ u8Number ] & 0x01u ) ) DrawSegment( TRUE, u16X0, u16Y0+u16WidthPx-u16WidthPx/8u, u16X0+u16WidthPx, u16Y0+u16WidthPx+u16WidthPx/8u, u16Color );
}

/*! *******************************************************************
 * \brief  Prints the clock to a given position
 * \param  u16X: horizontal coordinate of the upper left corner
 * \param  u16Y: vertical coordinate of the upper left corner
 * \return -
 *********************************************************************/
static void PrintClock( U16 u16X, U16 u16Y )
{
  // Clear display
  LCD_DrawFilledRectangle( u16X, u16Y, u16X+SEGMENTS_WIDTH*6.6, u16Y+2*SEGMENTS_WIDTH, COLOR_BG );

  // Draw hours
  if( TRUE == gabDisplayPartOn[ STATE_SET_HOUR ] )
  {
    Draw7Segment( gsTimeStructure.Hours/10u, u16X, u16Y, SEGMENTS_WIDTH, COLOR_INK );
    Draw7Segment( gsTimeStructure.Hours%10u, u16X+SEGMENTS_WIDTH*1.2, u16Y, SEGMENTS_WIDTH, COLOR_INK );
  }
  // Draw separator
  LCD_DrawFilledRectangle( u16X+SEGMENTS_WIDTH*2.4, u16Y+SEGMENTS_WIDTH*0.45, u16X+SEGMENTS_WIDTH*2.5, u16Y+SEGMENTS_WIDTH*0.55, COLOR_INK );
  LCD_DrawFilledRectangle( u16X+SEGMENTS_WIDTH*2.4, u16Y+SEGMENTS_WIDTH*1.45, u16X+SEGMENTS_WIDTH*2.5, u16Y+SEGMENTS_WIDTH*1.55, COLOR_INK );
  // Draw minutes
  if( TRUE == gabDisplayPartOn[ STATE_SET_MINUTES ] )
  {
    Draw7Segment( gsTimeStructure.Minutes/10u, u16X+SEGMENTS_WIDTH*2.7, u16Y, SEGMENTS_WIDTH, COLOR_INK );
    Draw7Segment( gsTimeStructure.Minutes%10u, u16X+SEGMENTS_WIDTH*3.9, u16Y, SEGMENTS_WIDTH, COLOR_INK );
  }
  // Draw seconds
  if( TRUE == gabDisplayPartOn[ STATE_SET_SECONDS ] )
  {
    Draw7Segment( gsTimeStructure.Seconds/10u, u16X+SEGMENTS_WIDTH*5.4, u16Y+SEGMENTS_WIDTH, SEGMENTS_WIDTH/2, COLOR_INK );
    Draw7Segment( gsTimeStructure.Seconds%10u, u16X+SEGMENTS_WIDTH*6.1, u16Y+SEGMENTS_WIDTH, SEGMENTS_WIDTH/2, COLOR_INK );
  }
  // Draw Daylight Saving Time (DST)
  if( ( ( TRUE == gabDisplayPartOn[ STATE_SET_DST ] )
     && ( TRUE == gbDaylightSavingTime ) )
   || ( ( FALSE == gabDisplayPartOn[ STATE_SET_DST ] )
       && ( FALSE == gbDaylightSavingTime ) ) )
  {
    // Print "DST" over the seconds display
    LCD_PrintString( u16X+SEGMENTS_WIDTH*5.5, u16Y, "DST", LCD_FONT_11x18, COLOR_INK, COLOR_BG );
  }
}

/*! *******************************************************************
 * \brief  Prints the date to a given position
 * \param  u16X: horizontal coordinate of the upper left corner
 * \param  u16Y: vertical coordinate of the upper left corner
 * \return -
 *********************************************************************/
static void PrintDate( U16 u16X, U16 u16Y )
{
  char acString[] = "    -  -      ";
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
  // Display time format: yyyy-mm-dd
  if( TRUE == gabDisplayPartOn[ STATE_SET_YEAR ] )
  {
    snprintf( &acString[0u], sizeof( acString ), "%04u-  -      ", 2000u+gsDateStructure.Year );
  }
  if( TRUE == gabDisplayPartOn[ STATE_SET_MONTH ] )
  {
    snprintf( &acString[5u], sizeof( acString )-5u, "%02u-      ", gsDateStructure.Month );
  }
  if( TRUE == gabDisplayPartOn[ STATE_SET_DAY ] )
  {
    snprintf( &acString[8u], sizeof( acString )-8u, "%02u    ", gsDateStructure.Date );
  }
  if( TRUE == gabDisplayPartOn[ STATE_SET_WEEKDAY ] )
  {
    snprintf( &acString[11u], sizeof( acString )-11u, "%3s", cau8WeekDays[ gsDateStructure.WeekDay - 1u ] );
  }
  LCD_PrintString( u16X, u16Y, acString, LCD_FONT_11x18, COLOR_INK, COLOR_BG );
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
  E_BUTTONS_EVENT eButtonMiddle = Buttons_GetEvent( BUTTON_SW3 );
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
      if( eState < STATE_SET_DST )
      {
        gabDisplayPartOn[ (U8)eState ] = TRUE;
        eState++;
        Buzzer_Beep( 2700u, 128u, 50u );
      }
      else  // everything is set
      {
        bSetTime = FALSE;
        gabDisplayPartOn[ (U8)eState ] = TRUE;
        Buzzer_Beep( 3100u, 128u, 50u );
        Buttons_SetRepeatedPresses( BUTTON_SW2, FALSE );
        Buttons_SetRepeatedPresses( BUTTON_SW3, FALSE );
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
    // Enter time setting state
    bSetTime = TRUE;
    eState = STATE_SET_HOUR;
    Buzzer_Beep( 2700u, 128u, 50u );
    Buttons_SetRepeatedPresses( BUTTON_SW2, TRUE );
    Buttons_SetRepeatedPresses( BUTTON_SW3, TRUE );
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
          if( gsTimeStructure.Seconds >= 30u )
          {
            gsTimeStructure.Minutes += 1u;
          }
          gsTimeStructure.Seconds = 0u;
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

        case STATE_SET_DST:
          gbDaylightSavingTime = (TRUE == gbDaylightSavingTime) ? FALSE : TRUE;
          if( TRUE == gbDaylightSavingTime )
          {
            gsTimeStructure.DayLightSaving = RTC_CR_ADD1H | RTC_CR_BKP;
          }
          else
          {
            gsTimeStructure.DayLightSaving = RTC_CR_SUB1H;
          }
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
      if( gsDateStructure.WeekDay > 7u )
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
      HAL_Delay( 5u );
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
      gabDisplayPartOn[ (U8)eState ] = FALSE;
    }
    else
    {
      gabDisplayPartOn[ (U8)eState ] = TRUE;
    }
  }

  // Middle button
  if( BUTTON_PRESSED == eButtonMiddle )
  {
    // If we're setting the time and date
    if( TRUE == bSetTime )
    {
      switch( eState )
      {
        case STATE_SET_HOUR:
          gsTimeStructure.Hours -= 1u;
          break;

        case STATE_SET_MINUTES:
          gsTimeStructure.Minutes -= 1u;
          break;

        case STATE_SET_SECONDS:
          if( gsTimeStructure.Seconds >= 30u )
          {
            gsTimeStructure.Minutes += 1u;
          }
          gsTimeStructure.Seconds = 0u;
          break;

        case STATE_SET_YEAR:
          gsDateStructure.Year -= 1u;
          break;

        case STATE_SET_MONTH:
          gsDateStructure.Month -= 1u;
          break;

        case STATE_SET_DAY:
          gsDateStructure.Date -= 1u;
          break;

        case STATE_SET_WEEKDAY:
          gsDateStructure.WeekDay -= 1u;
          break;

        case STATE_SET_DST:
          gbDaylightSavingTime = (TRUE == gbDaylightSavingTime) ? FALSE : TRUE;
          if( TRUE == gbDaylightSavingTime )
          {
            gsTimeStructure.DayLightSaving = RTC_CR_ADD1H | RTC_CR_BKP;
          }
          else
          {
            gsTimeStructure.DayLightSaving = RTC_CR_SUB1H;
          }
          break;

        default:
          //TODO: error handler
          break;
      }
      //Check for validity and adjust format
      if( gsTimeStructure.Minutes >= 60u )
      {
        gsTimeStructure.Minutes += 60u;
        gsTimeStructure.Hours -= 1u;
      }
      if( gsTimeStructure.Hours >= 24u )
      {
        gsTimeStructure.Hours += 24u;
        gsDateStructure.WeekDay -= 1u;
        gsDateStructure.Date -= 1u;
      }
      if( gsDateStructure.WeekDay > 7u )
      {
        gsDateStructure.WeekDay += 7u;
      }
#warning "FIXME: is this the right way?"
      if( gsDateStructure.Date >= 31u )
      {
        gsDateStructure.Date += 31u;
        gsDateStructure.Month -= 1u;
      }
      if( gsDateStructure.Month >= 12u )
      {
        gsDateStructure.Month += 12u;
        gsDateStructure.Year -= 1u;
      }
      // Update the time/date
      HAL_PWR_EnableBkUpAccess();
      __HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
      HAL_RTC_SetTime( &hrtc, &gsTimeStructure, RTC_FORMAT_BIN );
      HAL_Delay( 5u );
      HAL_RTC_SetDate( &hrtc, &gsDateStructure, RTC_FORMAT_BIN );
      __HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);
      HAL_PWR_DisableBkUpAccess();
    }
    else
    {
      // Enter deep sleep
      Housekeeping_DeepSleep();
    }
  }

  // Joystick push: go to the menu
  if( BUTTON_PRESSED == Buttons_GetEvent( BUTTON_SW4_PUSH ) )
  {
    Tasks_EndTask();
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
  U8 u8Index;

  // Clear screen
  LCD_Clear( COLOR_BG );
  
  // All parts of the screen should be on
  for( u8Index = 0u; u8Index < sizeof( gabDisplayPartOn )/sizeof( BOOL ); u8Index++ )
  {
    gabDisplayPartOn[ u8Index ] = TRUE;
  }

  // We don't need the repeated press function for buttons normally
  Buttons_SetRepeatedPresses( BUTTON_SW1, FALSE );
  Buttons_SetRepeatedPresses( BUTTON_SW2, FALSE );
  Buttons_SetRepeatedPresses( BUTTON_SW3, FALSE );
  Buttons_SetRepeatedPresses( BUTTON_SW4_UP, FALSE );
  Buttons_SetRepeatedPresses( BUTTON_SW4_DOWN, FALSE );
  Buttons_SetRepeatedPresses( BUTTON_SW4_LEFT, FALSE );
  Buttons_SetRepeatedPresses( BUTTON_SW4_RIGHT, FALSE );
  Buttons_SetRepeatedPresses( BUTTON_SW4_PUSH, FALSE );
}

/*! *******************************************************************
 * \brief  Task main cycle
 * \param  -
 * \return -
 *********************************************************************/
void Task_Clock_Cycle( void )
{
  static uint32_t u32Timer = 0u;
  
  // Refresh display content
  if( ( HAL_GetTick() - u32Timer > 100u )
   && ( hdma_spi2_tx.Instance->CNDTR <= LCD_WIDTH*(LCD_HEIGHT*1u/3u + 1u) )  // synchronization to the display DMA
   && ( hdma_spi2_tx.Instance->CNDTR > LCD_WIDTH*(LCD_HEIGHT*1u/3u) ) )
  {
    u32Timer = HAL_GetTick();
    // Get the current time from RTC
    HAL_RTC_GetTime( &hrtc, &gsTimeStructure, RTC_FORMAT_BIN );
    // Get the current date from RTC
    HAL_RTC_GetDate( &hrtc, &gsDateStructure, RTC_FORMAT_BIN );
    // Get the current DST status
    gbDaylightSavingTime = HAL_RTC_DST_ReadStoreOperation( &hrtc ) ? TRUE : FALSE;
    gsTimeStructure.DayLightSaving = ( TRUE == gbDaylightSavingTime ) ? RTC_CR_BKP : 0u;
    // Update display contents
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
