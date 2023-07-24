/*
 * Copyright (c) 2015 Bastien Nocera <hadess@hadess.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published by
 * the Free Software Foundation.
 *
 */

#include <locale.h>
#include <gio/gio.h>
#include <sys/stat.h>
#include <stdio.h>

static GMainLoop *loop;
static GDBusProxy *iio_proxy;
static gchararray run_script;
static gboolean show_output;
static char cwd[PATH_MAX];
static char sep[1] = "/";
const char *COMMAND_FORMAT = "%s%s%s \"%s\"";

static void handle_accel_orientation(const gchar *orientation) {
    if (show_output) g_print("Executing script '%s' with orientation: %s\n", run_script, orientation);
    size_t command_len = strlen(cwd) + strlen(COMMAND_FORMAT) + strlen(orientation) + strlen(run_script);
    char *command = malloc(command_len);
    if (command == NULL) {
        perror("malloc() error");
        exit(EXIT_FAILURE);
    }
    snprintf(command, command_len, COMMAND_FORMAT, cwd, sep, run_script, orientation);
    int retVal = system(command);
    free(command);
    if (retVal) {
        g_print("Exiting because Script Returned %d", retVal);
        exit(EXIT_FAILURE);
    }
}

static void properties_changed(GDBusProxy *proxy,
                               GVariant *changed_properties,
                               GStrv invalidated_properties,
                               gpointer user_data) {
    GVariant *v;
    GVariantDict dict;

    g_variant_dict_init(&dict, changed_properties);

    if (g_variant_dict_contains(&dict, "AccelerometerOrientation")) {
        v = g_dbus_proxy_get_cached_property(iio_proxy, "AccelerometerOrientation");
        handle_accel_orientation(g_variant_get_string(v, NULL));
        g_variant_unref(v);
    }

    g_variant_dict_clear(&dict);
}

static void print_initial_values(void) {
    GVariant *v;
    v = g_dbus_proxy_get_cached_property(iio_proxy, "HasAccelerometer");
    if (!g_variant_get_boolean(v)) {
        g_print("No Accelerometer. Exiting!!!");
        g_main_loop_quit(loop);
        return;
    }
    g_variant_unref(v);
    v = g_dbus_proxy_get_cached_property(iio_proxy, "AccelerometerOrientation");
    handle_accel_orientation(g_variant_get_string(v, NULL));
    g_variant_unref(v);
}

static void appeared_cb(GDBusConnection *connection,
                        const gchar *name,
                        const gchar *name_owner,
                        gpointer user_data) {
    GError *error = NULL;
    GVariant *ret = NULL;

    if (show_output) g_print("+++ iio-sensor-proxy appeared +++\n");

    iio_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                              G_DBUS_PROXY_FLAGS_NONE,
                                              NULL,
                                              "net.hadess.SensorProxy",
                                              "/net/hadess/SensorProxy",
                                              "net.hadess.SensorProxy",
                                              NULL, NULL);

    g_signal_connect (G_OBJECT(iio_proxy), "g-properties-changed",
                      G_CALLBACK(properties_changed), NULL);

    ret = g_dbus_proxy_call_sync(iio_proxy,
                                 "ClaimAccelerometer",
                                 NULL,
                                 G_DBUS_CALL_FLAGS_NONE,
                                 -1,
                                 NULL, &error);
    if (!ret) {
        if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
            g_warning ("Failed to claim accelerometer: %s", error->message);
        g_main_loop_quit(loop);
        return;
    }
    g_clear_pointer (&ret, g_variant_unref);

    print_initial_values();
}

static void
vanished_cb(GDBusConnection *connection,
            const gchar *name,
            gpointer user_data) {
    if (iio_proxy) {
        g_clear_object (&iio_proxy);
        if (show_output) g_print("--- iio-sensor-proxy vanished, waiting for it to appear ---\n");
    }
}

int main(int argc, char **argv) {
    g_autoptr(GOptionContext) option_context = NULL;
    g_autoptr(GError) error = NULL;
    gchararray para_run_script = "";
    gboolean para_show_output = FALSE;
    const GOptionEntry options[] = {
            {"path",        'p', 0, G_OPTION_ARG_FILENAME, &para_run_script, "Path to script to run. First param ($1) of script is screen orientation", NULL},
            {"show_output", 'o', 0, G_OPTION_ARG_NONE,     &para_show_output, "This binary will show an output when the screen orientation changes",     NULL},
            {NULL}
    };
    int ret = 0;

    setlocale(LC_ALL, "");
    option_context = g_option_context_new("");
    g_option_context_add_main_entries(option_context, options, NULL);
    ret = g_option_context_parse(option_context, &argc, &argv, &error);
    if (!ret) {
        g_print("Failed to parse arguments: %s\n", error->message);
        return EXIT_FAILURE;
    }

    if (strlen(para_run_script) == 0) {
        g_option_context_set_summary(option_context, "Path is not specified");
        g_print(g_option_context_get_help(option_context, TRUE, NULL));
        return EXIT_FAILURE;
    }

    if (access(para_run_script, F_OK)) {
        g_print("Script '%s' not found.\n", para_run_script);
        return EXIT_FAILURE;
    }

    struct stat sb;
    if (stat(para_run_script, &sb) != 0 || !(sb.st_mode & S_IXUSR)) {
        g_print("Script '%s' is not executable.\n", para_run_script);
        return EXIT_FAILURE;
    }

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return EXIT_FAILURE;
    }

    if (strncmp(cwd, para_run_script, strlen(cwd)) == 0) {
        memset(cwd, 0, sizeof(cwd));
        sep[0] = 0;
    }
    run_script = para_run_script;
    show_output = para_show_output;

    g_bus_watch_name(G_BUS_TYPE_SYSTEM,
                     "net.hadess.SensorProxy",
                     G_BUS_NAME_WATCHER_FLAGS_NONE,
                     appeared_cb,
                     vanished_cb,
                     NULL, NULL);
    if (show_output) g_print("Waiting for iio-sensor-proxy to appear\n");
    loop = g_main_loop_new(NULL, TRUE);
    g_main_loop_run(loop);

    return EXIT_SUCCESS;
}
