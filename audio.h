#ifndef AUDIO_H
#define AUDIO_H

#include <pulse/pulseaudio.h>

typedef struct {
    pa_context *ctx;
    pa_mainloop *m_loop;
    pa_mainloop_api *m_loop_api;

    float cur_vol;
    int pa_ready;
} audio_t;

int init_audio(audio_t *a);

void term_audio(audio_t *a);

#endif
