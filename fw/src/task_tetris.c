/*! *******************************************************************************************************
* Copyright (c) 2023-2026 K. Sz. Horvath
*
* All rights reserved
*
* \file task_tetris.c
*
* \brief Tetris game
*
* \author K. Sz. Horvath
*
**********************************************************************************************************/

/**********************************************************************************************************
Some notes about the implementation:
-- The tetris board is 10 blocks wide and 20 blocks tall
-- Each block is 2 pixels wide and 2 pixels tall
**********************************************************************************************************/

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"  //TODO: replace this later with an own housekeeping library
#include "types.h"
#include "lcd.h"
#include "buttons.h"
#include "buzzer.h"
#include "tasks.h"
//#include "tracker.h"


//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
#define PLAYFIELD_OFFSET_X    (1u)  //!< Bottom left X coordinate of the playfield
#define PLAYFIELD_OFFSET_Y    (1u)  //!< Bottom left Y coordinate of the playfield
#define PLAYFIELD_SIZE_X     (10u)  //!< Playfield horizontal size in blocks
#define PLAYFIELD_SIZE_Y     (20u)  //!< Playfield vertical size in blocks
#define DEFAULT_SPEED_MS    (500u)  //!< Default delay between two events/moves
#define TETROID_SIZE_X        (4u)  //!< Maximum horizontal size of the tetroid in blocks
#define TETROID_SIZE_Y        (4u)  //!< Maximum vertical size of the tetroid in blocks
#define BLOCK_SIZE           (10u)  //!< Size of the block in the playfield
#define FONT      (LCD_FONT_11x18)  //!< Font type used for printing score


//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/


//--------------------------------------------------------------------------------------------------------/
// Global/static variables
//--------------------------------------------------------------------------------------------------------/
static BOOL gbRunning;                                          //!< TRUE if the game is running, false otherwise
static BOOL gbGameOver;                                         //!< TRUE if the game has finished, false otherwise
static BOOL gabBlocks[ PLAYFIELD_SIZE_X ][ PLAYFIELD_SIZE_Y ];  //!< Blocks in the playfield
static U32  gu32TimerMS;                                        //!< Timer that counts the time between events
static BOOL gabTetroid[ TETROID_SIZE_X ][ TETROID_SIZE_Y ];     //!< The current tetroid
static I8   gi8TetroidX;                                        //!< Horizontal coordinate of the bottom left corner of the tetroid
static I8   gi8TetroidY;                                        //!< Vertical coordinate of the bottom left corner of the tetroid
static U32  gu32Score;                                          //!< Game score
volatile U16 gau16ADCMeasurements[ 3 ];                         //!< Potentiometer and more


//--------------------------------------------------------------------------------------------------------/
// Static function declarations
//--------------------------------------------------------------------------------------------------------/
static void DrawBlock( U32 u32X, U32 u32Y, U16 u16Color );
static void FixTetroid( void );
static void RollNewTetroid( void );
static BOOL CheckPlayfieldHit( void );
static void RotateTetroid( BOOL bClockWise );
static void DrawTetroid( I8 i8X, I8 i8Y, U16 u16Color );


//--------------------------------------------------------------------------------------------------------/
// Static functions
//--------------------------------------------------------------------------------------------------------/
/*! *******************************************************************
 * \brief  Draws a block on the screen
 * \param  u32X: horizontal coordinate of the block
 * \param  u32Y: vertical coordinate of the block
 * \param  u16Color: ink color
 * \return -
 * \note   The coordinate (0,0) is in the top left corner of the screen
 *********************************************************************/
