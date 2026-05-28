#include "buttons.h"
#include "sdl_backend.h"

#include <stdio.h>
#include <string.h>

//--------------------------------------------------------------------------------------------------------/
// Globals
//--------------------------------------------------------------------------------------------------------/
volatile E_BUTTONS_STATE gaeButtonsState[NUM_BUTTONS];

// Internal edge tracking
static BOOL gPrevState[NUM_BUTTONS];

//--------------------------------------------------------------------------------------------------------/
// Init
//--------------------------------------------------------------------------------------------------------/
void Buttons_Init(void)
{
  memset((void*)gaeButtonsState, 0, sizeof(gaeButtonsState));
  memset(gPrevState, 0, sizeof(gPrevState));

  SDL_Backend_Init();
}

//--------------------------------------------------------------------------------------------------------/
// Timer ISR simulation (call every frame ~30 FPS)
//--------------------------------------------------------------------------------------------------------/
void Buttons_TimerIT(void)
{
  SDL_Backend_PollEvents();

  for (unsigned int i = 0; i < NUM_BUTTONS; i++)
  {
    BOOL pressed = SDL_Backend_IsKeyDown((E_BUTTONS_INDEX)i);

    switch (gaeButtonsState[i])
    {
      case BUTTON_INACTIVE:
        if (pressed)
        {
          gaeButtonsState[i] = BUTTON_BOUNCING;
        }
        break;

      case BUTTON_BOUNCING:
        gaeButtonsState[i] = pressed ? BUTTON_ACTIVE : BUTTON_INACTIVE;
        break;

      case BUTTON_ACTIVE:
        if (!pressed)
        {
          gaeButtonsState[i] = BUTTON_RELEASING;
        }
        break;

      case BUTTON_RELEASING:
        gaeButtonsState[i] = pressed ? BUTTON_ACTIVE : BUTTON_INACTIVE;
        break;
    }

    gPrevState[i] = pressed;
  }
}

//--------------------------------------------------------------------------------------------------------/
// Event polling
//--------------------------------------------------------------------------------------------------------/
E_BUTTONS_EVENT Buttons_GetEvent(E_BUTTONS_INDEX eButton)
{
  static BOOL lastStable[NUM_BUTTONS];

  BOOL current = (gaeButtonsState[eButton] == BUTTON_ACTIVE);

  if (current && !lastStable[eButton])
  {
    lastStable[eButton] = TRUE;
    return BUTTON_PRESSED;
  }

  if (!current && lastStable[eButton])
  {
    lastStable[eButton] = FALSE;
    return BUTTON_RELEASED;
  }

  return BUTTON_NOEVENT;
}

//--------------------------------------------------------------------------------------------------------/
// Placeholder (unused in mock)
void Buttons_SetRepeatedPresses(BOOL bSet)
{
  (void)bSet;
}