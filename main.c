#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#include <pulse/pulseaudio.h>

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

#include <X11/Xlib.h>

/*
 * compile with 
 * gcc -o thing main.c $(pkg-config --libs --cflags libpulse cairo x11)
 */

#define POPUP_WIDTH 225
#define POPUP_HEIGHT 250

pa_mainloop* m;
pa_mainloop_api* m_api;
pa_context* c;

cairo_surface_t *sfc;
cairo_t *ctx;

float cur_vol, bar_vol;

void quit()
{
    if (c) {
        pa_context_disconnect(c);
        pa_context_unref(c);
    }
    if (m) pa_mainloop_free(m);

    if (ctx) cairo_destroy(ctx);
    Display *dsp = cairo_xlib_surface_get_display(sfc);
    if (sfc) cairo_surface_destroy(sfc);
    if (dsp) XCloseDisplay(dsp);

    exit(0);
}

// BEGIN CAIRO

cairo_surface_t *create_x11_surface(int x, int y)
{
    Display *dsp;
    Drawable da;
    Screen *screen;
    int screen_num;

    if ((dsp = XOpenDisplay(NULL)) == NULL)
        quit();
    screen_num = DefaultScreen(dsp);
    screen = DefaultScreenOfDisplay(dsp);
    da = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp),
            screen->width/2-x/2, screen->height/2-y/2, x, y, 
            0, 0, BlackPixel(dsp, 0));

    /* Make the window manager ignore the window */
    XSetWindowAttributes winattr;
    winattr.override_redirect = 1;
    XChangeWindowAttributes(dsp, da, CWOverrideRedirect, &winattr);

    /* XSelectInput(dsp, da, ButtonPressMask | KeyPressMask); */
    XMapWindow(dsp, da);

    sfc = cairo_xlib_surface_create(dsp, da,
            DefaultVisual(dsp, screen_num), x, y);
    cairo_xlib_surface_set_size(sfc, x, y);

    return sfc;
}

int animate()
{
    /* float vol_diff = cur_vol - bar_vol; */
    /* unsigned long int frame_duration = 1000 / fabsf(vol_diff); */
    /* struct timespec ts = {0, frame_duration}; */
    /* float delta_vol = vol_diff / frame_duration; */
    float vol_diff = cur_vol - bar_vol;
    float delta_vol = vol_diff / 60;
    struct timespec ts = {0, 1666667};

    int frames = 0;

    /* printf("voldiff: %f dvol: %f\n", vol_diff, delta_vol); */
    /* printf("voldiff: %f framedur: %ld dvol: %f\n", vol_diff, frame_duration, delta_vol); */
    /* for (; fabsf(vol_diff) > 2; bar_vol += delta_vol) { */
    for (int i = 0; i < 60; ++i) {
        frames++;
        /* printf("frames: %d\n", frames); */
        cairo_push_group(ctx);
        cairo_set_source_rgb (ctx, 255, 255, 255);
        cairo_paint(ctx);

        cairo_set_line_width (ctx, 5);
        cairo_set_source_rgb (ctx, 255, 0, 0);
        cairo_move_to(ctx, 0 + 10, POPUP_HEIGHT - 10);
        cairo_line_to(ctx, (POPUP_WIDTH - 20) * (bar_vol / 100) + 10, POPUP_HEIGHT - 10);
        cairo_stroke (ctx);
        cairo_pop_group_to_source(ctx);
        cairo_paint(ctx);
        cairo_surface_flush(sfc);

        /* vol_diff = cur_vol - bar_vol; */
        /* printf("voldiff: %f dvol: %f barvol: %f\n", vol_diff, delta_vol, bar_vol); */
        bar_vol += delta_vol;
        nanosleep(&ts, NULL);
    }
    /* bar_vol = cur_vol; */

    return 0;
}

// END CAIRO

// BEGIN PULSE

void sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, 
        void *userdata)
{
    if (eol > 0) return;
    cur_vol = (float)pa_cvolume_avg(&(i->volume)) / (float)PA_VOLUME_NORM * 100.0;
    /* printf("Sink: %s, Volume: %.0f%%, Muted: %s\n", i->name, cur_vol, */
    /*         i->mute ? "yes" : "no"); */  
    animate();
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

int run()
{
    sfc = create_x11_surface(POPUP_WIDTH, POPUP_HEIGHT);
    ctx = cairo_create(sfc);

    int ret = 1;
    while (1) {
        if (pa_mainloop_iterate(m, 1, &ret) < 0)
            fprintf(stderr, "pa_mainloop_iterate() error\n");
    }
    /* if (pa_mainloop_run(m, &ret) < 0) */
    /*     fprintf(stderr, "pa_mainloop_run() failed\n"); */

    return ret;
}

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