static void DrawBlock( U32 u32X, U32 u32Y, U16 u16Color )
{
  // We place the playfield at the bottom left corner of the screen, so it needs to have offset
  // "Box" part
  LCD_DrawLine( PLAYFIELD_OFFSET_X + u32Y*BLOCK_SIZE, (PLAYFIELD_OFFSET_Y + u32X*BLOCK_SIZE), PLAYFIELD_OFFSET_X + u32Y*BLOCK_SIZE + BLOCK_SIZE - 1, (PLAYFIELD_OFFSET_Y + u32X*BLOCK_SIZE), u16Color );
  LCD_DrawLine( PLAYFIELD_OFFSET_X + u32Y*BLOCK_SIZE, (PLAYFIELD_OFFSET_Y + u32X*BLOCK_SIZE), PLAYFIELD_OFFSET_X + u32Y*BLOCK_SIZE, (PLAYFIELD_OFFSET_Y + u32X*BLOCK_SIZE + BLOCK_SIZE - 1), u16Color );
  LCD_DrawLine( PLAYFIELD_OFFSET_X + u32Y*BLOCK_SIZE + BLOCK_SIZE - 1, (PLAYFIELD_OFFSET_Y + u32X*BLOCK_SIZE), PLAYFIELD_OFFSET_X + u32Y*BLOCK_SIZE + BLOCK_SIZE - 1, (PLAYFIELD_OFFSET_Y + u32X*BLOCK_SIZE + BLOCK_SIZE - 1), u16Color );
  LCD_DrawLine( PLAYFIELD_OFFSET_X + u32Y*BLOCK_SIZE, (PLAYFIELD_OFFSET_Y + u32X*BLOCK_SIZE + BLOCK_SIZE - 1), PLAYFIELD_OFFSET_X + u32Y*BLOCK_SIZE + BLOCK_SIZE - 1, (PLAYFIELD_OFFSET_Y + u32X*BLOCK_SIZE + BLOCK_SIZE - 1), u16Color );
  // Diagonals
  LCD_DrawLine( PLAYFIELD_OFFSET_X + u32Y*BLOCK_SIZE, (PLAYFIELD_OFFSET_Y + u32X*BLOCK_SIZE), PLAYFIELD_OFFSET_X + u32Y*BLOCK_SIZE + BLOCK_SIZE - 1, (PLAYFIELD_OFFSET_Y + u32X*BLOCK_SIZE + BLOCK_SIZE - 1), u16Color );
  LCD_DrawLine( PLAYFIELD_OFFSET_X + u32Y*BLOCK_SIZE + BLOCK_SIZE - 1, (PLAYFIELD_OFFSET_Y + u32X*BLOCK_SIZE), PLAYFIELD_OFFSET_X + u32Y*BLOCK_SIZE, (PLAYFIELD_OFFSET_Y + u32X*BLOCK_SIZE + BLOCK_SIZE - 1), u16Color );
}

/*! *******************************************************************
 * \brief  Makes the tetroid part of the playfield
 * \param  -
 * \return -
 *********************************************************************/
static void FixTetroid( void )
{
  U8   u8IndexX, u8IndexY, u8Iter;
  U32  u32ScoreIncrease = 1;
  BOOL bFullLine;
  
  Buzzer_Beep( 2700u, 127u, 100u );  //FIXME: proper chime instead of just one note
  for( u8IndexX = 0; u8IndexX < PLAYFIELD_SIZE_X; u8IndexX++ )
  {
    for( u8IndexY = 0; u8IndexY < PLAYFIELD_SIZE_Y; u8IndexY++ )
    {
      // If the coordinate is inside the tetroid boundaries
      if( ( u8IndexX >= gi8TetroidX ) && ( u8IndexX < ( gi8TetroidX + TETROID_SIZE_X ) )
         && ( u8IndexY >= gi8TetroidY ) && ( u8IndexY < ( gi8TetroidY + TETROID_SIZE_Y ) ) )
      {
        if( TRUE == gabTetroid[ u8IndexX - gi8TetroidX ][ u8IndexY - gi8TetroidY ] )
        {
          gabBlocks[ u8IndexX ][ u8IndexY ] = TRUE;
        }
      }
    }
  }
  // Check if this completed a line or not
  u8IndexY = 0;
  while( u8IndexY < PLAYFIELD_SIZE_Y )
  {
    // Check if the current line is full
    bFullLine = TRUE;
    for( u8IndexX = 0; u8IndexX < PLAYFIELD_SIZE_X; u8IndexX++ )
    {
      if( FALSE == gabBlocks[ u8IndexX ][ u8IndexY ] )
      {
        bFullLine = FALSE;
      }
    }
    // If the line is full...
    if( TRUE == bFullLine )
    {
      // ...then move everything 1 line down
      for( u8Iter = u8IndexY; u8Iter < (PLAYFIELD_SIZE_Y - 1); u8Iter++ )
      {
        for( u8IndexX = 0; u8IndexX < PLAYFIELD_SIZE_X; u8IndexX++ )
        {
          gabBlocks[ u8IndexX ][ u8Iter ] = gabBlocks[ u8IndexX ][ u8Iter + 1 ];
        }
      }
      u32ScoreIncrease *= 10;
      Buzzer_Beep( 3100u, 127u, 100u );  //FIXME: proper chime instead of just one note
    }
    else  // if it's not full, then we have nothing to do
    {
      u8IndexY++;
    }
  }
  // Increase score
  gu32Score += u32ScoreIncrease;
  // Next tetroid
  RollNewTetroid();
  gi8TetroidX = (PLAYFIELD_SIZE_X - TETROID_SIZE_X)/2;
  gi8TetroidY = PLAYFIELD_SIZE_Y - 1;
}

