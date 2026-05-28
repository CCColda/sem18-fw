/*! *******************************************************************************************************
* Copyright (c) 2026 K. Sz. Horvath
*
* All rights reserved
*
* \file lcd.c
*
* \brief Driver for ST7789-based 135x240 pixel IPS LCD
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

#ifndef LCD_H
#define LCD_H

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/
#include "types.h"

//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
#define LCD_WIDTH              (240u)  //!< Width of the screen in pixels
#define LCD_HEIGHT             (135u)  //!< Height of the screen in pixels


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/
//! \brief Some known colors in big-endian(!) RGB565 format. Feel free to extend!
typedef enum
{
  WHITE      = 0xFFFFu,
  BLACK      = 0x0000u,
  BLUE       = 0x1F00u,
  RED        = 0x00F8u,
  MAGENTA    = 0x1FF8u,
  GREEN      = 0xE007u,
  CYAN       = 0xFF7Fu,
  YELLOW     = 0xE0FFu,
  GRAY       = 0x3084u,
  BRED       = 0x1FF8u,
  GRED       = 0xE0FFu,
  GBLUE      = 0xFF07u,
  BROWN      = 0x40BCu,
  BRRED      = 0x07FCu,
  DARKBLUE   = 0xCF01u,
  LIGHTBLUE  = 0x7C7Du,
  GRAYBLUE   = 0x5854u,
  LIGHTGREEN = 0x1F84u,
  LGRAY      = 0x18C6u,
  LGRAYBLUE  = 0x51A6u,
  LBBLUE     = 0x122Bu
} E_LCD_COLORS;

//! \brief Font types
typedef enum
{
  LCD_FONT_7x10   = 0u,
  LCD_FONT_11x18,
  LCD_FONT_16x26
} E_LCD_FONT_TYPE;


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/
extern volatile U16 gau16LCDFrameBuffer[ LCD_WIDTH*LCD_HEIGHT ];


//--------------------------------------------------------------------------------------------------------/
// Interface functions
//--------------------------------------------------------------------------------------------------------/
// System functions
void LCD_Init( void );

// API functions
void LCD_Clear( U16 u16Color );
void LCD_Pixel( U8 u8X, U8 u8Y, U16 u16Color );
void LCD_DrawLine( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, U16 u16Color );
void LCD_DrawFilledRectangle( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, U16 u16Color );
void LCD_DrawFilledTriangle( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, U8 u8X2, U8 u8Y2, U16 u16Color );
void LCD_SwitchColorsInRectangle( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, U16 u16Color1, U16 u16Color2 );
void LCD_PrintChar( U16 u16X, U16 u16Y, char ch, E_LCD_FONT_TYPE eFont, U16 u16Color, U16 u16BGColor );
void LCD_PrintString( U16 u16X, U16 u16Y, const char* acString, E_LCD_FONT_TYPE eFont, U16 u16Color, U16 u16BGColor );


#endif  // LCD_H

//-----------------------------------------------< EOF >--------------------------------------------------/
