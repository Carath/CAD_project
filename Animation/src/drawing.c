#include <stdio.h>
#include <stdlib.h>

#include "drawing.h"


#define X_MARGIN 50
#define Y_MARGIN 0
#define Y_STEP 25
#define SIZE_GAP 100
#define Y_BIG_STEP 100
#define TEXT_SHIFT 175
#define SYMPTOM_GAP 160
#define CONFID_GAP 325
#define HEADLINE_YSTEP 45
#define FINAL_LAYER_STEP 100


#define BUTTON_OFFSET ((WINDOW_WIDTH - BUTTON_WIDTH) / 2)

static const int X_guess = (WINDOW_WIDTH + BUTTON_WIDTH) / 2 + SIZE_GAP;

SDL_Rect rect_button_clear =
{
	.x = BUTTON_OFFSET,
	.y = 0, // will be set during runtime.
	.w = BUTTON_WIDTH,
	.h = BUTTON_HEIGHT,
};

SDL_Rect rect_button_go =
{
	.x = BUTTON_OFFSET,
	.y = 0, // will be set during runtime.
	.w = BUTTON_WIDTH,
	.h = BUTTON_HEIGHT,
};


void draw_neurons_line(SDL_Rect *rectangle, const NeuronLayer *layer,
	int neurons_number_to_draw, int line_start_Xpos, int start)
{
	rectangle -> x = line_start_Xpos;

	for (int i = 0; i < neurons_number_to_draw; ++i)
	{
		if (!clear_state)
		{
			// Neurons activation:

			int channel_color = convert(layer -> Output[start + i]);

			SDLA_SetDrawColor(0, channel_color, 0);

			SDL_RenderFillRect(renderer, rectangle);
		}

		// Neurons frame:

		SDLA_SetDrawColor(0, 128, 0);

		SDL_RenderDrawRect(renderer, rectangle);

		rectangle -> x += NEURON_SIZE;
	}
}


// Returns the Y-coordinate at which the network drawing ends:
int draw_network(const NeuralNetwork *network)
{
	NeuronLayer *layer = network -> Layers;

	SDL_Rect rectangle = {0, Y_MARGIN, NEURON_SIZE, NEURON_SIZE};

	for (int l = 0; l < network -> LayersNumber; ++l)
	{
		rectangle.y += Y_STEP;

		// Neurons per line, unless said line is the last one:
		const int neurons_per_line = MIN(layer -> NeuronsNumber, (WINDOW_WIDTH - 2 * X_MARGIN) / NEURON_SIZE);

		const int neurons_line_number = 1 + layer -> NeuronsNumber / neurons_per_line;

		const int line_start_Xpos = MAX((WINDOW_WIDTH - neurons_per_line * NEURON_SIZE) / 2, X_MARGIN);

		sprintf(layer_str, "Layer %d activations: %d neurons", l + 1, layer -> NeuronsNumber);

		SDLA_SlowDrawText(font_medium, &Yellow, SDLA_CENTERED, rectangle.y, layer_str);

		rectangle.y += 2 * Y_STEP;

		int neurons_to_draw_left = layer -> NeuronsNumber;

		for (int k = 0; k < neurons_line_number; ++k)
		{
			int neurons_to_draw_now = MIN(neurons_to_draw_left, neurons_per_line);

			draw_neurons_line(&rectangle, layer, neurons_to_draw_now, line_start_Xpos, k * neurons_per_line);

			neurons_to_draw_left -= neurons_per_line;

			rectangle.y += NEURON_SIZE;
		}

		++layer;

		if (l == network -> LayersNumber - 1)
		{
			rectangle.y -= 10;

			// Too much illness for this to be printed:
			// SDLA_SlowDrawText(font_small, &Yellow, line_start_Xpos + 2, rectangle.y, "0 1 2 3 4");
		}
	}

	return rectangle.y + TTF_FontHeight(font_small);
}