/*! *******************************************************************
 * \brief  Generates a new tetroid using the rand() standard function
 * \param  -
 * \return -
 *********************************************************************/
static void RollNewTetroid( void )
{
  U8 u8TypeIndex = rand() % 7;

  memset( gabTetroid, FALSE, sizeof( gabTetroid ) );  
  switch( u8TypeIndex )
  {
    case 0:
      // +----+
      // | #  |
      // | #  |
      // | #  |
      // | #  |
      // +----+
      gabTetroid[ 1 ][ 3 ] = TRUE;
      gabTetroid[ 1 ][ 2 ] = TRUE;
      gabTetroid[ 1 ][ 1 ] = TRUE;
      gabTetroid[ 1 ][ 0 ] = TRUE;
      break;

    case 1:
      // +----+
      // |  # |
      // |  # |
      // | ## |
      // |    |
      // +----+
      gabTetroid[ 2 ][ 3 ] = TRUE;
      gabTetroid[ 2 ][ 2 ] = TRUE;
      gabTetroid[ 2 ][ 1 ] = TRUE;
      gabTetroid[ 1 ][ 1 ] = TRUE;
      break;

    case 2:
      // +----+
      // | #  |
      // | #  |
      // | ## |
      // |    |
      // +----+
      gabTetroid[ 1 ][ 3 ] = TRUE;
      gabTetroid[ 1 ][ 2 ] = TRUE;
      gabTetroid[ 1 ][ 1 ] = TRUE;
      gabTetroid[ 2 ][ 1 ] = TRUE;
      break;

    case 3:
      // +----+
      // |    |
      // | ## |
      // | ## |
      // |    |
      // +----+
      gabTetroid[ 1 ][ 2 ] = TRUE;
      gabTetroid[ 2 ][ 2 ] = TRUE;
      gabTetroid[ 1 ][ 1 ] = TRUE;
      gabTetroid[ 2 ][ 1 ] = TRUE;
      break;

    case 4:
      // +----+
      // |    |
      // | ## |
      // |##  |
      // |    |
      // +----+
      gabTetroid[ 1 ][ 2 ] = TRUE;
      gabTetroid[ 2 ][ 2 ] = TRUE;
      gabTetroid[ 1 ][ 1 ] = TRUE;
      gabTetroid[ 0 ][ 1 ] = TRUE;
      break;

    case 5:
      // +----+
      // |    |
      // |### |
      // | #  |
      // |    |
      // +----+
      gabTetroid[ 0 ][ 2 ] = TRUE;
      gabTetroid[ 1 ][ 2 ] = TRUE;
      gabTetroid[ 2 ][ 2 ] = TRUE;
      gabTetroid[ 1 ][ 1 ] = TRUE;
      break;

    case 6:
      // +----+
      // |    |
      // |##  |
      // | ## |
      // |    |
      // +----+
      gabTetroid[ 0 ][ 2 ] = TRUE;
      gabTetroid[ 1 ][ 2 ] = TRUE;
      gabTetroid[ 1 ][ 1 ] = TRUE;
      gabTetroid[ 2 ][ 1 ] = TRUE;
      break;

    default:
      //TODO: error handling
      break;
  }
}

/*! *******************************************************************
 * \brief  Checks if the tetroid hits a block in the playfield or not
 * \param  -
 * \return TRUE if the tetroid hits a block; FALSE otherwise
 *********************************************************************/
