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

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/
#include <string.h>  // memset, memcpy, etc.
#include <stdlib.h>  // abs, etc.
#include "main.h"
#include "spi.h"
#include "dma.h"
#include "types.h"
#include "fonts.h"

// Own include
#include "lcd.h"


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
#define LCD_HANDLE            (hspi2)  //!< HAL handle for SPI
#define LCD_X_SHIFT             (40u)  //!< Horizontal shift of framebuffer data in ST7789 memory
#define LCD_Y_SHIFT             (52u)  //!< Vertical shift of framebuffer data in ST7789 memory 


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/
//! \brief Command bytes of ST7789
typedef enum
{
  CMD_MADCTL_MY  = 0x80u,
  CMD_MADCTL_MX  = 0x40u,
  CMD_MADCTL_MV  = 0x20u,
  CMD_MADCTL_ML  = 0x10u,
  CMD_MADCTL_RGB = 0x00u,
  CMD_MADCTL_BGR = 0x08u,
  CMD_MADCTL_MH  = 0x04u,
  CMD_NOP        = 0x00u,
  CMD_SWRESET    = 0x01u,
  CMD_RDDID      = 0x04u,
  CMD_RDDST      = 0x09u,
  CMD_SLPIN      = 0x10u,
  CMD_SLPOUT     = 0x11u,
  CMD_PTLON      = 0x12u,
  CMD_NORON      = 0x13u,
  CMD_INVOFF     = 0x20u,
  CMD_INVON      = 0x21u,
  CMD_GAMSET     = 0x26u,
  CMD_DISPOFF    = 0x28u,
  CMD_DISPON     = 0x29u,
  CMD_CASET      = 0x2Au,
  CMD_RASET      = 0x2Bu,
  CMD_RAMWR      = 0x2Cu,
  CMD_RAMRD      = 0x2Eu,
  CMD_PTLAR      = 0x30u,
  CMD_MADCTL     = 0x36u,
  CMD_IDMOFF     = 0x38u,
  CMD_IDMON      = 0x39u,
  CMD_COLMOD     = 0x3Au,
  CMD_RAMCTRL    = 0xB0u,
  CMD_FRMCTR1    = 0xB1u,
  CMD_RGBCTRL    = 0xB1u,
  CMD_FRMCTR2    = 0xB2u,
  CMD_PORCTRL    = 0xB2u,
  CMD_FRMCTR3    = 0xB3u,
  CMD_FRCTRL1    = 0xB3u,
  CMD_INVCTR     = 0xB4u,
  CMD_PARCTRL    = 0xB5u,
  CMD_DISSET5    = 0xB6u,
  CMD_GCTRL      = 0xB7u,
  CMD_VCOMS      = 0xBBu,
  CMD_PWCTR1     = 0xC0u,
  CMD_LCMCTRL    = 0xC0u,
  CMD_PWCTR2     = 0xC1u,
  CMD_IDSET      = 0xC1u,
  CMD_PWCTR3     = 0xC2u,
  CMD_VDVVRHEN   = 0xC2u,
  CMD_PWCTR4     = 0xC3u,
  CMD_VRHS       = 0xC3u,
  CMD_PWCTR5     = 0xC4u,
  CMD_VDVS       = 0xC4u,
  CMD_VMCTR1     = 0xC5u,
  CMD_VCMOFSET   = 0xC5u,
  CMD_FRCTRL2    = 0xC6u,
  CMD_PWCTRL1    = 0xD0u,
  CMD_RDID1      = 0xDAu,
  CMD_RDID2      = 0xDBu,
  CMD_RDID3      = 0xDCu,
  CMD_RDID4      = 0xDDu,
  CMD_PWCTR6     = 0xFCu,
  CMD_GMCTRP1    = 0xE0u,
  CMD_GMCTRN1    = 0xE1u,
  CMD_COLOR_MODE_16bit = 0x55u,
  CMD_COLOR_MODE_18bit = 0x66u
} E_LCD_COMMANDS;


