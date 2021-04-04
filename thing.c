#include "audio.h"
#include "debug.h"
#include "indicator.h"

#include <stdio.h>
#include <time.h>

static audio_t aud;
static indicator_t ind;

static void quit() {
    term_audio(&aud);
    term_indicator(&ind);
    exit(0);
}

static int run() {
    int ret = 1;
    struct timespec ts = {0, 50000000};
    float vol_diff, delta_vol, bar_vol = 0;

    if (init_audio(&aud) < 0) {
        fprintf(stderr, "init_audio() error\n");
        quit();
    }

    if (init_indicator(&ind, IND_W, IND_H) < 0) {
        fprintf(stderr, "init_indicator() error\n");
        quit();
    }

    for (;;) {
        if (pa_mainloop_iterate(aud.m_loop, 0, &ret) < 0) {
            fprintf(stderr, "pa_mainloop_iterate() error\n");
            quit();
        }

        // Iterate again if not ready
        if (!aud.pa_ready) {
            continue;
        }

        delta_vol = 1.0;
        vol_diff = aud.cur_vol - bar_vol;
        if (vol_diff < 1.0 && vol_diff > -1.0) {
            delta_vol = vol_diff;
        } else if (vol_diff <= -1.0) {
            delta_vol = -1.0;
        }

        if (vol_diff != 0.0) {
            DEBUG_PRINT("vol_diff: %f\n", vol_diff);
            cairo_push_group(ind.ctx);

            // Background
            cairo_set_source_rgb(ind.ctx, 255, 255, 255);
            cairo_paint(ind.ctx);

            // Bar
            cairo_set_line_width(ind.ctx, 5);
            cairo_set_source_rgb(ind.ctx, 255, 0, 0);
            cairo_move_to(ind.ctx, 0 + 10, IND_H - 10);
            cairo_line_to(ind.ctx, (IND_W - 20) * (bar_vol / 100) + 10,
                          IND_H - 10);
            cairo_stroke(ind.ctx);

            cairo_pop_group_to_source(ind.ctx);
            cairo_paint(ind.ctx);
            cairo_surface_flush(ind.sfc);

            bar_vol += delta_vol;
        }

        nanosleep(&ts, NULL);
    }

    return ret;
}

static void sig_handler() {
    fprintf(stderr, "Terminating...\n");
    quit();
}

int main(int argc, char *argv[]) {
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_INTERRUPT;
    sigaction(SIGINT, &sa, NULL);

    return run();
}
