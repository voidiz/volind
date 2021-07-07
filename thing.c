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

static int draw(float *bar_vol) {
    SDL_ShowWindow(ind.window);

    float vol_diff;
    float previous_vol = iterate_and_get_volume(&aud, 0);
    do {
        float cur_vol = iterate_and_get_volume(&aud, 0);

        // If the volume changes during animation, directly skip
        // to the final value of the animation
        if (previous_vol != cur_vol) {
            *bar_vol = previous_vol;
        }

        previous_vol = cur_vol;

        vol_diff = cur_vol - *bar_vol;

        float delta_vol = 1.0f;
        if (fabsf(vol_diff) < 1.0f) {
            delta_vol = vol_diff;
        } else if (vol_diff <= -1.0f) {
            delta_vol = -1.0f;
        }

        draw_indicator(&ind, *bar_vol, 0);
        SDL_RenderPresent(ind.renderer);
        SDL_Delay(20);
        *bar_vol += delta_vol;
    } while (vol_diff != 0.0f);

    return SDL_GetTicks();
}

static int run() {
    // Exits on fail
    init_indicator(&ind, IND_W, IND_H);

    if (init_audio(&aud) < 0) {
        fprintf(stderr, "init_audio() error\n");
        quit(1);
    }

    float bar_vol = -1.0f;
    float fade_out_tick = SDL_GetTicks();
    int block = 1;
    for (;;) {
        // Block until new volume if not fading out
        iterate_and_get_volume(&aud, block);

        // Iterate again if not ready or if volume is still default
        if (!aud.pa_ready || aud.cur_vol == -1.0f) {
            continue;
        }

        // Set initial value
        if (bar_vol < 0.0f) {
            bar_vol = aud.cur_vol;
        }

        // Animate to new volume (aud.cur_vol) from bar_vol
        // and update bar_vol and fade out when done
        block = 0;
        if (bar_vol != aud.cur_vol) {
            fade_out_tick = draw(&bar_vol);
        } else {
            int alpha = (int) (255 * (SDL_GetTicks() - fade_out_tick) / FADE_OUT_DURATION_MS);
            if (alpha <= 255) {
                draw_indicator(&ind, bar_vol, 0);
                SDL_RenderPresent(ind.renderer);
                SDL_Delay(20);
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