//--------------------------------------------------------------------------------------------------------/
// Constants
//--------------------------------------------------------------------------------------------------------/
//! \brief Configuration constants of the display controller
//! \note  Formatted as: <number of parameters>,<command>,<parameters>
static const U8 cau8LCDConfiguration[] = 
{
  0,  CMD_SLPOUT,
  1,  CMD_COLMOD,  CMD_COLOR_MODE_16bit,
  5,  CMD_PORCTRL, 0x0C, 0x0C, 0x00, 0x33, 0x33,   // Standard porch
//5,  CMD_PORCTRL, 0x01, 0x01, 0x00, 0x11, 0x11,   // Minimum porch (7% faster screen refresh rate)
  1,  CMD_GCTRL,   0x35,                           // Gate Control, Default value
  1,  CMD_VCOMS,   0x19,                           // VCOM setting 0.725v (default 0.75v for 0x20)
  1,  CMD_LCMCTRL, 0X2C,                           // LCMCTRL, Default value
  1,  CMD_VDVVRHEN,0x01,                           // VDV and VRH command Enable, Default value
  1,  CMD_VRHS,    0x12,                           // VRH set, +-4.45v (default +-4.1v for 0x0B)
  1,  CMD_VDVS,    0x20,                           // VDV set, Default value
  1,  CMD_FRCTRL2, 0x0F,                           // Frame rate control in normal mode, Default refresh rate (60Hz)
//1,  CMD_FRCTRL2, 0x01,                           // Frame rate control in normal mode, Max refresh rate (111Hz)
  2,  CMD_PWCTRL1, 0xA4, 0xA1,
  1,  CMD_MADCTL,  (CMD_MADCTL_MY | CMD_MADCTL_MV | CMD_MADCTL_RGB),  // Set rotation and color format
  14, CMD_GMCTRP1, 0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23,
  14, CMD_GMCTRN1, 0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23,
  0,  CMD_INVON,
  0,  CMD_NORON,
  4,  CMD_CASET, (LCD_X_SHIFT)>>8u, (LCD_X_SHIFT) & 0xFFu, (LCD_X_SHIFT + LCD_WIDTH - 1u)>>8u, (LCD_X_SHIFT + LCD_WIDTH - 1u) & 0xFFu,     // Column address set
  4,  CMD_RASET, (LCD_Y_SHIFT)>>8u, (LCD_Y_SHIFT) & 0xFFu, (LCD_Y_SHIFT + LCD_HEIGHT - 1u)>>8u, (LCD_Y_SHIFT + LCD_HEIGHT - 1u) & 0xFFu,   // Row address set
};


//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/
//! \brief Framebuffer of the LCD driver
volatile U16 gau16LCDFrameBuffer[ LCD_WIDTH*LCD_HEIGHT ];


//--------------------------------------------------------------------------------------------------------/
// Static function declarations
//--------------------------------------------------------------------------------------------------------/
static void WriteCommand( U8 *pu8Cmd, U8 u8NumParameters );
static void TurnOnOff( BOOL bOn );
static void PlotLineLow( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, U16 u16Color );
static void PlotLineHigh( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, U16 u16Color );


//--------------------------------------------------------------------------------------------------------/
// Static functions
//--------------------------------------------------------------------------------------------------------/
/*! *******************************************************************
 * \brief  Write command to ST7789 controller
 * \param  pu8Cmd: command to write, including parameters
 * \param  u8NumParameters: number of parameters to be sent
 * \return -
 *********************************************************************/
