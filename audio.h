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

/*
 * Returns the current volume, -1.0f on fail.
 */
float init_audio(audio_t *a);

void term_audio(audio_t *a);

/*
 * Runs an iteration of the loop in and returns the (maybe updated) volume.
 * block specifies whether the operation should block until the volume changes
 * (0 does not block, nonzero blocks).
 */
float iterate_and_get_volume(audio_t *a, int block);

#endif // AUDIO_H
