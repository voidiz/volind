#include "audio.h"
#include "debug.h"
#include "indicator.h"

#include <stdio.h>

static audio_t aud;
static indicator_t ind;

static void quit(int code) {
    term_audio(&aud);
    term_indicator(1);
    exit(code);
}

static void draw(float *bar_vol) {
    SDL_ShowWindow(ind.window);

    float vol_diff;
    do {
        float cur_vol = get_volume(&aud, 0);
        if (cur_vol < 0) {
            quit(1);
        }

        vol_diff = cur_vol - *bar_vol;

        float delta_vol = 1.0f;
        if (fabsf(vol_diff) < 1.0f) {
            delta_vol = vol_diff;
        } else if (vol_diff <= -1.0f) {
            delta_vol = -1.0f;
        }

        // Background
        SDL_SetRenderDrawColor(ind.renderer, 255, 255, 255, 0);
        SDL_RenderClear(ind.renderer);

        // Bar
        SDL_SetRenderDrawColor(ind.renderer, 255, 0, 0, 0);
        SDL_Rect bar = {.x = 10,
                        .y = IND_H - 20,
                        .w = (IND_W - 20) * (*bar_vol / 100),
                        .h = 5};
        SDL_RenderFillRect(ind.renderer, &bar);

        SDL_RenderPresent(ind.renderer);
        SDL_Delay(20);
        *bar_vol += delta_vol;
    } while (vol_diff != 0.0f);

    SDL_Delay(500);
    SDL_HideWindow(ind.window);
}

static int run() {
    // Exits on fail
    init_indicator(&ind, IND_W, IND_H);

    if (init_audio(&aud) < 0) {
        fprintf(stderr, "init_audio() error\n");
        quit(1);
    }

    float bar_vol = 0;
    for (;;) {
        // Block until new volume
        if (get_volume(&aud, 1) < 0) {
            quit(1);
        }

        // Iterate again if not ready
        if (!aud.pa_ready) {
            continue;
        }

        // Animate to new volume (aud.cur_vol) from bar_vol
        // and update bar_vol
        if (bar_vol != aud.cur_vol) {
            draw(&bar_vol);
        }
    }

    return 0;
}

static void sig_handler() {
    fprintf(stderr, "Terminating...\n");
    quit(0);
}

int main(int argc, char *argv[]) {
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_INTERRUPT;
    sigaction(SIGINT, &sa, NULL);

    return run();
}
