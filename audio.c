#include "audio.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>

static void sink_info_callback(pa_context *c, const pa_sink_info *i, int eol,
                               void *userdata) {
    if (eol > 0) {
        return;
    }

    audio_t *a = userdata;
    a->cur_vol =
        (float)pa_cvolume_avg(&(i->volume)) / (float)PA_VOLUME_NORM * 100.0;

    DEBUG_PRINT("Sink: %s, Volume: %.0f%%, Muted: %s\n", i->name, a->cur_vol,
                i->mute ? "yes" : "no");
}

static void server_info_callback(pa_context *c, const pa_server_info *i,
                                 void *userdata) {
    // Query volume of default sink
    DEBUG_PRINT("Default sink: %s\n", i->default_sink_name);
    pa_operation *o;
    o = pa_context_get_sink_info_by_name(c, i->default_sink_name,
                                         &sink_info_callback, userdata);
    pa_operation_unref(o);
}

static void subscription_callback(pa_context *c, pa_subscription_event_type_t t,
                                  uint32_t idx, void *userdata) {
    unsigned int evf = t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;
    if (evf == PA_SUBSCRIPTION_EVENT_SINK) {
        pa_operation *o;
        o = pa_context_get_sink_info_by_index(c, idx, sink_info_callback,
                                              userdata);
        pa_operation_unref(o);
    }
}

static void query_volume(audio_t *a) {
    pa_operation *o;
    o = pa_context_get_server_info(a->ctx, &server_info_callback, a);
    pa_operation_unref(o);
}

static void context_state_callback(pa_context *c, void *userdata) {
    assert(c);
    audio_t *aud = userdata;

    switch (pa_context_get_state(c)) {
    case PA_CONTEXT_SETTING_NAME:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_UNCONNECTED:
    case PA_CONTEXT_AUTHORIZING:
    default:
        aud->pa_ready = 0;
        break;

    case PA_CONTEXT_READY:
        pa_context_set_subscribe_callback(c, subscription_callback, userdata);
        pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, NULL, NULL);
        fprintf(stderr, "Connection established.\n");
        aud->pa_ready = 1;
        query_volume(aud);
        break;

    case PA_CONTEXT_TERMINATED:
        fprintf(stderr, "Connection terminated.\n");
        break;

    case PA_CONTEXT_FAILED:
        fprintf(stderr, "Connection failed: %s\n",
                pa_strerror(pa_context_errno(c)));
        break;
    }
}

void term_audio(audio_t *a) {
    if (a->ctx) {
        pa_context_disconnect(a->ctx);
        pa_context_unref(a->ctx);
        a->ctx = NULL;
    }

    if (a->m_loop) {
        pa_mainloop_free(a->m_loop);
        a->m_loop = NULL;
    }
}

float iterate_and_get_volume(audio_t *a, int block) {
    if (pa_mainloop_iterate(a->m_loop, block, NULL) < 0) {
        fprintf(stderr, "pa_mainloop_iterate() failed\n");
        term_audio(a);
        exit(1);
    }

    return a->cur_vol;
}

float init_audio(audio_t *a) {
    a->m_loop = pa_mainloop_new();
    if (!a->m_loop) {
        fprintf(stderr, "pa_mainloop_new() failed\n");
        return -1;
    }

    a->m_loop_api = pa_mainloop_get_api(a->m_loop);
    if (!a->m_loop_api) {
        fprintf(stderr, "pa_mainloop_get_api() failed\n");
        return -1;
    }

    a->ctx = pa_context_new(a->m_loop_api, "Volume monitor");
    if (!a->ctx) {
        fprintf(stderr, "pa_context_new() failed\n");
        return -1;
    }

    pa_context_set_state_callback(a->ctx, context_state_callback, a);
    if (pa_context_connect(a->ctx, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) < 0) {
        fprintf(stderr, "pa_context_connect() failed\n");
        return -1;
    }

    a->pa_ready = 0;
    a->cur_vol = -1.0f;

    for (;;) {
        // Block until we get the first volume value
        iterate_and_get_volume(a, 1);

        // Break when pulseaudio is ready and the initial volume has been
        // queried
        if (a->pa_ready && a->cur_vol != -1.0f) {
            break;
        }
    }

    return a->cur_vol;
}
