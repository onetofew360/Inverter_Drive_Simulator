#include "waveform.h"
#include "inverter.h"
#include "motor.h"
#include "gui.h"
#include <math.h>
#include <time.h>
#define SAMPLES 100
#define PLOT_HEIGHT 120

void draw_waveform(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data) {
    g_print("Drawing waveform\n");
    AppWidgets *widgets = (AppWidgets *)user_data;
    double vdc = atof(gtk_editable_get_text(GTK_EDITABLE(widgets->rated_voltage_entry)));
    double mod_index = 0.8;
    double freq = atof(gtk_editable_get_text(GTK_EDITABLE(widgets->max_freq_entry))) *
                  gtk_range_get_value(GTK_RANGE(widgets->speed_ref_scale)) / 100.0;
    double max_v = vdc * mod_index * 1.1;
    double max_current = atof(gtk_editable_get_text(GTK_EDITABLE(widgets->rated_current_entry))) * 1.5;
    double max_speed = atof(gtk_editable_get_text(GTK_EDITABLE(widgets->rated_rpm_entry))) * 1.2;
    double max_torque = max_current * vdc / max_speed * 1.5;
    double max_freq = atof(gtk_editable_get_text(GTK_EDITABLE(widgets->max_freq_entry)));
    double waveform[SAMPLES];
    static double time = 0.0;
    time += 0.05;
    get_pwm_waveform(waveform, SAMPLES, time);
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_paint(cr);

    for (int plot = 0; plot < 5; plot++) {
        int y_offset = plot * (PLOT_HEIGHT + 10);
        cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
        for (int i = 0; i <= 4; i++) {
            cairo_move_to(cr, 0, y_offset + i * PLOT_HEIGHT / 4);
            cairo_line_to(cr, width, y_offset + i * PLOT_HEIGHT / 4);
            cairo_move_to(cr, i * width / 4, y_offset);
            cairo_line_to(cr, i * width / 4, y_offset + PLOT_HEIGHT);
        }
        cairo_stroke(cr);
        cairo_set_source_rgb(cr, 0.3, 0.7, 1.0);
        double *data = waveform;
        double max_val = max_v;
        if (plot == 1) {
                max_val = max_current; data = waveform;
        }
        if (plot == 2) {
                max_val = max_freq; data = waveform;
        }
        if (plot == 3) {
                max_val = max_torque; data = waveform;
        }
        if (plot == 4) { max_val = max_speed; data = waveform;
        }
        for (int i = 0; i < SAMPLES - 1; i++) {
            double x1 = (double)i * width / SAMPLES;
            double y1 = y_offset + PLOT_HEIGHT / 2 - (data[i] / max_val) * PLOT_HEIGHT / 2;
            double x2 = (double)(i + 1) * width / SAMPLES;
            double y2 = y_offset + PLOT_HEIGHT / 2 - (data[i + 1] / max_val) * PLOT_HEIGHT / 2;
            cairo_move_to(cr, x1, y1);
            cairo_line_to(cr, x2, y2);
        }
        cairo_stroke(cr);
        cairo_set_source_rgb(cr, 1, 1, 1);
        const char *label = plot == 0 ? "Voltage (V)" : plot == 1 ? "Current (A)" :
                            plot == 2 ? "Frequency (Hz)" : plot == 3 ? "Torque (Nm)" : "Speed (RPM)";
        cairo_move_to(cr, 10, y_offset + 20);
        cairo_show_text(cr, label);
    }
}