static void WriteCommand( U8 *pu8Cmd, U8 u8NumParameters )
{
  HAL_GPIO_WritePin( LCD_nSS_GPIO_Port, LCD_nSS_Pin, GPIO_PIN_RESET );  // select LCD
  HAL_GPIO_WritePin( LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET );    // we're sending commands
  HAL_SPI_Transmit( &LCD_HANDLE, pu8Cmd, 1u, HAL_MAX_DELAY );
  if( 0u != u8NumParameters )
  {
    HAL_GPIO_WritePin( LCD_nSS_GPIO_Port, LCD_nSS_Pin, GPIO_PIN_SET );    // deselect LCD
    HAL_GPIO_WritePin( LCD_nSS_GPIO_Port, LCD_nSS_Pin, GPIO_PIN_RESET );  // select LCD
    HAL_GPIO_WritePin( LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET );      // we're sending data
    HAL_SPI_Transmit( &LCD_HANDLE, (pu8Cmd+1u), u8NumParameters, HAL_MAX_DELAY );
  }
  HAL_GPIO_WritePin( LCD_nSS_GPIO_Port, LCD_nSS_Pin, GPIO_PIN_SET );  // deselect LCD
}

/*! *******************************************************************
 * \brief  Turn display on/off
 * \param  bOn: TRUE if turn on; FALSE if turn off
 * \return -
 *********************************************************************/
static void TurnOnOff( BOOL bOn )
{
  U8 au8cmd[] = { ((TRUE == bOn) ? CMD_DISPON /* TEON */ : CMD_DISPOFF /* TEOFF */) };
  WriteCommand( au8cmd, sizeof( au8cmd ) - 1u );
}

/*! *******************************************************************
 * \brief  Line drawing algorithm for low gradients
 * \param  u8X0: origin X coordinate
 * \param  u8Y0: origin Y coordinate
 * \param  u8X1: destination X coordinate
 * \param  u8Y1: destination Y coordinate
 * \param  u16Color: color of the line to be drawn
 * \return -
 *********************************************************************/
static void PlotLineLow( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, U16 u16Color )
{
  I8  i8dx, i8dy, i8yi;
  I16 i16D;
  U8  u8X, u8Y;
  
  i8dx = u8X1 - u8X0;
  i8dy = u8Y1 - u8Y0;

  i8yi = 1;
  if( i8dy < 0 )
  {
    i8yi = -1;
    i8dy = -i8dy;
  }
  i16D = ( i8dy * 2 ) - i8dx;
  
  u8Y = u8Y0;
  for( u8X = u8X0; u8X <= u8X1; u8X++ )
  {
    LCD_Pixel( u8X, u8Y, u16Color );
    if( i16D > 0 )
    {
      u8Y += i8yi;
      i16D = i16D - ( i8dx * 2 );
    }
    i16D += ( i8dy * 2 );
  }
}

/*! *******************************************************************
 * \brief  Line drawing algorithm for high gradients
 * \param  u8X0: origin X coordinate
 * \param  u8Y0: origin Y coordinate
 * \param  u8X1: destination X coordinate
 * \param  u8Y1: destination Y coordinate
 * \param  u16Color: color of the line to be drawn
 * \return -
 *********************************************************************/
