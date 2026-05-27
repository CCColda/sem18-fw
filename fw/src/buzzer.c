/*! *******************************************************************************************************
* Copyright (c) 2025-2026 K. Sz. Horvath
*
* All rights reserved
*
* \file buzzer.c
*
* \brief Buzzer driver using Timer in PWM mode
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/
#include "main.h"
#include "tim.h"
#include "types.h"

// Own include
#include "buzzer.h"


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
#define HAL_TIMER_PERIPH   (htim4)                  //!< Timer peripherial used by this driver
#define TIMER_CHANNEL      (TIM_CHANNEL_2)          //!< Timer channel used by this driver
#define PWM_DUTY           (htim4.Instance->CCR2)   //!< PWM duty cycle register
#define CLOCK_FREQ_HZ      (80000000)               //!< Clock frequency of the timer in Hz


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/
//! \brief Timer for time-limited beeps
static struct
{
  U32                u32TimerMs;       //!< Timer ms counter
  BOOL               bTimerIsRunning;  //!< Is the timer valid
  E_BUZZER_ALARMMODE eAlarmMode;       //!< Which alarm is played right now
} gsBuzzerTimer;


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
 * \brief  Initialize module
 * \param  -
 * \return -
 *********************************************************************/
void Buzzer_Init( void )
{
  // Initialize timer peripherial
  HAL_TIMER_PERIPH.Init.Period = (U32)( CLOCK_FREQ_HZ/440u ) - 1u;
  HAL_TIM_PWM_Init( &HAL_TIMER_PERIPH );
  PWM_DUTY = 0u;
  HAL_TIM_PWM_Start( &HAL_TIMER_PERIPH, TIMER_CHANNEL );
  // Init software timer
  gsBuzzerTimer.bTimerIsRunning = FALSE;
  gsBuzzerTimer.eAlarmMode = BUZZER_ALARMMODE_NONE;
}

/*! *******************************************************************
 * \brief  Module main cycle
 * \param  -
 * \return -
 *********************************************************************/
void Buzzer_Cycle( void )
{
  U32 u32CurrentTimeMs = HAL_GetTick();
  
  switch( gsBuzzerTimer.eAlarmMode )
  {
    case BUZZER_ALARMMODE_NONE:  // Just a short beep
      // If the timer expired...
      if( ( TRUE == gsBuzzerTimer.bTimerIsRunning )
         && ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) < 0 ) )
      {
        Buzzer_Silence();
        gsBuzzerTimer.bTimerIsRunning = FALSE;
      }
      break;
      
    case BUZZER_ALARMMODE_TWOBEEPS:  // Two short beeps
      // If the timer is running...
      if( TRUE == gsBuzzerTimer.bTimerIsRunning )
      {
        if( ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) < 0 )
           && ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) >= -100 ) )
        {
          Buzzer_Note( 2700u, 128u );
        }
        else if( ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) < -100 )
           && ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) >= -300 ) )
        {
          Buzzer_Silence();
        }
        else if( ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) < -300 )
           && ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) >= -400 ) )
        {
          Buzzer_Note( 2700u, 128u );
        }
        else if( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) < -400 )
        {
          Buzzer_Silence();
          gsBuzzerTimer.bTimerIsRunning = FALSE;
        }
      }
      else
      {
        // End of alarm
        Buzzer_Silence();
        gsBuzzerTimer.eAlarmMode = BUZZER_ALARMMODE_NONE;
      }
      break;
      
    case BUZZER_ALARMMODE_NORMAL:  // Normal alarm
      // If the timer is running...
      if( TRUE == gsBuzzerTimer.bTimerIsRunning )
      {
        if( ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) < 0 )
           && ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) >= -50 ) )
        {
          Buzzer_Note( 2700u, 128u );
        }
        else if( ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) < -50 )
           && ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) >= -100 ) )
        {
          Buzzer_Silence();
        }
        else if( ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) < -100 )
           && ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) >= -150 ) )
        {
          Buzzer_Note( 2700u, 128u );
        }
        else if( ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) < -150 )
           && ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) >= -200 ) )
        {
          Buzzer_Silence();
        }
        else if( ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) < -200 )
           && ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) >= -250 ) )
        {
          Buzzer_Note( 2700u, 128u );
        }
        else if( ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) < -250 )
           && ( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) >= -300 ) )
        {
          Buzzer_Silence();
        }
        else if( (I32)(gsBuzzerTimer.u32TimerMs - u32CurrentTimeMs) < -999 )
        {
          // Never stop
          gsBuzzerTimer.u32TimerMs = u32CurrentTimeMs;
        }
      }
      else
      {
        // End of alarm
        Buzzer_Silence();
        gsBuzzerTimer.eAlarmMode = BUZZER_ALARMMODE_NONE;
      }
      break;
      
    default:  // This should not happen
      //TODO: error handling
      Buzzer_Silence();
      gsBuzzerTimer.eAlarmMode = BUZZER_ALARMMODE_NONE;
      gsBuzzerTimer.bTimerIsRunning = FALSE;
      break;
  }
}

/*! *******************************************************************
 * \brief  Start buzzing
 * \param  u32FrequencyHz: frequency in Hz (min: 1)
 * \param  u8DutyCycle: duty cycle (0..255)
 * \return -
 *********************************************************************/
void Buzzer_Note( U32 u32FrequencyHz, U8 u8DutyCycle )
{
  U32 u32NewPeriod;
  
  // Minimum value: 1 Hz
  if( u32FrequencyHz > 0u )
  {
    // Set frequency
    u32NewPeriod = ( (CLOCK_FREQ_HZ/1000u)/u32FrequencyHz ) - 1u;
    if( u32NewPeriod != HAL_TIMER_PERIPH.Init.Period )  // Set only if it's different from previous value
    {
      HAL_TIMER_PERIPH.Init.Prescaler = 1000u - 1u;
      HAL_TIMER_PERIPH.Init.Period = u32NewPeriod;
      HAL_TIM_PWM_Init( &HAL_TIMER_PERIPH );
    }
    // Set duty cycle
    PWM_DUTY = ( (uint32_t)(HAL_TIMER_PERIPH.Init.Period + 1u) * u8DutyCycle )/255u;
  }
}

/*! *******************************************************************
 * \brief  Stops buzzing
 * \param  -
 * \return -
 *********************************************************************/
void Buzzer_Silence( void )
{
  PWM_DUTY = 0u;
}

/*! *******************************************************************
 * \brief  Beep for a given time
 * \param  u32FrequencyHz: frequency in Hz (min: 1)
 * \param  u8DutyCycle: duty cycle (0..255)
 * \param  u32DurationMs: beep duration in ms
 * \return -
 *********************************************************************/
void Buzzer_Beep( U32 u32FrequencyHz, U8 u8DutyCycle, U32 u32DurationMs )
{
  Buzzer_Note( u32FrequencyHz, u8DutyCycle );
  gsBuzzerTimer.u32TimerMs = HAL_GetTick() + u32DurationMs;
  gsBuzzerTimer.bTimerIsRunning = TRUE;
  gsBuzzerTimer.eAlarmMode = BUZZER_ALARMMODE_NONE;
}

/*! *******************************************************************
 * \brief  Sounds the alarm
 * \param  eAlarmMode: which chime should be played
 * \return -
 *********************************************************************/
void Buzzer_Alarm( E_BUZZER_ALARMMODE eAlarmMode )
{
  gsBuzzerTimer.eAlarmMode = eAlarmMode;
  gsBuzzerTimer.bTimerIsRunning = TRUE;
  gsBuzzerTimer.u32TimerMs = HAL_GetTick();
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
