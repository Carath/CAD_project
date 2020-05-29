#ifndef DRAWING_H
#define DRAWING_H


#include "SDLA.h"
#include "animation_settings.h"


extern SDL_Renderer *renderer;

extern TTF_Font *font_big;
extern TTF_Font *font_medium;
extern TTF_Font *font_small;

extern SDL_Color Yellow;
extern SDL_Color White;

extern int render_scene;
extern int redraw_scene;
extern int clear_state;
extern int warning_code;

extern char layer_str[50];

extern int String_index;

extern char SymptomArray[MAX_SYMPTOMS_INPUTS][MAX_NAME_LENGTH + 1];
extern char IllnessArray[DIAG_ILLNESS_NUMBER][MAX_NAME_LENGTH + 1];
extern char GuessesArray[DIAG_ILLNESS_NUMBER][10];
extern char CriticityLevel[10];
extern char WarningIncorrectSymptom[MAX_NAME_LENGTH + 50];


void draw_neurons_line(SDL_Rect *rectangle, const NeuronLayer *layer,
	int neurons_number_to_draw, int line_start_pos, int start);


// Returns the Y-coordinate at which the network drawing ends:
int draw_network(const NeuralNetwork *network);


// Main drawing function:
void drawing(void);


// Maps values from the real line to [0, 255]:
int convert(Number value);


#endif
