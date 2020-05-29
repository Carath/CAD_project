#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "animation.h"
#include "drawing.h"
#include "user_inputs.h"


// Global variables:

SDL_Window *window;
SDL_Renderer *renderer;

TTF_Font *font_big;
TTF_Font *font_medium;
TTF_Font *font_small;

SDL_Color Yellow = {255, 255, 0, 255};
SDL_Color White = {255, 255, 255, 255};

int quit = 0;
int render_scene = 1;
int redraw_scene = 1;
int clear_state = 1;
int warning_code = -1; // default

char layer_str[50];

char *Text_input;
int String_index = 0;

char SymptomArray[MAX_SYMPTOMS_INPUTS][MAX_NAME_LENGTH + 1];
char IllnessArray[DIAG_ILLNESS_NUMBER][MAX_NAME_LENGTH + 1];
char GuessesArray[DIAG_ILLNESS_NUMBER][10];
char CriticityLevel[10];
char WarningIncorrectSymptom[MAX_NAME_LENGTH + 50];

PreDiagnostic *prediagnostic;


// Animating the recognition of the medical dataset:
void animation(void)
{
	////////////////////////////////////////////////////////////
	// Initializing SDLA - rendering:

	SDLA_Init(&window, &renderer, "Doc9000 animation", WINDOW_WIDTH, WINDOW_HEIGHT,
		USE_HARDWARE_ACCELERATION, SDLA_BLENDED);

	// Initializing SDLA - text input:
	Text_input = SDLA_InitTextInput(MAX_NAME_LENGTH);

	////////////////////////////////////////////////////////////
	// Loading fonts:

	const char font_name[] = "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf";

	font_big = TTF_OpenFont(font_name, 50);
	font_medium = TTF_OpenFont(font_name, 25);
	font_small = TTF_OpenFont(font_name, 15);

	////////////////////////////////////////////////////////////
	// Loading internally the neural network and inputs to fill:

	initRecognitionRessources();

	////////////////////////////////////////////////////////////
	// Creating a prediagnostic to fill:

	prediagnostic = allocatePreDiagnostic(MAX_SYMPTOMS_INPUTS);

	prediagnostic -> patientConfidenceLevel = 1.f; // default.

	////////////////////////////////////////////////////////////

	// int frame_index = 0; // for monitoring the frames rendering.

	while (!quit)
	{
		input_control(); // causes a 10 ms wait.

		drawing();

		// Rendering:
		if (render_scene)
		{
			// printf("rendering: %d\n", frame_index);
			// ++frame_index;

			SDL_RenderPresent(renderer);

			render_scene = 0;
		}
	}

	// Freeing everything:

	freeCriticityArray();
	freeStringsArrays();

	freePreDiagnostic(&prediagnostic);
	freeRecognitionRessources();

	TTF_CloseFont(font_small);
	TTF_CloseFont(font_medium);
	TTF_CloseFont(font_big);

	free(Text_input);

	SDLA_Quit();
}
