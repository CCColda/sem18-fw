/*! *******************************************************************************************************
* Mock LCD using SDL2
**********************************************************************************************************/

#include "lcd.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//--------------------------------------------------------------------------------------------------------/
// Definitions
//--------------------------------------------------------------------------------------------------------/
#define LCD_WINDOW_SCALE   (4u)
#define LCD_WINDOW_WIDTH   (LCD_WIDTH * LCD_WINDOW_SCALE)
#define LCD_WINDOW_HEIGHT  (LCD_HEIGHT * LCD_WINDOW_SCALE)

#define LCD_FONT_PATH "res/DejaVuSans.ttf"

//--------------------------------------------------------------------------------------------------------/
// Types
//--------------------------------------------------------------------------------------------------------/
typedef struct mocklcd_context
{
  SDL_Window*   window;
  SDL_Renderer* renderer;
  SDL_Texture*   lcdTexture;
  TTF_Font*     fonts[3];
} mocklcd_context_t;

//--------------------------------------------------------------------------------------------------------/
// Globals
//--------------------------------------------------------------------------------------------------------/
volatile U16 gau16LCDFrameBuffer[LCD_WIDTH * LCD_HEIGHT];
static mocklcd_context_t mockLCDContext;

//--------------------------------------------------------------------------------------------------------/
// Helpers
//--------------------------------------------------------------------------------------------------------/
static void LCD_Fatal(const char* pcMessage)
{
  fprintf(stderr, "\n[FATAL] %s\n", pcMessage);
  fprintf(stderr, "SDL Error: %s\n", SDL_GetError());

  TTF_Quit();
  SDL_Quit();

  abort();
}

static void LCD_CheckSDL(BOOL bCondition, const char* pcMessage)
{
  if(!bCondition)
  {
    LCD_Fatal(pcMessage);
  }
}

static SDL_Color LCD_Color565ToSDL(U16 c)
{
  SDL_Color col;

  col.r = (Uint8)((((c >> 11) & 0x1Fu) * 255u) / 31u);
  col.g = (Uint8)((((c >> 5)  & 0x3Fu) * 255u) / 63u);
  col.b = (Uint8)((( c        & 0x1Fu) * 255u) / 31u);
  col.a = 255u;

  return col;
}

static void LCD_SetColor(U16 c)
{
  SDL_Color col = LCD_Color565ToSDL(c);

  LCD_CheckSDL(
    0 == SDL_SetRenderDrawColor(mockLCDContext.renderer,
                                col.r, col.g, col.b, col.a),
    "SDL_SetRenderDrawColor failed"
  );
}

static TTF_Font* LCD_GetFont(E_LCD_FONT_TYPE f)
{
  return mockLCDContext.fonts[f];
}

//--------------------------------------------------------------------------------------------------------/
// API
//--------------------------------------------------------------------------------------------------------/
void LCD_Init(void)
{
  LCD_CheckSDL(0 == SDL_Init(SDL_INIT_VIDEO), "SDL_Init failed");
  LCD_CheckSDL(0 == TTF_Init(), "TTF_Init failed");

  mockLCDContext.window = SDL_CreateWindow(
    "Mock LCD",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    LCD_WINDOW_WIDTH,
    LCD_WINDOW_HEIGHT,
    SDL_WINDOW_SHOWN
  );

  LCD_CheckSDL(mockLCDContext.window != NULL, "SDL_CreateWindow failed");

  mockLCDContext.renderer = SDL_CreateRenderer(
    mockLCDContext.window,
    -1,
    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
  );

  LCD_CheckSDL(mockLCDContext.renderer != NULL, "SDL_CreateRenderer failed");

  LCD_CheckSDL(
    0 == SDL_RenderSetLogicalSize(mockLCDContext.renderer, LCD_WIDTH, LCD_HEIGHT),
    "SDL_RenderSetLogicalSize failed"
  );

  // Create LCD render target texture
  mockLCDContext.lcdTexture = SDL_CreateTexture(
    mockLCDContext.renderer,
    SDL_PIXELFORMAT_RGB565,
    SDL_TEXTUREACCESS_TARGET,
    LCD_WIDTH,
    LCD_HEIGHT
  );

  LCD_CheckSDL(mockLCDContext.lcdTexture != NULL, "SDL_CreateTexture failed");

  SDL_SetRenderTarget(mockLCDContext.renderer, mockLCDContext.lcdTexture);

  mockLCDContext.fonts[LCD_FONT_7x10] = TTF_OpenFont(LCD_FONT_PATH, 10);
  LCD_CheckSDL(mockLCDContext.fonts[LCD_FONT_7x10] != NULL, "Failed to load font 7x10");

  mockLCDContext.fonts[LCD_FONT_11x18] = TTF_OpenFont(LCD_FONT_PATH, 18);
  LCD_CheckSDL(mockLCDContext.fonts[LCD_FONT_11x18] != NULL, "Failed to load font 11x18");

  mockLCDContext.fonts[LCD_FONT_16x26] = TTF_OpenFont(LCD_FONT_PATH, 26);
  LCD_CheckSDL(mockLCDContext.fonts[LCD_FONT_16x26] != NULL, "Failed to load font 16x26");

  printf("[INFO] SDL initialization successful\n");

  LCD_Clear(BLACK);
}

