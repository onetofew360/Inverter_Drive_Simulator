#include <gtk/gtk.h>
#include "gui.h"

static void activate(GtkApplication *app, gpointer user_data) {
    GtkSettings *settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-application-prefer-dark-theme", TRUE, NULL);
    create_main_window(app);
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("com.example.InverterSimulator", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
