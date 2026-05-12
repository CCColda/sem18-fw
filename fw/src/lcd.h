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


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
#define LCD_WIDTH              (240u)  //!< Width of the screen in pixels
#define LCD_HEIGHT             (135u)  //!< Height of the screen in pixels


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/
//! \brief Some known colors in RGB565 format. Feel free to extend!
typedef enum
{
  WHITE      = 0xFFFFu,
  BLACK      = 0x0000u,
  BLUE       = 0x001Fu,
  RED        = 0xF800u,
  MAGENTA    = 0xF81Fu,
  GREEN      = 0x07E0u,
  CYAN       = 0x7FFFu,
  YELLOW     = 0xFFE0u,
  GRAY       = 0x8430u,
  BRED       = 0xF81Fu,
  GRED       = 0xFFE0u,
  GBLUE      = 0x07FFu,
  BROWN      = 0xBC40u,
  BRRED      = 0xFC07u,
  DARKBLUE   = 0x01CFu,
  LIGHTBLUE  = 0x7D7Cu,
  GRAYBLUE   = 0x5458u,
  LIGHTGREEN = 0x841Fu,
  LGRAY      = 0xC618u,
  LGRAYBLUE  = 0xA651u,
  LBBLUE     = 0x2B12u
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
void LCD_PrintChar( U16 u16X, U16 u16Y, char ch, E_LCD_FONT_TYPE eFont, U16 u16Color, U16 u16BGColor );
void LCD_PrintString( U16 u16X, U16 u16Y, const char* acString, E_LCD_FONT_TYPE eFont, U16 u16Color, U16 u16BGColor );


#endif  // LCD_H

//-----------------------------------------------< EOF >--------------------------------------------------/
