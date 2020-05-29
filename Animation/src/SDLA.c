////////////////////////////////////////////////////////////////////////////////////
// SDLA v1.5
////////////////////////////////////////////////////////////////////////////////////

// This is an unofficial SDL2 addon, whose purpose is to make SDL easier to use.
// SDL, SDL_image and SDL_ttf are needed for this to work.

// Note: SDL2 (v2.0.8) suffers from a small memory leak, of a fixed size: 4,249 bytes in 4 blocks.
// This is unrelated to SDLA, and will happen when one simply calls SDL_Init() followed by SDL_Quit().


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDLA.h"


#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) < (y) ? (y) : (x))

#define CHAR_MIN 32
#define CHAR_MAX 126


// For the rendering system:
static SDL_Window *Window;
static SDL_Renderer *Renderer;
static SDLA_FontAliasing FontAliasing;

// For text input and font caching:
static int TextInputLen;
static int Cursor = 0;

// For initialization checking:
static int RenderingInit = 0;
static int TextInputInit = 0;


////////////////////////////////////////////////////////////////////////////////////
// SDLA error management and initialization:
////////////////////////////////////////////////////////////////////////////////////


// Kills the running program, and raises the given error message.
void SDLA_ExitWithError(const char *message)
{
	SDL_DestroyRenderer(Renderer);
	SDL_DestroyWindow(Window);
	SDL_Log("ERROR: %s -> %s\n", message, SDL_GetError());
	TTF_Quit();
	SDL_Quit();
	exit(EXIT_FAILURE);
}


// Quit using SDL, but doesn't kill the program.
void SDLA_Quit(void)
{
	SDL_DestroyRenderer(Renderer);
	SDL_DestroyWindow(Window);
	TTF_Quit();
	SDL_Quit();
}


// Initialize the rendering subsystem of SDLA. Used in texture creation, drawing, and font caching.
// 'aliasing': SDLA_BLENDED -> slow but beautiful, SDLA_SOLID -> fast but not as smooth.
void SDLA_Init(SDL_Window **window, SDL_Renderer **renderer, char *window_name, int window_width, int window_height,
	int hardware_acceleration, SDLA_FontAliasing aliasing)
{
	if (RenderingInit)
		SDLA_ExitWithError("SDLA's rendering subsystem is already initialized.");

	// SDL version check:
	SDL_version nb;
	SDL_VERSION(&nb);
	printf("SDL version: %d.%d.%d\n", nb.major, nb.minor, nb.patch);

	// Initializing SDL:
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		SDLA_ExitWithError("Unable to initialize SDL.");

	// Initializing SDL_ttf:
	if (TTF_Init() == -1)
		SDLA_ExitWithError(TTF_GetError());

	// Creating a window:
	*window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		window_width, window_height, 0);

	Window = *window; // Storing the window's address.

	if (Window == NULL)
		SDLA_ExitWithError("Unable to create a window.");

	// Hardware acceleration option:
	printf("Hardware acceleration used: %d\n\n", hardware_acceleration);
	int acceleration_option = hardware_acceleration ? SDL_RENDERER_ACCELERATED : SDL_RENDERER_SOFTWARE;

	// Creating a renderer:
	*renderer = SDL_CreateRenderer(Window, -1, acceleration_option);

	Renderer = *renderer; // Storing the renderer's address.

	if (Renderer == NULL)
		SDLA_ExitWithError("Unable to create a renderer.");

	FontAliasing = aliasing;

	RenderingInit = 1;
}


// Initialize the text input subsystem of SDLA. Returns the string to be filled with text input.
// That string has to be freed by the user upon exiting.
char* SDLA_InitTextInput(int text_input_length)
{
	if (TextInputInit)
		SDLA_ExitWithError("SDLA's text input subsystem is already initialized.");

	TextInputLen = text_input_length;

	if (TextInputLen < 1)
		SDLA_ExitWithError("Text input length must be at least 1.");

	char *text_input = (char*) calloc(TextInputLen + 1, sizeof(char));

	if (text_input == NULL)
		SDLA_ExitWithError("Impossible to allocate enough memory for 'text_input'.");

	TextInputInit = 1;

	return text_input;
}


////////////////////////////////////////////////////////////////////////////////////
// Texture creation:
////////////////////////////////////////////////////////////////////////////////////

// The following functions allow to create textures which can then be drawn.
// This needs a SDLA_Init() call in order to work.


