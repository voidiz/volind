#include "indicator.h"
#include "debug.h"
#include <stdio.h>

void cse(int ret_val) {
    if (ret_val < 0) {
        term_indicator(0);
        exit(1);
    }
}

void *csp(void *possibly_null) {
    if (possibly_null == NULL) {
        term_indicator(0);
        exit(1);
    }

    return possibly_null;
}

void term_indicator(int silent) {
    if (!silent) {
        fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
    }

    SDL_Quit();
}

void draw_indicator(indicator_t *i, int progress) {
    // Background
    SDL_SetRenderDrawColor(i->renderer, BACKGROUND_COLOR, 255);
    SDL_RenderClear(i->renderer);

    // Bar
    SDL_SetRenderDrawColor(i->renderer, BAR_COLOR, 255);
    SDL_Rect bar = {.x = BORDER_SIZE,
                    .y = BORDER_SIZE,
                    .w = (IND_W - 2 * BORDER_SIZE) * (progress / 100.0f),
                    .h = IND_H - 2 * BORDER_SIZE};
    SDL_RenderFillRect(i->renderer, &bar);
}

void init_indicator(indicator_t *i, int w, int h) {
    cse(SDL_Init(SDL_INIT_VIDEO));
    i->window = csp(SDL_CreateWindow(
        "volind", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h,
        SDL_WINDOW_BORDERLESS | SDL_WINDOW_TOOLTIP));
    i->renderer = csp(SDL_CreateRenderer(i->window, -1, 0));
    cse(SDL_SetRenderDrawBlendMode(i->renderer, SDL_BLENDMODE_BLEND));

    i->progress = -1.0f;
}
