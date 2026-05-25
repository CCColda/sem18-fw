/*! *******************************************************************************************************
* Copyright (c) 2026 K. Sz. Horvath
*
* All rights reserved
*
* \file housekeeping.c
*
* \brief Housekeeping task and routines
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/
#include "main.h"
#include "quadspi.h"
#include "adc.h"
#include "rtc.h"
#include "types.h"

// Own include
#include "housekeeping.h"


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
#define RTC_BACKUP_MAGICNUMBER  (0xA5C0FFEEu)  //!< Magic number for signaling an initialized RTC


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/
//! \brief Voltage of the battery in Volts
static float gfBatteryVoltage;

//! \brief RTC calibration (how late it is) in ppm represented in BCD 7.1 format
volatile U32 gu32HousekeepingRTCCalibrationLateppm;

//! \brief RTC calibration (how fast it is) in ppm represented in BCD 7.1 format
volatile U32 gu32HousekeepingRTCCalibrationFastppm;


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
void Housekeeping_Init( void )
{
  U32 u32CalibrationReg;
  // Initialize global variables
  gfBatteryVoltage = 4.2f;
  gu32HousekeepingRTCCalibrationLateppm = 0u;
  gu32HousekeepingRTCCalibrationFastppm = 0u;

  HAL_PWR_EnableBkUpAccess();
  u32CalibrationReg = READ_REG( RTC->CALR );
  HAL_PWR_DisableBkUpAccess();

  if( RTC_CALR_CALP & u32CalibrationReg )
  {
    // Calculate value as 0.1ppm
    gu32HousekeepingRTCCalibrationLateppm = ((512u - (u32CalibrationReg & 0x1FFu) )*312500u)/32768u;
    gu32HousekeepingRTCCalibrationLateppm = Housekeeping_U322BCD( gu32HousekeepingRTCCalibrationLateppm );
  }
  else
  {
    // Calculate value as 0.1ppm
    gu32HousekeepingRTCCalibrationFastppm = ((u32CalibrationReg & 0x1FFu)*312500u)/32768u;
    gu32HousekeepingRTCCalibrationFastppm = Housekeeping_U322BCD( gu32HousekeepingRTCCalibrationFastppm );
  }
}

/*! *******************************************************************
 * \brief  Task main cycle
 * \param  -
 * \return -
 *********************************************************************/
void Housekeeping_Cycle( void )
{
  static uint32_t u32Timer = 0u;
  U16 au16ADCmeasurement[ 2u ];

  // Measure battery/input voltage every 100 ms
  if( HAL_GetTick() - u32Timer > 100u )
  {
    u32Timer = HAL_GetTick();
    // Enable voltage divider
    HAL_GPIO_WritePin( VMEAS_GND_GPIO_Port, VMEAS_GND_Pin, GPIO_PIN_RESET );
    HAL_Delay(1u);
    //TODO: it would be better without waiting...
    HAL_ADC_Start( &hadc1 );
    HAL_ADC_PollForConversion( &hadc1, 100 );
    au16ADCmeasurement[ 0u ] = HAL_ADC_GetValue( &hadc1 );
    HAL_ADC_PollForConversion( &hadc1, 100 );
    au16ADCmeasurement[ 1u ] = HAL_ADC_GetValue( &hadc1 );
    HAL_ADC_Stop( &hadc1 );
    //TODO: disconnect ADC from pin
    // Disable voltage divider
    HAL_GPIO_WritePin( VMEAS_GND_GPIO_Port, VMEAS_GND_Pin, GPIO_PIN_SET );
    // Calculate voltage
    gfBatteryVoltage = ((float)au16ADCmeasurement[ 0u ])*(2.0f*((float)__HAL_ADC_CALC_VREFANALOG_VOLTAGE(au16ADCmeasurement[ 1u ],ADC_RESOLUTION_12B)/1000.0f)/4095.0f);
  }
}

/*! *******************************************************************
 * \brief  Go to deep sleep (point of no return)
 * \param  -
 * \return -
 *********************************************************************/