void LCD_Clear(U16 c)
{
  SDL_SetRenderTarget(mockLCDContext.renderer, mockLCDContext.lcdTexture);
  LCD_SetColor(c);

  LCD_CheckSDL(
    0 == SDL_RenderClear(mockLCDContext.renderer),
    "SDL_RenderClear failed"
  );

  SDL_SetRenderTarget(mockLCDContext.renderer, mockLCDContext.lcdTexture);
}

void LCD_Update(void)
{
  // Read back ONLY here into framebuffer
  SDL_SetRenderTarget(mockLCDContext.renderer, mockLCDContext.lcdTexture);

  LCD_CheckSDL(
    0 == SDL_RenderReadPixels(
      mockLCDContext.renderer,
      NULL,
      SDL_PIXELFORMAT_RGB565,
      (void*)gau16LCDFrameBuffer,
      LCD_WIDTH * sizeof(U16)
    ),
    "SDL_RenderReadPixels failed"
  );

  // Present texture to screen
  SDL_SetRenderTarget(mockLCDContext.renderer, NULL);

  LCD_CheckSDL(
    0 == SDL_RenderCopy(mockLCDContext.renderer, mockLCDContext.lcdTexture, NULL, NULL),
    "SDL_RenderCopy failed"
  );

  SDL_RenderPresent(mockLCDContext.renderer);

  // restore target for next frame
  SDL_SetRenderTarget(mockLCDContext.renderer, mockLCDContext.lcdTexture);
}

void LCD_Pixel(U8 x, U8 y, U16 c)
{
  if(x >= LCD_WIDTH || y >= LCD_HEIGHT) return;

  LCD_SetColor(c);

  SDL_RenderDrawPoint(mockLCDContext.renderer, x, y);
}

void LCD_DrawLine(U8 x0, U8 y0, U8 x1, U8 y1, U16 c)
{
  I16 dx = abs((I16)x1 - (I16)x0);
  I16 sx = (x0 < x1) ? 1 : -1;
  I16 dy = -abs((I16)y1 - (I16)y0);
  I16 sy = (y0 < y1) ? 1 : -1;
  I16 err = dx + dy;

  LCD_SetColor(c);

  while(1)
  {
    if(x0 < LCD_WIDTH && y0 < LCD_HEIGHT)
      SDL_RenderDrawPoint(mockLCDContext.renderer, x0, y0);

    if(x0 == x1 && y0 == y1) break;

    I16 e2 = 2 * err;

    if(e2 >= dy) { err += dy; x0 += sx; }
    if(e2 <= dx) { err += dx; y0 += sy; }
  }
}

void LCD_DrawFilledRectangle(U8 x0, U8 y0, U8 x1, U8 y1, U16 c)
{
  if(x0 > x1){ U8 t=x0; x0=x1; x1=t; }
  if(y0 > y1){ U8 t=y0; y0=y1; y1=t; }

  LCD_SetColor(c);

  for(U8 y=y0; y<=y1; y++)
    for(U8 x=x0; x<=x1; x++)
      if(x < LCD_WIDTH && y < LCD_HEIGHT)
        SDL_RenderDrawPoint(mockLCDContext.renderer, x, y);
}

void LCD_PrintChar(U16 x, U16 y, char ch, E_LCD_FONT_TYPE f, U16 c, U16 bg)
{
  char text[2] = {ch, 0};

  TTF_Font* font = LCD_GetFont(f);
  SDL_Color fg = LCD_Color565ToSDL(c);
  SDL_Color bgc = LCD_Color565ToSDL(bg);

  SDL_Surface* s = TTF_RenderText_Shaded(font, text, fg, bgc);
  SDL_Texture* t = SDL_CreateTextureFromSurface(mockLCDContext.renderer, s);

  SDL_Rect r = { (int)x, (int)y, s->w, s->h };

  SDL_RenderCopy(mockLCDContext.renderer, t, NULL, &r);

  SDL_DestroyTexture(t);
  SDL_FreeSurface(s);
}

void LCD_PrintString(U16 x, U16 y, const char* str, E_LCD_FONT_TYPE f, U16 c, U16 bg)
{
  if(!str) LCD_Fatal("NULL string");

  TTF_Font* font = LCD_GetFont(f);
  SDL_Color fg = LCD_Color565ToSDL(c);
  SDL_Color bgc = LCD_Color565ToSDL(bg);

  SDL_Surface* s = TTF_RenderText_Shaded(font, str, fg, bgc);
  SDL_Texture* t = SDL_CreateTextureFromSurface(mockLCDContext.renderer, s);

  SDL_Rect r = { (int)x, (int)y, s->w, s->h };

  SDL_RenderCopy(mockLCDContext.renderer, t, NULL, &r);

  SDL_DestroyTexture(t);
  SDL_FreeSurface(s);
}