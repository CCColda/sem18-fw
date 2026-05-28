/* test_task.c */

//--------------------------------------------------------------------------------------------------------/
// Include files
//--------------------------------------------------------------------------------------------------------/
#include "test_task.h"

#include "lcd.h"

//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
#define RECT_SIZE      (20u)
#define RECT_SPEED_X   (2)
#define RECT_SPEED_Y   (1)

//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/
typedef struct
{
  I16 s16PosX;
  I16 s16PosY;

  I16 s16VelocityX;
  I16 s16VelocityY;
} testtask_context_t;

//--------------------------------------------------------------------------------------------------------/
// Global variables
//--------------------------------------------------------------------------------------------------------/
static testtask_context_t testTaskContext;

//--------------------------------------------------------------------------------------------------------/
// Interface functions
//--------------------------------------------------------------------------------------------------------/

//--------------------------------------------------------------------------------------------------------/
/*! \brief Initialize task
*/
//--------------------------------------------------------------------------------------------------------/
void Task_TestTask_Init(void)
{
  testTaskContext.s16PosX = 10;
  testTaskContext.s16PosY = 10;

  testTaskContext.s16VelocityX = RECT_SPEED_X;
  testTaskContext.s16VelocityY = RECT_SPEED_Y;

  LCD_Clear(BLACK);
}

//--------------------------------------------------------------------------------------------------------/
/*! \brief Task cycle
*/
//--------------------------------------------------------------------------------------------------------/
void Task_TestTask_Cycle(void)
{
  I16 s16NextX;
  I16 s16NextY;

  LCD_Clear(BLACK);

  LCD_DrawFilledRectangle(
    (U8)testTaskContext.s16PosX,
    (U8)testTaskContext.s16PosY,
    (U8)(testTaskContext.s16PosX + RECT_SIZE),
    (U8)(testTaskContext.s16PosY + RECT_SIZE),
    GREEN
  );

  LCD_PrintString(
    5u,
    5u,
    "SDL LCD TEST",
    LCD_FONT_7x10,
    WHITE,
    BLACK
  );

  s16NextX = testTaskContext.s16PosX + testTaskContext.s16VelocityX;
  s16NextY = testTaskContext.s16PosY + testTaskContext.s16VelocityY;

  if ((s16NextX <= 0) ||
      ((s16NextX + RECT_SIZE) >= LCD_WIDTH))
  {
    testTaskContext.s16VelocityX =
      (I16)(-testTaskContext.s16VelocityX);
  }

  if ((s16NextY <= 0) ||
      ((s16NextY + RECT_SIZE) >= LCD_HEIGHT))
  {
    testTaskContext.s16VelocityY =
      (I16)(-testTaskContext.s16VelocityY);
  }

  testTaskContext.s16PosX += testTaskContext.s16VelocityX;
  testTaskContext.s16PosY += testTaskContext.s16VelocityY;
}