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
  
}

/*! *******************************************************************
 * \brief  Task main cycle
 * \param  -
 * \return -
 *********************************************************************/
void Housekeeping_Cycle( void )
{
  
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
BOOL Housekeeping_RTC_Check_Bkup( void )
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

//-----------------------------------------------< EOF >--------------------------------------------------/
