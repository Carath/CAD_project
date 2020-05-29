////////////////////////////////////////////////////////////////////////////////////
// SDLA v1.5
////////////////////////////////////////////////////////////////////////////////////

// This is an unofficial SDL2 addon, whose purpose is to make SDL easier to use.
// SDL, SDL_image and SDL_ttf are needed for this to work.

// Note: SDL2 (v2.0.8) suffers from a small memory leak, of a fixed size: 4,249 bytes in 4 blocks.
// This is unrelated to SDLA, and will happen when one simply calls SDL_Init() followed by SDL_Quit().


#ifndef SDLA_H
#define SDLA_H


#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

// N.B: SDL.h, SDL_image.h, and SDL_ttf.h all load math.h.
// Since SDLA.h loads all of them, math.h is therefore loaded too!


// Font rendering option: SDLA_BLENDED -> slow but beautiful, SDLA_SOLID -> fast but not as smooth.
typedef enum {SDLA_SOLID, SDLA_BLENDED} SDLA_FontAliasing;


// Structs for caching fonts:
typedef struct
{
	SDL_Texture *Texture;
	int xMin;
	int xMax;
} Glyph;


typedef struct
{
	short Size;
	short Height;
	char Ch_min;
	char Ch_max;
	Glyph *Table;
} CachedFont;


// Used for the centering option when drawing:
#define SDLA_CENTERED 2147483647 // INT_MAX, was chosen arbitrarily. This just needs to be big.


////////////////////////////////////////////////////////////////////////////////////
// SDLA error management and initialization:
////////////////////////////////////////////////////////////////////////////////////


// Kills the running program, and raises the given error message.
void SDLA_ExitWithError(const char *message);


// Quit using SDL, but doesn't kill the program.
void SDLA_Quit(void);


// Initialize the rendering subsystem of SDLA. Used in texture creation, drawing, and font caching.
// 'aliasing': SDLA_BLENDED -> slow but beautiful, SDLA_SOLID -> fast but not as smooth.
void SDLA_Init(SDL_Window **window, SDL_Renderer **renderer, char *window_name, int window_width, int window_height,
	int hardware_acceleration, SDLA_FontAliasing aliasing);


// Initialize the text input subsystem of SDLA. Returns the string to be filled with text input.
// That string has to be freed by the user upon exiting.
char* SDLA_InitTextInput(int text_input_length);


////////////////////////////////////////////////////////////////////////////////////
// Texture creation:
////////////////////////////////////////////////////////////////////////////////////

// The following functions allow to create textures which can then be drawn.
// This needs a SDLA_Init() call in order to work.


// Creates a texture from an image file of the following formats: bmp, gif, jpeg, lbm, pcx, png,
// pnm, svg, tga, tiff, webp, xcf, xv. Particularly, png files are to be considered, as they are
// lighter than bmp and natively support transparency.
SDL_Texture* SDLA_CreateTexture(char *image_name);


// Use this function only if you have to disable SDL_image from this addon, as SDLA_CreateTexture
// is better in most cases. This creates a texture from a .bmp file, and sets a transparency by
// ignoring the given color. No transparency is set if transparency_color is NULL.
SDL_Texture* SDLA_CreateTextureBMP(char *image_name, SDL_Color *transparency_color);


// Creates a texture from text, given a font and a color.
SDL_Texture* SDLA_CreateTextTexture(TTF_Font *font, SDL_Color *color, char *text);


////////////////////////////////////////////////////////////////////////////////////
// Drawing:
////////////////////////////////////////////////////////////////////////////////////

// This needs a SDLA_Init() call in order to work.


// Sets the current drawing RGB color:
void SDLA_SetDrawColor(Uint8 r, Uint8 g, Uint8 b);


// Clears the current window with the given RGB color. If 'color' is NULL, the window gets black:
void SDLA_ClearWindow(SDL_Color *color);


// Draws the texture at the given coordinates, where (0, 0) is at the window's top left corner,
// and the y-axis goes to the bottom of the screen. If one wants to center the texture horizontally,
// vertically, or both, the corresponding coordinates need to be replaced by SDLA_CENTERED.
void SDLA_DrawTexture(SDL_Texture *texture, int x, int y);


// Draws the text at the given coordinates. Same centering options as in SDLA_DrawTexture() are supported.
// The text must not contain any carriage return. This is slow due to the text's texture being dynamically
// created and freed. It is more efficient to create the texture beforehand and draw it when needed using
// SDLA_DrawTexture(). For drawing dynamic texts, using the cached font method (see below) may be more efficient...
void SDLA_SlowDrawText(TTF_Font *font, SDL_Color *color, int x, int y, char *text);


// Returns the horizontal size in pixels that the given text would take if drawn.
int SDLA_TextSize(TTF_Font *font, char *text);


////////////////////////////////////////////////////////////////////////////////////
// Text input:
////////////////////////////////////////////////////////////////////////////////////

// Method for getting a string from user inputs. Useful for entering names, and so on.
// This needs a SDLA_InitTextInput() call in order to work.


// Resets the text_input string to an empty string.
void SDLA_ResetTextInput(char *text_input);


// Updates the text_input string with user input. Returns 1 if text_input is modified, 0 otherwise.
int SDLA_UpdateTextInput(char *text_input, SDL_Event *event);


// Useful if the user wants to replace the content of 'text_input'.
void SDLA_SetTextInputContent(char *text_input, char *new_content);


////////////////////////////////////////////////////////////////////////////////////
// Font caching:
////////////////////////////////////////////////////////////////////////////////////

// Font caching method whose purpose is to draw dynamic text more proficiently.
// This needs a SDLA_Init() call in order to work.


// Cache characters (from ch_min to ch_max) of the given font as separated textures,
// in order to draw them dynamically and efficiently.
CachedFont* SDLA_CachingFontByRange(char *font_name, short size, SDL_Color *color, char ch_min, char ch_max);


// Cache all supported characters of the given font as separated textures.
CachedFont* SDLA_CachingFontAll(char *font_name, short size, SDL_Color *color);


// Draws the given text at the given coordinates, with the cached font method.
// Unsupported: automatic carriage return. DO NOT use this with SDLA_CENTERED.
void SDLA_DrawCachedFont(CachedFont *cached_font, int x, int y, char *text);


// Returns the horizontal size in pixels that the given text would take if drawn, using the cached font method.
// For now, the text needs to not contain a carriage return...
int SDLA_CachedTextSize(CachedFont *cached_font, char *text);


// Frees the given cached font from memory.
void SDLA_FreeCachedFont(CachedFont *cached_font);


// Prints the given cached font details in the console.
void SDLA_PrintCachedFontInfo(CachedFont *cached_font);


#endif
