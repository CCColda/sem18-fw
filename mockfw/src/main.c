#include "lcd.h"
#include "task_snake.h"
#include "buttons.h"

#include <SDL2/SDL.h>
#include <stdbool.h>

#define FPS 30u
#define FRAME_MS (1000u / FPS)

typedef void (*task_fn_t)(void);

typedef struct
{
  task_fn_t init;
  task_fn_t cycle;
} task_t;

static const task_t gTask =
{
  .init = Task_Snake_Init,
  .cycle = Task_Snake_Cycle
};

void QuitTask(void) {
  
}

int main(void)
{
  bool running = true;
  SDL_Event e;

  LCD_Init();
  Buttons_Init();
  gTask.init();

  while (running)
  {
    Uint32 start = SDL_GetTicks();

    while (SDL_PollEvent(&e))
    {
      if (e.type == SDL_QUIT)
      {
        running = false;
      }
    }

    Buttons_TimerIT();

    gTask.cycle();

    LCD_Update();

    Uint32 dt = SDL_GetTicks() - start;
    if (dt < FRAME_MS)
    {
      SDL_Delay(FRAME_MS - dt);
    }
  }

  SDL_Quit();
  return 0;
}