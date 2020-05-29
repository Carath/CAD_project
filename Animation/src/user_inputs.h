#ifndef USER_INPUTS_H
#define USER_INPUTS_H


#include "SDLA.h"
#include "animation_settings.h"


extern SDL_Renderer *renderer;

extern int quit;
extern int render_scene;
extern int redraw_scene;
extern int clear_state;
extern int warning_code;

extern SDL_Rect rect_button_clear;
extern SDL_Rect rect_button_go;
extern SDL_Rect rect_frame;

extern char *Text_input;
extern int String_index;

extern char SymptomArray[MAX_SYMPTOMS_INPUTS][MAX_NAME_LENGTH + 1];

extern PreDiagnostic *prediagnostic;


// Framework for user's inputs:
void input_control(void);


// Check if the given coordinates are inside the given rectangle:
int is_in_rect(const SDL_Rect *rect, int x, int y);


// Returns 1 if the given key is actually pressed.
int key_pressed(SDL_Keycode key);


// Returns th code for a warning:
// -1 -> no warning
// MAX_SYMPTOMS_INPUTS -> enter a symptom warning
// else -> index of the incorrect input string
int getNewWarningCode(const PreDiagnostic *prediag);


int isNonEmpty(const char *str);


#endif