// Creates a texture from an image file of the following formats: bmp, gif, jpeg, lbm, pcx, png,
// pnm, svg, tga, tiff, webp, xcf, xv. Particularly, png files are to be considered, as they are
// lighter than bmp and natively support transparency.
SDL_Texture* SDLA_CreateTexture(char *image_name)
{
	SDL_Surface *surface = IMG_Load(image_name); // Needs SDL_image.

	if (surface == NULL)
		SDLA_ExitWithError("Image not found.");

	SDL_Texture *texture = SDL_CreateTextureFromSurface(Renderer, surface);

	if (texture == NULL)
		SDLA_ExitWithError("Impossible to create the texture.");

	SDL_FreeSurface(surface);

	return texture;
}


// Use this function only if you have to disable SDL_image from this addon, as SDLA_CreateTexture
// is better in most cases. This creates a texture from a .bmp file, and sets a transparency by
// ignoring the given color. No transparency is set if transparency_color is NULL.
SDL_Texture* SDLA_CreateTextureBMP(char *image_name, SDL_Color *transparency_color)
{
	SDL_Surface *surface = SDL_LoadBMP(image_name);

	if (surface == NULL)
		SDLA_ExitWithError("Image not found.");

	if (transparency_color != NULL)
	{
		Uint32 opaque_pixel = SDL_MapRGB(surface -> format,
			transparency_color -> r, transparency_color -> g, transparency_color -> b);

		if (SDL_SetColorKey(surface, SDL_TRUE, opaque_pixel) != 0)
			SDLA_ExitWithError("Impossible to set a color transparency.");
	}

	SDL_Texture *texture = SDL_CreateTextureFromSurface(Renderer, surface);

	if (texture == NULL)
		SDLA_ExitWithError("Impossible to create the texture.");

	SDL_FreeSurface(surface);

	return texture;
}


// Creates a texture from text, given a font and a color.
SDL_Texture* SDLA_CreateTextTexture(TTF_Font *font, SDL_Color *color, char *text)
{
	if (font == NULL)
		SDLA_ExitWithError("Font not loaded.");

	if (color == NULL)
		SDLA_ExitWithError("Color not set.");

	if (text == NULL || text[0] == '\0')
		return NULL;

	SDL_Surface *surface;

	if (FontAliasing == SDLA_BLENDED)
		surface = TTF_RenderText_Blended(font, text, *color);
	else
		surface = TTF_RenderText_Solid(font, text, *color);

	if (surface == NULL)
		SDLA_ExitWithError("Impossible to create the surface.");

	SDL_Texture *texture = SDL_CreateTextureFromSurface(Renderer, surface);

	if (texture == NULL)
		SDLA_ExitWithError("Impossible to create the texture.");

	SDL_FreeSurface(surface);

	return texture;
}


////////////////////////////////////////////////////////////////////////////////////
// Drawing:
////////////////////////////////////////////////////////////////////////////////////

// This needs a SDLA_Init() call in order to work.


// Sets the current drawing RGB color:
void SDLA_SetDrawColor(Uint8 r, Uint8 g, Uint8 b)
{
	if (SDL_SetRenderDrawColor(Renderer, r, g, b, 255) != 0)
		SDLA_ExitWithError("Impossible to set the renderer color.");
}


// Clears the current window with the given RGB color. If 'color' is NULL, the window gets black:
void SDLA_ClearWindow(SDL_Color *color)
{
	if (color == NULL)
		SDLA_SetDrawColor(0, 0, 0); // default: black.

	else
		SDLA_SetDrawColor(color -> r, color -> g, color -> b);

	if (SDL_RenderClear(Renderer) != 0)
		SDLA_ExitWithError("Impossible to clear the window.");
}


// Draws the texture at the given coordinates, where (0, 0) is at the window's top left corner,
// and the y-axis goes to the bottom of the screen. If one wants to center the texture horizontally,
// vertically, or both, the corresponding coordinates need to be replaced by SDLA_CENTERED.
void SDLA_DrawTexture(SDL_Texture *texture, int x, int y)
{
	if (texture == NULL)
		return;

	SDL_Rect rectangle;

	if (SDL_QueryTexture(texture, NULL, NULL, &rectangle.w, &rectangle.h) != 0)
		SDLA_ExitWithError("Impossible to find the texture size.");

	int window_width, window_height;

	SDL_GetWindowSize(Window, &window_width, &window_height);

	rectangle.x = x == SDLA_CENTERED ? (window_width - rectangle.w) / 2 : x;
	rectangle.y = y == SDLA_CENTERED ? (window_height - rectangle.h) / 2 : y;

	if (SDL_RenderCopy(Renderer, texture, NULL, &rectangle) != 0)
		SDLA_ExitWithError("Impossible to draw the texture.");
}


