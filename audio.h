#ifndef AUDIO_H
#define AUDIO_H

#include <pulse/pulseaudio.h>

typedef struct {
    pa_context          *ctx;
    pa_mainloop         *m_loop;
    pa_mainloop_api     *m_loop_api;

    float cur_vol;
    int error;
} audio_t;

void
sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata);

void
subscription_callback(pa_context *c, pa_subscription_event_type_t t, 
        uint32_t idx, void *userdata);

void
context_state_callback(pa_context *c, void *userdata);

audio_t
init_audio();

void
term_audio(audio_t *a);

#endif
