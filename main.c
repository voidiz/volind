#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <pulse/pulseaudio.h>

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

#include <X11/Xlib.h>

// requires libpulse-dev

pa_mainloop* m;
pa_mainloop_api* m_api;
pa_context* c;

cairo_surface_t *sfc;

void quit()
{
    if (c) {
        pa_context_disconnect(c);
        pa_context_unref(c);
    }
    if (m) pa_mainloop_free(m);

    Display *dsp = cairo_xlib_surface_get_display(sfc);
    if (sfc) cairo_surface_destroy(sfc);
    if (dsp) XCloseDisplay(dsp);

    exit(0);
}

int run()
{
    int ret = 1;
    if (pa_mainloop_run(m, &ret) < 0)
        fprintf(stderr, "pa_mainloop_run() failed\n");

    return ret;
}

// BEGIN PULSE

void sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, 
        void *userdata)
{
    if (eol > 0) return;
    float vol = (float)pa_cvolume_avg(&(i->volume)) / (float)PA_VOLUME_NORM * 100.0;
    printf("Sink: %s, Volume: %.0f%%, Muted: %s\n", i->name, vol,
            i->mute ? "yes" : "no");  
}

void subscription_callback(pa_context *c, pa_subscription_event_type_t t,
        uint32_t idx, void *userdata)
{
    unsigned int evf = t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;
    if (evf == PA_SUBSCRIPTION_EVENT_SINK) {
        pa_operation *o;
        o = pa_context_get_sink_info_by_index(c, idx, sink_info_callback, userdata);
        pa_operation_unref(o);
    }
}

void context_state_callback(pa_context *c, void *userdata)
{
    assert(c);
    switch (pa_context_get_state(c))
    {
        case PA_CONTEXT_SETTING_NAME:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_AUTHORIZING:
            break;

        case PA_CONTEXT_READY:
            fprintf(stderr, "Connection established\n");
            pa_context_set_subscribe_callback(c, subscription_callback, userdata);
            pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, NULL, NULL);
            break;

        case PA_CONTEXT_TERMINATED:
            fprintf(stderr, "Connection terminated.\n");
            quit();
            break;

        case PA_CONTEXT_FAILED:
        default:
            fprintf(stderr, "Connection failed: %s\n", pa_strerror(pa_context_errno(c)));
            quit();
            break;
    }
}

int init_pulse()
{
    m = pa_mainloop_new();
    if (!m) {
        fprintf(stderr, "pa_mainloop_new() failed\n");
        return -1; 
    }

    m_api = pa_mainloop_get_api(m);
    if (!m_api) {
        fprintf(stderr, "pa_mainloop_get_api() failed\n");
        return -1; 
    }

    c = pa_context_new(m_api, "Test");
    if (!m_api) {
        fprintf(stderr, "pa_context_new() failed\n");
        return -1; 
    }

    pa_context_set_state_callback(c, context_state_callback, NULL);
    if (pa_context_connect(c, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) < 0) {
        fprintf(stderr, "pa_context_connect() failed\n");
        return -1;
    }

    return 0;
}

// END PULSE

// BEGIN CAIRO

cairo_surface_t *create_x11_surface(int x, int y)
{
    Display *dsp;
    Drawable da;
    int screen;

    if ((dsp = XOpenDisplay(NULL)) == NULL)
        quit();
    screen = DefaultScreen(dsp);
    da = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp),
            0, 0, x, y, 0, 0, 0);
    XSelectInput(dsp, da, ButtonPressMask | KeyPressMask);
    XMapWindow(dsp, da);

    sfc = cairo_xlib_surface_create(dsp, da,
            DefaultVisual(dsp, screen), x, y);
    cairo_xlib_surface_set_size(sfc, x, y);

    return sfc;
}

// END CAIRO

void sig_handler(int signum)
{
    fprintf(stderr, "Terminating...\n");
    quit();
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_INTERRUPT;
    sigaction(SIGINT, &sa, NULL);

    if (init_pulse() < 0)
        quit();

    return run();
}
