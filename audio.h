#ifndef AUDIO_H
#define AUDIO_H

#include <pulse/pulseaudio.h>

typedef struct {
    pa_context *ctx;
    pa_mainloop *m_loop;
    pa_mainloop_api *m_loop_api;

    /*
     * Should not be accessed directly, use get_volume().
     */
    float cur_vol;

    int pa_ready;
} audio_t;

int init_audio(audio_t *a);

void term_audio(audio_t *a);

/*
 * Returns the volume (0-100) or -1 on error.
 * block specifies whether the operation should block until the volume changes
 * (0 does not block, nonzero blocks).
 */
float get_volume(audio_t *a, int block);

#endif
