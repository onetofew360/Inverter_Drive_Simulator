#include "gui.h"
#include "inverter.h"
#include "waveform.h"
#include "motor.h"
#include "fault.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AppWidgets *app_widgets;
static gboolean is_running = FALSE;
static gboolean is_forward = TRUE;

static void cleanup_widgets(GtkWidget *widget, gpointer data) {
    g_print("Cleaning up widgets\n");
    if (app_widgets->timer_id != 0) {
        g_source_remove(app_widgets->timer_id);
        app_widgets->timer_id = 0;
    }
    g_free(app_widgets);
    app_widgets = NULL;
}

static void update_status_label(void) {
    char status[64];
    snprintf(status, sizeof(status), "Status: %s%s",
             is_running ? "Running" : (has_fault() ? "Faulted" : "Stopped"),
             is_running ? (is_forward ? " (Forward)" : " (Reverse)") : "");
    gtk_label_set_text(GTK_LABEL(app_widgets->status_label), status);
}

gboolean update_simulation(gpointer data) {
    if (!is_running || has_fault()) {
        is_running = FALSE;
        update_status_label();
        return G_SOURCE_CONTINUE;
    }
    double rated_voltage = atof(gtk_editable_get_text(GTK_EDITABLE(app_widgets->rated_voltage_entry)));
    double rated_current = atof(gtk_editable_get_text(GTK_EDITABLE(app_widgets->rated_current_entry)));
    double rated_freq = atof(gtk_editable_get_text(GTK_EDITABLE(app_widgets->rated_freq_entry)));
    double rated_rpm = atof(gtk_editable_get_text(GTK_EDITABLE(app_widgets->rated_rpm_entry)));
    double ramp_up = atof(gtk_editable_get_text(GTK_EDITABLE(app_widgets->ramp_up_entry)));
    double ramp_down = atof(gtk_editable_get_text(GTK_EDITABLE(app_widgets->ramp_down_entry)));
    double max_freq = atof(gtk_editable_get_text(GTK_EDITABLE(app_widgets->max_freq_entry)));
    double min_freq = atof(gtk_editable_get_text(GTK_EDITABLE(app_widgets->min_freq_entry)));
    double speed_ref = gtk_range_get_value(GTK_RANGE(app_widgets->speed_ref_scale)) * max_freq / 100.0;
    char error_msg[256] = "";
    if (rated_voltage <= 0 || rated_voltage > 10000) snprintf(error_msg, sizeof(error_msg), "Invalid Rated Voltage (1–10000 V)");
    else if (rated_current <= 0 || rated_current > 1000) snprintf(error_msg, sizeof(error_msg), "Invalid Rated Current (1–1000 A)");
    else if (rated_freq <= 0 || rated_freq > 1000) snprintf(error_msg, sizeof(error_msg), "Invalid Rated Frequency (1–1000 Hz)");
    else if (rated_rpm <= 0 || rated_rpm > 10000) snprintf(error_msg, sizeof(error_msg), "Invalid Rated RPM (1–10000)");
    else if (ramp_up < 0 || ramp_up > 60) snprintf(error_msg, sizeof(error_msg), "Invalid Ramp Up Time (0–60 s)");
    else if (ramp_down < 0 || ramp_down > 60) snprintf(error_msg, sizeof(error_msg), "Invalid Ramp Down Time (0–60 s)");
    else if (max_freq <= min_freq || max_freq > 1000) snprintf(error_msg, sizeof(error_msg), "Invalid Max Frequency");
    else if (min_freq < 0 || min_freq > max_freq) snprintf(error_msg, sizeof(error_msg), "Invalid Min Frequency");
    if (strlen(error_msg) > 0) {
        gtk_label_set_text(GTK_LABEL(app_widgets->error_label), error_msg);
        gtk_label_set_text(GTK_LABEL(app_widgets->output_label), "Output: Invalid input");
        is_running = FALSE;
        update_status_label();
        return G_SOURCE_CONTINUE;
    }
    set_motor_params(rated_voltage, rated_current, rated_freq, rated_rpm);
    set_drive_params(ramp_up, ramp_down, max_freq, min_freq);
    set_inverter_params(rated_voltage, speed_ref, 0.8);
    update_motor(speed_ref, is_forward, ramp_up, ramp_down);
    check_faults(rated_voltage, get_motor_current(), get_motor_temp());
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app_widgets->overcurrent_check))) trigger_fault(FAULT_OVERCURRENT);
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app_widgets->undervoltage_check))) trigger_fault(FAULT_UNDERVOLTAGE);
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app_widgets->overtemp_check))) trigger_fault(FAULT_OVERTEMP);
    double vll = calculate_vll();
    double current = get_motor_current();
    double speed = get_motor_speed();
    double torque = get_motor_torque();
    char output[256];
    snprintf(output, sizeof(output), "V_L-L: %.2f V\nCurrent: %.2f A\nSpeed: %.2f RPM\nTorque: %.2f Nm\nFreq: %.2f Hz",
             vll, current, speed, torque, speed_ref);
    gtk_label_set_text(GTK_LABEL(app_widgets->output_label), output);
    gtk_label_set_text(GTK_LABEL(app_widgets->error_label), get_fault_status());

    gtk_widget_queue_draw(app_widgets->plot_area);
    return G_SOURCE_CONTINUE;
}

