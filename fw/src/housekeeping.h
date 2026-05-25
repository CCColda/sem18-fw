/*! *******************************************************************************************************
* Copyright (c) 2026 K. Sz. Horvath
*
* All rights reserved
*
* \file housekeeping.h
*
* \brief Housekeeping task and routines
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

#ifndef HOUSEKEEPING_H
#define HOUSEKEEPING_H

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/
//! \brief Charging states
typedef enum
{
  CHARGER_STATE_NONE,
  CHARGER_STATE_CHARGING,
  CHARGER_STATE_STANDBY
} E_HOUSEKEEPING_CHARGERSTATE;


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/
extern volatile U32 gu32HousekeepingRTCCalibrationLateppm;
extern volatile U32 gu32HousekeepingRTCCalibrationFastppm;


//--------------------------------------------------------------------------------------------------------/
// Interface functions
//--------------------------------------------------------------------------------------------------------/
// System functions
void Housekeeping_Init( void );
void Housekeeping_Cycle( void );
BOOL Housekeeping_RTCCheckBkup( void );

// API functions
void Housekeeping_RTCCalibrateLate( void );
void Housekeeping_RTCCalibrateFast( void );
void Housekeeping_DeepSleep( void );
float Housekeeping_GetBatteryVoltage( void );
U8 Housekeeping_GetBatteryPercentage( void );
E_HOUSEKEEPING_CHARGERSTATE Housekeeping_GetChargerState( void );
U32 Housekeeping_BCD2U32( U32 u32BCD );
U32 Housekeeping_U322BCD( U32 u32Number );


#endif  // HOUSEKEEPING_H

//-----------------------------------------------< EOF >--------------------------------------------------/
