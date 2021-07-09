#ifndef INDICATOR_H
#define INDICATOR_H

#include <SDL2/SDL.h>

// Dimensions of the indicator
#define IND_W 450
#define IND_H 50

// Time until the indicator is hidden
#define FADE_OUT_DURATION_MS 500

// Delay between frames (not guaranteed, can be longer)
#define FRAME_TIME 20 // 50 fps

// Indicator border size in pixels
#define BORDER_SIZE 10

// Colors
#define BACKGROUND_COLOR 255, 255, 255
#define BAR_COLOR 102, 153, 204

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;

    /*
     * Value between 0-100, initially -1.0f.
     */
    float progress;

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
void draw_indicator(indicator_t *i, int progress);

#endif // INDICATOR_H
