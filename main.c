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

static int draw() {
    SDL_ShowWindow(ind.window);

    float vol_diff;
    float previous_vol = iterate_and_get_volume(&aud, 0);
    do {
        float cur_vol = iterate_and_get_volume(&aud, 0);

        // If the volume changes during animation, directly skip
        // to the final value of the animation
        if (previous_vol != cur_vol) {
            ind.progress = previous_vol;
        }

        previous_vol = cur_vol;

        vol_diff = cur_vol - ind.progress;

        float delta_vol = 1.0f;
        if (fabsf(vol_diff) < 1.0f) {
            delta_vol = vol_diff;
        } else if (vol_diff <= -1.0f) {
            delta_vol = -1.0f;
        }

        draw_indicator(&ind, ind.progress);
        SDL_RenderPresent(ind.renderer);
        SDL_Delay(FRAME_TIME);
        ind.progress += delta_vol;
    } while (vol_diff != 0.0f);

    return SDL_GetTicks();
}

static int run() {
    // Exits on fail
    init_indicator(&ind, IND_W, IND_H);

    if ((ind.progress = init_audio(&aud)) < 0) {
        fprintf(stderr, "init_audio() error\n");
        quit(1);
    }

    float hide_tick = SDL_GetTicks();
    int block = 1;
    for (;;) {
        // Block until new volume if not counting down until hide
        iterate_and_get_volume(&aud, block);

        // Animate to new volume (aud.cur_vol) and hide indicator when done
        // (fades out depending on compositor)
        block = 0;
        if (ind.progress != aud.cur_vol) {
            hide_tick = draw();
        } else {
            float hide_progress =
                (SDL_GetTicks() - hide_tick) / FADE_OUT_DURATION_MS;
            if (hide_progress < 1) {
                draw_indicator(&ind, ind.progress);
                SDL_RenderPresent(ind.renderer);
                SDL_Delay(FRAME_TIME);
            } else {
                SDL_HideWindow(ind.window);
                block = 1;
            }
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