// Main drawing function:
void drawing(void)
{
	if (!render_scene || !redraw_scene)
		return;

	// For monitoring the drawing time of each frame:
	// double time_1 = get_time();

	// Window is cleared only when the entire scene needs to be redrawn. This causes flickering
	// when used with hardware acceleration, but assures that the user drawings have no lag:
	SDLA_ClearWindow(NULL);

	////////////////////////////////////////////////////////////
	// Drawing the neural network activations. Needs to be done before drawing the buttons!

	const NeuralNetwork *network_loaded = getLoaded_NeuralNetwork();

	rect_button_clear.y = draw_network(network_loaded) + Y_BIG_STEP;
	rect_button_go.y = rect_button_clear.y + BUTTON_HEIGHT;

	int X_stringPos = BUTTON_OFFSET - SYMPTOM_GAP - SIZE_GAP;
	int Y_stringPos = rect_button_go.y - FINAL_LAYER_STEP;

	////////////////////////////////////////////////////////////
	// Drawing the button 'Clear':

	SDLA_SetDrawColor(192, 0, 0);
	SDL_RenderFillRect(renderer, &rect_button_clear);

	SDLA_SetDrawColor(255, 255, 0);
	SDL_RenderDrawRect(renderer, &rect_button_clear);

	////////////////////////////////////////////////////////////
	// Drawing the button 'Go':

	SDLA_SetDrawColor(0, 192, 0);
	SDL_RenderFillRect(renderer, &rect_button_go);

	SDLA_SetDrawColor(255, 255, 0);
	SDL_RenderDrawRect(renderer, &rect_button_go);

	////////////////////////////////////////////////////////////
	// Static text drawing:

	SDLA_SlowDrawText(font_medium, &Yellow, rect_button_clear.x + 12, rect_button_clear.y + 30, "Clear");
	SDLA_SlowDrawText(font_medium, &Yellow, rect_button_go.x + 25, rect_button_go.y + 30, "Go!");
	SDLA_SlowDrawText(font_medium, &Yellow, X_stringPos - TEXT_SHIFT, Y_stringPos - HEADLINE_YSTEP, "Type your symptoms!");
	SDLA_SlowDrawText(font_medium, &Yellow, X_guess, Y_stringPos - HEADLINE_YSTEP, "Guesses:");
	SDLA_SlowDrawText(font_medium, &Yellow, X_guess + CONFID_GAP, Y_stringPos - HEADLINE_YSTEP, "Confidence:");

	////////////////////////////////////////////////////////////
	// User inputs:

	const int height_medium = TTF_FontHeight(font_medium);
	const int Xsympt = X_stringPos - TEXT_SHIFT;

	for (int i = 0; i < MAX_SYMPTOMS_INPUTS; ++i)
	{
		SDLA_SlowDrawText(font_medium, &White, Xsympt - 20, Y_stringPos + height_medium * i, "- ");
		SDLA_SlowDrawText(font_medium, &White, Xsympt, Y_stringPos + height_medium * i, SymptomArray[i]);
	}

	const int current_length = SDLA_TextSize(font_medium, SymptomArray[String_index]);

	SDLA_SlowDrawText(font_medium, &Yellow, Xsympt + current_length, Y_stringPos + height_medium * String_index, "_");

	////////////////////////////////////////////////////////////
	// Warning message:

	const int criticity_height = Y_stringPos + height_medium * DIAG_ILLNESS_NUMBER + 50;

	if (warning_code >= MAX_SYMPTOMS_INPUTS)
		SDLA_SlowDrawText(font_medium, &Yellow, X_stringPos - TEXT_SHIFT, criticity_height + 40, "Please enter some symptoms...");

	else if (warning_code >= 0)
	{
		sprintf(WarningIncorrectSymptom, "Incorrect symptom: %s", SymptomArray[warning_code]);

		SDLA_SlowDrawText(font_medium, &Yellow, X_stringPos - TEXT_SHIFT, criticity_height + 50, WarningIncorrectSymptom);
	}

	warning_code = -1; // default

	////////////////////////////////////////////////////////////
	// Network answer:

	if (!clear_state)
	{
		const Diagnostic *filledDiagnostic = getFilledDiagnostic();

		for (int i = 0; i < DIAG_ILLNESS_NUMBER; ++i)
		{
			const int guess_height = Y_stringPos + height_medium * i;

			const char* illness_name = getIllnessName(filledDiagnostic -> illnessArray[i]);
			char *format = (strlen(illness_name) > MAX_NAME_DRAWING_LENGTH) ? "%.*s..." : "%.*s";

			sprintf(IllnessArray[i], format, MAX_NAME_DRAWING_LENGTH, illness_name);
			sprintf(GuessesArray[i], "%.0f %%", 100.f * filledDiagnostic -> illnessProbabilityArray[i]);

			SDLA_SlowDrawText(font_medium, &White, X_guess, guess_height, IllnessArray[i]);
			SDLA_SlowDrawText(font_medium, &White, X_guess + 50 + CONFID_GAP, guess_height, GuessesArray[i]);
		}

		sprintf(CriticityLevel, "%.0f %%", 100.f * filledDiagnostic -> criticity);

		SDLA_SlowDrawText(font_medium, &Yellow, X_guess, criticity_height, "Criticity level:");
		SDLA_SlowDrawText(font_medium, &White, X_guess + 60, criticity_height + HEADLINE_YSTEP, CriticityLevel);
	}

	redraw_scene = 0;

	// double time_2 = get_time();
	// printf("Frame time: %.3f ms\n", 1000. * (time_2 - time_1));
}


// Maps values from the real line to [0, 255]:
inline int convert(Number value)
{
	if (value > 1)
		return 255;

	if (value < 0)
		return 0;

	return 255 * value;
}