void on_run_button_clicked(GtkWidget *button, gpointer data) {
    g_print("Run button clicked\n");
    if (!is_running && !has_fault()) {
        is_running = TRUE;
        update_status_label();
        if (app_widgets->timer_id == 0) {
            app_widgets->timer_id = g_timeout_add(50, update_simulation, NULL);
            g_print("Started simulation timer\n");
        }
    }
}

void on_stop_button_clicked(GtkWidget *button, gpointer data) {
    g_print("Stop button clicked\n");
    is_running = FALSE;
    update_status_label();
}

void on_reset_button_clicked(GtkWidget *button, gpointer data) {
    g_print("Reset button clicked\n");
    reset_faults();
    is_running = FALSE;
    update_status_label();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app_widgets->overcurrent_check), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app_widgets->undervoltage_check), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app_widgets->overtemp_check), FALSE);
    gtk_widget_queue_draw(app_widgets->plot_area);
}

void on_forward_button_clicked(GtkWidget *button, gpointer data) {
    g_print("Forward button clicked\n");
    is_forward = TRUE;
    update_status_label();
}

void on_reverse_button_clicked(GtkWidget *button, gpointer data) {
    g_print("Reverse button clicked\n");
    is_forward = FALSE;
    update_status_label();
}

void on_keypad_entry_activate(GtkWidget *entry, gpointer data) {
    g_print("Keypad entry activated\n");
    const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));
    gtk_editable_set_text(GTK_EDITABLE(app_widgets->rated_voltage_entry), text);
    gtk_editable_set_text(GTK_EDITABLE(entry), "");
}

