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
  // Initialize global variables
  gfBatteryVoltage = 4.2f;
}

/*! *******************************************************************
 * \brief  Task main cycle
 * \param  -
 * \return -
 *********************************************************************/
void Housekeeping_Cycle( void )
{
  static uint32_t u32Timer = 0u;
  U16 u16ADCmeasurement;

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
    u16ADCmeasurement = HAL_ADC_GetValue( &hadc1 );
    HAL_ADC_Stop( &hadc1 );
    //TODO: disconnect ADC from pin
    // Disable voltage divider
    HAL_GPIO_WritePin( VMEAS_GND_GPIO_Port, VMEAS_GND_Pin, GPIO_PIN_SET );
    // Calculate voltage
    gfBatteryVoltage = ((float)u16ADCmeasurement)*(2.0f*3.0f/4095.0f);
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
  WRITE_REG( RTC->BKP31R, RTC_BACKUP_MAGICNUMBER );
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
  if( RTC_BACKUP_MAGICNUMBER == READ_REG(RTC->BKP31R) )
  {
     bRet = TRUE;  // don't reset the RTC
  }
  else  // Not initialized yet
  {
    WRITE_REG( RTC->BKP31R, RTC_BACKUP_MAGICNUMBER );
  }
  // Disable access to avoid spurious writes
  HAL_PWR_DisableBkUpAccess();
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
    // Return the measured voltage, as it is not connected to the charge
    fReturn = gfBatteryVoltage;
  }
  return fReturn;
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
