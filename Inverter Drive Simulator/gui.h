#ifndef GUI_H
#define GUI_H
#include <gtk/gtk.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *status_label;
    GtkWidget *error_label;
    GtkWidget *rated_voltage_entry;
    GtkWidget *rated_current_entry;
    GtkWidget *rated_freq_entry;
    GtkWidget *rated_rpm_entry;
    GtkWidget *ramp_up_entry;
    GtkWidget *ramp_down_entry;
    GtkWidget *max_freq_entry;
    GtkWidget *min_freq_entry;
    GtkWidget *control_mode_combo;
    GtkWidget *speed_ref_scale;
    GtkWidget *forward_button;
    GtkWidget *reverse_button;
    GtkWidget *run_button;
    GtkWidget *stop_button;
    GtkWidget *reset_button;
    GtkWidget *overcurrent_check;
    GtkWidget *undervoltage_check;
    GtkWidget *overtemp_check;
    GtkWidget *output_label;
    GtkWidget *plot_area;
    GtkWidget *keypad_entry;
    guint timer_id;
} AppWidgets;

void create_main_window(GtkApplication *app);
void on_run_button_clicked(GtkWidget *button, gpointer data);
void on_stop_button_clicked(GtkWidget *button, gpointer data);
void on_reset_button_clicked(GtkWidget *button, gpointer data);
void on_forward_button_clicked(GtkWidget *button, gpointer data);
void on_reverse_button_clicked(GtkWidget *button, gpointer data);
void on_keypad_entry_activate(GtkWidget *entry, gpointer data);

#endif
