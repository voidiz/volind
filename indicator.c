#include "indicator.h"
#include "debug.h"

void term_indicator(indicator_t *i) {
    if (i->ctx)
        cairo_destroy(i->ctx);

    Display *dsp = cairo_xlib_surface_get_display(i->sfc);

    if (i->sfc)
        cairo_surface_destroy(i->sfc);

    if (dsp)
        XCloseDisplay(dsp);
}

int init_indicator(indicator_t *i, int w, int h) {
    Display *dsp;
    Drawable da;
    int screen_num;

    if ((dsp = XOpenDisplay(NULL)) == NULL) {
        return -1;
    }

    screen_num = DefaultScreen(dsp);

    /* Get dimensions of primary monitor */
    int nsizes;
    XRRScreenSize *sizes = XRRSizes(dsp, 0, &nsizes);

    DEBUG_PRINT("Monitor width %d, height %d\n", sizes->width, sizes->height);

    /* Drawable for indicator */
    da = XCreateSimpleWindow(
        dsp, XDefaultRootWindow(dsp), sizes->width / 2 - w / 2,
        sizes->height / 2 - h / 2, w, h, 0, 0, BlackPixel(dsp, 0));

    /* Make the window manager ignore the window */
    XSetWindowAttributes winattr;
    winattr.override_redirect = 1;
    XChangeWindowAttributes(dsp, da, CWOverrideRedirect, &winattr);

    /* XSelectInput(dsp, da, ButtonPressMask | KeyPressMask); */
    XMapWindow(dsp, da);

    i->sfc = cairo_xlib_surface_create(dsp, da, DefaultVisual(dsp, screen_num),
                                       w, h);
    cairo_xlib_surface_set_size(i->sfc, w, h);
    i->ctx = cairo_create(i->sfc);

    return 0;
}
