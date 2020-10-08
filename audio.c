#include "audio.h"

#include <stdio.h>

void
sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, 
        void *userdata)
{
    audio_t *a = userdata;
    if (eol > 0) return;
    /* printf("%.0f%%", (float) pa_cvolume_avg(&(i->volume)) / */ 
    /*     (float) PA_VOLUME_NORM * 100.0); */
    a->cur_vol = (float) pa_cvolume_avg(&(i->volume)) / 
        (float) PA_VOLUME_NORM * 100.0;
    printf("Sink: %s, Volume: %.0f%%, Muted: %s\n", i->name, a->cur_vol,
            i->mute ? "yes" : "no");  
}

void
subscription_callback(pa_context *c, pa_subscription_event_type_t t,
        uint32_t idx, void *userdata)
{
    unsigned int evf = t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;
    if (evf == PA_SUBSCRIPTION_EVENT_SINK) {
        pa_operation *o;
        o = pa_context_get_sink_info_by_index(c, idx, sink_info_callback,
                userdata);
        pa_operation_unref(o);
    }
}

void
context_state_callback(pa_context *c, void *userdata)
{
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
            pa_context_set_subscribe_callback(c, subscription_callback,
                    userdata);
            pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, NULL, NULL);
            fprintf(stderr, "Connection established.\n");
            aud->pa_ready = 1;
            break;

        case PA_CONTEXT_TERMINATED:
            term_audio(aud);
            fprintf(stderr, "Connection terminated.\n");
            break;

        case PA_CONTEXT_FAILED:
            term_audio(aud);
            fprintf(stderr, "Connection failed: %s\n", 
                    pa_strerror(pa_context_errno(c)));
            break;
    }
}

void
term_audio(audio_t *a)
{
    if (a->ctx) {
        pa_context_disconnect(a->ctx);
        pa_context_unref(a->ctx);
    }
    
    if (a->m_loop) pa_mainloop_free(a->m_loop);
}

int
init_audio(audio_t *a)
{
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

    return 0;
}