static BOOL CheckPlayfieldHit( void )
{
  BOOL bReturn = FALSE;
  U8 u8IndexX, u8IndexY;
  
  for( u8IndexX = 0; u8IndexX < PLAYFIELD_SIZE_X; u8IndexX++ )
  {
    for( u8IndexY = 0; u8IndexY < PLAYFIELD_SIZE_Y; u8IndexY++ )
    {
      // If the coordinate is inside the tetroid boundaries
      if( ( u8IndexX >= gi8TetroidX ) && ( u8IndexX < ( gi8TetroidX + TETROID_SIZE_X ) )
         && ( u8IndexY >= gi8TetroidY ) && ( u8IndexY < ( gi8TetroidY + TETROID_SIZE_Y ) ) )
      {
        // Check if a block of the tetroid and a block of the playfield has the same coordinate
        if( ( TRUE == gabTetroid[ u8IndexX - gi8TetroidX ][ u8IndexY - gi8TetroidY ] )
           && ( TRUE == gabBlocks[ u8IndexX ][ u8IndexY ] ) )
        {
          bReturn = TRUE;
          // End for loop
          u8IndexX = PLAYFIELD_SIZE_X;
          u8IndexY = PLAYFIELD_SIZE_Y;
          break;
        }
      }
    }
  }
  // Check if the tetroid is inside playfield boundaries (except for top boundary)
  for( u8IndexX = 0; u8IndexX < TETROID_SIZE_X; u8IndexX++ )
  {
    for( u8IndexY = 0; u8IndexY < TETROID_SIZE_Y; u8IndexY++ )
    {
      if( ( ( (gi8TetroidX + (I8)u8IndexX) < 0 ) || ( (gi8TetroidX + (I8)u8IndexX) >= PLAYFIELD_SIZE_X ) || ( (gi8TetroidY + (I8)u8IndexY) < 0 ) )
       && ( TRUE == gabTetroid[ u8IndexX ][ u8IndexY ] ) )
      {
        bReturn = TRUE;
      }
    }
  }
  return bReturn;
}

/*! *******************************************************************
 * \brief  Rotates the current tetroid clockwise
 * \param  bClockWise: rotates clockwise if TRUE; else it rotates anti-clockwise
 * \return -
 *********************************************************************/
static void RotateTetroid( BOOL bClockWise )
{
  BOOL abRotatedTetroid[ TETROID_SIZE_X ][ TETROID_SIZE_Y ];
  U8   u8IndexX, u8IndexY;
  
  for( u8IndexX = 0; u8IndexX < TETROID_SIZE_X; u8IndexX++ )
  {
    for( u8IndexY = 0; u8IndexY < TETROID_SIZE_Y; u8IndexY++ )
    {
      if( TRUE == bClockWise )  // Clockwise rotation
      {
        abRotatedTetroid[ TETROID_SIZE_Y - u8IndexY - 1 ][ u8IndexX ] = gabTetroid[ u8IndexX ][ u8IndexY ];
      }
      else  // Anti-clockwise rotation
      {
        abRotatedTetroid[ u8IndexY ][ TETROID_SIZE_X - u8IndexX - 1 ] = gabTetroid[ u8IndexX ][ u8IndexY ];
      }
    }
  }
  memcpy( gabTetroid, abRotatedTetroid, sizeof( gabTetroid ) );
}

/*! *******************************************************************
 * \brief  Draws the tetroid that has its lower left corner in given coordinates
 * \param  u8X: horizontal coordinate
 * \param  u8Y: vertical coordinate
 * \param  u16Color: draws in this color
 * \return -
 *********************************************************************/
static void DrawTetroid( I8 i8X, I8 i8Y, U16 u16Color )
{
  U8 u8IndexX, u8IndexY;

  for( u8IndexX = 0; u8IndexX < PLAYFIELD_SIZE_X; u8IndexX++ )
  {
    for( u8IndexY = 0; u8IndexY < PLAYFIELD_SIZE_Y; u8IndexY++ )
    {
      // If the coordinate is inside the tetroid boundaries
      if( ( u8IndexX >= i8X ) && ( u8IndexX < ( i8X + TETROID_SIZE_X ) )
       && ( u8IndexY >= i8Y ) && ( u8IndexY < ( i8Y + TETROID_SIZE_Y ) ) )
      {
        if( TRUE == gabTetroid[ u8IndexX - i8X ][ u8IndexY - i8Y ] )
        {
          DrawBlock( u8IndexX, u8IndexY, u16Color );
        }
      }
    }
  }

}