void create_main_window(GtkApplication *app) {
    g_print("Creating main window\n");
    app_widgets = g_new0(AppWidgets, 1);
    app_widgets->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(app_widgets->window), "Inverter Drive Simulator");
    gtk_window_set_default_size(GTK_WINDOW(app_widgets->window), 1200, 750); // Increased width for plots
    g_signal_connect(app_widgets->window, "destroy", G_CALLBACK(cleanup_widgets), NULL);
    g_print("Window created\n");
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_widget_set_margin_start(grid, 10);
    gtk_widget_set_margin_end(grid, 10);
    gtk_widget_set_margin_top(grid, 10);
    gtk_widget_set_margin_bottom(grid, 10);
    gtk_window_set_child(GTK_WINDOW(app_widgets->window), grid);
    gtk_widget_set_visible(grid, TRUE);
    g_print("Grid created\n");
    int row = 0;
    app_widgets->status_label = gtk_label_new("Status: Stopped");
    gtk_grid_attach(GTK_GRID(grid), app_widgets->status_label, 0, row++, 3, 1);
    gtk_widget_set_visible(app_widgets->status_label, TRUE);
    g_print("Status label created\n");
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Motor Parameters"), 0, row++, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Rated Voltage (V):"), 0, row, 1, 1);
    app_widgets->rated_voltage_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(app_widgets->rated_voltage_entry), "400");
    gtk_grid_attach(GTK_GRID(grid), app_widgets->rated_voltage_entry, 1, row++, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Rated Current (A):"), 0, row, 1, 1);
    app_widgets->rated_current_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(app_widgets->rated_current_entry), "10");
    gtk_grid_attach(GTK_GRID(grid), app_widgets->rated_current_entry, 1, row++, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Rated Frequency (Hz):"), 0, row, 1, 1);
    app_widgets->rated_freq_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(app_widgets->rated_freq_entry), "50");
    gtk_grid_attach(GTK_GRID(grid), app_widgets->rated_freq_entry, 1, row++, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Rated RPM:"), 0, row, 1, 1);
    app_widgets->rated_rpm_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(app_widgets->rated_rpm_entry), "1500");
    gtk_grid_attach(GTK_GRID(grid), app_widgets->rated_rpm_entry, 1, row++, 1, 1);
    g_print("Motor parameters created\n");
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Drive Settings"), 2, row - 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Ramp Up Time (s):"), 2, row - 3, 1, 1);
    app_widgets->ramp_up_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(app_widgets->ramp_up_entry), "2");
    gtk_grid_attach(GTK_GRID(grid), app_widgets->ramp_up_entry, 3, row - 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Ramp Down Time (s):"), 2, row - 2, 1, 1);
    app_widgets->ramp_down_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(app_widgets->ramp_down_entry), "2");
    gtk_grid_attach(GTK_GRID(grid), app_widgets->ramp_down_entry, 3, row - 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Max Frequency (Hz):"), 2, row - 1, 1, 1);
    app_widgets->max_freq_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(app_widgets->max_freq_entry), "100");
    gtk_grid_attach(GTK_GRID(grid), app_widgets->max_freq_entry, 3, row - 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Min Frequency (Hz):"), 2, row, 1, 1);
    app_widgets->min_freq_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(app_widgets->min_freq_entry), "10");
    gtk_grid_attach(GTK_GRID(grid), app_widgets->min_freq_entry, 3, row++, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Control Mode:"), 0, row, 1, 1);
    app_widgets->control_mode_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app_widgets->control_mode_combo), "V/f");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app_widgets->control_mode_combo), "Vector Control");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app_widgets->control_mode_combo), "Sensorless Vector");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app_widgets->control_mode_combo), "DTC");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app_widgets->control_mode_combo), 0);
    gtk_grid_attach(GTK_GRID(grid), app_widgets->control_mode_combo, 1, row++, 1, 1);
    g_print("Drive settings created\n");
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Input Signals"), 0, row++, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Speed Reference (%):"), 0, row, 1, 1);
    app_widgets->speed_ref_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(app_widgets->speed_ref_scale), 50);
    gtk_grid_attach(GTK_GRID(grid), app_widgets->speed_ref_scale, 1, row++, 1, 1);
    app_widgets->forward_button = gtk_button_new_with_label("Forward");
    g_signal_connect(app_widgets->forward_button, "clicked", G_CALLBACK(on_forward_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), app_widgets->forward_button, 0, row, 1, 1);
    app_widgets->reverse_button = gtk_button_new_with_label("Reverse");
    g_signal_connect(app_widgets->reverse_button, "clicked", G_CALLBACK(on_reverse_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), app_widgets->reverse_button, 1, row++, 1, 1);
    g_print("Input signals created\n");
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Control Panel"), 2, row - 2, 2, 1);
    app_widgets->run_button = gtk_button_new_with_label("Run");
    g_signal_connect(app_widgets->run_button, "clicked", G_CALLBACK(on_run_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), app_widgets->run_button, 2, row - 1, 1, 1);
    app_widgets->stop_button = gtk_button_new_with_label("Stop");
    g_signal_connect(app_widgets->stop_button, "clicked", G_CALLBACK(on_stop_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), app_widgets->stop_button, 3, row - 1, 1, 1);
    app_widgets->reset_button = gtk_button_new_with_label("Reset");
    g_signal_connect(app_widgets->reset_button, "clicked", G_CALLBACK(on_reset_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), app_widgets->reset_button, 2, row, 1, 1);
    app_widgets->keypad_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app_widgets->keypad_entry), "Enter parameter value");
    g_signal_connect(app_widgets->keypad_entry, "activate", G_CALLBACK(on_keypad_entry_activate), NULL);
    gtk_grid_attach(GTK_GRID(grid), app_widgets->keypad_entry, 3, row++, 1, 1);
    g_print("Control panel created\n");
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Fault Inputs"), 0, row++, 2, 1);
    app_widgets->overcurrent_check = gtk_check_button_new_with_label("Overcurrent");
    gtk_grid_attach(GTK_GRID(grid), app_widgets->overcurrent_check, 0, row, 1, 1);
    app_widgets->undervoltage_check = gtk_check_button_new_with_label("Undervoltage");
    gtk_grid_attach(GTK_GRID(grid), app_widgets->undervoltage_check, 1, row, 1, 1);
    app_widgets->overtemp_check = gtk_check_button_new_with_label("Overtemperature");
    gtk_grid_attach(GTK_GRID(grid), app_widgets->overtemp_check, 2, row++, 1, 1);
    g_print("Fault inputs created\n");
    app_widgets->output_label = gtk_label_new("Output: Waiting...");
    gtk_grid_attach(GTK_GRID(grid), app_widgets->output_label, 0, row++, 3, 1);
    app_widgets->error_label = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), app_widgets->error_label, 0, row++, 3, 1);
    g_print("Output labels created\n");
    app_widgets->plot_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(app_widgets->plot_area, 600, 600); // Adjusted for vertical space
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(app_widgets->plot_area), draw_waveform, app_widgets, NULL);
    gtk_grid_attach(GTK_GRID(grid), app_widgets->plot_area, 4, 0, 1, row); // Span all rows
    gtk_widget_set_visible(app_widgets->plot_area, TRUE);
    g_print("Plot area created\n");
    gtk_widget_set_visible(app_widgets->window, TRUE);
    g_print("Window set visible\n");
}
