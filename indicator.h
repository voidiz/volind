#ifndef INDICATOR_H
#define INDICATOR_H

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

#include <X11/Xlib.h>

#define IND_W 225
#define IND_H 250

typedef struct {
    cairo_surface_t *sfc;
    cairo_t *ctx;

    int error;
} indicator_t;

indicator_t
init_indicator(int w, int h);

void
term_indicator(indicator_t *i);

#endif
