#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <gtk/gtk.h>

void draw_waveform(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data);

#endif