// Draws the text at the given coordinates. Same centering options as in SDLA_DrawTexture() are supported.
// The text must not contain any carriage return. This is slow due to the text's texture being dynamically
// created and freed. It is more efficient to create the texture beforehand and draw it when needed using
// SDLA_DrawTexture(). For drawing dynamic texts, using the cached font method (see below) may be more efficient...
void SDLA_SlowDrawText(TTF_Font *font, SDL_Color *color, int x, int y, char *text)
{
	SDL_Texture *text_texture = SDLA_CreateTextTexture(font, color, text);

	SDLA_DrawTexture(text_texture, x, y);

	SDL_DestroyTexture(text_texture);
}


// Returns the horizontal size in pixels that the given text would take if drawn.
int SDLA_TextSize(TTF_Font *font, char *text)
{
	if (font == NULL)
		SDLA_ExitWithError("Font not loaded.");

	if (text == NULL)
		return 0;

	int size;

	if (TTF_SizeText(font, text, &size, NULL) != 0)
		SDLA_ExitWithError("Impossible to find the text size.");

	return size;
}


////////////////////////////////////////////////////////////////////////////////////
// Text input:
////////////////////////////////////////////////////////////////////////////////////

// Method for getting a string from user inputs. Useful for entering names, and so on.
// This needs a SDLA_InitTextInput() call in order to work.


// Resets the text_input string to an empty string.
void SDLA_ResetTextInput(char *text_input)
{
	if (!TextInputInit)
		SDLA_ExitWithError("SDLA's text input subsystem is not initialized.");

	for (int i = 0; i < TextInputLen; ++i)
		text_input[i] = '\0';

	Cursor = 0;
}


// Updates the text_input string with user input. Returns 1 if text_input is modified, 0 otherwise.
int SDLA_UpdateTextInput(char *text_input, SDL_Event *event)
{
	if (!TextInputInit)
		SDLA_ExitWithError("SDLA's text input subsystem is not initialized.");

	if (event == NULL)
		SDLA_ExitWithError("Invalid event address.");

	if (event -> type == SDL_KEYDOWN) // For deleting one char at a time.
	{
		if (event -> key.keysym.sym == SDLK_BACKSPACE)
		{
			if (Cursor < TextInputLen - 1 || text_input[Cursor] == '\0')
				Cursor = Max(0, Cursor - 1);

			text_input[Cursor] = '\0';
		}

		return 1;
	}

	else if (event -> type == SDL_TEXTINPUT)
	{
		char input_char = (event -> text.text)[0];

		if (CHAR_MIN <= input_char && input_char <= CHAR_MAX)
		{
			text_input[Cursor] = input_char;
			Cursor = Min(TextInputLen - 1, Cursor + 1);
		}

		return 1;
	}

	else
		return 0;
}


// Useful if the user wants to replace the content of 'text_input'.
void SDLA_SetTextInputContent(char *text_input, char *new_content)
{
	if (new_content == NULL)
		return;

	int length = strlen(new_content);

	Cursor = Min(TextInputLen - 1, length);

	for (int i = 0; i < TextInputLen; ++i)
		text_input[i] = i < Cursor ? new_content[i] : '\0'; // cleaning the rest of the string.
}


////////////////////////////////////////////////////////////////////////////////////
// Font caching:
////////////////////////////////////////////////////////////////////////////////////

// Font caching method whose purpose is to draw dynamic text more proficiently.
// This needs a SDLA_Init() call in order to work.


// Cache characters (from ch_min to ch_max) of the given font as separated textures,
// in order to draw them dynamically and efficiently.
CachedFont* SDLA_CachingFontByRange(char *font_name, short size, SDL_Color *color, char ch_min, char ch_max)
{
	if (color == NULL)
		SDLA_ExitWithError("Color not set.");

	TTF_Font *font = TTF_OpenFont(font_name, size);

	if (font == NULL)
		SDLA_ExitWithError("Impossible to load the font.");

	ch_min = (char) Max(ch_min, CHAR_MIN);
	ch_max = (char) Min(ch_max, CHAR_MAX);

	CachedFont *cache = (CachedFont*) calloc(1, sizeof(CachedFont));

	cache -> Size = size;
	cache -> Height = TTF_FontHeight(font);
	cache -> Ch_min = ch_min;
	cache -> Ch_max = ch_max;

	int number = ch_max - ch_min + 1;

	cache -> Table = (Glyph*) calloc(number, sizeof(Glyph));

	if (cache -> Table == NULL)
		SDLA_ExitWithError("Impossible to allocate enough memory for 'cache -> Table'.");

	for (int i = 0; i < number; ++i)
	{
		SDL_Surface *surface;

		if (FontAliasing == SDLA_BLENDED)
			surface = TTF_RenderGlyph_Blended(font, i + ch_min, *color);
		else
			surface = TTF_RenderGlyph_Solid(font, i + ch_min, *color);

		if (surface == NULL)
			SDLA_ExitWithError("Impossible to create the surface.");

		Glyph *glyph = cache -> Table + i;

		glyph -> Texture = SDL_CreateTextureFromSurface(Renderer, surface);

		if (glyph -> Texture == NULL)
			SDLA_ExitWithError("Impossible to create the texture.");

		SDL_FreeSurface(surface);

		if (TTF_GlyphMetrics(font, i + ch_min, &(glyph -> xMin), &(glyph -> xMax), NULL, NULL, NULL) == -1)
			SDLA_ExitWithError("Impossible to find the glyphe size.");
	}

	TTF_CloseFont(font);

	return cache;
}