void Housekeeping_DeepSleep( void )
{
  // Wait for deep sleep button to be released
  while( GPIO_PIN_RESET == HAL_GPIO_ReadPin( BUTTON_SW3_GPIO_Port, BUTTON_SW3_Pin ) );
  
  // Wait for debounce
  HAL_Delay( 200u );
  
  // Disable LCD & backlight
  HAL_GPIO_WritePin( LCD_nPWR_GPIO_Port, LCD_nPWR_Pin, GPIO_PIN_SET );
  HAL_GPIO_WritePin( LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET );
  
  // Send QSPI flash to deep power-down
  QSPI_CommandTypeDef sCommand = { 0 };
  sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction = 0xB9u;  // Deep power down command
  sCommand.AddressMode = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode = QSPI_DATA_NONE;
  sCommand.DummyCycles = 0;
  sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
  if( (HAL_QSPI_Command(&hqspi, &sCommand,HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK )
  {
    //TODO: error handling
    HAL_Delay( 1000u );
  }
  
  // Enable wakeup on SW3 (== SYS_WAKEUP_PIN5, PC5)
  HAL_PWR_DisableWakeUpPin( PWR_CR3_EWUP );
  __HAL_PWR_CLEAR_FLAG( PWR_FLAG_WUF5 );
  HAL_PWREx_EnablePullUpPullDownConfig();
  HAL_PWREx_EnableGPIOPullUp( PWR_GPIO_C, PWR_GPIO_BIT_5 );
  HAL_PWR_EnableWakeUpPin( PWR_WAKEUP_PIN5_LOW );
  HAL_Delay( 50 );  // The pull-up resistor needs some time to activate
  
  // Disable SysTick timer
  HAL_SuspendTick();
  
  // Refresh magic number before deep sleep
  HAL_PWR_EnableBkUpAccess();
  HAL_RTCEx_BKUPWrite( &hrtc, RTC_BKP_DR31, RTC_BACKUP_MAGICNUMBER );
  HAL_PWR_DisableBkUpAccess();
  
  // Enter deep sleep
  HAL_PWREx_EnableLowPowerRunMode();
  // Clear WUFx flags (Wakeup Flags) in PWR Status Register
  __HAL_PWR_CLEAR_FLAG( PWR_FLAG_WUF1 | PWR_FLAG_WUF2 | PWR_FLAG_WUF3 | PWR_FLAG_WUF4 | PWR_FLAG_WUF5 );
  // Enter Shutdown Mode --> point of no return
  HAL_PWREx_EnterSHUTDOWNMode();
  
  // If for some reason we get here...
  HAL_NVIC_SystemReset();  
}

/*! *******************************************************************
 * \brief  Check for RTC and backup state 
 * \param  -
 * \return TRUE if RTC was initialized earlier
 *********************************************************************/
BOOL Housekeeping_RTCCheckBkup( void )
{
  BOOL bRet = FALSE;
  
  // Enable access to the backup registers
  HAL_PWR_EnableBkUpAccess();
  // If the RTC has been initialized before
  if( RTC_BACKUP_MAGICNUMBER == HAL_RTCEx_BKUPRead( &hrtc, RTC_BKP_DR31 ) )
  {
     bRet = TRUE;  // don't reset the RTC
  }
  else  // Not initialized yet
  {
    HAL_RTCEx_BKUPWrite( &hrtc, RTC_BKP_DR31, RTC_BACKUP_MAGICNUMBER );
  }
  return bRet;
}

/*! *******************************************************************
 * \brief  Returns the voltage of the battery
 * \param  -
 * \return Battery voltage in Volts.
 *********************************************************************/
float Housekeeping_GetBatteryVoltage( void )
{
  float fReturn = 4.2f;

  if( ( GPIO_PIN_SET == HAL_GPIO_ReadPin( CHARGER_nCHARGE_GPIO_Port, CHARGER_nCHARGE_Pin ) )
   && ( GPIO_PIN_SET == HAL_GPIO_ReadPin( CHARGER_STANDBY_GPIO_Port, CHARGER_STANDBY_Pin ) ) )
  {
    // Return the measured voltage, as it is not connected to the charger
    fReturn = gfBatteryVoltage;
  }
  return fReturn;
}

/*! *******************************************************************
 * \brief  Returns the charge of the battery in percentage
 * \param  -
 * \return Battery charge in percentage [0..100]
 *********************************************************************/
U8 Housekeeping_GetBatteryPercentage( void )
{
  U8 u8Return = 100u;

  if( ( GPIO_PIN_SET == HAL_GPIO_ReadPin( CHARGER_nCHARGE_GPIO_Port, CHARGER_nCHARGE_Pin ) )
   && ( GPIO_PIN_SET == HAL_GPIO_ReadPin( CHARGER_STANDBY_GPIO_Port, CHARGER_STANDBY_Pin ) ) )
  {
    //TODO: better algorithm for calculating the charge level
    if( gfBatteryVoltage >= 3.0f )
    {
      u8Return = (U8)(100.0f*(gfBatteryVoltage-3.0f)/1.2f);
    }
    else  // undervoltage
    {
      u8Return = 0u;
    }
  }
  return u8Return;
}

/*! *******************************************************************
 * \brief  Returns the current status of charging
 * \param  -
 * \return Charger status
 *********************************************************************/
E_HOUSEKEEPING_CHARGERSTATE Housekeeping_GetChargerState( void )
{
  E_HOUSEKEEPING_CHARGERSTATE eReturn = CHARGER_STATE_NONE;
  GPIO_PinState enCharge = HAL_GPIO_ReadPin( CHARGER_nCHARGE_GPIO_Port, CHARGER_nCHARGE_Pin );
  GPIO_PinState enStandby = HAL_GPIO_ReadPin( CHARGER_STANDBY_GPIO_Port, CHARGER_STANDBY_Pin );

  if( ( GPIO_PIN_RESET == enCharge )
   && ( GPIO_PIN_SET == enStandby ) )
  {
    eReturn = CHARGER_STATE_CHARGING;
  }
  else if( ( GPIO_PIN_SET == enCharge )
   && ( GPIO_PIN_RESET == enStandby ) )
  {
    eReturn = CHARGER_STATE_STANDBY;
  }
  else if( ( GPIO_PIN_RESET == enCharge )
   && ( GPIO_PIN_RESET == enStandby ) )
  {
    //TODO: this should not happen, should be fatal error
    eReturn = CHARGER_STATE_STANDBY;
  }

  return eReturn;
}

/*! *******************************************************************
 * \brief  Calibrate the clock assuming it's late
 * \param  -
 * \return -
 *********************************************************************/
void Housekeeping_RTCCalibrateLate( void )
{
  U32 u32Errorppm = Housekeeping_BCD2U32( gu32HousekeepingRTCCalibrationLateppm );  // in 0.1 ppm units
  U32 u32ErrorPulses;

  // If the error is zero, then do nothing
  if( 0u == u32Errorppm ) return;

  // Calculate error in clock cycles per 32 seconds (=2^20 cycles)
  u32ErrorPulses = (u32Errorppm*32768u + (312500u/2u))/312500u;
  u32ErrorPulses = 512u - u32ErrorPulses;
  HAL_RTCEx_SetSmoothCalib( &hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, RTC_SMOOTHCALIB_PLUSPULSES_SET, u32ErrorPulses );
}

/*! *******************************************************************
 * \brief  Calibrate the clock assuming it's fast
 * \param  -
 * \return -
 *********************************************************************/
void Housekeeping_RTCCalibrateFast( void )
{
  U32 u32Errorppm = Housekeeping_BCD2U32( gu32HousekeepingRTCCalibrationFastppm );  // in 0.1 ppm units
  U32 u32ErrorPulses;

  // If the error is zero, then do nothing
  if( 0u == u32Errorppm ) return;

  // Calculate error in clock cycles per 32 seconds (=2^20 cycles)
  u32ErrorPulses = (u32Errorppm*32768u + (312500u/2u))/312500u;
  HAL_RTCEx_SetSmoothCalib( &hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, RTC_SMOOTHCALIB_PLUSPULSES_RESET, u32ErrorPulses );
}

/*! *******************************************************************
 * \brief  Convert BCD number to U32
 * \param  u32BCD: BCD number
 * \return Number in base-2
 *********************************************************************/
U32 Housekeeping_BCD2U32( U32 u32BCD )
{
  U32 u32Index;
  U32 u32Result = 0u;
  U32 u32Multiplier = 1u;
  U8  u8Nibble;

  // Going through the nibbles
  for( u32Index = 0u; u32Index < 8u; u32Index++ )
  {
    u8Nibble = ( u32BCD & (0x0Fu<<(u32Index*4u)) )>>(u32Index*4u);
    if( u8Nibble > 10u )
    {
      //TODO: error handling
      //ErrorHandler_Fatal( __FILE__, __LINE__, u32BCD, 0u, 0u );
      return 0u;
    }
    else
    {
      u32Result += u32Multiplier*u8Nibble;
    }
    u32Multiplier *= 10u;
  }

  return u32Result;
}

/*! *******************************************************************
 * \brief  Convert U32 number to BCD
 * \param  u32Number: number in base-2
 * \return Number in BCD format
 *********************************************************************/
U32 Housekeeping_U322BCD( U32 u32Number )
{
  U32 u32BCD = 0u;
  U32 u32Index;

  // Going through the nibbles
  for( u32Index = 0u; u32Index < 8u; u32Index++ )
  {
    u32BCD += ( ( u32Number % 10u ) << ( u32Index * 4u) );
    u32Number /= 10u;
  }

  return u32BCD;
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