static void PlotLineHigh( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, U16 u16Color )
{
  I8 i8dx, i8dy, i8xi;
  I16 i16D;
  U8 u8X, u8Y;
  
  i8dx = u8X1 - u8X0;
  i8dy = u8Y1 - u8Y0;

  i8xi = 1;
  if( i8dx < 0 )
  {
    i8xi = -1;
    i8dx = -i8dx;
  }
  i16D = ( i8dx * 2 ) - i8dy;
  
  u8X = u8X0;
  for( u8Y = u8Y0; u8Y <= u8Y1; u8Y++ )
  {
    LCD_Pixel( u8X, u8Y, u16Color );
    if( i16D > 0 )
    {
      u8X += i8xi;
      i16D = i16D - ( i8dy * 2 );
    }
    i16D += ( i8dx * 2 );
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
void LCD_Init( void )
{
  U32 u32Index;
  U8  u8Command;
  
  // Turn on power and backlight
  HAL_GPIO_WritePin( LCD_nPWR_GPIO_Port, LCD_nPWR_Pin, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET );

  // Set IO pins to correct position
  HAL_GPIO_WritePin( LCD_nSS_GPIO_Port, LCD_nSS_Pin, GPIO_PIN_SET );
  HAL_GPIO_WritePin( LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET );
  
  // Wait for initialization
  HAL_Delay( 40u );
  __HAL_SPI_ENABLE( &LCD_HANDLE );
  
  // Software reset
  u8Command = 0x01u;
  WriteCommand( &u8Command, 0u );
  HAL_Delay( 6u );
  
  // Write configuration
  for( u32Index = 0u; u32Index < sizeof( cau8LCDConfiguration ); )
  {
    WriteCommand( (U8*)&cau8LCDConfiguration[ u32Index + 1u ], cau8LCDConfiguration[ u32Index ] );
    u32Index += cau8LCDConfiguration[ u32Index ] + 2u;
  }
  
  // Clear framebuffer (set it to black)
  memset( (void*)gau16LCDFrameBuffer, 0x00, sizeof( gau16LCDFrameBuffer ) );

  // Turn display on
  TurnOnOff( TRUE );
  
  // Start sending framebuffer using DMA
  u8Command = CMD_RAMWR;  // Write to framebuffer RAM command
  WriteCommand( &u8Command, 0u );
  HAL_GPIO_WritePin( LCD_nSS_GPIO_Port, LCD_nSS_Pin, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET );
  HAL_SPI_Transmit_DMA( &LCD_HANDLE, (U8*)gau16LCDFrameBuffer, sizeof( gau16LCDFrameBuffer ) );
}

/*! *******************************************************************
 * \brief  Clears the screen using a specific color
 * \param  u16Color: color word (BGR565 format)
 * \return -
 *********************************************************************/
void LCD_Clear( U16 u16Color )
{
  U32 u32Index;
  u16Color = (u16Color<<8u) | (u16Color>>8u);
  for( u32Index = 0u; u32Index < sizeof( gau16LCDFrameBuffer )/sizeof( U16 ); u32Index++ )
  {
    gau16LCDFrameBuffer[ u32Index ] = u16Color;
  }
}

/*! *******************************************************************
 * \brief  Set a pixel to a specific color
 * \param  u8X: horizontal coordinate
 * \param  u8Y: vertical coordinate
 * \param  u16Color: color word (BGR565 format)
 * \return -
 *********************************************************************/
void LCD_Pixel( U8 u8X, U8 u8Y, U16 u16Color )
{
  u16Color = (u16Color<<8u) | (u16Color>>8u);
  if( ( u8X < LCD_WIDTH ) && ( u8Y < LCD_HEIGHT ) )
  {
    gau16LCDFrameBuffer[ u8Y*LCD_WIDTH + u8X ] = u16Color;
  }
}

/*! *******************************************************************
 * \brief  Draws a line on screen using Bresenham's line algorithm
 * \param  u8X0: origin X coordinate
 * \param  u8Y0: origin Y coordinate
 * \param  u8X1: destination X coordinate
 * \param  u8Y1: destination Y coordinate
 * \param  u16Color: color of the line to be drawn
 * \return -
 *********************************************************************/
void LCD_DrawLine( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, U16 u16Color )
{
  // If the line is just 1 pixel
  if( ( u8X0 == u8X1 ) && ( u8Y0 == u8Y1 ) )
  {
    LCD_Pixel( u8X0, u8Y0, u16Color );
    return;
  }
  
  // Note: this can be optimized!
  if( abs( u8Y1 - u8Y0 ) < abs( u8X1 - u8X0 ) )
  {
    if( u8X0 > u8X1 )
    {
      PlotLineLow( u8X1, u8Y1, u8X0, u8Y0, u16Color );
    }
    else
    {
      PlotLineLow( u8X0, u8Y0, u8X1, u8Y1, u16Color );
    }
  }
  else
  {
    if( u8Y0 > u8Y1 )
    {
      PlotLineHigh( u8X1, u8Y1, u8X0, u8Y0, u16Color );
    }
    else
    {
      PlotLineHigh( u8X0, u8Y0, u8X1, u8Y1, u16Color );
    }
  }
}

/*! *******************************************************************
 * \brief  Draws a filled rectangle on the screen
 * \param  u8X0: origin X coordinate
 * \param  u8Y0: origin Y coordinate
 * \param  u8X1: destination X coordinate
 * \param  u8Y1: destination Y coordinate
 * \param  u16Color: color of the rectange to be drawn
 * \return -
 *********************************************************************/
void LCD_DrawFilledRectangle( U8 u8X0, U8 u8Y0, U8 u8X1, U8 u8Y1, U16 u16Color )
{
  U8 u8X, u8Y;
  
  for( u8Y = u8Y0; u8Y <= u8Y1; u8Y++ )
  {
    for( u8X = u8X0; u8X <= u8X1; u8X++ )
    {
      LCD_Pixel( u8X, u8Y, u16Color );
    }
  }
}

/*! *******************************************************************
 * \brief  Print a character to the screen
 * \param  u16X, u16Y: cursor of the start point
 * \param  cCharacter: character to be printed
 * \param  eFont: font type to be used
 * \param  u16Color: ink color
 * \param  u16BGColor: background color
 * \return -
 *********************************************************************/
void LCD_PrintChar( U16 u16X, U16 u16Y, char cCharacter, E_LCD_FONT_TYPE eFont, U16 u16Color, U16 u16BGColor )
{
  U8 u8Width = Font_GetWidth( eFont );
  U8 u8Height = Font_GetHeight( eFont );
  U16 const* pu16Font = Font_GetFont( eFont );
  U32 u32LineIndex, b, u32WidthIndex;

  u16Color = (u16Color<<8u) | (u16Color>>8u);
  u16BGColor = (u16BGColor<<8u) | (u16BGColor>>8u);
  
  //ST7789_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);
  
  for( u32LineIndex = 0; u32LineIndex < u8Height; u32LineIndex++ )
  {
    b = pu16Font[ (cCharacter - 32u) * u8Height + u32LineIndex ];
    for( u32WidthIndex = 0; u32WidthIndex < u8Width; u32WidthIndex++ )
    {
      if ((b << u32WidthIndex) & 0x8000)
      {
        //uint8_t data[] = {color >> 8, color & 0xFF};
        //ST7789_WriteData(data, sizeof(data));
        gau16LCDFrameBuffer[ (u16Y+u32LineIndex)*LCD_WIDTH + u16X + u32WidthIndex ] = u16Color;
      }
      else
      {
        //uint8_t data[] = {bgcolor >> 8, bgcolor & 0xFF};
        //ST7789_WriteData(data, sizeof(data));
        gau16LCDFrameBuffer[ (u16Y+u32LineIndex)*LCD_WIDTH + u16X + u32WidthIndex ] = u16BGColor;
     }
    }
  }
}

/*! *******************************************************************
 * \brief  Print a string to the screen
 * \param  u16X, u16Y: cursor of the start point
 * \param  acString: string to be printed
 * \param  eFont: font type to be used
 * \param  u16Color: ink color
 * \param  u16BGColor: background color
 * \return -
 *********************************************************************/
void LCD_PrintString( U16 u16X, U16 u16Y, const char* acString, E_LCD_FONT_TYPE eFont, U16 u16Color, U16 u16BGColor )
{
  U8 u8Width = Font_GetWidth( eFont );
  U8 u8Height = Font_GetHeight( eFont );

  while( *acString )
  {
    if( u16X + u8Width >= LCD_WIDTH )
    {
      u16X = 0u;
      u16Y += u8Height;
      if( u16Y + u8Height >= LCD_HEIGHT)
      {
        break;
      }
      
      if( ' ' == *acString )
      {
        // skip spaces in the beginning of the new line
        acString++;
        continue;
      }
    }
    LCD_PrintChar( u16X, u16Y, *acString, eFont, u16Color, u16BGColor );
    u16X += u8Width;
    acString++;
  }
}

/*! *******************************************************************
 * \brief
 * \param
 * \return
 *********************************************************************/


//-----------------------------------------------< EOF >--------------------------------------------------/
