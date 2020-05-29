#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "user_inputs.h"


static SDL_Event event;


// Framework for user's inputs:
void input_control(void)
{
	SDL_WaitEvent(&event); // 10 ms wait.

	if (event.type == SDL_QUIT || key_pressed(SDLK_ESCAPE))
		quit = 1;


	// Get the mouse coordinates if the left button is down.
	// N.B: Dont get said coordinates with the 'SDL_GetMouseState' function, as it is inaccurate.
	// Moreover, using said function with NULL args may causes fast mouse clics to go unnoticed...
	else if ((event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEMOTION) && event.button.button == SDL_BUTTON_LEFT)
	{
		// Accurate coordinates:
		int mouse_x = event.motion.x;
		int mouse_y = event.motion.y;


		// "Clear" button:
		if (is_in_rect(&rect_button_clear, mouse_x, mouse_y))
		{
			// Clear the strings:

			for (int i = 0; i < MAX_SYMPTOMS_INPUTS; ++i)
			{
				for (int j = 0; j < MAX_NAME_LENGTH; ++j)
					SymptomArray[i][j] = '\0';
			}

			SDLA_SetTextInputContent(Text_input, "");

			String_index = 0;

			render_scene = 1;
			redraw_scene = 1;
			clear_state = 1;
			warning_code = -1; // default
		}


		// "Go" button -> printing the neural network response:
		else if (is_in_rect(&rect_button_go, mouse_x, mouse_y))
		{
			// Filling a prediagnostic from the text inputs:

			for (int i = 0; i < MAX_SYMPTOMS_INPUTS; ++i)
			{
				prediagnostic -> declaredSymptoms[i] = getSymptomID(SymptomArray[i]);
				prediagnostic -> declaredSymptomsConfidences[i] = 1.f;
			}

			warning_code = getNewWarningCode(prediagnostic);

			// Making an actual diagnostic:

			clear_state = !makeDiagnostic(prediagnostic, NULL);

			render_scene = 1;
			redraw_scene = 1;
		}
	}


	else if (key_pressed(SDLK_DOWN) || key_pressed(SDLK_RETURN))
	{
		String_index = (String_index + 1) % MAX_SYMPTOMS_INPUTS;

		SDLA_SetTextInputContent(Text_input, SymptomArray[String_index]);

		render_scene = 1;
		redraw_scene = 1;
	}


	else if (key_pressed(SDLK_UP))
	{
		String_index = (MAX_SYMPTOMS_INPUTS + String_index - 1) % MAX_SYMPTOMS_INPUTS;

		SDLA_SetTextInputContent(Text_input, SymptomArray[String_index]);

		render_scene = 1;
		redraw_scene = 1;
	}


	// Updates the text_input string with user input. Returns 1 if text_input is modified, 0 otherwise.
	else if (SDLA_UpdateTextInput(Text_input, &event))
	{
		sprintf(SymptomArray[String_index], "%s", Text_input);

		render_scene = 1;
		redraw_scene = 1;
	}
}


// Check if the given coordinates are inside the given rectangle:
inline int is_in_rect(const SDL_Rect *rect, int x, int y)
{
	return (rect -> x <= x && x <= (rect -> x + rect -> w))
		&& (rect -> y <= y && y <= (rect -> y + rect -> h));
}


// Returns 1 if the given key is actually pressed.
inline int key_pressed(SDL_Keycode key)
{
	// return event.type == SDL_KEYDOWN && event.key.keysym.sym == key;
	return event.type == SDL_KEYDOWN && event.key.keysym.sym == key && !event.key.repeat;
}


// Returns th code for a warning:
// -1 -> no warning
// MAX_SYMPTOMS_INPUTS -> enter a symptom warning
// else -> index of the incorrect input string
int getNewWarningCode(const PreDiagnostic *prediag)
{
	int count = 0;

	for (int i = 0; i < MAX_SYMPTOMS_INPUTS; ++i)
	{
		if (prediag -> declaredSymptoms[i] == no_symptom)
		{
			if (isNonEmpty(SymptomArray[i]))
				return i;

			++count;
		}
	}

	return count == MAX_SYMPTOMS_INPUTS ? MAX_SYMPTOMS_INPUTS : -1;
}


int isNonEmpty(const char *str)
{
	return str && *str != '\0' && *str != ' ';
}