/*! *******************************************************************
 * \brief
 * \param
 * \return
 *********************************************************************/


//--------------------------------------------------------------------------------------------------------/
// Interface functions
//--------------------------------------------------------------------------------------------------------/
/*! *******************************************************************
 * \brief  Initialize game logic
 * \param  -
 * \return -
 *********************************************************************/
void Task_Tetris_Init( void )
{
  //Tracker_Init( HAL_GetTick() );
  gbRunning = FALSE;
  gbGameOver = FALSE;
  memset( gabBlocks, FALSE, sizeof( gabBlocks ) );
  memset( gabTetroid, FALSE, sizeof( gabTetroid ) );
  gu32TimerMS = 0;
  gi8TetroidX = 0;
  gi8TetroidY = PLAYFIELD_SIZE_Y - 1;
  gu32Score = 0;
  // Put "Tetris" text on playfield
  //   +----------+
  // 19|          |
  // 18| ###  ### |
  // 17|  #   #   |
  // 16|  #   ### |
  // 15|  #   #   |
  // 14|  #   ### |
  // 13|          |
  // 12| ###  ### |
  // 11|  #   # # |
  // 10|  #   ### |
  // 09|  #   ##  |
  // 08|  #   # # |
  // 07|          |
  // 06|  #    ## |
  // 05|  #   #   |
  // 04|  #    #  |
  // 03|  #     # |
  // 02|  #   ##  |
  // 01|          |
  // 00|          |
  //   +----------+
  // T
  gabBlocks[1][18] = TRUE;
  gabBlocks[2][18] = TRUE;
  gabBlocks[3][18] = TRUE;
  gabBlocks[2][17] = TRUE;
  gabBlocks[2][16] = TRUE;
  gabBlocks[2][15] = TRUE;
  gabBlocks[2][14] = TRUE;
  // E
  gabBlocks[6][18] = TRUE;
  gabBlocks[7][18] = TRUE;
  gabBlocks[8][18] = TRUE;
  gabBlocks[6][17] = TRUE;
  gabBlocks[6][16] = TRUE;
  gabBlocks[7][16] = TRUE;
  gabBlocks[8][16] = TRUE;
  gabBlocks[6][15] = TRUE;
  gabBlocks[6][14] = TRUE;
  gabBlocks[7][14] = TRUE;
  gabBlocks[8][14] = TRUE;
  // T
  gabBlocks[1][12] = TRUE;
  gabBlocks[2][12] = TRUE;
  gabBlocks[3][12] = TRUE;
  gabBlocks[2][11] = TRUE;
  gabBlocks[2][10] = TRUE;
  gabBlocks[2][ 9] = TRUE;
  gabBlocks[2][ 8] = TRUE;
  // R
  gabBlocks[6][12] = TRUE;
  gabBlocks[7][12] = TRUE;
  gabBlocks[8][12] = TRUE;
  gabBlocks[6][11] = TRUE;
  gabBlocks[8][11] = TRUE;
  gabBlocks[6][10] = TRUE;
  gabBlocks[7][10] = TRUE;
  gabBlocks[8][10] = TRUE;
  gabBlocks[6][ 9] = TRUE;
  gabBlocks[7][ 9] = TRUE;
  gabBlocks[6][ 8] = TRUE;
  gabBlocks[8][ 8] = TRUE;
  // I
  gabBlocks[2][ 6] = TRUE;
  gabBlocks[2][ 5] = TRUE;
  gabBlocks[2][ 4] = TRUE;
  gabBlocks[2][ 3] = TRUE;
  gabBlocks[2][ 2] = TRUE;
  // S
  gabBlocks[7][ 6] = TRUE;
  gabBlocks[8][ 6] = TRUE;
  gabBlocks[6][ 5] = TRUE;
  gabBlocks[7][ 4] = TRUE;
  gabBlocks[8][ 3] = TRUE;
  gabBlocks[6][ 2] = TRUE;
  gabBlocks[7][ 2] = TRUE;
  // Initial tetroid for testing
  gabTetroid[0][0] = TRUE;
  gabTetroid[0][1] = TRUE;
  gabTetroid[0][2] = TRUE;
  gabTetroid[0][3] = TRUE;
  gabTetroid[1][0] = TRUE;
  gabTetroid[1][1] = TRUE;
  gabTetroid[1][2] = TRUE;
  gabTetroid[1][3] = TRUE;
  gabTetroid[2][0] = TRUE;
  gabTetroid[2][1] = TRUE;
  gabTetroid[2][2] = TRUE;
  gabTetroid[2][3] = TRUE;
  gabTetroid[3][0] = TRUE;
  gabTetroid[3][1] = TRUE;
  gabTetroid[3][2] = TRUE;
  gabTetroid[3][3] = TRUE;

  // Clear display
  LCD_Clear( BLACK );

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
 * \brief  Game logic running in main loop
 * \param  -
 * \return -
 *********************************************************************/
void Task_Tetris_Cycle( void )
{
  volatile U32 u32RandomNumber = rand();  // roll the random number generator so it will be more random
  U8 u8IndexX, u8IndexY;
  char au8HighScoreString[ 10 ];
  U32 u32TimeNow = HAL_GetTick();
  BOOL bRedrawTetroid = FALSE;
  I8   i8TetroidX = gi8TetroidX;  // Tetroid coordinate before moving
  I8   i8TetroidY = gi8TetroidY;  // Tetroid coordinate before moving

  // Draw playfield frame (scalable by BLOCK_SIZE)
  LCD_DrawLine( 0, 0, 0, (PLAYFIELD_SIZE_X*BLOCK_SIZE + PLAYFIELD_OFFSET_X), WHITE );
  LCD_DrawLine( 0, 0, (PLAYFIELD_SIZE_Y*BLOCK_SIZE + PLAYFIELD_OFFSET_Y), 0u, WHITE );
  LCD_DrawLine( (PLAYFIELD_SIZE_Y*BLOCK_SIZE + PLAYFIELD_OFFSET_Y), 0, (PLAYFIELD_SIZE_Y*BLOCK_SIZE + PLAYFIELD_OFFSET_Y), (PLAYFIELD_SIZE_X*BLOCK_SIZE + PLAYFIELD_OFFSET_X), WHITE );
  LCD_DrawLine( (PLAYFIELD_SIZE_Y*BLOCK_SIZE + PLAYFIELD_OFFSET_Y), (PLAYFIELD_SIZE_X*BLOCK_SIZE + PLAYFIELD_OFFSET_X), 0, (PLAYFIELD_SIZE_X*BLOCK_SIZE + PLAYFIELD_OFFSET_X), WHITE );
  
  // Display game over text if the game has finished
  if( TRUE == gbGameOver )
  {
    LCD_PrintString( LCD_WIDTH-1u-9u*11u, LCD_HEIGHT-19u, "Game over", FONT, WHITE, BLACK );
  }
  
  // Print out score
  if( ( TRUE == gbGameOver ) || ( TRUE == gbRunning ) )
  {
    LCD_PrintString( 0, LCD_HEIGHT-19u, "Score:", FONT, WHITE, BLACK );
    sprintf( (char*)au8HighScoreString, "%lu", gu32Score );
    LCD_PrintString( 6u*11u, LCD_HEIGHT-19u, au8HighScoreString, FONT, WHITE, BLACK );
  }
  
  // Draw playfield blocks
  for( u8IndexX = 0; u8IndexX < PLAYFIELD_SIZE_X; u8IndexX++ )
  {
    for( u8IndexY = 0; u8IndexY < PLAYFIELD_SIZE_Y; u8IndexY++ )
    {
      if( TRUE == gabBlocks[ u8IndexX ][ u8IndexY ] )
      {
        DrawBlock( u8IndexX, u8IndexY, WHITE );
      }
    }
  }

  // If the game is not running yet
  if( FALSE == gbRunning )
  {
    // Check start button and rotate button
    if( ( BUTTON_PRESSED == Buttons_GetEvent( BUTTON_SW2 ) )
     || ( BUTTON_PRESSED == Buttons_GetEvent( BUTTON_SW4_PUSH ) ) )
    {
      // Pressing start button will start/restart the game
      gbRunning = TRUE;
      gbGameOver = FALSE;
      memset( gabBlocks, FALSE, sizeof( gabBlocks ) );  // clear playfield
      gu32Score = 0;
      gu32TimerMS = u32TimeNow + DEFAULT_SPEED_MS;
//      Tracker_Init( u32TimeNow );
      // Roll a random tetroid and place it on the top of screen
      RollNewTetroid();
      gi8TetroidX = (PLAYFIELD_SIZE_X - TETROID_SIZE_X)/2;
      gi8TetroidY = PLAYFIELD_SIZE_Y - 1;
      // Clear display
      LCD_Clear( BLACK );
    }
  }
  else  // If the game is running
  {
    // Play music
//    Tracker_Play( u32TimeNow );
    
    // Check timer and down button, and move tetroid vertically
    if( ( u32TimeNow >= gu32TimerMS )                           // if timer is expired...
     || ( BUTTON_PRESSED == Buttons_GetEvent( BUTTON_SW4_LEFT ) ) )  // ...or down button is pressed
    {
      // Move tetroid vertically
      gi8TetroidY -= 1;
      bRedrawTetroid = TRUE;
      // Check if the tetroid hit anything and add it to the playfield if it hit
      if( TRUE == CheckPlayfieldHit() )
      {
        // The tetroid should not be here -- it hit the playfield
        gi8TetroidY += 1;
        bRedrawTetroid = FALSE;
        if( (PLAYFIELD_SIZE_Y - 1) == gi8TetroidY )
        {
          // Game over
          gbGameOver = TRUE;
          gbRunning = FALSE;
        }
        FixTetroid();  // this fixes the tetroid and generates a new one
      }
      gu32TimerMS = u32TimeNow + DEFAULT_SPEED_MS;  // re-wind timer
    }
    // Check up button event and if triggered, place tetroid to the bottom
    if( BUTTON_PRESSED == Buttons_GetEvent( BUTTON_SW4_RIGHT ) )
    {
      // Immediately clear the tetroid on screen
      DrawTetroid( i8TetroidX, i8TetroidY, BLACK );
      // Move down as low as possible
      while( FALSE == CheckPlayfieldHit() )
      {
        gi8TetroidY -= 1;
      }
      // The tetroid should not be here -- it hit the playfield
      gi8TetroidY += 1;
      if( (PLAYFIELD_SIZE_Y - 1) == gi8TetroidY )
      {
        // Game over
        gbGameOver = TRUE;
        gbRunning = FALSE;
      }
      FixTetroid();  // this fixes the tetroid and generates a new one
    }
    // Check left-right button events to move tetroid horizontally
    if( BUTTON_PRESSED == Buttons_GetEvent( BUTTON_SW4_UP ) )
    {
      gi8TetroidX -= 1;
      bRedrawTetroid = TRUE;
      // Check for playfield hit
      if( TRUE == CheckPlayfieldHit() )
      {
        gi8TetroidX += 1;
        bRedrawTetroid = FALSE;
      }
    }
    if( BUTTON_PRESSED == Buttons_GetEvent( BUTTON_SW4_DOWN ) )
    {
      gi8TetroidX += 1;
      bRedrawTetroid = TRUE;
      // Check for playfield hit
      if( TRUE == CheckPlayfieldHit() )
      {
        gi8TetroidX -= 1;
        bRedrawTetroid = FALSE;
      }
    }
    // Check rotation button
    if( ( BUTTON_PRESSED == Buttons_GetEvent( BUTTON_SW4_PUSH ) ) )
    {
      DrawTetroid( gi8TetroidX, gi8TetroidY, BLACK );
      RotateTetroid( TRUE );
      bRedrawTetroid = TRUE;
      if( TRUE == CheckPlayfieldHit() )
      {
        // Invalid rotation, reverse it
        RotateTetroid( FALSE );
      }
    }
    // If the tetroid was moved
    if( TRUE == bRedrawTetroid )
    {
      DrawTetroid( i8TetroidX, i8TetroidY, BLACK );
    }
    // Plot tetroid in new position
    DrawTetroid( gi8TetroidX, gi8TetroidY, LIGHTBLUE );
  }

  // Middle button push: exit game
  if( BUTTON_PRESSED == Buttons_GetEvent( BUTTON_SW3 ) )
  {
    Tasks_EndTask();
  }
}

//-----------------------------------------------< EOF >--------------------------------------------------/