// Cache all supported characters of the given font as separated textures.
CachedFont* SDLA_CachingFontAll(char *font_name, short size, SDL_Color *color)
{
	return SDLA_CachingFontByRange(font_name, size, color, CHAR_MIN, CHAR_MAX);
}


// Draws the given text at the given coordinates, with the cached font method.
// Unsupported: automatic carriage return. DO NOT use this with SDLA_CENTERED.
void SDLA_DrawCachedFont(CachedFont *cached_font, int x, int y, char *text)
{
	if (cached_font == NULL)
		SDLA_ExitWithError("Font not cached.");

	if (text == NULL)
		return;

	int size = cached_font -> Size, height = cached_font -> Height;
	int xStep = size / 25, xSpace = size / 3, yStep = height;
	int xPos = x, yPos = y, i = 0;

	while (text[i] != '\0')
	{
		char current_ch = text[i];
		char next_ch = text[i+1];

		if (current_ch == '\n')
		{
			xPos = x;
			yPos += yStep;
		}

		else if (current_ch == ' ')
			xPos += xSpace;

		else if (current_ch == '\t')
			xPos += 4 * xSpace;

		else if (current_ch < cached_font -> Ch_min || current_ch > cached_font -> Ch_max)
		{
			printf("char: %c %d\n", current_ch, current_ch);
			SDLA_ExitWithError("Unsupported character.");
		}

		else
		{
			Glyph *current_glyph = cached_font -> Table + current_ch - cached_font -> Ch_min;

			SDLA_DrawTexture(current_glyph -> Texture, xPos, yPos);

			if (next_ch >= cached_font -> Ch_min && next_ch <= cached_font -> Ch_max)
			{
				Glyph *next_glyph = cached_font -> Table + next_ch - cached_font -> Ch_min;

				xPos += current_glyph -> xMax - next_glyph -> xMin + xStep;
			}
		}

		++i;
	}
}


// Returns the horizontal size in pixels that the given text would take if drawn, using the cached font method.
// For now, the text needs to not contain a carriage return...
int SDLA_CachedTextSize(CachedFont *cached_font, char *text)
{
	if (cached_font == NULL)
		SDLA_ExitWithError("Font not cached.");

	if (text == NULL || text[0] == '\0')
		return 0;

	int size = cached_font -> Size;
	int xStep = size / 25, xSpace = size / 3;
	int xSize = 0, i = 0;

	while (text[i] != '\0')
	{
		if (text[i] == '\n')
			SDLA_ExitWithError("A carriage return is present in 'text'.");

		else if (text[i] == ' ')
			xSize += xSpace;

		else if (text[i] == '\t')
			xSize += 4 * xSpace;

		else
		{
			Glyph *glyph = cached_font -> Table + text[i] - cached_font -> Ch_min;

			xSize += glyph -> xMax - glyph -> xMin;
		}

		++i;
	}

	return xSize + (i - 1) * xStep;
}


// Frees the given cached font from memory.
void SDLA_FreeCachedFont(CachedFont *cached_font)
{
	if (cached_font == NULL)
		SDLA_ExitWithError("Font not cached.");

	int number = cached_font -> Ch_max - cached_font -> Ch_min + 1;

	for (int i = 0; i < number; ++i)
	{
		Glyph *glyph = cached_font -> Table + i;
		SDL_DestroyTexture(glyph -> Texture);
	}

	free(cached_font -> Table);
	free(cached_font);
}


// Prints the given cached font details in the console.
void SDLA_PrintCachedFontInfo(CachedFont *cached_font)
{
	if (cached_font == NULL)
		SDLA_ExitWithError("Font not cached.");

	int number = cached_font -> Ch_max - cached_font -> Ch_min + 1;
	int size = cached_font -> Size;
	int height = cached_font -> Height;

	printf("size: %d, height: %d\n", size, height);

	for (int i = 0; i < number; ++i)
	{
		Glyph *glyph = cached_font -> Table + i;

		char current_ch = i + cached_font -> Ch_min;
		printf("char: %c, ascii: %d, xMin: %d, xMax: %d\n", current_ch, current_ch, glyph -> xMin, glyph -> xMax);
	}
}
