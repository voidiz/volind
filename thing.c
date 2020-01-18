#include "audio.h"
#include "indicator.h"

#include <stdio.h>
#include <time.h>

audio_t aud;
indicator_t ind;

void
quit()
{
    term_audio(&aud);
    term_indicator(&ind);
    exit(0);
}

int
run()
{
    int ret = 1;
    struct timespec ts = {0, 50000000};
    float vol_diff, delta_vol, bar_vol = 0;

    aud = init_audio();
    if (aud.error < 0) quit();
    ind = init_indicator(IND_W, IND_H);
    if (ind.error < 0) quit();

    for (;;) {
        if (pa_mainloop_iterate(aud.m_loop, 0, &ret) < 0)
            fprintf(stderr, "pa_mainloop_iterate() error\n");

        delta_vol = 1.0;
        vol_diff = aud.cur_vol - bar_vol;
        if (vol_diff < 1.0 && vol_diff > -1) {
            delta_vol = vol_diff;
        } else if (vol_diff <= -1) {
            delta_vol = -1.0;
        }

        fprintf(stderr, "vol_diff: %f\n", vol_diff);

        if (vol_diff != 0) {
            cairo_push_group(ind.ctx);
            cairo_set_source_rgb (ind.ctx, 255, 255, 255);
            cairo_paint(ind.ctx);

            cairo_set_line_width (ind.ctx, 5);
            cairo_set_source_rgb (ind.ctx, 255, 0, 0);
            cairo_move_to(ind.ctx, 0 + 10, IND_H - 10);
            cairo_line_to(ind.ctx, (IND_W - 20) * (bar_vol / 100) + 10, 
                    IND_H - 10);
            cairo_stroke (ind.ctx);
            cairo_pop_group_to_source(ind.ctx);
            cairo_paint(ind.ctx);
            cairo_surface_flush(ind.sfc);

            bar_vol += delta_vol;
        }

        nanosleep(&ts, NULL);
    }

    return ret;
}

void
sig_handler(int signum)
{
    fprintf(stderr, "Terminating...\n");
    quit();
}

int
main(int argc, char *argv[])
{
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_INTERRUPT;
    sigaction(SIGINT, &sa, NULL);

    return run();
}
