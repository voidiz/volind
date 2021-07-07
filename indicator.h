#ifndef INDICATOR_H
#define INDICATOR_H

#include <SDL2/SDL.h>

#define IND_W 450
#define IND_H 50
#define FADE_OUT_DURATION_MS 500

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;

    int error;
} indicator_t;

/*
 * Check SDL error, quit on error.
 */
void cse(int ret_val);

/*
 * Check SDL error but ptr. Quit on error, otherwise return ptr.
 */
void *csp(void *possibly_null);

/*
 * Initialize SDL, exits on error.
 */
void init_indicator(indicator_t *i, int w, int h);

void term_indicator(int silent);

/*
 * 0 is empty, 100 is complete.
 */
void draw_indicator(indicator_t *i, int progress, int alpha);

#endif // INDICATOR_H
